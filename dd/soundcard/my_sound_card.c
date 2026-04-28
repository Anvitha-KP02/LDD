// SPDX-License-Identifier: GPL-2.0
/*
 * my_sound_card.c - Virtual ALSA PCM playback card (no real hardware output)
 *
 * Creates a standalone ALSA sound card visible in `aplay -l` as "MySoundCard".
 * It supports playback only with fixed format:
 *   - 16-bit little-endian
 *   - 44.1 kHz
 *   - stereo
 *
 * Audio data is accepted into a vmalloc PCM buffer and playback progress is
 * simulated with an hrtimer that periodically calls snd_pcm_period_elapsed().
 */

#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/time.h>

#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#define MY_CARD_NAME            "MySoundCard"
#define MY_PCM_NAME             "MyPCMPlayback"

#define MY_RATE                 44100U
#define MY_CHANNELS             2U
#define MY_PERIOD_BYTES_MIN     4096U
#define MY_PERIOD_BYTES_MAX     4096U
#define MY_PERIODS_MIN          2U
#define MY_PERIODS_MAX          64U
#define MY_BUFFER_BYTES_MAX     (MY_PERIOD_BYTES_MAX * MY_PERIODS_MAX)

static int index = -1;
module_param(index, int, 0444);
MODULE_PARM_DESC(index, "ALSA card index");

static char *id = MY_CARD_NAME;
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ALSA card id");

struct my_runtime_data {
	struct snd_pcm_substream *substream;
	spinlock_t lock;
	struct hrtimer timer;
	bool running;
	snd_pcm_uframes_t hw_ptr;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;
	u64 period_ns;
};

struct my_card_data {
	struct snd_card *card;
	struct snd_pcm *pcm;
};

static struct snd_card *g_card;

static const struct snd_pcm_hardware my_pcm_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER |
		SNDRV_PCM_INFO_MMAP_VALID,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_44100,
	.rate_min = MY_RATE,
	.rate_max = MY_RATE,
	.channels_min = MY_CHANNELS,
	.channels_max = MY_CHANNELS,
	.buffer_bytes_max = MY_BUFFER_BYTES_MAX,
	.period_bytes_min = MY_PERIOD_BYTES_MIN,
	.period_bytes_max = MY_PERIOD_BYTES_MAX,
	.periods_min = MY_PERIODS_MIN,
	.periods_max = MY_PERIODS_MAX,
};

static enum hrtimer_restart my_pcm_timer_callback(struct hrtimer *hrtimer)
{
	struct my_runtime_data *prtd;
	struct snd_pcm_substream *substream;
	unsigned long flags;
	bool elapsed = false;

	prtd = container_of(hrtimer, struct my_runtime_data, timer);
	substream = prtd->substream;

	spin_lock_irqsave(&prtd->lock, flags);
	if (prtd->running) {
		prtd->hw_ptr += prtd->period_size;
		if (prtd->hw_ptr >= prtd->buffer_size)
			prtd->hw_ptr -= prtd->buffer_size;
		hrtimer_forward_now(&prtd->timer, ns_to_ktime(prtd->period_ns));
		elapsed = true;
	}
	spin_unlock_irqrestore(&prtd->lock, flags);

	if (elapsed) {
		snd_pcm_period_elapsed(substream);
		return HRTIMER_RESTART;
	}

	return HRTIMER_NORESTART;
}

static int my_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct my_runtime_data *prtd;
	int ret;

	runtime->hw = my_pcm_hardware;

	ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		return ret;

	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (!prtd)
		return -ENOMEM;

	prtd->substream = substream;
	spin_lock_init(&prtd->lock);
	hrtimer_init(&prtd->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
	prtd->timer.function = my_pcm_timer_callback;

	runtime->private_data = prtd;

	return 0;
}

static int my_pcm_close(struct snd_pcm_substream *substream)
{
	struct my_runtime_data *prtd = substream->runtime->private_data;

	if (prtd) {
		hrtimer_cancel(&prtd->timer);
		kfree(prtd);
		substream->runtime->private_data = NULL;
	}

	return 0;
}

static int my_pcm_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

static int my_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

static int my_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct my_runtime_data *prtd = runtime->private_data;
	unsigned long flags;
	u64 period_ns;

	if (!runtime->period_size || !runtime->buffer_size || !runtime->rate)
		return -EINVAL;

	period_ns = div_u64((u64)runtime->period_size * NSEC_PER_SEC,
			    runtime->rate);

	spin_lock_irqsave(&prtd->lock, flags);
	prtd->period_size = runtime->period_size;
	prtd->buffer_size = runtime->buffer_size;
	prtd->period_ns = period_ns;
	prtd->hw_ptr = 0;
	spin_unlock_irqrestore(&prtd->lock, flags);

	return 0;
}

static int my_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct my_runtime_data *prtd = substream->runtime->private_data;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&prtd->lock, flags);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		prtd->running = true;
		hrtimer_start(&prtd->timer,
			      ns_to_ktime(prtd->period_ns),
			      HRTIMER_MODE_REL_PINNED);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		prtd->running = false;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	spin_unlock_irqrestore(&prtd->lock, flags);

	if (!ret && (cmd == SNDRV_PCM_TRIGGER_STOP ||
		     cmd == SNDRV_PCM_TRIGGER_SUSPEND ||
		     cmd == SNDRV_PCM_TRIGGER_PAUSE_PUSH))
		hrtimer_cancel(&prtd->timer);

	return ret;
}

static snd_pcm_uframes_t my_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct my_runtime_data *prtd = substream->runtime->private_data;
	unsigned long flags;
	snd_pcm_uframes_t pos;

	spin_lock_irqsave(&prtd->lock, flags);
	pos = prtd->hw_ptr;
	spin_unlock_irqrestore(&prtd->lock, flags);

	return pos;
}

static const struct snd_pcm_ops my_pcm_ops = {
	.open = my_pcm_open,
	.close = my_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = my_pcm_hw_params,
	.hw_free = my_pcm_hw_free,
	.prepare = my_pcm_prepare,
	.trigger = my_pcm_trigger,
	.pointer = my_pcm_pointer,
};

static int __init my_card_init(void)
{
	struct snd_card *card = NULL;
	struct my_card_data *data;
	int ret;

	pr_info("my_sound_card: init start (index=%d, id=%s)\n",
		index, id ? id : "NULL");

	pr_info("my_sound_card: calling snd_card_new()\n");
	ret = snd_card_new(NULL, index, id, THIS_MODULE, sizeof(*data), &card);
	if (ret < 0) {
		pr_err("my_sound_card: snd_card_new failed: %d\n", ret);
		return ret;
	}
	pr_info("my_sound_card: snd_card_new ok\n");

	data = card->private_data;
	if (!data) {
		pr_err("my_sound_card: card->private_data is NULL\n");
		ret = -ENOMEM;
		goto err_free_card;
	}
	data->card = card;

	strscpy(card->driver, "my_sound_card", sizeof(card->driver));
	strscpy(card->shortname, MY_CARD_NAME, sizeof(card->shortname));
	strscpy(card->longname,
		"MySoundCard Virtual PCM (S16_LE/44.1kHz/stereo)",
		sizeof(card->longname));
	pr_info("my_sound_card: card identity set (%s)\n", card->shortname);

	pr_info("my_sound_card: calling snd_pcm_new()\n");
	ret = snd_pcm_new(card, MY_PCM_NAME, 0, 1, 0, &data->pcm);
	if (ret < 0) {
		pr_err("my_sound_card: snd_pcm_new failed: %d\n", ret);
		goto err_free_card;
	}
	pr_info("my_sound_card: snd_pcm_new ok\n");

	strscpy(data->pcm->name, MY_PCM_NAME, sizeof(data->pcm->name));
	snd_pcm_set_ops(data->pcm, SNDRV_PCM_STREAM_PLAYBACK, &my_pcm_ops);
	pr_info("my_sound_card: snd_pcm_set_ops done\n");

	pr_info("my_sound_card: preallocating vmalloc pages\n");
	snd_pcm_lib_preallocate_pages_for_all(data->pcm,
					      SNDRV_DMA_TYPE_VMALLOC,
					      NULL,
					      MY_PERIOD_BYTES_MIN *
					      MY_PERIODS_MIN,
					      MY_BUFFER_BYTES_MAX);
	pr_info("my_sound_card: preallocation done\n");

	pr_info("my_sound_card: calling snd_card_register()\n");
	ret = snd_card_register(card);
	if (ret < 0) {
		pr_err("my_sound_card: snd_card_register failed: %d\n", ret);
		goto err_prealloc_free;
	}
	pr_info("my_sound_card: snd_card_register ok\n");

	g_card = card;
	pr_info("my_sound_card: init complete, card='%s'\n", card->shortname);
	return 0;

err_prealloc_free:
	if (data->pcm)
		snd_pcm_lib_preallocate_free_for_all(data->pcm);
err_free_card:
	snd_card_free(card);
	pr_err("my_sound_card: init failed, resources freed (ret=%d)\n", ret);
	return ret;
}

static void __exit my_card_exit(void)
{
	struct my_card_data *data;

	if (!g_card)
		return;

	data = g_card->private_data;
	if (data && data->pcm)
		snd_pcm_lib_preallocate_free_for_all(data->pcm);

	snd_card_free(g_card);
	g_card = NULL;

	pr_info("my_sound_card: module unloaded\n");
}

module_init(my_card_init);
module_exit(my_card_exit);

MODULE_AUTHOR("Embedded Linux ALSA/ASoC");
MODULE_DESCRIPTION("Standalone virtual ALSA PCM sound card");
MODULE_LICENSE("GPL");
