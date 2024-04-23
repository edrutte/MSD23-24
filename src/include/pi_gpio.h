#ifndef CHESS_PI_GPIO_H
#define CHESS_PI_GPIO_H
#ifdef __aarch64__
#include <wiringPi.h>
#else //Stub defines to not break build
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define digitalWrite(pin, value)
#define digitalRead(pin) 0
#define pinMode(pin, mode)
#define delay(msec)
#define delayMicroseconds(usec)
#define pwmToneWrite(pin, freq)
#define wiringPiSetup() 1
#endif
#endif //CHESS_PI_GPIO_H
