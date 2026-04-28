#ifndef STM32F401_PLATFORM_H
#define STM32F401_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Minimal STM32F401 register map subset for GPIO/UART.
 * Values are from STM32F401 reference manual.
 */

#define STM32_PERIPH_BASE             (0x40000000UL)
#define STM32_AHB1PERIPH_BASE         (STM32_PERIPH_BASE + 0x00020000UL)
#define STM32_APB1PERIPH_BASE         (STM32_PERIPH_BASE + 0x00000000UL)
#define STM32_APB2PERIPH_BASE         (STM32_PERIPH_BASE + 0x00010000UL)

#define STM32_GPIOA_BASE              (STM32_AHB1PERIPH_BASE + 0x0000UL)
#define STM32_GPIOB_BASE              (STM32_AHB1PERIPH_BASE + 0x0400UL)
#define STM32_GPIOC_BASE              (STM32_AHB1PERIPH_BASE + 0x0800UL)
#define STM32_GPIOD_BASE              (STM32_AHB1PERIPH_BASE + 0x0C00UL)
#define STM32_GPIOE_BASE              (STM32_AHB1PERIPH_BASE + 0x1000UL)
#define STM32_GPIOH_BASE              (STM32_AHB1PERIPH_BASE + 0x1C00UL)

#define STM32_RCC_BASE                (STM32_AHB1PERIPH_BASE + 0x3800UL)
#define STM32_USART1_BASE             (STM32_APB2PERIPH_BASE + 0x1000UL)
#define STM32_USART2_BASE             (STM32_APB1PERIPH_BASE + 0x4400UL)
#define STM32_USART6_BASE             (STM32_APB2PERIPH_BASE + 0x1400UL)

typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} stm32_gpio_reg_t;

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t RESERVED0[2];
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t RESERVED4[2];
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    volatile uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t RESERVED6[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
    volatile uint32_t DCKCFGR;
} stm32_rcc_reg_t;

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} stm32_usart_reg_t;

#define STM32_GPIOA                   ((stm32_gpio_reg_t *)STM32_GPIOA_BASE)
#define STM32_GPIOB                   ((stm32_gpio_reg_t *)STM32_GPIOB_BASE)
#define STM32_GPIOC                   ((stm32_gpio_reg_t *)STM32_GPIOC_BASE)
#define STM32_GPIOD                   ((stm32_gpio_reg_t *)STM32_GPIOD_BASE)
#define STM32_GPIOE                   ((stm32_gpio_reg_t *)STM32_GPIOE_BASE)
#define STM32_GPIOH                   ((stm32_gpio_reg_t *)STM32_GPIOH_BASE)

#define STM32_RCC                     ((stm32_rcc_reg_t *)STM32_RCC_BASE)
#define STM32_USART1                  ((stm32_usart_reg_t *)STM32_USART1_BASE)
#define STM32_USART2                  ((stm32_usart_reg_t *)STM32_USART2_BASE)
#define STM32_USART6                  ((stm32_usart_reg_t *)STM32_USART6_BASE)

typedef enum
{
    STM32_OK = 0,
    STM32_ERR_INVALID_ARG = -1,
    STM32_ERR_TIMEOUT = -2,
    STM32_ERR_UNSUPPORTED = -3
} stm32_status_t;

#endif /* STM32F401_PLATFORM_H */
