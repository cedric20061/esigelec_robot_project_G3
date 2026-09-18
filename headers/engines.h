#include <msp430.h>

#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3
#define CRAN 12

void engines_configs();

void action_robot(int action);

void robot_avancer();

void robot_tourner_gauche();

void robot_tourner_droite();

void robot_arret();

void octo_coupleur_reading_config();
