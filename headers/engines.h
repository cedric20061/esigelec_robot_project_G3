#ifndef ENGINES_H
#define ENGINES_H

#include <msp430.h>
#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3
#define CRAN 12
#define RADIUS 2
#define PERIMETER (RADIUS * 44/7)

#define PERCENT_CONTROL(percent) (percent*(TA1CCR0/100))
#define VALUE_TO_PERCENT(value) (value/(TA1CCR0/100))
#define K_PROPORTION 10 // Valeur de correction pour un rapport cyclique de 50% (K = 100 à PWM = 100%)

volatile char is_moving;

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