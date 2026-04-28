/* ================= ILI9225 + GC07 + TCP + PL011 UART =================*/

#include <linux/module.h>      // Required for all kernel modules
#include <linux/kernel.h>      // pr_info(), pr_err()
#include <linux/kthread.h>     // Kernel thread support
#include <linux/net.h>         // Socket APIs
#include <linux/in.h>          // sockaddr_in structure
#include <net/sock.h>          // Kernel socket functions
#include <linux/string.h>      // strcmp(), memset()
#include <linux/delay.h>       // msleep()
#include <linux/kmod.h>        // call_usermodehelper()
#include <linux/spi/spi.h>     // SPI subsystem
#include <linux/gpio/consumer.h> // GPIO descriptor API
#include <linux/io.h>          // ioremap(), readl(), writel()
#include <linux/types.h>       // for u8, u16 etc
#include "characters.h"        // 8x8 ASCII bitmap font

#define SERVER_PORT 8082    // TCP server port number
#define RX_MAX 512          // Max receive buffer size
#define DRIVER_NAME "ili9225_driver"  // SPI driver name

/* ================= PL011 UART MMIO ================= */
#define UART_BASE  0xFE201000  // Base physical address of PL011 UART
#define UART_SIZE  0x100       // Size of mapped register region

//regsister offsetsfrom base address
#define UART_DR    0x00  //data register
#define UART_FR    0x18  //flag register
#define UART_IBRD  0x24  //integer baud rate
#define UART_FBRD  0x28  //fractional baud rate
#define UART_LCRH  0x2C  //Line Control

#define UART_CR    0x30  // Control Register
#define UART_ICR   0x44  // Interrupt Clear Register

// Flag bits
#define FR_TXFF (1 << 5) // TX FIFO Full
#define FR_RXFE (1 << 4) // RX FIFO Empty
			 //control bits
#define CR_UARTEN (1 << 0) // UART Enable
#define CR_TXE    (1 << 8) // TX Enable
#define CR_RXE    (1 << 9) // RX Enable

//line control configuration
#define LCRH_8BIT (3 << 5) //8 bit data
#define LCRH_FEN  (1 << 4) //FIFO enable

static void __iomem *uart_base; // Pointer to mapped UART registers

/* ================= GLOBALS ================= */
static struct task_struct *server_thread; // Kernel thread for TCP server
static struct socket *listen_sock;        // Listening socket
static bool first_scan = true;            // True = registration mode

struct ili9225 {
	struct spi_device *spi;  // SPI device pointer
	struct gpio_desc *dc;        // Data/Command GPIO
	struct gpio_desc *reset;     // Reset GPIO
};

static struct ili9225 *g_lcd;// Global LCD pointer


static int ili9225_write16(struct ili9225 *lcd, u16 value);
static int ili9225_write_reg(struct ili9225 *lcd, u16 reg, u16 data);

/* ================= GC07 PACKETS ================= */
static u8 get_image[] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x01,0x00,0x05};
static u8 fp_add[]    = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x06,0x06,0x01,0x00,0x01,0x00,0x0F};
static u8 fp_second_person[] =  {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x04,0x02,0x00,0x00,0x00,0x0F};
static u8 fp_search[] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x0F};

/* ================= UART LOW LEVEL ================= */

// Write single byte to UART
static void uart_write_byte(u8 byte)
{
	while (readl(uart_base + UART_FR) & FR_TXFF) // Wait until TX FIFO is not full
		cpu_relax();

	writel(byte, uart_base + UART_DR); // Write byte to Data Register
}

// Read single byte from UART (non-blocking)
static int uart_read_byte(u8 *byte)
{
	
	// If RX FIFO empty ->no data
	if (readl(uart_base + UART_FR) & FR_RXFE)
		return 0;

	//read byte from data register
	*byte = readl(uart_base + UART_DR) & 0xFF;


	return 1;
}
//rad multiple bytes
static void uart_write_data(u8 *buf, int len)
{
	pr_info("write...........\n");
	int i;
	for (i = 0; i < len; i++)
		uart_write_byte(buf[i]);
	pr_info("UART: wrote %d bytes\n", len);
}
//clear RX FIFO
static void uart_flush_rx(void)
{
	u8 dummy;
	while (uart_read_byte(&dummy));

}
//read data with timeout mechanism
static int uart_read_with_timeout(u8 *buf, int maxlen, int timeout_ms)
{
	int total = 0;
	int wait = timeout_ms;

	while (total < maxlen && wait > 0) {

		if (uart_read_byte(&buf[total])) {
			total++;
			wait = timeout_ms;   // reset timeout after each byte
		} else {
			msleep(2);
			wait -= 2;
		}
	}

	return total;
}
//initialize PL011 UART hardware
static void uart_hw_init(void)
{
	writel(0, uart_base + UART_CR);  //disable uart
	writel(0x7FF, uart_base + UART_ICR); //clear pending interrupts

	/* 115200 baud @ 48MHz UART clock */
	writel(26, uart_base + UART_IBRD);
	writel(3,  uart_base + UART_FBRD);

	//configure 8-bit data + FIFO enable
	writel(LCRH_8BIT | LCRH_FEN, uart_base + UART_LCRH);
	//Enable UART,TX,RX
	writel(CR_UARTEN | CR_TXE | CR_RXE, uart_base + UART_CR);

	pr_info("PL011 UART initialized\n");
}

/* ================= AUDIO ================= */

static void play_audio(void)
{
	char *argv[] = {"/usr/bin/aplay",
		"/home/project/project/welcome.wav",
		NULL};
	
	pr_info("playing welcome audio\n");
	//plays audio
	call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_PROC);
	pr_info("played audio\n");
}

/* ================= LCD ================= */
// Draw a single pixel at (x,y) with given color
static void drawPixel(int x, int y, uint16_t color)
{
	struct ili9225 *lcd = g_lcd; //lcd global pointer

	// rotation adjustment
	int x_rotated = 219 - x; //flip x axis
	int y_rotated = y;  //flip y

	// Set Y
	gpiod_set_value(lcd->dc, 0);   //command mode
	ili9225_write16(lcd, 0x0020);  //Y address register
	gpiod_set_value(lcd->dc, 1);   //data mode
	ili9225_write16(lcd,y_rotated); // Use y_rotated for Gram Address Set


    // Set X
    gpiod_set_value(lcd->dc, 0);  
    ili9225_write16(lcd, 0x0021);   //X register address
    gpiod_set_value(lcd->dc, 1);
    ili9225_write16(lcd,  x_rotated); // Use x_rotated for Gram Address Set

    // Write pixel color
    gpiod_set_value(lcd->dc, 0);
    ili9225_write16(lcd, 0x0022);  //GRAM write
    gpiod_set_value(lcd->dc, 1);
    ili9225_write16(lcd, color);   //color
}
// Draw a single ASCII character using 8x8 bitmap font
static void drawChar(int x, int y, char c, u16 color)
{
	int scale=2;  // Character scaling factor (2x size)
    if(c < 32 || c > 126) return; // ignore non-printable ASCII chars
    const uint8_t *bitmap = font8x8[c - 32];  // Get bitmap for character from font array

     // Loop through each row (8 rows) and each col(8 cols)
    for(int row = 0; row < 8; row++)
    {
        for(int col = 0; col < 8; col++)
        {
            if(bitmap[row] & (1 << (7 - col))) // bit 7 = leftmost( Check if bit is set (pixel ON) )
            {
                for (int dy = 0; dy < scale; dy++) // Scale pixel (draw multiple pixels for bigger font)
                {
                    for (int dx = 0; dx < scale; dx++)
                    {
                        drawPixel(x + col * scale + dx, y + row * scale + dy, color);
                    }
                }

            }
        }
    }
}

// Draw a string starting at position (x,y)
static void drawString(int x,int y,const char *str,u16 color)
{
	int scale=2;
	for(int i=0;str[i];i++)
	{
		drawChar(x,y,str[i],color);
		x+=(8*scale)+2;
	}
}

// Send 16-bit value over SPI (MSB first)
static int ili9225_write16(struct ili9225 *lcd, u16 value)
{

    u8 buf[2] = { value >> 8, value & 0xFF };  // Split into two bytes
    return spi_write(lcd->spi, buf, 2); //send via spi
}

// Write register + data to LCD
static int ili9225_write_reg(struct ili9225 *lcd, u16 reg, u16 data)
{
    gpiod_set_value(lcd->dc, 0); //command mode
    ili9225_write16(lcd, reg); //send register address

    gpiod_set_value(lcd->dc, 1);  //data mode
    return ili9225_write16(lcd, data); //send data
}
//Hardware reset sequence for lcd
static void ili9225_reset(struct ili9225 *lcd)
{
    gpiod_set_value(lcd->reset, 1);
    msleep(5);
    gpiod_set_value(lcd->reset, 0);
    msleep(20);
    gpiod_set_value(lcd->reset, 1);
    msleep(50);
}

// LCD initialization sequence (Power + Display config)
static void ili9225_init(struct ili9225 *lcd)
{
    ili9225_reset(lcd); //reset lcd

    ili9225_write_reg(lcd, 0x0001, 0x011C); //scan screen drim top to bottom
    ili9225_write_reg(lcd, 0x0002, 0x0100); //enabling flipping of volatge
    ili9225_write_reg(lcd, 0x0003, 0x1030);//LANDSCAPE EMTRY MODE
    ili9225_write_reg(lcd,0x0008, 0x0808);//controls idle time between frames
    ili9225_write_reg(lcd, 0x000C, 0x0000);//selects internal clock
    ili9225_write_reg(lcd, 0x000F, 0x0B01); //select frequency
    ili9225_write_reg(lcd, 0x0010, 0x0A00); //sets internal voltage
    ili9225_write_reg(lcd, 0x0011, 0x1038); //enable internalcircuts and starts charge pumps(capacitors)
    msleep(50);                      //to get voltage stability
    ili9225_write_reg(lcd, 0x0012, 0x1121); //sets refernce voltage for pixel brightness
    ili9225_write_reg(lcd, 0x0013, 0x0063);//controls voltage switching
    ili9225_write_reg(lcd, 0x0014, 0x5A00); //fine tuning(equal volt for each pixel)
    msleep(50);
    ili9225_write_reg(lcd, 0x0007, 0x1017); // display ON
    msleep(50);
}
static void ili9225_fill(struct ili9225 *lcd, u16 color)
{
    int x, y;
	pr_info("FILL COLOR=%04x\n",color);
    ili9225_write_reg(lcd,0x0036,175); //vertical bottom
    ili9225_write_reg(lcd,0x0037,0); //vertical top
    ili9225_write_reg(lcd,0x0038,219);  //horizontal end
    ili9225_write_reg(lcd,0x0039,0); //horizontal start
    ili9225_write_reg(lcd,0x0020,0); //col pointer to zero
    ili9225_write_reg(lcd,0x0021,0); //row pointer to zero

    gpiod_set_value(lcd->dc,0); //command mode
    ili9225_write16(lcd,0x0022);  //GRAM address
    gpiod_set_value(lcd->dc,1); //data mode

    for(y=0;y<220;y++)
        for(x=0;x<176;x++)
            ili9225_write16(lcd,color);
}

static void lcd_show_success(void)
{
    ili9225_fill(g_lcd, 0x07E0); // GREEN
}

static void lcd_show_fail(void)
{
    ili9225_fill(g_lcd, 0xF800); // RED
}

/* ================= TCP SERVER ================= */

//function to receive data from a kernel socket
static int krecv(struct socket *sock, char *buf, size_t len)
{ 	// Message header (empty/default)
	struct msghdr msg = {0};
	struct kvec iov = {.iov_base = buf, .iov_len = len};// Buffer where data will be stored
	return kernel_recvmsg(sock, &msg, &iov, 1, len, 0);// Maximum bytes to read
}
/*============fingerprint handler========*/
static void finger_print_handler(void)
{

	int ret;
	u8 ack[32];
	uart_flush_rx();     //clear old data in UART RX FIFO       
			     //clear pending interrupts
	writel(0x7FF,uart_base+UART_ICR);
	uart_write_data(get_image,sizeof(get_image)); //send capture command
	msleep(300);
	ret = uart_read_with_timeout(ack, sizeof(ack), 1500);//read ack
	pr_info("UART READ %d bytes: ", ret);
	for (int i = 0; i < ret; i++)
		pr_cont("%02X ", ack[i]);
	pr_cont("\n");
	if (ret > 9 && ack[9] == 0x00) {
		pr_info("GET IMAGE SUCCESS\n");
		lcd_show_success(); //green screen
	}
	else{
		pr_info("GET IMAGE FAILED\n");
		lcd_show_fail(); //red screen
	}
	if (first_scan) {

		uart_write_data(fp_add,sizeof(fp_add));//send add command
		first_scan = false; //Next time, search mode

		msleep(300);

		memset(ack,0,sizeof(ack));
		//read registration response
		ret = uart_read_with_timeout(ack,
				sizeof(ack),
				1500);
		pr_info("UART READ %d bytes: ", ret);
		for (int i = 0; i < ret; i++)
			pr_cont("%02X ", ack[i]);
		pr_cont("\n");

		if (ret > 9 && ack[9] == 0x00)
		{
			pr_info("registered\n");
			ili9225_fill(g_lcd, 0xFFFF);//white background
			drawString(20, 80, "REGISTERED", 0x07E0);//green text
		}
		else{
			pr_info("registration failed\n");
			ili9225_fill(g_lcd, 0xFFFF);//white background
			drawString(20, 80, "NOTREGISTERED", 0xF800);//red text
			first_scan=true;
		}

	}
	else {

		uart_write_data(fp_search,sizeof(fp_search));//send search command


		msleep(300);

		memset(ack,0,sizeof(ack));
		//read ack
		ret = uart_read_with_timeout(ack,
				sizeof(ack),
				1500);
		pr_info("UART READ %d bytes: ", ret);
		for (int i = 0; i < ret; i++)
			pr_cont("%02X ", ack[i]);
		pr_cont("\n");

		if (ret > 9 && ack[9] == 0x00)
		{
			pr_info("matched\n");
			
			ili9225_fill(g_lcd, 0xFFFF);//white background
			drawString(20, 80, "MATCHED", 0x07E0);//green text
		}
		else{
			pr_info("no match\n");
			
			ili9225_fill(g_lcd, 0xFFFF);//white background
			drawString(20, 80, "NOT MATCHED", 0xF800);//red text
		}


	}

}


static int server_fn(void *data)
{ 
     	struct sockaddr_in addr;  //socket address structure
	int ret;
	u8 ack[32];

	//create kernel socket
	sock_create_kern(&init_net, AF_INET, SOCK_STREAM,
			IPPROTO_TCP, &listen_sock);
	//configure address
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	//bind socket
	kernel_bind(listen_sock,
			(struct sockaddr *)&addr,
			sizeof(addr));
	//Start Listening
	kernel_listen(listen_sock, 5);

	pr_info("Server listening on %d\n", SERVER_PORT);
	
	//Main Server Loop(loop until module is removed)
	while (!kthread_should_stop())
	{
		struct socket *client_sock = NULL;
		char rx[RX_MAX];

		//acccept client
		ret = kernel_accept(listen_sock, &client_sock, 0);
		if (ret < 0) {
			msleep(100);
			continue;
		}

		memset(rx, 0, sizeof(rx));
		//recieve data from client
		ret = krecv(client_sock, rx, sizeof(rx)-1);

		if (ret > 0) {
			rx[ret] = '\0';

			if(strcmp(rx,"PLACE_FINGER")==0)
			{
				play_audio(); //playing welcome audio

				finger_print_handler(); 
			}
			else if((strcmp(rx,"NEW")==0)&&(!first_scan))
			{
				

				uart_write_data(fp_second_person,sizeof(fp_second_person));//send search command

				msleep(300);

				memset(ack,0,sizeof(ack));
				//read ack
				ret = uart_read_with_timeout(ack,
						sizeof(ack),
						1500);
				pr_info("UART READ %d bytes: ", ret);
				for (int i = 0; i < ret; i++)
					pr_cont("%02X ", ack[i]);
				pr_cont("\n");
				if (ret > 9 && ack[9] == 0x00)
				{
					pr_info("matched\n");
					
					ili9225_fill(g_lcd, 0xFFFF);//white background
					drawString(20, 80, "MATCHED", 0x07E0);//green text
				}
				else{
					pr_info("no match\n");
					ili9225_fill(g_lcd, 0xFFFF);//white background
					drawString(20, 80, "NOT MATCHED", 0xF800);//red text
					}
			}
			else
			{
				  ili9225_fill(g_lcd, 0xFFFF);//white background

                 drawString(20, 80, "INVALID", 0xF800);//red

			}
			sock_release(client_sock);
		}
	}

	return 0;
}


/* ================= SPI PROBE ================= */

static int ili9225_probe(struct spi_device *spi)
{
	pr_info("ili9225 probe called\n");
	struct ili9225 *lcd;

	uart_base = ioremap(UART_BASE, UART_SIZE); //map uart registers
	if (!uart_base)
		return -ENOMEM;

	uart_hw_init(); //uart initialization

	lcd = devm_kzalloc(&spi->dev,sizeof(*lcd),GFP_KERNEL);//allocate memory for struct ili9225 and automatically freed when driver is removed
	if(!lcd)
	{
		pr_info("memory not allocated\n");
		return -ENOMEM;
	}

	lcd->spi = spi; //store the SPI device pointer to our structure (in driver)

	spi_set_drvdata(spi,lcd);//store lcd structure inside SPI device (in kernel)

	lcd->dc = devm_gpiod_get(&spi->dev,"dc",GPIOD_OUT_LOW);// get GPIO pin from DT and set to low

	if (IS_ERR(lcd->dc))
		pr_err("DC GPIO failed\n");
	//get GPIO pin from Device Tree and set to high
	lcd->reset = devm_gpiod_get(&spi->dev,"reset",GPIOD_OUT_HIGH);

	if (IS_ERR(lcd->reset))
		pr_err("RESET GPIO failed\n");
	//configure SPI
	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	spi->max_speed_hz=4000000; //4MHz
	if(spi_setup(spi))
	{
		pr_err("SPI setup failed\n");
	}

	ili9225_init(lcd); // initailizing display
	msleep(50);
	ili9225_fill(lcd,0xFFFF); //filling screen with white
	g_lcd=lcd;//save pointer to globally

	//start kernel server thread
	server_thread = kthread_run(server_fn,NULL,"gc07_server");
	if (IS_ERR(server_thread))
		return PTR_ERR(server_thread);
	

	pr_info("FINAL GC07 + LCD + PL011 READY\n");
	return 0;
}

static void ili9225_remove(struct spi_device *spi)
{
	if(server_thread)
		kthread_stop(server_thread); //stop kernel thread

	if(listen_sock)
		sock_release(listen_sock); //release socket

	if(uart_base)
		iounmap(uart_base); //unmap memory mapped timer registers

	pr_info("Driver removed\n");
}
static const struct of_device_id ili9225_of_match[] = {
	{ .compatible = "project,spi" },// device tree compatible string to match driver  
	{ }
};
MODULE_DEVICE_TABLE(of, ili9225_of_match);

//for spi
static const struct spi_device_id ili9225_id[] = {
	{ "ili9225_driver", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, ili9225_id); // auto module loading using device tree match

//SPI driver structure
static struct spi_driver ili9225_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = ili9225_of_match,

	},
	.probe = ili9225_probe,
	.remove = ili9225_remove,
	.id_table=ili9225_id,

};

module_spi_driver(ili9225_driver);//register SPI driver

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fully Integrated Version");
MODULE_DESCRIPTION("TCP + GC07 + ILI9225 LCD + PL011 UART (MMIO)");

