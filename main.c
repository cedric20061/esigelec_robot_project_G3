#include "../projet_robot/headers/ADC.h"
#include "../projet_robot/headers/Afficheur.h"
#include "../projet_robot/headers/engine.h"
#include <msp430.h>


volatile int nbr_front_roue1 = 0;
volatile int nbr_front_roue2 = 0;

#pragma vector=PORT2_VECTOR
__interrupt void octo_coupleur(void)
{
    if((P2IFG & BIT0) == BIT0){
        nbr_front_roue1++;
        P2IES &= ~BIT0; // Sophie : i think it should be P2IES ^= BIT0 instead to alteranate between rising & falling edges
        //that's what we did for the blinking LED ---> TBC
        if(nbr_front_roue1 == 48) P2IE &= ~BIT0;
        P2IFG &= ~BIT0;

    }

    if((P2IFG & BIT3) == BIT3){
        nbr_front_roue2++;
        P2IES &= ~BIT3; //same here
        if(nbr_front_roue2 == 48) P2IE &= ~BIT3;
        P2IFG &= ~BIT3;
    }
}

int main(void) {

  volatile unsigned int i;
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
  BCSCTL1= CALBC1_1MHZ;
  DCOCTL= CALDCO_1MHZ;

  engines_configs();
  octo_coupleur_reading_config();
  __enable_interrupt();

  ADC_init();
  Aff_Init();
  volatile int result = 0;

  
  while (1) {
    int val = 0;
  
    Aff_Efface();
    ADC_Demarrer_conversion(3);
    Aff_valeur(convert_Hex_Dec(ADC_Lire_resultat()));
    val = convert_Hex_Dec(ADC_Lire_resultat());

    __delay_cycles(200000);
    

    if (val >= 0x150) {
      //robot_tourner_droite();
      robot_arret();
      // a 0
    } else {
      robot_avancer();

    }
  }
      
}
