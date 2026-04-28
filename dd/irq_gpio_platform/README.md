# IRQ GPIO Platform Driver

This folder contains a Linux kernel module implementing a simple, DT-based,
interrupt-driven GPIO input driver using the GPIO descriptor API.

## Device Tree snippet

```dts
/ {
	irq_gpio_demo: irq-gpio-demo {
		compatible = "demo,irq-gpio-platform";

		/* Passed to devm_gpiod_get(dev, "irq", GPIOD_IN) */
		irq-gpios = <&gpio1 12 GPIO_ACTIVE_HIGH>;

		/* Optional: debounce in microseconds */
		debounce-us = <10000>;

		status = "okay";
	};
};
```

## Build and run

```bash
cd /home/mirafra/Desktop/batch-5/dd/irq_gpio_platform
make
sudo insmod irq_gpio_platform.ko
dmesg | tail -n 30

# Generate a rising edge on the configured pin externally.
# Then verify interrupt logs:
dmesg | tail -n 50

sudo rmmod irq_gpio_platform
dmesg | tail -n 30
```

## Notes

- The GPIO is acquired from DT using the `<function>-gpios` convention:
  `irq-gpios` maps to function name `"irq"` in `devm_gpiod_get()`.
- `devm_request_irq()` and other devm-managed resources handle cleanup
  automatically on remove/error paths.
