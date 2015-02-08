// -*- c -*-
//
// barndoor.ino: arduino code for an astrophotography barndoor mount
//
// Copyright (C) 2014-2015 Daniel P. Berrange
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// This code is used to drive the circuit described at
//
//  http://fstop138.berrange.com/2014/01/building-an-barn-door-mount-part-1-arduino-stepper-motor-control/
//
// Based on the maths calculations documented at
//
//  http://fstop138.berrange.com/2014/01/building-an-barn-door-mount-part-2-calculating-mount-movements/
//
// The code assumes an **isosceles** drive barn door mount design.
//
// Other barndoor drive designs will require different mathematical
// formulas to correct errors

// http://arduino-info.wikispaces.com/HAL-LibrariesUpdates
#include <FiniteStateMachine.h>

// http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>

// We don't want to send debug over the serial port by default since
// it seriously slows down the main loop causing tracking errors
//#define DEBUG

// Constants to set based on hardware construction specs
//
// Assuming you followed the blog linked above, these few variables
// should be the only things that you need to change in general
//
static const float STEP_SIZE_DEG = 1.8;  // Degrees rotation per step
static const float MICRO_STEPS = 8;      // Number of microsteps per step
static const float THREADS_PER_CM = 8;   // Number of threads in rod per cm of length
static const float BASE_LEN_CM = 30.5;   // Length from hinge to center of rod in cm
static const float INITIAL_ANGLE = 0;    // Initial angle of barn doors when switched on
static const float MAXIMUM_ANGLE = 30;   // Maximum angle to allow barn doors to open (30 deg == 2 hours)

// Nothing below this line should require changing unless your barndoor
// is not an Isoceles mount, or you changed the electrical circuit design

// Constants to set based on electronic construction specs
static const int pinOutStep = 9;      // Arduino digital pin connected to EasyDriver step
static const int pinOutDirection = 8; // Arduino digital pin connected to EasyDriver direction

static const int pinInSidereal = 4;  // Arduino analogue pin connected to sidereal mode switch
static const int pinInHighspeed = 5; // Arduino analogue pin connected to highspeed mode switch
static const int pinInDirection = 3; // Arduino analogue pin connected to direction switch


// Derived constants
static const float USTEPS_PER_ROTATION = 360.0 / STEP_SIZE_DEG * MICRO_STEPS; // usteps per rod rotation


// Standard constants
static const float SIDE_REAL_SECS = 86164.0419; // time in seconds for 1 rotation of earth


// Setup motor class with parameters targetting an EasyDriver board
static AccelStepper motor(AccelStepper::DRIVER,
                          pinOutStep,
                          pinOutDirection);

// A finite state machine with 3 states - sidereal, highspeed and off
static State stateSidereal = State(state_sidereal_enter, state_sidereal_update, state_sidereal_exit);
static State stateHighspeed = State(state_highspeed_enter, state_highspeed_update, state_highspeed_update);
static State stateOff = State(state_off_enter, state_off_update, state_off_exit);
static FSM barndoor = FSM(stateOff);


// Given time offset from the 100% closed position, figure out
// the total number of steps required to achieve that
long time_to_usteps(long tsecs)
{
    return (long)(USTEPS_PER_ROTATION *
                  THREADS_PER_CM * 2.0 * BASE_LEN_CM *
                  sin(tsecs * PI / SIDE_REAL_SECS));
}


// Given an angle, figure out the usteps required to get to
// that point.
long angle_to_usteps(float angle)
{
    return time_to_usteps(SIDE_REAL_SECS / 360.0 * angle);
}


// Given total number of steps from 100% closed position, figure out
// the corresponding total tracking time in seconds
long usteps_to_time(long usteps)
{
    return (long)(asin(usteps /
                       (USTEPS_PER_ROTATION * THREADS_PER_CM * 2.0 * BASE_LEN_CM)) *
                  SIDE_REAL_SECS / PI);
}

// These variables are initialized when the motor switches
// from stopped to running, so we know our starting conditions

// If the barn door doesn't go to 100% closed, this records
// the inital offset we started from for INITIAL_ANGLE
static long offsetPositionUSteps;
// The maximum we're willing to open the mount to avoid the
// doors falling open and smashing the camera. Safety first :-)
static long maximumPositionUSteps;
// Total motor steps since 100% closed, at the time the
// motor started running
static long startPositionUSteps;
// Total tracking time associated with total motor steps
static long startPositionSecs;
// Wall clock time at the point the motor switched from
// stopped to running.
static long startWallClockSecs;


// These variables are used while running to calculate our
// constantly changing targets

// The wall clock time where we need to next calculate tracking
// rate / target
static long targetWallClockSecs;
// Total tracking time associated with our target point
static long targetPositionSecs;
// Total motor steps associated with target point
static long targetPositionUSteps;


// Global initialization when first turned off
void setup(void)
{
    pinMode(pinInSidereal, OUTPUT);
    pinMode(pinInHighspeed, OUTPUT);
    pinMode(pinInDirection, OUTPUT);

    motor.setPinsInverted(true, false, false);
    motor.setMaxSpeed(3000);

    offsetPositionUSteps = angle_to_usteps(INITIAL_ANGLE);
    maximumPositionUSteps = angle_to_usteps(MAXIMUM_ANGLE);

#ifdef DEBUG
    Serial.begin(9600);
#endif
}


// The logical motor position which takes into account the
// fact that we have an initial opening angle
long motor_position()
{
    return motor.currentPosition() + offsetPositionUSteps;
}


// This is called whenever the motor is switch from stopped to running.
//
// It reads the current motor position which lets us see how many steps
// have run since the device was first turned on in the 100% closed
// position. From that we then figure out the total tracking time that
// corresponds to our open angle. This is then used by plan_tracking()
// to figure out subsequent deltas
void start_tracking(void)
{
    startPositionUSteps = motor_position();
    startPositionSecs = usteps_to_time(startPositionUSteps);
    startWallClockSecs = millis() / 1000;
    targetWallClockSecs = startWallClockSecs;

#ifdef DEBUG
    Serial.print("Enter sidereal\n");
    Serial.print("offset pos usteps: ");
    Serial.print(offsetPositionUSteps);
    Serial.print(", start pos usteps: ");
    Serial.print(startPositionUSteps);
    Serial.print(", start pos secs: ");
    Serial.print(startPositionSecs);
    Serial.print(", start wclk secs: ");
    Serial.print(startWallClockSecs);
    Serial.print("\n\n");
#endif
}


// This is called when we need to figure out a new target position
//
// The tangent errors are small enough that over a short period,
// we can assume constant linear motion will give constant angular
// motion.
//
// So we set our target values to what we expect them all to be
// 15 seconds  in the future
void plan_tracking(void)
{
    targetWallClockSecs = targetWallClockSecs + 15;
    targetPositionSecs = startPositionSecs + (targetWallClockSecs - startWallClockSecs);
    targetPositionUSteps = time_to_usteps(targetPositionSecs);

#ifdef DEBUG
    Serial.print("target pos usteps: ");
    Serial.print(targetPositionUSteps);
    Serial.print(", target pos secs: ");
    Serial.print(targetPositionSecs);
    Serial.print(", target wclk secs: ");
    Serial.print(targetWallClockSecs);
    Serial.print("\n");
#endif
}


// This is called on every iteration of the main loop
//
// It looks at our target steps and target wall clock time and
// figures out the rate of steps required to get to the target
// in the remaining wall clock time. This applies the constant
// linear motion expected by  plan_tracking()
//
// By re-calculating rate of steps on every iteration, we are
// self-correcting if we are not invoked by the arduino at a
// constant rate
void apply_tracking(long currentWallClockSecs)
{
    long timeLeft = targetWallClockSecs - currentWallClockSecs;
    long stepsLeft = targetPositionUSteps - motor_position();
    float stepsPerSec = (float)stepsLeft / (float)timeLeft;

#ifdef DEBUG32
    Serial.print("Target ");
    Serial.print(targetPositionUSteps);
    Serial.print("  curr ");
    Serial.print(motor_position());
    Serial.print("  left");
    Serial.print(stepsLeft);
    Serial.print("\n");
#endif

    if (motor_position() >= maximumPositionUSteps) {
        motor.stop();
    } else {
        motor.setSpeed(stepsPerSec);
        motor.runSpeed();
    }
}


// Called when switching from stopped to running
// in sidereal tracking mode
void state_sidereal_enter(void)
{
    start_tracking();
    plan_tracking();
}


// Called on every tick, when running in sidereal
// tracking mode
//
// XXX we don't currently use the direction switch
// in sidereal mode. Could use it for sidereal vs lunar
// tracking rate perhaps ?
void state_sidereal_update(void)
{
    long currentWallClockSecs = millis() / 1000;

    if (currentWallClockSecs >= targetWallClockSecs) {
        plan_tracking();
    }

    apply_tracking(currentWallClockSecs);
}

void state_sidereal_exit(void)
{
    // nada
}

void state_highspeed_enter(void)
{
#ifdef DEBUG
    Serial.print("Enter highspeed\n");
#endif
}


// Called on every iteration when in non-tracking highspeed
// forward/back mode. Will automatically step when it
// hits the 100% closed position to avoid straining
// the motor
void state_highspeed_update(void)
{
    // pinInDirection is a 2-position switch for choosing direction
    // of motion
    if (analogRead(pinInDirection) < 512) {
        if (motor_position() >= maximumPositionUSteps) {
            motor.stop();
        } else {
            motor.setSpeed(5000);
            motor.runSpeed();
        }
    } else {
        if (motor.currentPosition() <= 0) {
            motor.stop();
        } else {
            motor.setSpeed(-5000);
            motor.runSpeed();
        }
    }
}

void state_highspeed_exit(void)
{
    // nada
}

void state_off_enter(void)
{
#ifdef DEBUG
    Serial.print("Enter off\n");
#endif
    motor.stop();
}

void state_off_update(void)
{
    // nada
}

void state_off_exit(void)
{
    // nada
}


void loop(void)
{
    // pinInSidereal/pinInHighspeed are two poles of a 3-position
    // switch, that let us choose between sidereal tracking,
    // stopped and highspeed mode
    if (analogRead(pinInSidereal) < 512) {
        barndoor.transitionTo(stateSidereal);
    } else if (analogRead(pinInHighspeed) < 512) {
        barndoor.transitionTo(stateHighspeed);
    } else {
        barndoor.transitionTo(stateOff);
    }
    barndoor.update();
}

//
// Local variables:
//  c-indent-level: 4
//  c-basic-offset: 4
//  indent-tabs-mode: nil
// End:
//
