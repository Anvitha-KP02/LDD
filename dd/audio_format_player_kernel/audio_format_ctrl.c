#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

enum audio_fmt {
	AUDIO_FMT_MP3 = 1,
	AUDIO_FMT_AAC = 2,
	AUDIO_FMT_FLAC = 3,
};

static DEFINE_MUTEX(fmt_lock);
static int current_fmt = AUDIO_FMT_MP3;

static const char *fmt_to_str(int fmt)
{
	switch (fmt) {
	case AUDIO_FMT_MP3:
		return "mp3";
	case AUDIO_FMT_AAC:
		return "aac";
	case AUDIO_FMT_FLAC:
		return "flac";
	default:
		return "unknown";
	}
}

static int str_to_fmt(const char *s)
{
	if (!s)
		return -EINVAL;
	if (sysfs_streq(s, "mp3") || sysfs_streq(s, "1"))
		return AUDIO_FMT_MP3;
	if (sysfs_streq(s, "aac") || sysfs_streq(s, "2"))
		return AUDIO_FMT_AAC;
	if (sysfs_streq(s, "flac") || sysfs_streq(s, "3"))
		return AUDIO_FMT_FLAC;
	return -EINVAL;
}

static ssize_t afc_read(struct file *f, char __user *ubuf, size_t len, loff_t *ppos)
{
	char buf[16];
	ssize_t n;
	int fmt;

	mutex_lock(&fmt_lock);
	fmt = current_fmt;
	mutex_unlock(&fmt_lock);

	n = scnprintf(buf, sizeof(buf), "%s\n", fmt_to_str(fmt));
	return simple_read_from_buffer(ubuf, len, ppos, buf, n);
}

static ssize_t afc_write(struct file *f, const char __user *ubuf, size_t len, loff_t *ppos)
{
	char buf[16];
	int fmt;

	if (len == 0)
		return 0;
	if (len >= sizeof(buf))
		return -EINVAL;
	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;
	buf[len] = '\0';

	fmt = str_to_fmt(buf);
	if (fmt < 0)
		return fmt;

	mutex_lock(&fmt_lock);
	current_fmt = fmt;
	mutex_unlock(&fmt_lock);

	return len;
}

static const struct file_operations afc_fops = {
	.owner = THIS_MODULE,
	.read = afc_read,
	.write = afc_write,
	.llseek = no_llseek,
};

static ssize_t format_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int fmt;

	mutex_lock(&fmt_lock);
	fmt = current_fmt;
	mutex_unlock(&fmt_lock);

	return scnprintf(buf, PAGE_SIZE, "%s\n", fmt_to_str(fmt));
}

static ssize_t format_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int fmt = str_to_fmt(buf);

	if (fmt < 0)
		return fmt;

	mutex_lock(&fmt_lock);
	current_fmt = fmt;
	mutex_unlock(&fmt_lock);

	return count;
}

static DEVICE_ATTR_RW(format);

static struct miscdevice afc_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "audio_format_ctrl",
	.fops = &afc_fops,
	.mode = 0666,
};

static int __init afc_init(void)
{
	int ret;

	ret = misc_register(&afc_misc);
	if (ret)
		return ret;

	ret = device_create_file(afc_misc.this_device, &dev_attr_format);
	if (ret) {
		misc_deregister(&afc_misc);
		return ret;
	}

	pr_info("audio_format_ctrl: loaded (/dev/%s, sysfs format)\n", afc_misc.name);
	pr_info("audio_format_ctrl: NOTE: kernel does NOT decode; user space must use ffmpeg\n");
	return 0;
}

static void __exit afc_exit(void)
{
	device_remove_file(afc_misc.this_device, &dev_attr_format);
	misc_deregister(&afc_misc);
	pr_info("audio_format_ctrl: unloaded\n");
}

module_init(afc_init);
module_exit(afc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anvitha");
MODULE_DESCRIPTION("Audio playing");
