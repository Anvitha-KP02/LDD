#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define GPIO_LED (17 + 512)  // For Raspberry Pi 4
#define DRIVER_NAME "led_gpio_driver"

static struct kobject *led_kobj;

static ssize_t led_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int value = gpio_get_value(GPIO_LED);
    return sprintf(buf, "%d\n", value);
}

static ssize_t led_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    if (buf[0] == '1')
        gpio_set_value(GPIO_LED, 1);
    else if (buf[0] == '0')
        gpio_set_value(GPIO_LED, 0);
    return count;
}

static struct kobj_attribute led_attr = __ATTR(led, 0660, led_show, led_store);

static int __init led_sysfs_init(void)
{
    int ret;

    pr_info("LED: Initializing with sysfs...\n");

    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO\n");
        return -ENODEV;
    }

    ret = gpio_request(GPIO_LED, DRIVER_NAME);
    if (ret) {
        pr_err("LED: GPIO request failed\n");
        return ret;
    }

    gpio_direction_output(GPIO_LED, 0);

    led_kobj = kobject_create_and_add("led_gpio", kernel_kobj);
    if (!led_kobj) {
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to create kobject\n");
        return -ENOMEM;
    }

    ret = sysfs_create_file(led_kobj, &led_attr.attr);
    if (ret) {
        kobject_put(led_kobj);
        gpio_free(GPIO_LED);
        pr_err("LED: sysfs create file failed\n");
        return ret;
    }

    pr_info("LED: sysfs interface created at /sys/kernel/led_gpio/led\n");
    return 0;
}

static void __exit led_sysfs_exit(void)
{
    gpio_set_value(GPIO_LED, 0);
    sysfs_remove_file(led_kobj, &led_attr.attr);
    kobject_put(led_kobj);
    gpio_free(GPIO_LED);
    pr_info("LED: sysfs interface removed\n");
}

module_init(led_sysfs_init);
module_exit(led_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED control via sysfs (no device tree)");
