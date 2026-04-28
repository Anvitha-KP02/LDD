/*
 * rpi_alsa_raw_driver.c
 *
 * Interview-ready reference driver:
 * - ALSA PCM playback interface (kernel space).
 * - Control misc device with two ioctls:
 *      PLAY_RAW: arm playback path for RAW data.
 *      PLAY_MP3: request MP3->RAW conversion (via ffmpeg in user space context)
 *                and arm playback path.
 *
 * Important: MP3 decoding is NOT done in kernel.
 * The PLAY_MP3 ioctl demonstrates flow by invoking ffmpeg via usermode helper.
 * In production, prefer a dedicated user-space daemon/service for conversion.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#define DRV_NAME            "rpi_alsa_raw"
#define CTRL_DEV_NAME       "rpi_raw_ctl"
#define PCM_DEV_NAME        "RPI-RAW-PCM"

#define PCM_RATE            44100
#define PCM_CHANNELS        2
#define PCM_BITS            16
#define PCM_PERIOD_BYTES    4096
#define PCM_BUFFER_BYTES    (64 * 1024)
#define TIMER_PERIOD_MS     10

#define RPI_IOC_MAGIC       'R'
#define PLAY_RAW            _IO(RPI_IOC_MAGIC, 1)
#define PLAY_MP3            _IOW(RPI_IOC_MAGIC, 2, struct rpi_mp3_req)

struct rpi_mp3_req {
	char mp3_path[256];
	char raw_path[256];
};

struct rpi_dev {
	struct snd_card *card;
	struct snd_pcm *pcm;
	struct snd_pcm_substream *playback_substream;

	struct timer_list period_timer;
	spinlock_t lock;
	size_t hw_ptr_bytes;
	bool running;
	bool play_armed;

	struct miscdevice miscdev;
	struct mutex io_mutex;
};

static struct rpi_dev *gdev;

static const struct snd_pcm_hardware rpi_pcm_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_BLOCK_TRANSFER |
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_44100,
	.rate_min = PCM_RATE,
	.rate_max = PCM_RATE,
	.channels_min = PCM_CHANNELS,
	.channels_max = PCM_CHANNELS,
	.buffer_bytes_max = PCM_BUFFER_BYTES,
	.period_bytes_min = PCM_PERIOD_BYTES,
	.period_bytes_max = PCM_PERIOD_BYTES,
	.periods_min = 2,
	.periods_max = PCM_BUFFER_BYTES / PCM_PERIOD_BYTES,
};

static void rpi_pcm_timer_cb(struct timer_list *t)
{
	struct rpi_dev *dev = from_timer(dev, t, period_timer);
	struct snd_pcm_substream *substream;
	unsigned long flags;
	size_t period_bytes;
	size_t buf_bytes;

	spin_lock_irqsave(&dev->lock, flags);
	if (!dev->running || !dev->playback_substream) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return;
	}

	substream = dev->playback_substream;
	period_bytes = frames_to_bytes(substream->runtime,
				       snd_pcm_lib_period_bytes(substream));
	if (!period_bytes)
		period_bytes = PCM_PERIOD_BYTES;

	buf_bytes = snd_pcm_lib_buffer_bytes(substream);
	if (!buf_bytes)
		buf_bytes = PCM_BUFFER_BYTES;

	dev->hw_ptr_bytes = (dev->hw_ptr_bytes + period_bytes) % buf_bytes;
	spin_unlock_irqrestore(&dev->lock, flags);

	snd_pcm_period_elapsed(substream);
	mod_timer(&dev->period_timer, jiffies + msecs_to_jiffies(TIMER_PERIOD_MS));
}

static int rpi_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = rpi_pcm_hardware;
	gdev->playback_substream = substream;
	pr_info("%s: PCM open\n", DRV_NAME);
	return 0;
}

static int rpi_pcm_close(struct snd_pcm_substream *substream)
{
	gdev->playback_substream = NULL;
	pr_info("%s: PCM close\n", DRV_NAME);
	return 0;
}

static int rpi_pcm_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params)
{
	int ret;

	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
	if (ret < 0)
		return ret;

	pr_info("%s: hw_params: buffer=%u\n", DRV_NAME, params_buffer_bytes(params));
	return 0;
}

static int rpi_pcm_hw_free(struct snd_pcm_substream *substream)
{
	pr_info("%s: hw_free\n", DRV_NAME);
	return snd_pcm_lib_free_pages(substream);
}

static int rpi_pcm_prepare(struct snd_pcm_substream *substream)
{
	unsigned long flags;

	spin_lock_irqsave(&gdev->lock, flags);
	gdev->hw_ptr_bytes = 0;
	spin_unlock_irqrestore(&gdev->lock, flags);

	pr_info("%s: prepare\n", DRV_NAME);
	return 0;
}

static int rpi_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	unsigned long flags;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (!gdev->play_armed) {
			pr_err("%s: START denied: call PLAY_RAW or PLAY_MP3 first\n",
			       DRV_NAME);
			return -EPERM;
		}
		spin_lock_irqsave(&gdev->lock, flags);
		gdev->running = true;
		spin_unlock_irqrestore(&gdev->lock, flags);
		mod_timer(&gdev->period_timer,
			  jiffies + msecs_to_jiffies(TIMER_PERIOD_MS));
		pr_info("%s: trigger START\n", DRV_NAME);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		spin_lock_irqsave(&gdev->lock, flags);
		gdev->running = false;
		spin_unlock_irqrestore(&gdev->lock, flags);
		del_timer_sync(&gdev->period_timer);
		pr_info("%s: trigger STOP\n", DRV_NAME);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static snd_pcm_uframes_t rpi_pcm_pointer(struct snd_pcm_substream *substream)
{
	snd_pcm_uframes_t frames;
	unsigned long flags;
	size_t hw_ptr;

	spin_lock_irqsave(&gdev->lock, flags);
	hw_ptr = gdev->hw_ptr_bytes;
	spin_unlock_irqrestore(&gdev->lock, flags);

	frames = bytes_to_frames(substream->runtime, hw_ptr);
	return frames;
}

static const struct snd_pcm_ops rpi_pcm_ops = {
	.open = rpi_pcm_open,
	.close = rpi_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = rpi_pcm_hw_params,
	.hw_free = rpi_pcm_hw_free,
	.prepare = rpi_pcm_prepare,
	.trigger = rpi_pcm_trigger,
	.pointer = rpi_pcm_pointer,
};

static ssize_t rpi_ctl_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *ppos)
{
	/*
	 * Control node accepts writes for debug/demo only.
	 * Real PCM samples are written by user space to ALSA PCM device
	 * using snd_pcm_writei()/aplay, not to this misc node.
	 */
	pr_info("%s: /dev/%s write count=%zu (control/debug)\n",
		DRV_NAME, CTRL_DEV_NAME, count);
	return count;
}

static long rpi_ctl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct rpi_mp3_req *req;
	char *ffmpeg_cmd = NULL;
	char *argv[] = { "/bin/sh", "-c", NULL, NULL };
	static char *envp[] = {
		"HOME=/",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL
	};
	int ret;

	mutex_lock(&gdev->io_mutex);

	switch (cmd) {
	case PLAY_RAW:
		gdev->play_armed = true;
		pr_info("%s: ioctl PLAY_RAW -> playback armed\n", DRV_NAME);
		mutex_unlock(&gdev->io_mutex);
		return 0;

	case PLAY_MP3:
		req = kzalloc(sizeof(*req), GFP_KERNEL);
		if (!req) {
			mutex_unlock(&gdev->io_mutex);
			return -ENOMEM;
		}

		ffmpeg_cmd = kzalloc(640, GFP_KERNEL);
		if (!ffmpeg_cmd) {
			kfree(req);
			mutex_unlock(&gdev->io_mutex);
			return -ENOMEM;
		}

		if (copy_from_user(req, (void __user *)arg, sizeof(*req))) {
			kfree(ffmpeg_cmd);
			kfree(req);
			mutex_unlock(&gdev->io_mutex);
			return -EFAULT;
		}

		req->mp3_path[sizeof(req->mp3_path) - 1] = '\0';
		req->raw_path[sizeof(req->raw_path) - 1] = '\0';

		snprintf(ffmpeg_cmd, 640,
			 "/usr/bin/ffmpeg -y -i '%s' -f s16le "
			 "-acodec pcm_s16le -ac %d -ar %d '%s'",
			 req->mp3_path, PCM_CHANNELS, PCM_RATE, req->raw_path);
		argv[2] = ffmpeg_cmd;

		pr_info("%s: ioctl PLAY_MP3 -> %s\n", DRV_NAME, ffmpeg_cmd);
		ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
		kfree(ffmpeg_cmd);
		kfree(req);
		if (ret) {
			pr_err("%s: ffmpeg conversion failed, ret=%d\n", DRV_NAME, ret);
			mutex_unlock(&gdev->io_mutex);
			return ret;
		}

		gdev->play_armed = true;
		pr_info("%s: PLAY_MP3 done, playback armed; now write RAW via ALSA PCM\n",
			DRV_NAME);
		mutex_unlock(&gdev->io_mutex);
		return 0;

	default:
		mutex_unlock(&gdev->io_mutex);
		return -ENOTTY;
	}
}

static const struct file_operations rpi_ctl_fops = {
	.owner = THIS_MODULE,
	.write = rpi_ctl_write,
	.unlocked_ioctl = rpi_ctl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = rpi_ctl_ioctl,
#endif
};

static int __init rpi_init(void)
{
	int ret;

	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;

	spin_lock_init(&gdev->lock);
	mutex_init(&gdev->io_mutex);
	timer_setup(&gdev->period_timer, rpi_pcm_timer_cb, 0);

	ret = snd_card_new(NULL, -1, DRV_NAME, THIS_MODULE, 0, &gdev->card);
	if (ret < 0) {
		pr_err("%s: snd_card_new failed: %d\n", DRV_NAME, ret);
		goto err_free_dev;
	}

	strscpy(gdev->card->driver, DRV_NAME, sizeof(gdev->card->driver));
	strscpy(gdev->card->shortname, "RPI RAW PCM Card",
		sizeof(gdev->card->shortname));
	strscpy(gdev->card->longname, "Raspberry Pi RAW PCM Demo Card",
		sizeof(gdev->card->longname));

	ret = snd_pcm_new(gdev->card, PCM_DEV_NAME, 0, 1, 0, &gdev->pcm);
	if (ret < 0) {
		pr_err("%s: snd_pcm_new failed: %d\n", DRV_NAME, ret);
		goto err_free_card;
	}

	snd_pcm_set_ops(gdev->pcm, SNDRV_PCM_STREAM_PLAYBACK, &rpi_pcm_ops);
	gdev->pcm->private_data = gdev;
	strscpy(gdev->pcm->name, PCM_DEV_NAME, sizeof(gdev->pcm->name));

	snd_pcm_lib_preallocate_pages_for_all(gdev->pcm, SNDRV_DMA_TYPE_VMALLOC,
					      NULL, PCM_BUFFER_BYTES,
					      PCM_BUFFER_BYTES);

	ret = snd_card_register(gdev->card);
	if (ret < 0) {
		pr_err("%s: snd_card_register failed: %d\n", DRV_NAME, ret);
		goto err_free_prealloc;
	}

	gdev->miscdev.minor = MISC_DYNAMIC_MINOR;
	gdev->miscdev.name = CTRL_DEV_NAME;
	gdev->miscdev.fops = &rpi_ctl_fops;
	gdev->miscdev.mode = 0666;

	ret = misc_register(&gdev->miscdev);
	if (ret) {
		pr_err("%s: misc_register failed: %d\n", DRV_NAME, ret);
		goto err_unregister_card;
	}

	pr_info("%s: loaded. ALSA card registered + /dev/%s control node\n",
		DRV_NAME, CTRL_DEV_NAME);
	return 0;

err_unregister_card:
	snd_card_free(gdev->card);
	goto err_free_dev;
err_free_prealloc:
	snd_pcm_lib_preallocate_free_for_all(gdev->pcm);
err_free_card:
	snd_card_free(gdev->card);
err_free_dev:
	kfree(gdev);
	gdev = NULL;
	return ret;
}

static void __exit rpi_exit(void)
{
	if (!gdev)
		return;

	del_timer_sync(&gdev->period_timer);
	misc_deregister(&gdev->miscdev);

	if (gdev->pcm)
		snd_pcm_lib_preallocate_free_for_all(gdev->pcm);
	if (gdev->card)
		snd_card_free(gdev->card);

	kfree(gdev);
	gdev = NULL;
	pr_info("%s: unloaded\n", DRV_NAME);
}

module_init(rpi_init);
module_exit(rpi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Linux Interview Design");
MODULE_DESCRIPTION("Raspberry Pi RAW PCM ALSA kernel module with ioctl control");
