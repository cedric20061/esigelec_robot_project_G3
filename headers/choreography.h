#ifndef CHOREOGRAPHY_H
#define CHOREOGRAPHY_H

#include "homologation.h"

//Definig roles to choose from the two robots since they should not function the same
#define ROBOT_ROLE_FAST 1
#define ROBOT_ROLE_SLOW 2

//Defines which robot has been choosen
#define ROBOT_ROLE ROBOT_ROLE_FAST   

// Total time for the whole choreography
#define CHOREO_DURATION_MS 90000UL
#define CHOREO_DURATION_S 90

//Distance treshold when an obstacle is detected. This one is lower than the one used in homologation because 
//we want the sensor to be more precise here
#define CHOREO_OBSTACLE_THRESHOLD 0x150

// The available moves
typedef enum {
    MOVE_FORWARD,
    MOVE_CURVE_LEFT,
    MOVE_CURVE_RIGHT,
    MOVE_PAUSE,
    MOVE_BRAKE_FLOURISH
    } ChoreoMove;

// Defines the dance move to make and the duration of it
typedef struct {
    ChoreoMove move;
    unsigned int duration_ms;
} ChoreoStep;


void run_choreography(void);

#endif
