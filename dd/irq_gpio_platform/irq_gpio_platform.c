// SPDX-License-Identifier: GPL-2.0
/*
 * irq_gpio_platform.c - Platform GPIO interrupt driver (DT based)
 *
 * A simple production-style example that:
 * - Gets an input GPIO via GPIO descriptor API from device tree
 * - Uses DT "interrupts" (preferred) with GPIO-to-IRQ fallback
 * - Requests rising-edge interrupt
 * - Logs an interrupt message
 * - Uses devm-managed resources for robust cleanup
 */

#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct irq_gpio_drvdata {
	struct gpio_desc *in_gpiod;
	int irq;
	u32 debounce_us;
};

static irqreturn_t irq_gpio_irq_handler(int irq, void *dev_id)
{
	struct device *dev = dev_id;

	dev_info_ratelimited(dev, "Rising edge interrupt received on IRQ %d\n",
			     irq);

	return IRQ_HANDLED;
}

static int irq_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct irq_gpio_drvdata *drvdata;
	int ret;

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	/*
	 * The input GPIO is read from DT property:
	 *     gpios = <&gpio N GPIO_ACTIVE_HIGH>;
	 * by requesting descriptor with con_id = NULL.
	 */
	drvdata->in_gpiod = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(drvdata->in_gpiod))
		return dev_err_probe(dev, PTR_ERR(drvdata->in_gpiod),
				     "Failed to acquire input GPIO\n");

	/*
	 * Optional debounce from DT:
	 *     debounce-us = <10000>;
	 *
	 * If controller does not support debounce, continue gracefully.
	 */
	ret = device_property_read_u32(dev, "debounce-us", &drvdata->debounce_us);
	if (!ret && drvdata->debounce_us) {
		ret = gpiod_set_debounce(drvdata->in_gpiod, drvdata->debounce_us);
		if (ret)
			dev_warn(dev,
				 "Debounce (%u us) not applied: %d\n",
				 drvdata->debounce_us, ret);
	}

	/*
	 * Prefer explicit DT interrupt mapping:
	 *     interrupt-parent = <&gpio>;
	 *     interrupts = <GPIO_NUM IRQ_TYPE_EDGE_RISING>;
	 */
	drvdata->irq = platform_get_irq(pdev, 0);
	if (drvdata->irq < 0) {
		/* Fallback for platforms that only provide a GPIO specifier */
		drvdata->irq = gpiod_to_irq(drvdata->in_gpiod);
		if (drvdata->irq < 0)
			return dev_err_probe(dev, drvdata->irq,
					     "Failed to get IRQ\n");
	}

	platform_set_drvdata(pdev, drvdata);

	ret = devm_request_irq(dev, drvdata->irq, irq_gpio_irq_handler,
			       IRQF_TRIGGER_RISING, dev_name(dev), dev);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to request IRQ\n");

	dev_info(dev, "Driver probed: GPIO IRQ=%d trigger=rising%s\n",
		 drvdata->irq,
		 drvdata->debounce_us ? " debounce=enabled" : "");

	return 0;
}

static void irq_gpio_remove(struct platform_device *pdev)
{
	struct irq_gpio_drvdata *drvdata = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Driver removed (IRQ %d)\n", drvdata->irq);
}

static const struct of_device_id irq_gpio_of_match[] = {
	{ .compatible = "demo,irq-gpio-platform" },
	{ }
};
MODULE_DEVICE_TABLE(of, irq_gpio_of_match);

static struct platform_driver irq_gpio_driver = {
	.probe = irq_gpio_probe,
	.remove_new = irq_gpio_remove,
	.driver = {
		.name = "irq_gpio_platform",
		.of_match_table = irq_gpio_of_match,
	},
};
module_platform_driver(irq_gpio_driver);

MODULE_AUTHOR("Example Author");
MODULE_DESCRIPTION("DT-based GPIO rising-edge interrupt platform driver");
MODULE_LICENSE("GPL");
