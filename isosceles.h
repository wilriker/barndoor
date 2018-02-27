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
static const float SIDEREAL_SECS = 86164.0916; // time in seconds for 1 rotation of earth


// Setup motor class with parameters targetting an EasyDriver board
static AccelStepper motor(AccelStepper::DRIVER,
                          pinOutStep,
                          pinOutDirection);

// Given time offset from the 100% closed position, figure out
// the total number of steps required to achieve that
long time_to_usteps(long tsecs)
{
    return (long)(USTEPS_PER_ROTATION *
                  THREADS_PER_CM * 2.0 * BASE_LEN_CM *
                  sin(tsecs * PI / SIDEREAL_SECS));
}


// Given an angle, figure out the usteps required to get to
// that point.
long angle_to_usteps(float angle)
{
    return time_to_usteps(SIDEREAL_SECS / 360.0 * angle);
}


// Given total number of steps from 100% closed position, figure out
// the corresponding total tracking time in seconds
long usteps_to_time(long usteps)
{
    return (long)(asin(usteps /
                       (USTEPS_PER_ROTATION * THREADS_PER_CM * 2.0 * BASE_LEN_CM)) *
                  SIDEREAL_SECS / PI);
}
