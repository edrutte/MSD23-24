#ifndef CHESS_BOARD_DEFS_H
#define CHESS_BOARD_DEFS_H

#include <math.h>

#define BOARD_SQUARE_SIDE_LEN_MM 50.0
#define STEPPER_STEP_REV 200.0
#define STEPPER_PULLEY_RAD_MM 6.0
#define MOVE_SPEED_MM_S 25.0
#define STEP_PWM_FREQ (MOVE_SPEED_MM_S * (STEPPER_STEP_REV / (2.0 * M_PI * STEPPER_PULLEY_RAD_MM)))
#define X_STEPPER_PIN 1234 //TODO: determine
#define Y_STEPPER_PIN 4321 //TODO: determine

#endif //CHESS_BOARD_DEFS_H
