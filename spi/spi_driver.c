#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/io.h>
#include<linux/device.h>
#include<linux/delay.h>

#define SPI_BASE 0xFE204000
#define SPI_SIZE 0x100  //256 bytes

//SPI registers
#define SPI_CS 0x00   // spi master control and status
#define SPI_FIFO 0x40  // spi master tx and rx FIFO's
#define SPI_CLK 0x08   // spi master clock divider
#define SPI_DLEN 0x0c  // spi master data length
#define SPI_LTOH 0x10  // spi LoSSI mode TOH
#define SPI_DC 0x14     // spi DMA DREQ controld

//CS register bits
#define CS_TA (1<<7)
#define CS_DONE (1<<16)
#define CS_TXD (1<<18)
#define CS_RXD (1<<17)
#define CS_CLEAR (3<<4)

#define DEVICE_NAME "rpi_spi"

static dev_t devno;
static struct cdev spi_cdev;
static struct class *spi_class;

static void __iomem *spi_base;

//MMIO helper functions
#define spi_read(off) readl(spi_base+(off))
#define uart_write(val,off) write((val),spi_base+(off))

static void spi_hw_init(void)
{
// Reset SPI
spi_write(0,SPI_CS);  // disable spi, stops any ongoing transfer
spi_write(CS_CLEAR,SPI_CS);  // flush tx and rx FIFO, ensures clean startup

// clock divider (250MHz/256)
spi_write(256,SPI_CLK);

pr_info("spi initialized\n");
}

// File operation
static ssize_t spi_write_user(struct file *f,const char __user *buf, size/-t len, loff/-t *off)
{

// echo "ABC" > /dev/rpi_spi
char ch;
size_t i;

spi_write(len,SPI_DLEN);   //send len bytes
spi_write(spi_read(SPI_CS) | CS_TA, SPI_CS);   //sets transfer bit,  spi clock starts toggling

for(i=0;i<len;i++)
{
if(copy_from_user(&ch,buf+i,1))
return -EFAULT;

while(!(spi_read(SPI_CS) & CS_TXD))   // waits until TX_FIFO has space, prevents overflow
cpu_relax();

spi_write(ch,SPI_FIFO);  //pushes bytes to FIFO,shifted out on MOSI
}

while(!spi_read(SPI_CS) & (CS_DONE))  //wait until transfer completes
cpu_relax();

spi_write(spi_read(SPI_CS) & ~CS_TA, SPI_CS);  // stops spi clock, end transaction
return len;
}

static ssize_t spi_read_user(struct file *f, char __user *buf, size_t len, loff_t *off)
{
char ch;
size_t i;

spi_write(len,SPI_DLEN);    // number of bytes expected
spi_write(spi_read(SPI_CS)| CS_TA, SPI_CS);   // starts the spi clock

for(i=0;i<len,i++)
{
while(!(spi_read(SPI_CS) & CS_RXD))   // wait until RX FIFO has data
cpu_relax();

ch=spi_read(SPI_FIFO) & 0xFF;   // read received bytes from MISO

if(copy_to_user(buf+i,&ch,1))   // transfer data to user space
return -EFAULT;

spi_write(spi_read(SPI_CS) & ~CS_TA, SPI_CS);  // stop spi
return i;
}

static struct file_operations spi_fops={
.owner=THIS_MODULE,
.read=spi_read_user,
.write=spi_write_user,
};

// Module_init
static int __init spi_init(void)
{
    int ret;

    spi_base = ioremap(SPI_BASE, SPI_SIZE);
    if (!spi_base) {
        pr_err("rpi_spi: ioremap failed\n");
        return -ENOMEM;
    }

    ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    if (ret)
        goto unmap;

    cdev_init(&spi_cdev, &spi_fops);
    ret = cdev_add(&spi_cdev, devno, 1);
    if (ret)
        goto unregister;

    spi_class = class_create(DEVICE_NAME);
    device_create(spi_class, NULL, devno, NULL, DEVICE_NAME);

    spi_hw_init();

    pr_info("rpi_spi: loaded (ioremap only)\n");
    return 0;

unregister:
    unregister_chrdev_region(devno, 1);
unmap:
    iounmap(spi_base);
    return ret;
}

/* ---------------- MODULE EXIT ---------------- */
static void __exit spi_exit(void)
{
    device_destroy(spi_class, devno);
    class_destroy(spi_class);
    cdev_del(&spi_cdev);
    unregister_chrdev_region(devno, 1);
    iounmap(spi_base);

    pr_info("rpi_spi: unloaded\n");
}

module_init(spi_init);
module_exit(spi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Raspberry Pi SPI driver using ioremap only");
