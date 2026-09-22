#include <msp430.h>
#include "./headers/engines.h"


volatile char nbr_front_roue1 = 0;
volatile char nbr_front_roue2 = 0;

volatile char distance_parcourue = 0;

extern volatile char is_moving;
volatile char error = 0;

#pragma vector=PORT2_VECTOR
__interrupt void octo_coupleur(void)
{
    if((P2IFG & BIT0) == BIT0){
        nbr_front_roue1++;
        P2IES ^= BIT0;
        P2IFG &= ~BIT0;

    }

    if((P2IFG & BIT3) == BIT3){
        nbr_front_roue2++;
        P2IES ^= BIT3;
        P2IFG &= ~BIT3;
    }
}

int limit(char val){
    if(val<0 || val>100) {
        return 50;
    }
    return val;
}
#pragma vector=TIMER0_A1_VECTOR
__interrupt void enslavement(void)
{
    if(TA0CTL & TAIFG == TAIFG){
         if(is_moving){
            if(nbr_front_roue1 > nbr_front_roue2){
                error = nbr_front_roue1-nbr_front_roue2;
                TA1CCR1 = PERCENT_CONTROL(limit((VALUE_TO_PERCENT(TA1CCR1))));
                TA1CCR2 = PERCENT_CONTROL(limit((VALUE_TO_PERCENT(TA1CCR2))));
                distance_parcourue = (nbr_front_roue1 * 11)/21;
            }else {
                error = nbr_front_roue2-nbr_front_roue1;
                TA1CCR1 = PERCENT_CONTROL(limit(VALUE_TO_PERCENT(TA1CCR1)));
                TA1CCR2 = PERCENT_CONTROL(limit(VALUE_TO_PERCENT(TA1CCR2)));
                distance_parcourue = (nbr_front_roue1 * 11)/21;
            }
        }
        TA0CTL &= ~TAIFG;
    }
}


void afficher_nbr_front(){
    Aff_Efface();
    Aff_valeur(convert_Hex_Dec(nbr_front_roue1));
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

    BCSCTL1= CALBC1_1MHZ;
    DCOCTL= CALDCO_1MHZ;
    is_moving = 0;
    engines_configs();
    octo_coupleur_reading_config();
    timer_set();
    timer_start();
    Aff_Init();
    __enable_interrupt();

    afficher_nbr_front();
    robot_tourner_droite();
    afficher_nbr_front();
    robot_avancer();
    __delay_cycles(10000000);
   afficher_nbr_front();
    robot_tourner_gauche();
    robot_avancer();
    __delay_cycles(10000000); 
    robot_arret();
    afficher_nbr_front();
    while(1);

}
