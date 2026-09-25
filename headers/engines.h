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

//wheel radius in milimiters (2 cm measured with Mary's lovely ruler)
#define WHEEL_RADIUS_MM 20
// Wheel circumference --> 2*pi*Radius
#define WHEEL_PERIMETER_MM ((2 * WHEEL_RADIUS_MM * 22) / 7)

// Converts a percentage into a PWM compare value for the current PWM period (our TA1CCR0)
#define PERCENT_CONTROL(percent) ((percent) * (TA1CCR0 / 100))

// Converts a /1000 instead. It Allowed us to corret the wheels more precisely with a wider range
#define PERMILL_CONTROL(percent) ((percent) * (TA1CCR0 / 1000))

// Converts a PWM compare value back into a percentage
#define VALUE_TO_PERCENT(value) ((value) / (TA1CCR0 / 100))

//Converts number of ticks into a distance 
#define TICKS_TO_DISTANCE(ticks) ((long)ticks * WHEEL_PERIMETER_MM) / ENCODER_SLOTS_PER_REV

// Proportional gain used by the wheel-speed regulation loop. Since the MSP430G253 is too slow, we cannot uese a PID
#define SPEED_CORRECTION_GAIN 3


//Robot state : 1--> is moving and 0--> is not
volatile char robot_is_moving;

//Ticks value collected by each encoder respectively
volatile int encoder_ticks_left;
volatile int encoder_ticks_right;


//Function prototypes to be defined in engine.c
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
