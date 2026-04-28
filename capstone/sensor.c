#include <linux/module.h> 	//module_init  module_exit
#include <linux/init.h>         //for __init,__exit macros
#include <linux/io.h>		//for ioremap(),readl(),writel()	
#include <linux/kthread.h>      //kernel thread support
#include <linux/delay.h>	//msleep

#define UART_BASE  0xFE201000 	//uart base address
#define UART_SIZE  0x100	

#define UART_DR    0x00		//uart data register  
#define UART_FR    0x18		//uart flag register 
#define UART_IBRD  0x24		//integer baud register
#define UART_FBRD  0x28		//fractional baud register 
#define UART_LCRH  0x2C		//line control register
#define UART_CR    0x30		//control register offset
#define UART_ICR   0x44		//interrupt clear register

#define FR_TXFF (1 << 5)	//transmit fifo full
#define FR_RXFE (1 << 4)	//receive fifo full

#define CR_UARTEN (1 << 0)	//uart enable bit
#define CR_TXE    (1 << 8)	//transmit enable
#define CR_RXE    (1 << 9)	//receive enable

#define LCRH_8BIT (3 << 5)	//word length
#define LCRH_FEN  (1 << 4)	//fifo enable

#define MAX_IMAGE_SIZE 512

static u8 registered_image[MAX_IMAGE_SIZE]; //buffer to store finger print data
static int image_size = 0; //size

static void __iomem *uart_base;		//kernel virtual address for uart
static struct task_struct *mimic_thread;  //kernel thread for mimiced sensor
static int registered = 0;

/* ===== GC07 ACK PACKETS ===== */
// success packet
static u8 success_ack[] = {
	0xEF,0x01,0xFF,0xFF,0xFF,0xFF,
	0x07,0x00,0x03,
	0x00,
	0x00,0x0A
};
// failure packet
static u8 fail_ack[] = {
	0xEF,0x01,0xFF,0xFF,0xFF,0xFF,
	0x07,0x00,0x03,
	0x01,
	0x00,0x0B
};

/* ================= UART INIT ================= */

static void uart_hw_init(void)
{
	writel(0, uart_base + UART_CR);	//disabling uart before initialisation
	writel(0x7FF, uart_base + UART_ICR);	//clearing all pending interrupts

	//setting uart baud rate
	writel(26, uart_base + UART_IBRD);
	writel(3,  uart_base + UART_FBRD);
	//setting uart 8-bit, enabling fifo, enabling Tx and Rx
	writel(LCRH_8BIT | LCRH_FEN, uart_base + UART_LCRH);
	writel(CR_UARTEN | CR_TXE | CR_RXE, uart_base + UART_CR);

	pr_info("MIMIC: UART initialized\n");
}

/* ================= UART HELPERS ================= */
//write 1 byte
static void uart_write_byte(u8 byte)
{
	//waits until tramit fifo is empty
	while (readl(uart_base + UART_FR) & FR_TXFF)
		cpu_relax();
	//writing data into data register
	writel(byte, uart_base + UART_DR);
}
//read 1 byte
static int uart_read_byte(u8 *byte)
{
	//returns 0 if no data is available 
	if (readl(uart_base + UART_FR) & FR_RXFE)
		return 0;
	//returns 1 if data is available
	*byte = readl(uart_base + UART_DR) & 0xFF;
	return 1;
}
//write multiple bytes
static void uart_send(u8 *data, int len)
{
	int i;
	//sending byte by byte
	for (i = 0; i < len; i++)
		uart_write_byte(data[i]);
}

/* ================= MIMIC THREAD ================= */

static int mimic_fn(void *data)
{
	u8 buffer[32];
	int index = 0;

	pr_info("GC07 Mimic Started...\n");

	while (!kthread_should_stop())	//runs until the module is unloaded
	{
		u8 byte;

		if (uart_read_byte(&byte))
		{
			// Start-of-packet check
			if (index == 0 && byte != 0xEF){
				continue;  // discard any garbage
			}
			buffer[index++] = byte;
			//pr_info("byte :%d  0x%02X   ",index-1,buffer[index-1]);
			if (index >= 12)  // minimum GC07 packet
			{
				pr_info("MIMIC: Packet received, CMD=0x%02X\n", buffer[9]);

				switch (buffer[9])
				{
					case 0x01:  // GET_IMAGE
						pr_info("MIMIC: GET_IMAGE\n");
						uart_send(success_ack, sizeof(success_ack));
						break;

					case 0x06:  // FP_ADD
						pr_info("MIMIC: FP_ADD (Register)\n");
			
						// extract packet length from bytes 7-8 because finger print protocols store length in 2 bytes 
						int pkt_len = (buffer[7] << 8) | buffer[8];

						int storing_len = pkt_len - 3; // exclude cmd + 2-byte checksum
						//to prevent buffer overloading
						if (storing_len > MAX_IMAGE_SIZE)
							storing_len = MAX_IMAGE_SIZE;

						memcpy(registered_image, buffer + 10, storing_len);
						image_size = storing_len;
						registered = 1;
						uart_send(success_ack, sizeof(success_ack));
						break;

					case 0x04:  // FP_SEARCH
						pr_info("MIMIC: FP_SEARCH\n");
						
						int search_len = ((buffer[7] << 8) | buffer[8])- 3; // EXCLUDING(Command + Check sum)
						if (search_len > image_size)
							search_len = image_size;

						if (registered && (memcmp(registered_image, buffer + 10, search_len) == 0))
						{
							uart_send(success_ack, sizeof(success_ack));
							pr_info("MIMIC: MATCHED\n");
						}
						else
							uart_send(fail_ack, sizeof(fail_ack));
						break;

					default:
						pr_info("MIMIC: Unknown CMD\n");
						break;
				}

				index = 0;
				memset(buffer,0,sizeof(buffer));
			}
		}

		msleep(5);
	}

	return 0;
}

/* ================= MODULE INIT ================= */

static int __init mimic_init(void)
{
	//mapping physical address to virtual address
	uart_base = ioremap(UART_BASE, UART_SIZE);
	if (!uart_base)
		return -ENOMEM;

	uart_hw_init();

	mimic_thread = kthread_run(mimic_fn, NULL, "gc07_mimic"); //start mimic sensor thread
	if (IS_ERR(mimic_thread))
		return PTR_ERR(mimic_thread);

	pr_info("GC07 Mimic Module Loaded\n");
	return 0;
}

/* ================= MODULE EXIT ================= */

static void __exit mimic_exit(void)
{
	if (mimic_thread)
		kthread_stop(mimic_thread);  //stop kernel thread

	if (uart_base)
		iounmap(uart_base);	//release the hardware mapping

	pr_info("GC07 Mimic Module Unloaded\n");
}

module_init(mimic_init);
module_exit(mimic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GC07 Kernel Mimic");
MODULE_DESCRIPTION("Kernel GC07 Fingerprint Sensor Emulator using PL011 MMIO");
