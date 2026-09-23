#ifndef HOMOLOGATION_H
#define HOMOLOGATION_H

// ADC threshold (raw units) at which the front sensor is considered
// "obstacle detected" / stop condition reached. Shared between main.c
// and actions.c so both compare against the exact same value.
#define STOP_DISTANCE_THRESHOLD 0x150

// Total distance for the qualifying run, in millimeters (1.30m, as
// required by the contest rules) but we use 1.20m because of bias in our calculation of the distance
#define HOMOLOGATION_DISTANCE_MM 1200

// Latest raw ADC reading from the front distance sensor.
// Defined once in actions.c, shared (as extern) with main.c
volatile int distance_sensor_reading;

// Seconds elapsed since homologation() started, paused automatically
// while an obstacle is detected (incremented in main.c's Timer0 ISR).
// Used by homologation() to enforce the contest's 10s time limit.
volatile int elapsed_seconds;

void homologation();

#endif
