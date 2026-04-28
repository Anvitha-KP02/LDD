#include "stm32f401_gpio.h"

/*
 * GPIO functional test for STM32F401.
 * Example board mapping:
 *   - PA5  : LED output (Nucleo onboard LED on many variants)
 *   - PC13 : Button input (if available)
 */

static void busy_delay(volatile uint32_t cycles)
{
    while (cycles > 0U)
    {
        --cycles;
    }
}

int main(void)
{
    stm32_gpio_cfg_t led_cfg;
    stm32_gpio_cfg_t btn_cfg;
    bool button_high = false;
    stm32_status_t rc;

    led_cfg.port = STM32_GPIOA;
    led_cfg.pin_mask = (uint16_t)(1U << 5);
    led_cfg.mode = STM32_GPIO_MODE_OUTPUT;
    led_cfg.output_type = STM32_GPIO_OTYPE_PP;
    led_cfg.speed = STM32_GPIO_SPEED_LOW;
    led_cfg.pull = STM32_GPIO_NOPULL;
    led_cfg.alternate = 0U;

    btn_cfg.port = STM32_GPIOC;
    btn_cfg.pin_mask = (uint16_t)(1U << 13);
    btn_cfg.mode = STM32_GPIO_MODE_INPUT;
    btn_cfg.output_type = STM32_GPIO_OTYPE_PP;
    btn_cfg.speed = STM32_GPIO_SPEED_LOW;
    btn_cfg.pull = STM32_GPIO_PULLUP;
    btn_cfg.alternate = 0U;

    rc = stm32_gpio_init(&led_cfg);
    if (rc != STM32_OK)
    {
        return -1;
    }

    rc = stm32_gpio_init(&btn_cfg);
    if (rc != STM32_OK)
    {
        return -2;
    }

    for (;;)
    {
        rc = stm32_gpio_read_pin(STM32_GPIOC, 13U, &button_high);
        if (rc != STM32_OK)
        {
            continue;
        }

        /* Button is often active-low on STM32 boards. */
        if (!button_high)
        {
            (void)stm32_gpio_write_pin(STM32_GPIOA, 5U, true);
        }
        else
        {
            (void)stm32_gpio_toggle_pin(STM32_GPIOA, 5U);
            busy_delay(200000UL);
        }
    }
}
