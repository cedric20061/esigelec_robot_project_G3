#include <msp430.h>
#include "../headers/ADC.h"
#include "../headers/engines.h"
#include "../headers/Afficheur.h"

extern volatile char distance_parcourue;
extern volatile char is_moving;
extern volatile int capt;
void homologation(){
    is_moving =1;
    capt = 0;
    while(distance_parcourue <= 130){
        ADC_Demarrer_conversion(3);
        capt = convert_Hex_Dec(ADC_Lire_resultat());
        if(capt >= 0x150 && is_moving) {
            robot_arret();
        }
        if(capt < 0x150) {
            robot_avancer();
        }
    }
    robot_arret();
}

void choreography(){
    while(1){
        
    }
}