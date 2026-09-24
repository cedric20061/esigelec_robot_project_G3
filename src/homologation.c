#include <msp430.h>
#include "../headers/ADC.h"
#include "../headers/engines.h"
#include "../headers/Afficheur.h"
#include "../headers/homologation.h"


// Time limit for the homologation thing
#define HOMOLOGATION_TIMEOUT_SECONDS 10

extern volatile int distance_sensor_reading;
extern volatile char robot_is_moving;
extern volatile int elapsed_seconds;

extern volatile int encoder_ticks_left;
extern volatile int encoder_ticks_right;

//Distance travelled by the robot
static volatile int distance_traveled_mm;

// Checks if an obstacle is detected --> does it twice to be sure and precise
static int obstacle_detected(){
    static int previous_reading_was_high = 0;

    int raw = ADC_Lire_resultat();
    distance_sensor_reading = convert_Hex_Dec(raw);

    int reading_is_high = (raw >= STOP_DISTANCE_THRESHOLD);
    int confirmed = reading_is_high && previous_reading_was_high;
    previous_reading_was_high = reading_is_high;

    return confirmed;
}

//Homologation : en gros the robot must move forward, straight, for 1.30 m in less/eq 10s (avec de la marge)
void homologation(){
    robot_is_moving = 1;
    distance_sensor_reading = 0;
    elapsed_seconds = 0;
    long delta = 0;
    // Stops as soon as the distance is covered 
    while(distance_traveled_mm <= HOMOLOGATION_DISTANCE_MM){
          //&& elapsed_seconds < HOMOLOGATION_TIMEOUT_SECONDS){
        ADC_Demarrer_conversion(3);
        if(obstacle_detected()){
            if(robot_is_moving){
                robot_stop();
            }
        } else {
            robot_move_forward();
        }
        //Calculates the distance travelled
        distance_traveled_mm = encoder_ticks_left*5;

    }

    robot_stop();
}
