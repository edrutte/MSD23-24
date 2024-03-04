#ifndef CHESS_PI_GPIO_H
#define CHESS_PI_GPIO_H
#ifdef __aarch64__
#include <wiringPi.h>
#endif
int read_gpio(int gpio_pin);
int write_gpio(int gpio_pin, int value);
#endif //CHESS_PI_GPIO_H
