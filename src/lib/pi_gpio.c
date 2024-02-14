// Parts taken from https://forums.raspberrypi.com/viewtopic.php?t=358676&sid=33fab08771d794f0527f0d4985b1012d&start=25#p2175169
#include "pi_gpio.h"

/**
 * Initialize gpio on the Pi 5
 * @param gpio_chip struct gpiod_chip to initialize
 * @return -1 on error, 0 otherwise
 */
int init_gpio(struct gpiod_chip *gpio_chip) {
	gpio_chip = gpiod_chip_open_by_name("gpiochip4");
	if (gpio_chip == NULL) {
		return -1;
	}
	return 0;
}

/**
 * Read from a gpio pin
 * @param gpio_chip struct gpiod_chip used for reading
 * @param gpio_pin pin to read from
 * @return -1 on error, 0 if pin low, 1 if pin high
 */
int read_gpio(struct gpiod_chip *gpio_chip, int gpio_pin) {
	struct gpiod_line *gpio_line = gpiod_chip_get_line(gpio_chip, gpio_pin);
	if (gpio_line == NULL) {
		return -1;
	}
	if (gpiod_line_direction(gpio_line) == GPIOD_LINE_DIRECTION_OUTPUT) {
		return -1;// Attempted to read from an output
	}
	return gpiod_line_get_value(gpio_line);
}

/**
 * Write to a gpio pin
 * @param gpio_chip struct gpiod_chip used for writing
 * @param gpio_pin pin to write to
 * @param value set pin high or low
 * @return -1 on error, 0 otherwise
 */
int write_gpio(struct gpiod_chip *gpio_chip, int gpio_pin, int value) {
	struct gpiod_line *gpio_line = gpiod_chip_get_line(gpio_chip, gpio_pin);
	if (gpio_line == NULL) {
		return -1;
	}
	if (gpiod_line_direction(gpio_line) == GPIOD_LINE_DIRECTION_INPUT) {
		return -1;// Attempted to write to an input
	}
	if (value < 0 || value > 1) {
		return -1;// Attempted to write invalid value
	}
	return gpiod_line_set_value(gpio_line, value);
}