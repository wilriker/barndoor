#include <AccelStepper.h>
#include <FiniteStateMachine.h>
#include "mp6500.h"
#include "curved_rod.h"

static const MP6500::Microsteps STEPS = MP6500::EIGHTH_STEP;
static const float HIGHSPEED = 800*STEPS; // Maximum speed/highspeed mode
static const float INITIAL_ANGLE =  4; // Initial angle of barn doors when switched on
static const float MAXIMUM_ANGLE = 33; // Maximum angle to allow barn doors to open (30 deg == 2 hours)

// static const int pinOutEnable    =  9; // Arduino digital pin connected to Driver ENABLE
static const int pinOutMS1       =  8; // Arduino digital pin connected to Driver MS1
static const int pinOutMS2       =  7; // Arduino digital pin connected to Driver MS2
static const int pinOutI1        =  6; // Arduino digital pin connected to Driver I1 for current limiting
static const int pinOutI2        =  5; // Arduino digital pin connected to Driver I2 for current limiting
static const int pinOutSleep     =  4; // Arduino digital pin connected to Driver SLEEP
static const int pinOutStep      =  3; // Arduino digital pin connected to Driver STEP
static const int pinOutDirection =  2; // Arduino digital pin connected to Driver DIR

static const int pinInHighspeed  = 12; // Arduino digital pin connected to highspeed mode switch
static const int pinInSidereal   = 11; // Arduino digital pin connected to sidereal mode switch
static const int pinInDirection  = 10; // Arduino digital pin connected to direction switch

// If the barn door doesn't go to 100% closed, this records
// the inital offset we started from for INITIAL_ANGLE
static const long offsetPositionUSteps = angle_to_usteps(INITIAL_ANGLE);
// The maximum we're willing to open the mount to avoid the
// doors falling open and smashing the camera. Safety first :-)
static const long maximumPositionUSteps = angle_to_usteps(MAXIMUM_ANGLE);

// Setup motor driver class
static MP6500 driver(pinOutDirection,
					pinOutStep,
					pinOutSleep,
					pinOutI2,
					pinOutI1,
					pinOutMS2,
					pinOutMS1);

// Setup motor class
static AccelStepper motor(AccelStepper::DRIVER,
							pinOutStep,
							pinOutDirection);

// Global initialization when first turned on
void setup(void) {
	driver.init(STEPS);
	driver.wakeup();

	motor.setMaxSpeed(HIGHSPEED);
	motor.setCurrentPosition(offsetPositionUSteps);

	pinMode(pinInSidereal, INPUT_PULLUP);
	pinMode(pinInHighspeed, INPUT_PULLUP);
	pinMode(pinInDirection, INPUT_PULLUP);
}

void stateSiderealUpdate() {
	int dir = digitalRead(pinInDirection);
	long curPos = motor.currentPosition();
	if ((dir == LOW && curPos == offsetPositionUSteps) ||
			(dir == HIGH && curPos == maximumPositionUSteps)) {
			return;
	}
	float speed = USTEPS_PER_SECOND;
	if (dir == LOW) {
		speed *= -1.0;
	}
	motor.setSpeed(speed);
	motor.runSpeed();
}

void stateResetEnter() {
	/* driver.setStepping(MP6500::FULL_STEP); */
	motor.setAcceleration(100*STEPS);
}

void stateResetUpdate() {
	long target = maximumPositionUSteps;
	if (digitalRead(pinInDirection) == LOW) {
		target = offsetPositionUSteps;
	}
	motor.moveTo(target);
	motor.run();
}

void stateResetExit() {
	motor.stop();
	motor.runToPosition();
	/* driver.setStepping(STEPS); */
	motor.setAcceleration(1);
}

void stateOffEnter() {
	driver.sleep();
}

void stateOffExit() {
	driver.wakeup();
}

// A finite state machine with 3 states - sidereal, highspeed and off
static State stateSidereal = State(NO_ENTER, &stateSiderealUpdate, NO_EXIT);
static State stateReset = State(&stateResetEnter, &stateResetUpdate, &stateResetExit);
static State stateOff = State(&stateOffEnter, NO_UPDATE, &stateOffExit);
static FSM barndoor = FSM(stateOff);

void loop(void) {
	if (digitalRead(pinInSidereal) == LOW) {
		barndoor.transitionTo(stateSidereal);
	} else if (digitalRead(pinInHighspeed) == LOW) {
		barndoor.transitionTo(stateReset);
	} else {
		barndoor.transitionTo(stateOff);
	}
	barndoor.update();
}
