#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Basic SysFS Driver Example");

/* Kernel object */
static struct kobject *sysfs_kobj;

/* Data exposed via sysfs */
static int demo_value = 0;

/* -------- show() : cat -------- */
static ssize_t value_show(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char *buf)
{
    return sprintf(buf, "%d\n", demo_value);
}

/* -------- store() : echo -------- */
static ssize_t value_store(struct kobject *kobj,
                           struct kobj_attribute *attr,
                           const char *buf,
                           size_t count)
{
    int ret;

    ret = kstrtoint(buf, 10, &demo_value);
    if (ret)
        return ret;

    pr_info("sysfs_demo: value updated to %d\n", demo_value);
    return count;
}

/* Sysfs attribute */
static struct kobj_attribute value_attr =
    __ATTR(value, 0664, value_show, value_store);

/* -------- Module Init -------- */
static int __init sysfs_demo_init(void)
{
    int ret;

    /* Create /sys/kernel/sysfs_demo */
    sysfs_kobj = kobject_create_and_add("sysfs_demo", kernel_kobj);
    if (!sysfs_kobj)
        return -ENOMEM;

    /* Create /sys/kernel/sysfs_demo/value */
    ret = sysfs_create_file(sysfs_kobj, &value_attr.attr);
    if (ret) {
        kobject_put(sysfs_kobj);
        return ret;
    }

    pr_info("sysfs_demo: loaded\n");
    return 0;
}

/* -------- Module Exit -------- */
static void __exit sysfs_demo_exit(void)
{
    sysfs_remove_file(sysfs_kobj, &value_attr.attr);
    kobject_put(sysfs_kobj);
    pr_info("sysfs_demo: unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);
