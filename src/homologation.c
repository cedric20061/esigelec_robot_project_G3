#include <msp430.h>
#include "../headers/ADC.h"
#include "../headers/engines.h"
#include "../headers/Afficheur.h"
#include "../headers/homologation.h"

// Total distance for the qualifying run, in millimeters (1.30m, as
// required by the contest rules)
#define HOMOLOGATION_DISTANCE_MM 1300

// Hard time limit for the qualifying run (contest rules: 10s, paused
// while an obstacle is being presented - see main.c's Timer0 ISR, which
// only increments elapsed_seconds while no obstacle is detected).
#define HOMOLOGATION_TIMEOUT_SECONDS 10

volatile int distance_sensor_reading = 0;

// Reports "obstacle present" only once the raw ADC reading has been
// above the threshold on two consecutive samples. A single noisy
// sample used to be enough to flip the stop/go decision - this adds a
// tiny debounce so a borderline reading near the threshold doesn't make
// the robot flicker between stopping and restarting.
static int obstacle_detected(){
    static int previous_reading_was_high = 0;

    int raw = ADC_Lire_resultat();
    // Kept raw on purpose: convert_Hex_Dec() is for the 7-segment
    // display, not for threshold comparisons (see the previous review).
    distance_sensor_reading = raw;

    int reading_is_high = (raw >= STOP_DISTANCE_THRESHOLD);
    int confirmed = reading_is_high && previous_reading_was_high;
    previous_reading_was_high = reading_is_high;
    return confirmed;
}

void homologation(){
    robot_is_moving = 1;
    distance_sensor_reading = 0;
    elapsed_seconds = 0;

    // Stops as soon as EITHER the distance is covered OR the 10s budget
    // (paused while an obstacle is present) runs out.
    while(distance_traveled_mm <= HOMOLOGATION_DISTANCE_MM
          && elapsed_seconds < HOMOLOGATION_TIMEOUT_SECONDS){

        ADC_Demarrer_conversion(3);

        if(obstacle_detected()){
            if(robot_is_moving){
                robot_stop();
            }
        } else {
            robot_move_forward();
        }
    }

    robot_stop();
}
