#include "stm32f401_uart.h"

#define USART_SR_PE                (1UL << 0)
#define USART_SR_FE                (1UL << 1)
#define USART_SR_NE                (1UL << 2)
#define USART_SR_ORE               (1UL << 3)
#define USART_SR_RXNE              (1UL << 5)
#define USART_SR_TC                (1UL << 6)
#define USART_SR_TXE               (1UL << 7)

#define USART_CR1_SBK              (1UL << 0)
#define USART_CR1_RE               (1UL << 2)
#define USART_CR1_TE               (1UL << 3)
#define USART_CR1_PCE              (1UL << 10)
#define USART_CR1_PS               (1UL << 9)
#define USART_CR1_M                (1UL << 12)
#define USART_CR1_UE               (1UL << 13)

#define USART_CR2_STOP_POS         (12UL)
#define USART_CR2_STOP_MASK        (0x3UL << USART_CR2_STOP_POS)

static bool is_valid_uart(const stm32_usart_reg_t *uart)
{
    return (uart == STM32_USART1) || (uart == STM32_USART2) || (uart == STM32_USART6);
}

static void clear_error_flags(stm32_usart_reg_t *uart)
{
    volatile uint32_t tmp = uart->SR;
    tmp = uart->DR;
    (void)tmp;
}

static uint32_t uart_to_rcc_bit(const stm32_usart_reg_t *uart)
{
    if (uart == STM32_USART1) { return 4UL; }  /* APB2ENR USART1EN */
    if (uart == STM32_USART2) { return 17UL; } /* APB1ENR USART2EN */
    if (uart == STM32_USART6) { return 5UL; }  /* APB2ENR USART6EN */
    return 0xFFFFFFFFUL;
}

stm32_status_t stm32_uart_enable_clock(stm32_usart_reg_t *uart)
{
    const uint32_t bit = uart_to_rcc_bit(uart);
    if ((bit == 0xFFFFFFFFUL) || (!is_valid_uart(uart)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    if (uart == STM32_USART2)
    {
        STM32_RCC->APB1ENR |= (1UL << bit);
        (void)STM32_RCC->APB1ENR;
    }
    else
    {
        STM32_RCC->APB2ENR |= (1UL << bit);
        (void)STM32_RCC->APB2ENR;
    }

    return STM32_OK;
}

stm32_status_t stm32_uart_init(const stm32_uart_cfg_t *cfg)
{
    uint32_t brr = 0UL;
    uint32_t cr1 = 0UL;

    if ((cfg == NULL) || (!is_valid_uart(cfg->instance)) ||
        (cfg->baudrate == 0UL) || (cfg->pclk_hz == 0UL) ||
        ((!cfg->enable_tx) && (!cfg->enable_rx)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    if ((cfg->word_length > STM32_UART_WORDLEN_9B) ||
        (cfg->parity > STM32_UART_PARITY_ODD) ||
        !((cfg->stop_bits == STM32_UART_STOPBITS_1) || (cfg->stop_bits == STM32_UART_STOPBITS_2)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    (void)stm32_uart_enable_clock(cfg->instance);

    cfg->instance->CR1 &= ~USART_CR1_UE;

    /* Integer BRR with rounding, OVER8=0. */
    brr = (cfg->pclk_hz + (cfg->baudrate / 2UL)) / cfg->baudrate;
    if (brr == 0UL || brr > 0xFFFFUL)
    {
        return STM32_ERR_INVALID_ARG;
    }
    cfg->instance->BRR = brr;

    cfg->instance->CR2 = (cfg->instance->CR2 & ~USART_CR2_STOP_MASK) |
                         ((uint32_t)cfg->stop_bits << USART_CR2_STOP_POS);

    if (cfg->enable_tx) { cr1 |= USART_CR1_TE; }
    if (cfg->enable_rx) { cr1 |= USART_CR1_RE; }
    if (cfg->word_length == STM32_UART_WORDLEN_9B) { cr1 |= USART_CR1_M; }

    if (cfg->parity != STM32_UART_PARITY_NONE)
    {
        cr1 |= USART_CR1_PCE;
        if (cfg->parity == STM32_UART_PARITY_ODD)
        {
            cr1 |= USART_CR1_PS;
        }
    }

    cfg->instance->CR1 = cr1 | USART_CR1_UE;
    clear_error_flags(cfg->instance);
    return STM32_OK;
}

stm32_status_t stm32_uart_write_byte(stm32_usart_reg_t *uart, uint8_t data, uint32_t timeout_cycles)
{
    if (!is_valid_uart(uart))
    {
        return STM32_ERR_INVALID_ARG;
    }

    while ((uart->SR & USART_SR_TXE) == 0UL)
    {
        if (timeout_cycles == 0UL)
        {
            return STM32_ERR_TIMEOUT;
        }
        --timeout_cycles;
    }

    uart->DR = (uint32_t)data;

    timeout_cycles = (timeout_cycles == 0UL) ? 1UL : timeout_cycles;
    while ((uart->SR & USART_SR_TC) == 0UL)
    {
        if (timeout_cycles == 0UL)
        {
            return STM32_ERR_TIMEOUT;
        }
        --timeout_cycles;
    }

    return STM32_OK;
}

stm32_status_t stm32_uart_read_byte(stm32_usart_reg_t *uart, uint8_t *data, uint32_t timeout_cycles)
{
    if ((!is_valid_uart(uart)) || (data == NULL))
    {
        return STM32_ERR_INVALID_ARG;
    }

    while ((uart->SR & USART_SR_RXNE) == 0UL)
    {
        if ((uart->SR & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0UL)
        {
            clear_error_flags(uart);
            return STM32_ERR_UNSUPPORTED;
        }

        if (timeout_cycles == 0UL)
        {
            return STM32_ERR_TIMEOUT;
        }
        --timeout_cycles;
    }

    *data = (uint8_t)(uart->DR & 0xFFUL);
    return STM32_OK;
}

stm32_status_t stm32_uart_write(stm32_usart_reg_t *uart, const uint8_t *data, size_t len, uint32_t timeout_cycles)
{
    size_t i = 0U;
    if ((!is_valid_uart(uart)) || ((data == NULL) && (len > 0U)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    for (i = 0U; i < len; ++i)
    {
        stm32_status_t rc = stm32_uart_write_byte(uart, data[i], timeout_cycles);
        if (rc != STM32_OK)
        {
            return rc;
        }
    }

    return STM32_OK;
}

stm32_status_t stm32_uart_read(stm32_usart_reg_t *uart, uint8_t *data, size_t len, uint32_t timeout_cycles)
{
    size_t i = 0U;
    if ((!is_valid_uart(uart)) || ((data == NULL) && (len > 0U)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    for (i = 0U; i < len; ++i)
    {
        stm32_status_t rc = stm32_uart_read_byte(uart, &data[i], timeout_cycles);
        if (rc != STM32_OK)
        {
            return rc;
        }
    }

    return STM32_OK;
}

stm32_status_t stm32_uart_write_cstr(stm32_usart_reg_t *uart, const char *str, uint32_t timeout_cycles)
{
    if ((!is_valid_uart(uart)) || (str == NULL))
    {
        return STM32_ERR_INVALID_ARG;
    }

    while (*str != '\0')
    {
        const stm32_status_t rc = stm32_uart_write_byte(uart, (uint8_t)(*str), timeout_cycles);
        if (rc != STM32_OK)
        {
            return rc;
        }
        ++str;
    }

    return STM32_OK;
}
