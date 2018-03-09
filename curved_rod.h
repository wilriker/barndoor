// Constants to set based on hardware construction specs
//
// Assuming you followed the blog linked above, these few variables
// should be the only things that you need to change in general
//
static const float STEP_SIZE_DEG = 1.8;    // Degrees rotation per step
static const float MICRO_STEPS = 8;     // Number of microsteps per step
static const float MOTOR_GEAR_TEETH = 24; // Number of teeth on the motor gear
static const float ROD_GEAR_TEETH = 75;   // Number of teeth on the gear that drives the curved rod
static const float THREAD_PITCH_MM = 0.8; // Thread pitch in mm of threaded rod (M5: 0.8, M6: 1, M8: 1.25)
static const float BASE_LEN_MM = 182.8;   // Length from hinge to center of rod in mm

// Nothing below this line should require changing


// Standard constants
static const float SIDEREAL_SECS = 86164.0916; // time in seconds for 1 rotation of earth


// Derived constants
static const float USTEPS_PER_ROTATION = 360.0 / STEP_SIZE_DEG * MICRO_STEPS * (ROD_GEAR_TEETH / MOTOR_GEAR_TEETH); // usteps per rod rotation
static const float DEGREE_PER_SECOND = 360.0 / SIDEREAL_SECS; // angle of earth rotation per second
static const float MM_LIFT_PER_SECOND = tan(DEGREE_PER_SECOND) * BASE_LEN_MM; // how many mm does the rod have to travel in one second to match earth's rotation
static const float USTEPS_PER_SECOND = (MM_LIFT_PER_SECOND / THREAD_PITCH_MM) * USTEPS_PER_ROTATION; // usteps per second to match earth's rotation


// Given time offset from the 100% closed position, figure out
// the total number of steps required to achieve that
long time_to_usteps(long tsecs) {
  return (long)(USTEPS_PER_SECOND * tsecs);
}


// Given an angle, figure out the usteps required to get to
// that point.
long angle_to_usteps(float angle) {
  return time_to_usteps(SIDEREAL_SECS / 360.0 * angle);
}


// Given total number of steps from 100% closed position, figure out
// the corresponding total tracking time in seconds
long usteps_to_time(long usteps) {
  return (long)(USTEPS_PER_SECOND * usteps);
}
