// Parts taken from https://forums.raspberrypi.com/viewtopic.php?t=358676&sid=33fab08771d794f0527f0d4985b1012d&start=25#p2175169
#include "pi_gpio.h"
#ifdef __aarch64__
/**
 * Read from a gpio pin
 * @param gpio_pin pin to read from
 * @return -1 on error, 0 if pin low, 1 if pin high
 */
int read_gpio(int gpio_pin) {
	pinMode(gpio_pin, INPUT);
	return digitalRead(gpio_pin);
}

/**
 * Write to a gpio pin
 * @param gpio_pin pin to write to
 * @param value set pin high or low
 * @return -1 on error, 0 otherwise
 */
int write_gpio(int gpio_pin, int value) {
	pinMode(gpio_pin, OUTPUT);
	return digitalWrite(gpio_pin, value);
}
#endif