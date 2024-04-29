#ifndef CHESS_STEPPER_H
#define CHESS_STEPPER_H

#include <stdbool.h>
#include <stdint.h>
#include "pi_gpio.h"

// defines pins numbers
#define stepPinRmotor (0)
#define dirPin_Rmotor (1)
#define stepPinLmotor (2)
#define dirPin_Lmotor (3)
#define sleepPin (4)
#define resetPin (5)

//newly added from my code
//defines pins numbers for each limit switch
#define limitSwitchLeft (6)
#define limitSwitchDown (7)
#define electromagFET (8)

typedef struct {
	uint8_t x;
	uint8_t y;
} Square_st;

#define gravesteps (131.57143)
#define numSteps (252)  //252? but center to center is 2250 so 250 idk if is good
#define timeSpeed (550)  // how slow the motor spins; time in ms

void init_motors();
void make_move(Square_st from, Square_st to, Square_st capturing, char captType, bool castling);

#endif //CHESS_STEPPER_H
