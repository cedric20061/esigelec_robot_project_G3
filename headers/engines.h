//M. Labarre KIND reminder : the two following lines prevents the file from being included more than once
//-->Error we used to have before this : one variable was declared twice in two deferent files
#ifndef ENGINES_H
#define ENGINES_H

#include <msp430.h>

// Robot action codes, used by set_robot_action()
#define ACTION_STOP     0
#define ACTION_LEFT     1
#define ACTION_FORWARD  2
#define ACTION_RIGHT    3
#define ACTION_BRAKE    4

// Number of light/dark marks on the encoder wheel (inner white wheel)
#define ENCODER_SLOTS_PER_REV 24

// Wheel geometry, in millimeters.
// Millimeters (rather than centimeters) are used on purpose: the
// distance-per-tick calculation involves an integer division that
// truncates, and the smaller the unit, the smaller the *relative*
// error that truncation introduces.
#define WHEEL_RADIUS_MM 20
// Wheel circumference, approximated with pi ~= 22/7 (2*pi*r)
#define WHEEL_PERIMETER_MM ((2 * WHEEL_RADIUS_MM * 22) / 7)

// Converts a duty-cycle percentage (0-100) into a PWM compare value for
// the current PWM period (TA1CCR0)
#define PERCENT_CONTROL(percent) ((percent) * (TA1CCR0 / 100))

// Converts a PWM compare value back into a duty-cycle percentage
#define VALUE_TO_PERCENT(value) ((value) / (TA1CCR0 / 100))

#define TICKS_TO_DISTANCE(ticks) ((long)ticks * WHEEL_PERIMETER_MM) / ENCODER_SLOTS_PER_REV

// Proportional gain used by the wheel-speed regulation loop (see the
// TIMER0_A1_VECTOR interrupt in main.c).
//
// This value is NOT universal - it depends on:
//   - the control loop period (currently ~1s, see timer_set() below)
//   - the PWM frequency / period (TA1CCR0)
//   - the encoder resolution (ENCODER_SLOTS_PER_REV)
// A real PID controller would adapt to these automatically, but running
// one every control tick is more than the MSP430G2553 needs for this
// project, so the gain has to be tuned by hand instead. Start low and
// increase gradually while watching for oscillation (the wheel speeds
// hunting back and forth instead of settling).
#define SPEED_CORRECTION_GAIN 2

// Shared engine/robot state - defined once in engines.c, declared here
// as extern so every file that needs them sees the SAME variable
// instead of each getting its own private copy (which is what used to
// happen when these were declared here without `extern`).
volatile char robot_is_moving;

volatile int encoder_ticks_left;
volatile int encoder_ticks_right;

void engines_configs();
int  clamp_percentage(int percent);
void set_robot_action(int action);
void robot_move_forward();
void robot_turn_left();
void robot_turn_right();
void robot_stop();
void robot_brake();
void optocoupler_config();
void timer_set();
void timer_start();
void timer_reset();

#endif
