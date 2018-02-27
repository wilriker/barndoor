// Constants to set based on hardware construction specs
//
// Assuming you followed the blog linked above, these few variables
// should be the only things that you need to change in general
//
static const float STEP_SIZE_DEG = 1.8;  // Degrees rotation per step
static const float MICRO_STEPS = 8;      // Number of microsteps per step
static const float THREADS_PER_CM = 8;   // Number of threads in rod per cm of length
static const float BASE_LEN_CM = 30.5;   // Length from hinge to center of rod in cm

// Nothing below this line should require changing


// Derived constants
static const float USTEPS_PER_ROTATION = 360.0 / STEP_SIZE_DEG * MICRO_STEPS; // usteps per rod rotation


// Standard constants
static const float SIDEREAL_SECS = 86164.0916; // time in seconds for 1 rotation of earth


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
