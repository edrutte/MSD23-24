#include "board_defs.h"
#include "pi_gpio.h"
#include "stepper.h"

void stepper_move_rel_mm(double x_mm, double y_mm) {
	int move_time_x_ms = (int) (x_mm / (MOVE_SPEED_MM_S / 1000.0));
	int move_time_y_ms = (int) (y_mm / (MOVE_SPEED_MM_S / 1000.0));
#ifdef __aarch64__
	pwmToneWrite(X_STEPPER_PIN, STEP_PWM_FREQ);
	pwmToneWrite(Y_STEPPER_PIN, STEP_PWM_FREQ);
	if (move_time_x_ms > move_time_y_ms) {
		delay(move_time_y_ms);
		pwmToneWrite(Y_STEPPER_PIN, 0);
		delay(move_time_x_ms - move_time_y_ms);
		pwmToneWrite(X_STEPPER_PIN, 0);
	} else {
		delay(move_time_x_ms);
		pwmToneWrite(X_STEPPER_PIN, 0);
		delay(move_time_y_ms - move_time_x_ms);
		pwmToneWrite(Y_STEPPER_PIN, 0);
	}
#endif
}

void stepper_move_rel_sq(int x_sq, int y_sq) {
	stepper_move_rel_mm(x_sq * BOARD_SQUARE_SIDE_LEN_MM, y_sq * BOARD_SQUARE_SIDE_LEN_MM);
}