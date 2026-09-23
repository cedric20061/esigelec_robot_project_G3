#ifndef CHOREOGRAPHY_H
#define CHOREOGRAPHY_H

// Build this firmware for the FAST or the SLOW robot by picking one of
// these two - it selects which move table gets compiled in, in
// choreography.c. Both robots run the exact same choreography ENGINE;
// only the table (and therefore the actual dance) differs, which is the
// whole point of the modular design based on the transition table see in state engine.
#define ROBOT_ROLE_FAST 1
#define ROBOT_ROLE_SLOW 2
#define ROBOT_ROLE ROBOT_ROLE_FAST   // change this per robot build

// Total time budget for the contest routine (contest rules: 90s)
#define CHOREO_DURATION_MS 90000UL

// Distance-sensor threshold used to react to something in front of the
// robot WHILE dancing (the other robot, a prop, etc). Reuses the
// homologation threshold by default.
#define CHOREO_OBSTACLE_THRESHOLD 0x150

// The available move "verbs".
typedef enum {
    MOVE_FORWARD,
    MOVE_CURVE_LEFT,
    MOVE_CURVE_RIGHT,
    MOVE_PAUSE,
    MOVE_BRAKE_FLOURISH
    /* MOVE_BACKWARD, MOVE_SPIN_LEFT, MOVE_SPIN_RIGHT: not wired up yet. */
} ChoreoMove;

// One line of the dance: a move and how long to hold it (ms).
// Edit the tables in choreography.c - nothing else needs to change.
typedef struct {
    ChoreoMove move;
    unsigned int duration_ms;
} ChoreoStep;

void run_choreography(void);

#endif
