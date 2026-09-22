#include <msp430.h>
#include "./headers/engines.h"
#include "./headers/actions.h"

#define SEUIL_LUM 30

volatile int nbr_front_roue1 = 0;
volatile int nbr_front_roue2 = 0;

extern volatile char distance_parcourue;
extern volatile char is_moving;
extern volatile int capt;
volatile char sec = 0;

volatile char error = 0;

void afficheTime(char sec){
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(sec));
}

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

void headlight_power(){
    ADC_Demarrer_conversion(2);
    int lum = ADC_Lire_resultat()/10;
    if(lum <= SEUIL_LUM) P1OUT |= (BIT0 | BIT6);
    else P1OUT &= ~(BIT0 | BIT6);
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void enslavement(void)
{
    if(TA0CTL & TAIFG == TAIFG){
         if(is_moving){
            if(nbr_front_roue1 > nbr_front_roue2){
                error = nbr_front_roue1-nbr_front_roue2;
                TA1CCR1 = PERCENT_CONTROL(limit((VALUE_TO_PERCENT(TA1CCR1) - K_PROPORTION*error)));
                TA1CCR2 = PERCENT_CONTROL(limit((VALUE_TO_PERCENT(TA1CCR2) + K_PROPORTION*error)));
            }else {
                error = nbr_front_roue2-nbr_front_roue1;
                TA1CCR1 = PERCENT_CONTROL(limit(VALUE_TO_PERCENT(TA1CCR1) + K_PROPORTION*error));
                TA1CCR2 = PERCENT_CONTROL(limit(VALUE_TO_PERCENT(TA1CCR2) - K_PROPORTION*error));
            }
            distance_parcourue = (nbr_front_roue1 * 11)/21;
        }
        if(capt < 0x150){
            sec++;
        }
        // afficheTime(sec);
        afficher_nbr_front();

        headlight_power();
        TA0CTL &= ~TAIFG;
    }
}

void afficher_nbr_front(){
    Aff_Efface();
    Aff_valeur(convert_Hex_Dec(nbr_front_roue1));
}

void headlight_config(){
    P1SEL &= ~(BIT0 | BIT6);
    P1SEL2 &= ~(BIT0 | BIT6);

    P1DIR |= (BIT0 | BIT6);

    P1OUT &= ~(BIT0 | BIT6);
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

    BCSCTL1= CALBC1_1MHZ;
    DCOCTL= CALDCO_1MHZ;
    is_moving = 0;
    distance_parcourue = 0;
    sec=0;
    engines_configs();
    headlight_config();
    octo_coupleur_reading_config();
    timer_set();
    timer_start();
    ADC_init();
    Aff_Init();
    __enable_interrupt();
    
    homologation();
//     afficher_nbr_front();
//     robot_tourner_droite();
//     afficher_nbr_front();
//     robot_avancer();
//     __delay_cycles(10000000);
//    afficher_nbr_front();
//     robot_tourner_gauche();
//     robot_avancer();
//     __delay_cycles(10000000); 
//     robot_arret();
//     afficher_nbr_front();
}
