#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

#include <linux/io.h>

/* GPIO config for Pi 2 / Pi 3 Zero */
#define GPIO_BASE 0x3F200000
#define GPIO_SIZE 0x1000

/* GPIO register offsets (byte offsets) */
#define GPFSEL0 0x00 /* Function select pins 0-9  */
#define GPFSEL1 0x04 /* Function select pins 10-19 */
#define GPFSEL2 0x08 /* Function select pins 20-29 */
#define GPFSEL3 0x0C /* Function select pins 30-39 */
#define GPFSEL4 0x10 /* Function select pins 40-49 */
#define GPFSEL5 0x14 /* Function select pins 50-53 */

#define GPSET0 0x1C /* Set pins high  (pins 0-31) */
#define GPSET1 0x20 /* Set pins high  (pins 32-53) */
#define GPCLR0 0x28 /* Set pins low   (pins 0-31) */
#define GPCLR1 0x2C /* Set pins low   (pins 32-53) */

#define GPLEV0 0x34 /* Read pin level (pins 0-31) */
#define GPLEV1 0x38 /* Read pin level (pins 32-53) */

/* Pin modes */
#define GPIO_INPUT  0
#define GPIO_OUTPUT 1

#define GPIO_LED_PIN 29

extern void __iomem *gpio_base;

/* Mapping and unmapping of memory, lifecycle */
int  gpio_hw_init(void);
void gpio_hw_exit(void);

/* Pin configuration and LED control */
void gpio_set_mode(unsigned int pin, unsigned int mode);
void led_on(void);
void led_off(void);

#endif /* GPIO_COMMON_H */