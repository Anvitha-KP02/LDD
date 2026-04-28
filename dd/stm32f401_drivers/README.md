# STM32F401 GPIO + UART Drivers (Register-Level)

This folder contains production-style embedded C drivers for STM32F401:
- `src/stm32f401_gpio.c`: GPIO driver
- `src/stm32f401_uart.c`: UART driver
- `inc/*.h`: public APIs and platform register map
- `tests/test_gpio.c`: GPIO hardware test app
- `tests/test_uart.c`: UART echo/loopback test app

## Design notes
- No HAL dependency (direct register access).
- Type-safe enums and explicit fixed-width integer types.
- API-level argument validation and timeout handling.
- Read-back after clock enable writes to avoid ordering hazards.
- Comments included for non-obvious hardware behavior.

## Build integration
Add these files to your STM32 project build:
- Include path: `stm32f401_drivers/inc`
- Sources:
  - `stm32f401_drivers/src/stm32f401_gpio.c`
  - `stm32f401_drivers/src/stm32f401_uart.c`
  - one test entrypoint (`tests/test_gpio.c` or `tests/test_uart.c`) as `main()`

## Hardware test checklist
1. Confirm system/APB clocks, then set `pclk_hz` in `test_uart.c`.
2. Configure board pinout:
   - GPIO test: LED pin and button pin in `test_gpio.c`.
   - UART test: AF pins for selected USART instance.
3. For loopback test, wire UART TX to RX.
4. Flash and observe:
   - GPIO test: LED behavior based on input.
   - UART test: banner then echoed bytes.
