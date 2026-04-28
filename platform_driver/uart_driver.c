#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "simple_uart"
#define UART_MEM_BASE 0xFE201000 
#define UART_MEM_SIZE 0x100
#define UART_TX 0x00
#define UART_RX 0x00
#define UART_LSR 0x14
#define UART_LSR_THRE 0x20
#define UART_LSR_DR   0x01

static void __iomem *uart_base;
static int major;

/* Simple TX */
static void uart_write_char(char c)
{
    while (!(readb(uart_base + UART_LSR) & UART_LSR_THRE))
        cpu_relax();
    writeb(c, uart_base + UART_TX);
}

/* Simple RX */
static char uart_read_char(void)
{
    while (!(readb(uart_base + UART_LSR) & UART_LSR_DR))
        cpu_relax();
    return readb(uart_base + UART_RX);
}

/* char device write() */
static ssize_t uart_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char kbuf[64];
    size_t i;

    if (len > sizeof(kbuf))
        len = sizeof(kbuf);

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    for (i = 0; i < len; i++)
        uart_write_char(kbuf[i]);

    return len;
}

/* char device read() */
static ssize_t uart_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    char kbuf[64];
    size_t i;

    if (len > sizeof(kbuf))
        len = sizeof(kbuf);

    for (i = 0; i < len; i++)
        kbuf[i] = uart_read_char();

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    return len;
}

static struct file_operations uart_fops = {
    .owner = THIS_MODULE,
    .write = uart_dev_write,
    .read  = uart_dev_read,
};

static int uart_probe(struct platform_device *pdev)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    uart_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(uart_base))
        return PTR_ERR(uart_base);

    major = register_chrdev(0, DRIVER_NAME, &uart_fops);
    pr_info("Simple UART: registered at /dev/%s, major=%d\n", DRIVER_NAME, major);

    return 0;
}

static int uart_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "Remove function called\n");
    unregister_chrdev(major, DRIVER_NAME);
    return 0;
}

static struct platform_driver uart_driver = {
    .probe  = uart_probe,
    .remove = uart_remove,
    .driver = {
        .name = DRIVER_NAME,
    },
};

/* For testing without DTS, create platform_device here */
static struct resource uart_resources[] = {
    {
        .start = UART_MEM_BASE,
        .end   = UART_MEM_BASE + UART_MEM_SIZE - 1,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device *uart_pdev;

static int __init uart_init(void)
{
    int ret;

    ret = platform_driver_register(&uart_driver);
    if (ret)
        return ret;

    uart_pdev = platform_device_register_simple(DRIVER_NAME, -1,
                                                uart_resources,
                                                ARRAY_SIZE(uart_resources));
    if (IS_ERR(uart_pdev)) {
        platform_driver_unregister(&uart_driver);
        return PTR_ERR(uart_pdev);
    }

    pr_info("Simple UART driver loaded\n");
    return 0;
}

static void __exit uart_exit(void)
{
    platform_device_unregister(uart_pdev);
    platform_driver_unregister(&uart_driver);
    pr_info("Simple UART driver unloaded\n");
}

module_init(uart_init);
module_exit(uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Priya");
MODULE_DESCRIPTION("Very Simple UART Platform Driver without DTS");
