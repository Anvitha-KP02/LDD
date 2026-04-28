#ifndef STM32F401_UART_H
#define STM32F401_UART_H

#include "stm32f401_platform.h"

typedef enum
{
    STM32_UART_WORDLEN_8B = 0U,
    STM32_UART_WORDLEN_9B = 1U
} stm32_uart_wordlen_t;

typedef enum
{
    STM32_UART_PARITY_NONE = 0U,
    STM32_UART_PARITY_EVEN = 1U,
    STM32_UART_PARITY_ODD = 2U
} stm32_uart_parity_t;

typedef enum
{
    STM32_UART_STOPBITS_1 = 0U,
    STM32_UART_STOPBITS_2 = 2U
} stm32_uart_stopbits_t;

typedef struct
{
    stm32_usart_reg_t *instance;
    uint32_t pclk_hz;
    uint32_t baudrate;
    stm32_uart_wordlen_t word_length;
    stm32_uart_parity_t parity;
    stm32_uart_stopbits_t stop_bits;
    bool enable_tx;
    bool enable_rx;
} stm32_uart_cfg_t;

stm32_status_t stm32_uart_enable_clock(stm32_usart_reg_t *uart);
stm32_status_t stm32_uart_init(const stm32_uart_cfg_t *cfg);
stm32_status_t stm32_uart_write_byte(stm32_usart_reg_t *uart, uint8_t data, uint32_t timeout_cycles);
stm32_status_t stm32_uart_read_byte(stm32_usart_reg_t *uart, uint8_t *data, uint32_t timeout_cycles);
stm32_status_t stm32_uart_write(stm32_usart_reg_t *uart, const uint8_t *data, size_t len, uint32_t timeout_cycles);
stm32_status_t stm32_uart_read(stm32_usart_reg_t *uart, uint8_t *data, size_t len, uint32_t timeout_cycles);
stm32_status_t stm32_uart_write_cstr(stm32_usart_reg_t *uart, const char *str, uint32_t timeout_cycles);

#endif /* STM32F401_UART_H */
