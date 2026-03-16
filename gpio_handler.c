#include <linux/kernel.h>
#include <linux/io.h>
#include "gpio_handler.h"
#include <linux/module.h>

void __iomem *gpio_base;


/* Helper function to keep one global gpio_base address */
int gpio_hw_init(void)
{
	if (gpio_base)
		return 0; // already mapped

	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_hw_init: Failed to map GPIO\n");
		return -ENOMEM;
	}

	return 0;
}

/* Unmap the gpio */
void gpio_hw_exit(void)
{
	if (gpio_base) {
		iounmap(gpio_base);
		gpio_base = NULL;
	}
}

// gpio_set_mode - configure a pin as input or output
// 000 = input, 001 = output
void gpio_set_mode(unsigned int pin, unsigned int mode)
{
	unsigned int offset = GPFSEL0 + (pin / 10) * 4; /* byte offset of the right GPFSEL */
	unsigned int shift  = (pin % 10) * 3;           /* bit position within that register */
	u32 val;
 
	val  = ioread32(gpio_base + offset);
	val &= ~(7u << shift);           /* clear the 3 bits for this pin */
	val |=  (mode & 7u) << shift;   /* write the new mode */
	iowrite32(val, gpio_base + offset);
}

/* Turns led on, write GPIO_LED_PIN high */
void led_on(void)
{
	iowrite32(1u << GPIO_LED_PIN, gpio_base + GPSET0);
}

/* turns led off, write GPIO_LED_PIN low */
void led_off(void)
{
	iowrite32(1u << GPIO_LED_PIN, gpio_base + GPCLR0);
}

/* Export symbols to other modules */
EXPORT_SYMBOL(gpio_base);
EXPORT_SYMBOL(gpio_set_mode);
EXPORT_SYMBOL(gpio_hw_init);
EXPORT_SYMBOL(gpio_hw_exit);
EXPORT_SYMBOL(led_on);
EXPORT_SYMBOL(led_off);

MODULE_AUTHOR( "...Mar-Joh, Jen-Wil" );
MODULE_LICENSE( "GPL" );