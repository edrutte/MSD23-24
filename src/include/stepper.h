#ifndef CHESS_STEPPER_H
#define CHESS_STEPPER_H

#include <stdbool.h>
#include <stdint.h>
#include "pi_gpio.h"

// defines pins numbers
#define stepPinRmotor 3
#define dirPin_Rmotor 26
#define stepPinLmotor 16
#define dirPin_Lmotor 10
#define sleepPin 4
#define resetPin 1

//newly added from my code
//defines pins numbers for each limit switch
#define limitSwitchLeft 7
#define limitSwitchDown 0
#define electromagFET 2

#define numSteps 262.5    //motor rotation for one square
#define timeSpeed 550 // motor speed, with time in ms

typedef struct {
	uint8_t x;
	uint8_t y;
} Square;

void init_motors();
void rotateMotor(int steps);
void make_move(Square from, Square to, Square capturing, bool castle);

#endif //CHESS_STEPPER_H
