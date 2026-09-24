#ifndef HOMOLOGATION_H
#define HOMOLOGATION_H

//Defines the limit where the front sensor detects an obstacle
#define STOP_DISTANCE_THRESHOLD 0x150

//Defines the distance the robot must reach
#define HOMOLOGATION_DISTANCE_MM 1200

//Value read by the front sensor
volatile int distance_sensor_reading;

//Defines the time in seconds since the homologation() started running. It should not exced 10s
volatile int elapsed_seconds;

//Function to be defined in homologation.c
void homologation();

#endif
