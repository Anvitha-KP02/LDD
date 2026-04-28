// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal ALSA PCM playback driver (no real hardware).
 * Data path: .copy -> dma buffer; timer + workqueue -> snd_pcm_period_elapsed().
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uio.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#define CARD_SHORT_NAME "custom_alsa_card"
#define CARD_DRIVER_NAME "custom_alsa_pcm"
#define PCM_DEVICE_NAME "custom_pcm_playback"

#define CUSTOM_RATE 44100
#define CUSTOM_CHANNELS 2

#define CUSTOM_PERIOD_BYTES_MIN 4096
#define CUSTOM_PERIOD_BYTES_MAX (64 * 1024)
#define CUSTOM_PERIODS_MIN 2
#define CUSTOM_PERIODS_MAX 8
#define CUSTOM_BUFFER_BYTES_MAX (CUSTOM_PERIOD_BYTES_MAX * CUSTOM_PERIODS_MAX)

struct custom_alsa_dev {
	struct snd_card *card;
	struct snd_pcm *pcm;
	spinlock_t lock;
	struct timer_list timer;
	struct work_struct period_work;
	struct snd_pcm_substream *playback_substream;
	bool running;
	snd_pcm_uframes_t hw_ptr;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;
	unsigned int rate;
};

static struct custom_alsa_dev *g_chip;

static const struct snd_pcm_hardware custom_hw = {
	.info = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_44100,
	.rate_min = CUSTOM_RATE,
	.rate_max = CUSTOM_RATE,
	.channels_min = CUSTOM_CHANNELS,
	.channels_max = CUSTOM_CHANNELS,
	.buffer_bytes_max = CUSTOM_BUFFER_BYTES_MAX,
	.period_bytes_min = CUSTOM_PERIOD_BYTES_MIN,
	.period_bytes_max = CUSTOM_PERIOD_BYTES_MAX,
	.periods_min = CUSTOM_PERIODS_MIN,
	.periods_max = CUSTOM_PERIODS_MAX,
};

static unsigned long period_to_jiffies(unsigned int rate, snd_pcm_uframes_t period_frames)
{
	unsigned long ms;

	if (!rate || !period_frames)
		return 1;

	ms = DIV_ROUND_UP((unsigned long)period_frames * 1000UL, rate);
	return ms ? msecs_to_jiffies(ms) : 1;
}

/* Runs in process context — safe for snd_pcm_period_elapsed(). */
static void custom_period_work_fn(struct work_struct *work)
{
	struct custom_alsa_dev *chip = container_of(work, struct custom_alsa_dev, period_work);
	struct snd_pcm_substream *substream;

	spin_lock_irq(&chip->lock);
	substream = chip->playback_substream;
	spin_unlock_irq(&chip->lock);

	if (substream)
		snd_pcm_period_elapsed(substream);
}

static void custom_timer_fn(struct timer_list *t)
{
	struct custom_alsa_dev *chip = from_timer(chip, t, timer);
	unsigned long flags;

	spin_lock_irqsave(&chip->lock, flags);
	if (!chip->running || !chip->playback_substream ||
	    !chip->buffer_size || !chip->period_size) {
		spin_unlock_irqrestore(&chip->lock, flags);
		return;
	}

	chip->hw_ptr += chip->period_size;
	chip->hw_ptr %= chip->buffer_size;

	mod_timer(&chip->timer,
		  jiffies + period_to_jiffies(chip->rate, chip->period_size));
	spin_unlock_irqrestore(&chip->lock, flags);

	schedule_work(&chip->period_work);
}

static int custom_open(struct snd_pcm_substream *substream)
{
	struct custom_alsa_dev *chip = substream->pcm->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;

	if (!chip)
		return -ENODEV;

	substream->private_data = chip;
	runtime->hw = custom_hw;
	return 0;
}

static int custom_close(struct snd_pcm_substream *substream)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);

	del_timer_sync(&chip->timer);
	cancel_work_sync(&chip->period_work);

	spin_lock_irq(&chip->lock);
	chip->running = false;
	chip->playback_substream = NULL;
	spin_unlock_irq(&chip->lock);

	return 0;
}

static int custom_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);

	chip->period_size = params_period_size(params);
	chip->buffer_size = params_buffer_size(params);
	chip->rate = params_rate(params);

	if (!chip->period_size || !chip->buffer_size || !chip->rate)
		return -EINVAL;

	chip->playback_substream = substream;

	return snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
}

static int custom_hw_free(struct snd_pcm_substream *substream)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);

	spin_lock_irq(&chip->lock);
	chip->playback_substream = NULL;
	spin_unlock_irq(&chip->lock);

	return snd_pcm_lib_free_pages(substream);
}

static int custom_prepare(struct snd_pcm_substream *substream)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);
	unsigned long flags;

	spin_lock_irqsave(&chip->lock, flags);
	chip->hw_ptr = 0;
	spin_unlock_irqrestore(&chip->lock, flags);
	return 0;
}

static int custom_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);
	unsigned long flags;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (!chip->period_size || !chip->buffer_size || !chip->rate)
			return -EINVAL;
		spin_lock_irqsave(&chip->lock, flags);
		chip->running = true;
		spin_unlock_irqrestore(&chip->lock, flags);
		mod_timer(&chip->timer,
			  jiffies + period_to_jiffies(chip->rate, chip->period_size));
		return 0;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		spin_lock_irqsave(&chip->lock, flags);
		chip->running = false;
		spin_unlock_irqrestore(&chip->lock, flags);
		del_timer_sync(&chip->timer);
		cancel_work_sync(&chip->period_work);
		return 0;

	default:
		return -EINVAL;
	}
}

static snd_pcm_uframes_t custom_pointer(struct snd_pcm_substream *substream)
{
	struct custom_alsa_dev *chip = snd_pcm_substream_chip(substream);
	snd_pcm_uframes_t pos;
	unsigned long flags;

	spin_lock_irqsave(&chip->lock, flags);
	pos = chip->hw_ptr;
	spin_unlock_irqrestore(&chip->lock, flags);
	return pos;
}

static int custom_copy(struct snd_pcm_substream *substream, int channel,
		       unsigned long pos, struct iov_iter *src, unsigned long bytes)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	u8 *dst;
	size_t offset, first;

	(void)channel;

	if (!bytes)
		return 0;
	if (!runtime->dma_area || !runtime->dma_bytes)
		return -EIO;

	dst = runtime->dma_area;

	if (runtime->buffer_size)
		pos %= runtime->buffer_size;

	offset = frames_to_bytes(runtime, pos);
	if (offset >= runtime->dma_bytes)
		offset %= runtime->dma_bytes;

	first = min_t(size_t, bytes, runtime->dma_bytes - offset);
	if (copy_from_iter(dst + offset, first, src) != first)
		return -EFAULT;
	if (bytes > first) {
		if (copy_from_iter(dst, bytes - first, src) != bytes - first)
			return -EFAULT;
	}
	return 0;
}

static const struct snd_pcm_ops custom_pcm_ops = {
	.open = custom_open,
	.close = custom_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = custom_hw_params,
	.hw_free = custom_hw_free,
	.prepare = custom_prepare,
	.trigger = custom_trigger,
	.pointer = custom_pointer,
	.copy = custom_copy,
};

static int __init custom_alsa_init(void)
{
	struct snd_card *card;
	struct custom_alsa_dev *chip;
	int ret;

	ret = snd_card_new(NULL, -1, CARD_DRIVER_NAME, THIS_MODULE,
			   sizeof(struct custom_alsa_dev), &card);
	if (ret < 0)
		return ret;

	chip = card->private_data;
	memset(chip, 0, sizeof(*chip));
	chip->card = card;
	spin_lock_init(&chip->lock);
	timer_setup(&chip->timer, custom_timer_fn, 0);
	INIT_WORK(&chip->period_work, custom_period_work_fn);

	ret = snd_pcm_new(card, PCM_DEVICE_NAME, 0, 1, 0, &chip->pcm);
	if (ret < 0)
		goto err_card;

	snd_pcm_set_ops(chip->pcm, SNDRV_PCM_STREAM_PLAYBACK, &custom_pcm_ops);
	chip->pcm->private_data = chip;
	chip->pcm->info_flags = 0;
	strscpy(chip->pcm->name, PCM_DEVICE_NAME, sizeof(chip->pcm->name));

	ret = snd_pcm_set_managed_buffer_all(chip->pcm, SNDRV_DMA_TYPE_VMALLOC,
					     NULL, 0, CUSTOM_BUFFER_BYTES_MAX);
	if (ret < 0)
		goto err_card;

	strscpy(card->driver, CARD_DRIVER_NAME, sizeof(card->driver));
	strscpy(card->shortname, CARD_SHORT_NAME, sizeof(card->shortname));
	strscpy(card->longname, "Custom ALSA PCM Playback Card", sizeof(card->longname));

	ret = snd_card_register(card);
	if (ret < 0)
		goto err_card;

	g_chip = chip;
	/*
	 * pr_info() is often filtered from the serial console; pr_warn() is
	 * much more likely to appear after "quiet" or low printk settings.
	 */
	pr_warn("custom_alsa_pcm: registered card %d \"%s\" — use hw:%d,0 (check: aplay -l)\n",
		chip->card->number, card->shortname, chip->card->number);
	return 0;

err_card:
	snd_card_free(card);
	return ret;
}

static void __exit custom_alsa_exit(void)
{
	if (!g_chip)
		return;

	del_timer_sync(&g_chip->timer);
	cancel_work_sync(&g_chip->period_work);
	snd_card_free(g_chip->card);
	g_chip = NULL;
	pr_warn("custom_alsa_pcm: unloaded\n");
}

module_init(custom_alsa_init);
module_exit(custom_alsa_exit);

MODULE_AUTHOR("Custom ALSA Example");
MODULE_DESCRIPTION("Custom ALSA PCM playback (timer + workqueue)");
MODULE_LICENSE("GPL");
