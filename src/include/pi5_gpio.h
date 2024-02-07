#ifndef CHESS_PI5_GPIO_H
#define CHESS_PI5_GPIO_H
#include <gpiod.h>

int init_gpio(struct gpiod_chip *gpio_chip);
int read_gpio(struct gpiod_chip *gpio_chip, int gpio_pin);
int write_gpio(struct gpiod_chip *gpio_chip, int gpio_pin, int value);
#endif //CHESS_PI5_GPIO_H
