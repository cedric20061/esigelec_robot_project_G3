#ifndef HOMOLOGATION_H
#define HOMOLOGATION_H

// ADC threshold (raw units) at which the front sensor is considered
// "obstacle detected" / stop condition reached. Shared between main.c
// and actions.c so both compare against the exact same value.
#define STOP_DISTANCE_THRESHOLD 0x150

// Latest raw ADC reading from the front distance sensor.
// Defined once in actions.c, shared (as extern) with main.c
extern volatile int distance_sensor_reading;

// Seconds elapsed since homologation() started, paused automatically
// while an obstacle is detected (incremented in main.c's Timer0 ISR).
// Used by homologation() to enforce the contest's 10s time limit.
extern volatile int elapsed_seconds;

void homologation();

#endif
