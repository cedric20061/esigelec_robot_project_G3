#include <msp430.h>
#include "../headers/ADC.h"
#include "../headers/engines.h"
#include "../headers/homologation.h"
#include "../headers/choreography.h"

// This defines how often the sensor should be starting the obstacle detection
#define OBSTACLE_POLL_CYCLES 50000UL //50ms
#define OBSTACLE_POLL_MS 50

// How long the avoidance turn lasts before resuming the choreography
#define AVOIDANCE_TURN_CYCLES 400000UL // 0.4s

// Brake flourish move duration : the robot slows down before completly stoping itself
#define BRAKE_FLOURISH_DURATION_MS 1800

extern volatile int elapsed_seconds;

// Dance setup for each robot

//FAST
#if ROBOT_ROLE == ROBOT_ROLE_FAST
static const ChoreoStep choreography[] = {
    { MOVE_FORWARD,          500 },
    { MOVE_CURVE_RIGHT,      600 },
    { MOVE_FORWARD,          800 },
    { MOVE_CURVE_LEFT,       600 },
    { MOVE_FORWARD,          300 },
    { MOVE_BRAKE_FLOURISH,     0 },
    { MOVE_CURVE_LEFT,       900 },
    { MOVE_FORWARD,          300 },
    { MOVE_CURVE_RIGHT,      700 },
};
#else //SLOW
static const ChoreoStep choreography[] = {
    { MOVE_FORWARD,1200 },
    { MOVE_CURVE_RIGHT,600 },
    { MOVE_FORWARD,800 },
    { MOVE_CURVE_LEFT,600 },
    { MOVE_FORWARD,800 },
    { MOVE_BRAKE_FLOURISH,0 },
    { MOVE_CURVE_LEFT,900 },
    { MOVE_FORWARD,900 },
    { MOVE_CURVE_RIGHT,700 },
    { MOVE_FORWARD,900 },
};
#endif

#define CHOREOGRAPHY_STEP_COUNT (sizeof(choreography) / sizeof(choreography[0]))

//Robot action to apply when related move selected
static void apply_move(ChoreoMove move){
    switch(move){
        case MOVE_FORWARD:      
            robot_move_forward();           
            break;
        case MOVE_CURVE_LEFT:   
            set_robot_action(ACTION_LEFT);  
            break;
        case MOVE_CURVE_RIGHT:  
            set_robot_action(ACTION_RIGHT); 
            break;
        case MOVE_PAUSE:        
            robot_stop();                   
            break;
        default:                
            robot_stop();                   
            break;
    }
}

// Turns briefly away from the detected obstacle then lets run_choreography() resume the interrupted move 
//This fucntion was meant to allow both robot to be on the dance floor without touching one another. Unfortunately , it stop working the way it did 
static void avoid_obstacle(){
    robot_stop();
    if((TA0R & 0x01) == 0){
        set_robot_action(ACTION_LEFT);
    } else {
        set_robot_action(ACTION_RIGHT);
    }
    __delay_cycles(AVOIDANCE_TURN_CYCLES);
    robot_stop();
}

// Holds "move" for "duration_ms"
static void hold_move_ms(ChoreoMove move, unsigned int duration_ms){
    unsigned int elapsed_ms = 0;

    apply_move(move);

    while(elapsed_ms < duration_ms){
        __delay_cycles(OBSTACLE_POLL_CYCLES);
        elapsed_ms += OBSTACLE_POLL_MS;

        ADC_Demarrer_conversion(3);
        distance_sensor_reading = ADC_Lire_resultat();

        if(distance_sensor_reading >= CHOREO_OBSTACLE_THRESHOLD){
            //avoid_obstacle(); ---> thus, it's no longer used
            apply_move(move); // resume the interrupted move
        }
    }
}

void run_choreography(void){
    unsigned long elapsed_ms = 0;
    unsigned int i = 0;

    robot_is_moving = 1;

    while(elapsed_seconds < CHOREO_DURATION_S && 
        elapsed_ms < CHOREO_DURATION_MS && 
        (elapsed_ms + choreography[i].duration_ms) < CHOREO_DURATION_MS
    ){
        ChoreoStep step = choreography[i];

        if(step.move == MOVE_BRAKE_FLOURISH){
            robot_brake();
            elapsed_ms += BRAKE_FLOURISH_DURATION_MS;
        } else {
            hold_move_ms(step.move, step.duration_ms);
            elapsed_ms += step.duration_ms;
        }

        i++;
        //loops when the time has not reached 90s yet
        if(i >= CHOREOGRAPHY_STEP_COUNT){
            i = 0; 
        }
    }

    robot_brake();
}
