#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/ioctl.h>

#define DRIVER_NAME "led_gpio_driver"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

#define GPIO_LED (17 + 512)

// IOCTL commands
#define LED_ON  _IO('L', 0)
#define LED_OFF _IO('L', 1)

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;

static int led_open(struct inode *inodep, struct file *filep) {
    pr_info("LED: Device opened\n");
    return 0;
}

static int led_release(struct inode *inodep, struct file *filep) {
    pr_info("LED: Device closed\n");
    return 0;
}

static long led_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case LED_ON:
            gpio_set_value(GPIO_LED, 1);
            pr_info("LED: ON via ioctl\n");
            break;
        case LED_OFF:
            gpio_set_value(GPIO_LED, 0);
            pr_info("LED: OFF via ioctl\n");
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .unlocked_ioctl = led_ioctl,
};

static int __init led_init(void) {
    pr_info("LED: Initializing driver...\n");

    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }

    if (gpio_request(GPIO_LED, DRIVER_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return -EBUSY;
    }

    gpio_direction_output(GPIO_LED, 0);

    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to register major number\n");
        return major;
    }

    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        return PTR_ERR(led_class);
    }

    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        class_destroy(led_class);
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        return PTR_ERR(led_device);
    }

    pr_info("LED: Driver loaded. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void) {
    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);

    device_destroy(led_class, MKDEV(major, 0));
    class_unregister(led_class);
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("LED: Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED Driver with ioctl()");
