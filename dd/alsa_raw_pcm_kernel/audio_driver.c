// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal ALSA PCM playback driver for Raspberry Pi (or any Linux 5.x+).
 * Exposes a stereo S16_LE @ 44100 Hz RAW PCM device. No MP3/decoding.
 * Playback is simulated: a hrtimer advances the hardware pointer and drains
 * the ALSA ring buffer so userspace tools (aplay) complete successfully.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/version.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#define CARD_NAME	"RPIRAWPCM"
#define PCM_NAME	"rpi_raw_pcm"
#define DRV_NAME	"rpi_alsa_raw_pcm"

/* Fixed RAW format: stereo 16-bit little-endian @ 44.1 kHz */
#define RPI_RATE	44100
#define RPI_CHANNELS	2
#define RPI_FORMAT	SNDRV_PCM_FORMAT_S16_LE

struct rpi_pcm_priv {
	struct snd_pcm_substream *substream;
	struct hrtimer timer;
	atomic_t running;
	/* hw position in frames; updated under snd_pcm_stream lock */
	snd_pcm_uframes_t hw_ptr;
	u64 period_ns;
};

struct rpi_card {
	struct snd_card *card;
	struct snd_pcm *pcm;
	struct platform_device *pdev;
};

static struct platform_device *rpi_platform_dev;

/* ---- PCM hardware description (only what we support) ---- */
static const struct snd_pcm_hardware rpi_pcm_hw = {
	.info = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_44100,
	.rate_min = RPI_RATE,
	.rate_max = RPI_RATE,
	.channels_min = RPI_CHANNELS,
	.channels_max = RPI_CHANNELS,
	.buffer_bytes_max = 256 * 1024,
	.period_bytes_min = 256,
	.period_bytes_max = 64 * 1024,
	.periods_min = 2,
	.periods_max = 32,
};

/* Forward */
static void rpi_stop_timer(struct rpi_pcm_priv *priv);

static enum hrtimer_restart rpi_pcm_timer_cb(struct hrtimer *t)
{
	struct rpi_pcm_priv *priv = container_of(t, struct rpi_pcm_priv, timer);
	struct snd_pcm_substream *ss = priv->substream;
	struct snd_pcm_runtime *runtime;
	unsigned long flags;

	if (!atomic_read(&priv->running))
		return HRTIMER_NORESTART;

	if (!ss)
		return HRTIMER_NORESTART;

	runtime = ss->runtime;
	if (!runtime)
		return HRTIMER_NORESTART;

	snd_pcm_stream_lock_irqsave(ss, flags);
	priv->hw_ptr += runtime->period_size;
	priv->hw_ptr %= runtime->buffer_size;
	snd_pcm_period_elapsed(ss);
	snd_pcm_stream_unlock_irqrestore(ss, flags);

	hrtimer_forward_now(t, ns_to_ktime(priv->period_ns));
	return HRTIMER_RESTART;
}

static void rpi_start_timer(struct rpi_pcm_priv *priv)
{
	if (!priv->substream || !priv->substream->runtime)
		return;
	if (priv->period_ns == 0)
		return;

	hrtimer_start(&priv->timer, ns_to_ktime(priv->period_ns),
		      HRTIMER_MODE_REL);
}

static void rpi_stop_timer(struct rpi_pcm_priv *priv)
{
	atomic_set(&priv->running, 0);
	hrtimer_cancel(&priv->timer);
}

/* ---- snd_pcm_ops callbacks ---- */

static int rpi_pcm_open(struct snd_pcm_substream *substream)
{
	struct rpi_pcm_priv *priv;

	printk(KERN_INFO "%s: open (stream=%d)\n", DRV_NAME, substream->stream);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->substream = substream;
	atomic_set(&priv->running, 0);
	priv->hw_ptr = 0;
	hrtimer_init(&priv->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->timer.function = rpi_pcm_timer_cb;

	substream->runtime->hw = rpi_pcm_hw;
	substream->runtime->private_data = priv;

	return 0;
}

static int rpi_pcm_close(struct snd_pcm_substream *substream)
{
	struct rpi_pcm_priv *priv = substream->runtime->private_data;

	printk(KERN_INFO "%s: close\n", DRV_NAME);

	if (priv) {
		rpi_stop_timer(priv);
		kfree(priv);
		substream->runtime->private_data = NULL;
	}
	return 0;
}

static int rpi_pcm_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *hw_params)
{
	int err;

	printk(KERN_INFO "%s: hw_params (rate=%u ch=%u fmt=%d)\n", DRV_NAME,
	       params_rate(hw_params),
	       params_channels(hw_params),
	       params_format(hw_params));

	/* Enforce fixed RAW layout */
	if (params_rate(hw_params) != RPI_RATE ||
	    params_channels(hw_params) != RPI_CHANNELS ||
	    params_format(hw_params) != RPI_FORMAT) {
		printk(KERN_WARNING "%s: hw_params rejected (need %d Hz, %d ch, S16_LE)\n",
		       DRV_NAME, RPI_RATE, RPI_CHANNELS);
		return -EINVAL;
	}

	err = snd_pcm_lib_malloc_pages(substream,
				       params_buffer_bytes(hw_params));
	if (err < 0) {
		printk(KERN_ERR "%s: snd_pcm_lib_malloc_pages failed: %d\n",
		       DRV_NAME, err);
		return err;
	}

	return 0;
}

static int rpi_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct rpi_pcm_priv *priv = substream->runtime->private_data;

	printk(KERN_INFO "%s: hw_free\n", DRV_NAME);

	if (priv)
		rpi_stop_timer(priv);

	return snd_pcm_lib_free_pages(substream);
}

static int rpi_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rpi_pcm_priv *priv = runtime->private_data;

	printk(KERN_INFO "%s: prepare (buf_size=%lu frames, period=%lu)\n",
	       DRV_NAME,
	       (unsigned long)runtime->buffer_size,
	       (unsigned long)runtime->period_size);

	if (!priv)
		return -EINVAL;

	rpi_stop_timer(priv);
	priv->hw_ptr = 0;

	/* Time for one period in nanoseconds */
	priv->period_ns = div64_u64((u64)NSEC_PER_SEC * (u64)runtime->period_size,
				    (u64)runtime->rate);
	if (priv->period_ns == 0)
		priv->period_ns = 1;

	return 0;
}

static int rpi_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct rpi_pcm_priv *priv = substream->runtime->private_data;

	printk(KERN_INFO "%s: trigger cmd=%d\n", DRV_NAME, cmd);

	if (!priv)
		return -EINVAL;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		atomic_set(&priv->running, 1);
		rpi_start_timer(priv);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		rpi_stop_timer(priv);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static snd_pcm_uframes_t rpi_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct rpi_pcm_priv *priv = runtime->private_data;
	snd_pcm_uframes_t pos;
	unsigned long flags;

	snd_pcm_stream_lock_irqsave(substream, flags);
	pos = priv ? priv->hw_ptr : 0;
	snd_pcm_stream_unlock_irqrestore(substream, flags);

	return pos;
}

static int rpi_pcm_ioctl(struct snd_pcm_substream *substream,
			 unsigned int cmd, void *arg)
{
	/* Standard buffer/ioctl handling for mmap etc. */
	return snd_pcm_lib_ioctl(substream, cmd, arg);
}

static const struct snd_pcm_ops rpi_pcm_ops = {
	.open = rpi_pcm_open,
	.close = rpi_pcm_close,
	.ioctl = rpi_pcm_ioctl,
	.hw_params = rpi_pcm_hw_params,
	.hw_free = rpi_pcm_hw_free,
	.prepare = rpi_pcm_prepare,
	.trigger = rpi_pcm_trigger,
	.pointer = rpi_pcm_pointer,
};

/* ---- Card registration ---- */

static int rpi_pcm_new(struct rpi_card *chip)
{
	struct snd_pcm *pcm;
	int err;

	err = snd_pcm_new(chip->card, PCM_NAME, 0, 1, 0, &pcm);
	if (err < 0)
		return err;

	chip->pcm = pcm;
	pcm->private_data = chip;
	strscpy(pcm->name, PCM_NAME, sizeof(pcm->name));

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &rpi_pcm_ops);

	/*
	 * VMALLOC buffer type so snd_pcm_lib_malloc_pages() works in hw_params.
	 * Return type of snd_pcm_lib_preallocate_pages_for_all became void in 6.4+.
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_VMALLOC,
					      NULL, 0, 256 * 1024);
#else
	err = snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_VMALLOC,
						    NULL, 0, 256 * 1024);
	if (err < 0)
		return err;
#endif

	return 0;
}

static int rpi_audio_probe(struct platform_device *pdev)
{
	struct snd_card *card;
	struct rpi_card *chip;
	int err;

	printk(KERN_INFO "%s: probe\n", DRV_NAME);

	err = snd_card_new(&pdev->dev, -1, CARD_NAME, THIS_MODULE,
			    sizeof(struct rpi_card), &card);
	if (err < 0)
		return err;

	chip = card->private_data;
	chip->card = card;
	chip->pdev = pdev;
	platform_set_drvdata(pdev, chip);

	strscpy(card->driver, DRV_NAME, sizeof(card->driver));
	strscpy(card->shortname, "RPI Raw PCM", sizeof(card->shortname));
	snprintf(card->longname, sizeof(card->longname),
		 "Raspberry Pi minimal RAW PCM %s", card->shortname);

	err = rpi_pcm_new(chip);
	if (err < 0)
		goto err_card;

	err = snd_card_register(card);
	if (err < 0)
		goto err_card;

	dev_info(&pdev->dev, "registered ALSA card %s, device hw:%i,0\n",
		 card->shortname, card->number);

	return 0;

err_card:
	snd_card_free(card);
	return err;
}

static int rpi_audio_remove(struct platform_device *pdev)
{
	struct rpi_card *chip = platform_get_drvdata(pdev);

	printk(KERN_INFO "%s: remove\n", DRV_NAME);

	if (chip && chip->card)
		snd_card_free(chip->card);
	return 0;
}

static struct platform_driver rpi_audio_driver = {
	.driver = {
		.name = "rpi_alsa_raw_pcm",
		.owner = THIS_MODULE,
	},
	.probe = rpi_audio_probe,
	.remove = rpi_audio_remove,
};

static int __init rpi_alsa_init(void)
{
	int err;

	rpi_platform_dev = platform_device_register_simple("rpi_alsa_raw_pcm", -1,
							   NULL, 0);
	if (IS_ERR(rpi_platform_dev))
		return PTR_ERR(rpi_platform_dev);

	err = platform_driver_register(&rpi_audio_driver);
	if (err) {
		platform_device_unregister(rpi_platform_dev);
		return err;
	}

	return 0;
}

static void __exit rpi_alsa_exit(void)
{
	platform_driver_unregister(&rpi_audio_driver);
	platform_device_unregister(rpi_platform_dev);
}

module_init(rpi_alsa_init);
module_exit(rpi_alsa_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Minimal ALSA RAW PCM playback (S16_LE stereo 44100) for RPi");
MODULE_AUTHOR("Embedded ALSA example");
MODULE_ALIAS("platform:rpi_alsa_raw_pcm");
