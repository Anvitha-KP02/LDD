#include "stm32f401_gpio.h"
#include "stm32f401_uart.h"

/*
 * UART functional test for STM32F401.
 * Pinout for USART2 (common on Nucleo):
 *   PA2 -> USART2_TX (AF7)
 *   PA3 -> USART2_RX (AF7)
 * Wire loopback externally: PA2 <-> PA3 to self-test RX path.
 */

#define UART_TIMEOUT_CYCLES   (2000000UL)

static stm32_status_t uart2_gpio_init(void)
{
    stm32_gpio_cfg_t cfg;
    cfg.port = STM32_GPIOA;
    cfg.pin_mask = (uint16_t)((1U << 2) | (1U << 3));
    cfg.mode = STM32_GPIO_MODE_AF;
    cfg.output_type = STM32_GPIO_OTYPE_PP;
    cfg.speed = STM32_GPIO_SPEED_HIGH;
    cfg.pull = STM32_GPIO_PULLUP;
    cfg.alternate = 7U;
    return stm32_gpio_init(&cfg);
}

int main(void)
{
    stm32_uart_cfg_t uart_cfg;
    stm32_status_t rc;
    uint8_t rx = 0U;
    static const char banner[] = "STM32F401 UART test ready\r\n";

    rc = uart2_gpio_init();
    if (rc != STM32_OK)
    {
        return -1;
    }

    uart_cfg.instance = STM32_USART2;
    uart_cfg.pclk_hz = 16000000UL; /* APB1 clock; adjust to your clock tree. */
    uart_cfg.baudrate = 115200UL;
    uart_cfg.word_length = STM32_UART_WORDLEN_8B;
    uart_cfg.parity = STM32_UART_PARITY_NONE;
    uart_cfg.stop_bits = STM32_UART_STOPBITS_1;
    uart_cfg.enable_tx = true;
    uart_cfg.enable_rx = true;

    rc = stm32_uart_init(&uart_cfg);
    if (rc != STM32_OK)
    {
        return -2;
    }

    rc = stm32_uart_write(STM32_USART2, (const uint8_t *)banner, sizeof(banner) - 1U, UART_TIMEOUT_CYCLES);
    if (rc != STM32_OK)
    {
        return -3;
    }

    for (;;)
    {
        rc = stm32_uart_read_byte(STM32_USART2, &rx, UART_TIMEOUT_CYCLES);
        if (rc == STM32_OK)
        {
            /* Echo each byte back to terminal/loopback wire. */
            (void)stm32_uart_write_byte(STM32_USART2, rx, UART_TIMEOUT_CYCLES);
        }
    }
}
