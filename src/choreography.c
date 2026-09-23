#include <msp430.h>
#include "../headers/ADC.h"
#include "../headers/engines.h"
#include "../headers/actions.h"
#include "../headers/choreography.h"

// How often we check the front sensor while a move is playing.
// Smaller = more reactive to obstacles, but more ADC conversions and
// less time actually "dancing" per check.
#define OBSTACLE_POLL_CYCLES 50000UL // ~50ms at 1MHz
#define OBSTACLE_POLL_MS 50

// How long the avoidance turn lasts before resuming the choreography
#define AVOIDANCE_TURN_CYCLES 400000UL // ~0.4s at 1MHz

// Matches BRAKE_STEPS * BRAKE_STEP_DELAY_CYCLES in engines.c, so the
// 90s budget accounts correctly for time spent in a brake flourish.
#define BRAKE_FLOURISH_DURATION_MS 1800

/* THE DANCE ITSELF. This is the only block that have to be edited to
 change the choreography - nothing below it needs to change. */
#if ROBOT_ROLE == ROBOT_ROLE_FAST
static const ChoreoStep choreography[] = {
    { MOVE_FORWARD,         1200 },
    { MOVE_CURVE_RIGHT,      600 },
    { MOVE_FORWARD,          800 },
    { MOVE_CURVE_LEFT,       600 },
    { MOVE_PAUSE,            300 },
    { MOVE_BRAKE_FLOURISH,     0 },
    { MOVE_CURVE_LEFT,       900 },
    { MOVE_FORWARD,         1000 },
    { MOVE_CURVE_RIGHT,      700 },
    { MOVE_PAUSE,            300 },
};
#else // ROBOT_ROLE_SLOW
static const ChoreoStep choreography[] = {
    { MOVE_FORWARD,         2000 },
    { MOVE_PAUSE,            500 },
    { MOVE_CURVE_LEFT,      1200 },
    { MOVE_PAUSE,            500 },
    { MOVE_CURVE_RIGHT,     1200 },
    { MOVE_BRAKE_FLOURISH,     0 },
};
#endif

#define CHOREOGRAPHY_STEP_COUNT (sizeof(choreography) / sizeof(choreography[0]))

static void apply_move(ChoreoMove move){
    switch(move){
        case MOVE_FORWARD:      robot_move_forward();           break;
        case MOVE_CURVE_LEFT:   set_robot_action(ACTION_LEFT);  break;
        case MOVE_CURVE_RIGHT:  set_robot_action(ACTION_RIGHT); break;
        case MOVE_PAUSE:        robot_stop();                   break;
        default:                robot_stop();                   break;
    }
}

// Turns briefly away from whatever was just detected, then lets
// run_choreography() resume the interrupted move where it left off.
static void avoid_obstacle(){
    robot_stop();
    // Alternate side using the free-running Timer0 count as a cheap
    // pseudo-random bit, so two robots meeting head-on don't both
    // always turn the same way. This is a PARTIAL mitigation only: the
    // front sensor can't see anything to the side or behind the robot,
    // so a collision approaching from there won't be caught at all.
    if((TA0R & 0x01) == 0){
        set_robot_action(ACTION_LEFT);
    } else {
        set_robot_action(ACTION_RIGHT);
    }
    __delay_cycles(AVOIDANCE_TURN_CYCLES);
    robot_stop();
}

// Holds `move` for `duration_ms`, polling the front sensor every
// OBSTACLE_POLL_MS. If something gets too close, breaks into
// avoid_obstacle() and then resumes `move` automatically - the
// interrupted step of the choreography isn't lost or skipped.
static void hold_move_ms(ChoreoMove move, unsigned int duration_ms){
    unsigned int elapsed_ms = 0;

    apply_move(move);

    while(elapsed_ms < duration_ms){
        __delay_cycles(OBSTACLE_POLL_CYCLES);
        elapsed_ms += OBSTACLE_POLL_MS;

        ADC_Demarrer_conversion(3);
        distance_sensor_reading = ADC_Lire_resultat();

        if(distance_sensor_reading >= CHOREO_OBSTACLE_THRESHOLD){
            avoid_obstacle();
            apply_move(move); // resume the interrupted move
        }
    }
}

void run_choreography(void){
    unsigned long elapsed_ms = 0;
    unsigned int i = 0;

    robot_is_moving = 1;

    while(elapsed_ms < CHOREO_DURATION_MS){
        ChoreoStep step = choreography[i];

        if(step.move == MOVE_BRAKE_FLOURISH){
            // robot_brake() already ramps down over its own fixed
            // duration (see engines.c) - no need to also run it
            // through hold_move_ms()'s obstacle-polling loop.
            robot_brake();
            elapsed_ms += BRAKE_FLOURISH_DURATION_MS;
        } else {
            hold_move_ms(step.move, step.duration_ms);
            elapsed_ms += step.duration_ms;
        }

        i++;
        if(i >= CHOREOGRAPHY_STEP_COUNT){
            i = 0; // loop the routine if the table is shorter than 90s
        }
    }

    robot_brake();
}
