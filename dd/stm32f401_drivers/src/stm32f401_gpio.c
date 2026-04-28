#include "stm32f401_gpio.h"

static bool is_valid_port(const stm32_gpio_reg_t *port)
{
    return (port == STM32_GPIOA) || (port == STM32_GPIOB) || (port == STM32_GPIOC) ||
           (port == STM32_GPIOD) || (port == STM32_GPIOE) || (port == STM32_GPIOH);
}

static uint32_t port_to_rcc_bit(const stm32_gpio_reg_t *port)
{
    if (port == STM32_GPIOA) { return 0UL; }
    if (port == STM32_GPIOB) { return 1UL; }
    if (port == STM32_GPIOC) { return 2UL; }
    if (port == STM32_GPIOD) { return 3UL; }
    if (port == STM32_GPIOE) { return 4UL; }
    if (port == STM32_GPIOH) { return 7UL; }
    return 0xFFFFFFFFUL;
}

stm32_status_t stm32_gpio_enable_clock(stm32_gpio_reg_t *port)
{
    const uint32_t bit = port_to_rcc_bit(port);
    if ((bit == 0xFFFFFFFFUL) || (!is_valid_port(port)))
    {
        return STM32_ERR_INVALID_ARG;
    }

    STM32_RCC->AHB1ENR |= (1UL << bit);
    (void)STM32_RCC->AHB1ENR; /* Read-back ensures write completion before next access. */
    return STM32_OK;
}

stm32_status_t stm32_gpio_init(const stm32_gpio_cfg_t *cfg)
{
    uint8_t pin = 0U;

    if ((cfg == NULL) || (!is_valid_port(cfg->port)) || (cfg->pin_mask == 0U))
    {
        return STM32_ERR_INVALID_ARG;
    }

    if ((cfg->mode > STM32_GPIO_MODE_ANALOG) ||
        (cfg->output_type > STM32_GPIO_OTYPE_OD) ||
        (cfg->speed > STM32_GPIO_SPEED_HIGH) ||
        (cfg->pull > STM32_GPIO_PULLDOWN) ||
        (cfg->alternate > 15U))
    {
        return STM32_ERR_INVALID_ARG;
    }

    (void)stm32_gpio_enable_clock(cfg->port);

    for (pin = 0U; pin < 16U; ++pin)
    {
        const uint16_t bit = (uint16_t)(1U << pin);
        if ((cfg->pin_mask & bit) == 0U)
        {
            continue;
        }

        const uint32_t shift2 = ((uint32_t)pin * 2UL);
        const uint32_t shift1 = (uint32_t)pin;

        cfg->port->MODER = (cfg->port->MODER & ~(0x3UL << shift2)) |
                           ((uint32_t)cfg->mode << shift2);
        cfg->port->OTYPER = (cfg->port->OTYPER & ~(0x1UL << shift1)) |
                            ((uint32_t)cfg->output_type << shift1);
        cfg->port->OSPEEDR = (cfg->port->OSPEEDR & ~(0x3UL << shift2)) |
                             ((uint32_t)cfg->speed << shift2);
        cfg->port->PUPDR = (cfg->port->PUPDR & ~(0x3UL << shift2)) |
                           ((uint32_t)cfg->pull << shift2);

        if (cfg->mode == STM32_GPIO_MODE_AF)
        {
            const uint32_t afr_idx = (pin < 8U) ? 0UL : 1UL;
            const uint32_t afr_shift = ((uint32_t)(pin & 0x7U) * 4UL);
            cfg->port->AFR[afr_idx] = (cfg->port->AFR[afr_idx] & ~(0xFUL << afr_shift)) |
                                      ((uint32_t)cfg->alternate << afr_shift);
        }
    }

    return STM32_OK;
}

stm32_status_t stm32_gpio_write_pin(stm32_gpio_reg_t *port, uint8_t pin, bool high)
{
    if ((!is_valid_port(port)) || (pin > 15U))
    {
        return STM32_ERR_INVALID_ARG;
    }

    if (high)
    {
        port->BSRR = (1UL << pin);
    }
    else
    {
        port->BSRR = (1UL << ((uint32_t)pin + 16UL));
    }

    return STM32_OK;
}

stm32_status_t stm32_gpio_toggle_pin(stm32_gpio_reg_t *port, uint8_t pin)
{
    if ((!is_valid_port(port)) || (pin > 15U))
    {
        return STM32_ERR_INVALID_ARG;
    }

    port->ODR ^= (1UL << pin);
    return STM32_OK;
}

stm32_status_t stm32_gpio_read_pin(const stm32_gpio_reg_t *port, uint8_t pin, bool *high)
{
    if ((!is_valid_port(port)) || (pin > 15U) || (high == NULL))
    {
        return STM32_ERR_INVALID_ARG;
    }

    *high = ((port->IDR & (1UL << pin)) != 0UL);
    return STM32_OK;
}

stm32_status_t stm32_gpio_write_mask(stm32_gpio_reg_t *port, uint16_t set_mask, uint16_t reset_mask)
{
    if (!is_valid_port(port))
    {
        return STM32_ERR_INVALID_ARG;
    }

    port->BSRR = ((uint32_t)reset_mask << 16UL) | (uint32_t)set_mask;
    return STM32_OK;
}
