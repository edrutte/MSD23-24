#include "board_defs.h"
#include "stepper.h"

// declaring/initializing coordinate array for each motor
static int coordinate_motors[8];

void init_motors() {
	for (int i=0; i<8; i++)
	{
		coordinate_motors[i] = (int) round(numSteps * i);
	}

	// set values of all pins
	pinMode(stepPinRmotor, OUTPUT);
	pinMode(dirPin_Rmotor, OUTPUT);
	pinMode(stepPinLmotor, OUTPUT);
	pinMode(dirPin_Lmotor, OUTPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(sleepPin, OUTPUT);

	pinMode(limitSwitchLeft, INPUT);
	pinMode(limitSwitchDown, INPUT);
	pinMode(electromagFET, OUTPUT);

	digitalWrite(dirPin_Rmotor, HIGH);
	digitalWrite(dirPin_Lmotor, HIGH);
	digitalWrite(sleepPin, HIGH);
	digitalWrite(resetPin, HIGH);
	digitalWrite(electromagFET, LOW);


	//move to origin shown below 0,0
	//first it's set to move to the bottom left corner and
	//then adjusting to the middle of the bottom left square
	//do we even need delays? we can change delays to make it have a better feel


	while ( digitalRead(limitSwitchDown) == HIGH )
	{
		digitalWrite(dirPin_Rmotor, LOW);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  down
		digitalWrite(dirPin_Lmotor, HIGH);
		rotateMotor(1);

	}

	delay(1000);


	while ( digitalRead(limitSwitchLeft) == HIGH )
	{
		digitalWrite(dirPin_Rmotor, LOW);
		digitalWrite(dirPin_Lmotor, LOW);  // left
		rotateMotor(1);
	}

	delay(1000);

	//above makes use of limit switches to move it all the way to the bottom right that it can go
	//below moves it to origin (0,0) to initialize grid

	digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4 DIR2 = 2  //up
	digitalWrite(dirPin_Lmotor, LOW);
	rotateMotor(300);  //we need to figure out after board is there
	delay(100);

	digitalWrite(dirPin_Rmotor, HIGH);
	digitalWrite(dirPin_Lmotor, HIGH);  // right
	rotateMotor(300);  //we need to figure out after board is there
	delay(1000);
}

void rotateMotor(int steps) {
	for (int i = 0; i < steps; i++) {
		digitalWrite(stepPinRmotor, HIGH);
		digitalWrite(stepPinLmotor, HIGH);
		digitalWrite(ledPin, HIGH);
		delayMicroseconds(timeSpeed);
		digitalWrite(stepPinRmotor, LOW);
		digitalWrite(stepPinLmotor, LOW);
		digitalWrite(ledPin, LOW);
		delayMicroseconds(timeSpeed);
	}
}
