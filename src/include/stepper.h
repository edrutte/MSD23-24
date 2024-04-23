#ifndef CHESS_STEPPER_H
#define CHESS_STEPPER_H
#include <stdint.h>
#include "pi_gpio.h"

// defines pins numbers
#define stepPinRmotor 27
#define dirPin_Rmotor 26
#define stepPinLmotor 6
#define dirPin_Lmotor 5
#define sleepPin 4
#define resetPin 1

//newly added from my code
//defines pins numbers for each limit switch
#define limitSwitchLeft 7
#define limitSwitchDown 0
#define electromagFET 2

#define numSteps 262.5    //motor rotation for one square
#define timeSpeed 550 // motor speed, with time in ms

void init_motors();
void rotateMotor(int steps);

#endif //CHESS_STEPPER_H
