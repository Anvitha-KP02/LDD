#ifndef STM32F401_GPIO_H
#define STM32F401_GPIO_H

#include "stm32f401_platform.h"

typedef enum
{
    STM32_GPIO_MODE_INPUT = 0U,
    STM32_GPIO_MODE_OUTPUT = 1U,
    STM32_GPIO_MODE_AF = 2U,
    STM32_GPIO_MODE_ANALOG = 3U
} stm32_gpio_mode_t;

typedef enum
{
    STM32_GPIO_OTYPE_PP = 0U,
    STM32_GPIO_OTYPE_OD = 1U
} stm32_gpio_otype_t;

typedef enum
{
    STM32_GPIO_SPEED_LOW = 0U,
    STM32_GPIO_SPEED_MEDIUM = 1U,
    STM32_GPIO_SPEED_FAST = 2U,
    STM32_GPIO_SPEED_HIGH = 3U
} stm32_gpio_speed_t;

typedef enum
{
    STM32_GPIO_NOPULL = 0U,
    STM32_GPIO_PULLUP = 1U,
    STM32_GPIO_PULLDOWN = 2U
} stm32_gpio_pull_t;

typedef struct
{
    stm32_gpio_reg_t *port;
    uint16_t pin_mask; /* bit0..bit15 */
    stm32_gpio_mode_t mode;
    stm32_gpio_otype_t output_type;
    stm32_gpio_speed_t speed;
    stm32_gpio_pull_t pull;
    uint8_t alternate; /* 0..15 */
} stm32_gpio_cfg_t;

stm32_status_t stm32_gpio_enable_clock(stm32_gpio_reg_t *port);
stm32_status_t stm32_gpio_init(const stm32_gpio_cfg_t *cfg);
stm32_status_t stm32_gpio_write_pin(stm32_gpio_reg_t *port, uint8_t pin, bool high);
stm32_status_t stm32_gpio_toggle_pin(stm32_gpio_reg_t *port, uint8_t pin);
stm32_status_t stm32_gpio_read_pin(const stm32_gpio_reg_t *port, uint8_t pin, bool *high);
stm32_status_t stm32_gpio_write_mask(stm32_gpio_reg_t *port, uint16_t set_mask, uint16_t reset_mask);

#endif /* STM32F401_GPIO_H */
