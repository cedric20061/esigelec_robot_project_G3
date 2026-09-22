//M. Labarre KIND reminder : the two following lines prevents the file from being included more than once
//-->Error we used to have before this : one variable was declared twice in two deferent files
#ifndef ENGINES_H
#define ENGINES_H

#include <msp430.h>

//defining values for robot actions (cf previous tps)
#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3

//number of "white" or "dark" marks on the encoder wheel (inner white wheel)
#define CRAN 12

//wheel radius in cm
#define RADIUS 2 
#define PERIMETER (RADIUS * 44/7)

//maccro that converts a %age into a pwm timer value (with a period of TA1CCR0)
#define PERCENT_CONTROL(percent) (percent*(TA1CCR0/100))

//maccro that converts a pwm value into a %
#define VALUE_TO_PERCENT(value) (value/(TA1CCR0/100))

//proportional correction coefficient used by the regulation
//here we cannot use a PID as adviced coz the MSP430 is not fast enough to dot hose calculus
#define K_PROPORTION 100 // Valeur de correction pour un rapport cyclique de 50% (K = 100 à PWM = 100%)

//varaible that stores whether the robot is currently moving (volatile bc the value can be changed inside an interrupt)
volatile char is_moving;
//variable that stores the etimated teavelled distance by the robot
volatile char distance_parcourue;

//Function prototypes en bas are implemented in engine.c

void engines_configs(); 

void action_robot(int action);

void robot_avancer();

void robot_tourner_gauche();

void robot_tourner_droite();

void robot_arret();

void octo_coupleur_reading_config();

void timer_set();

void timer_start();

void timer_reset();

#endif
