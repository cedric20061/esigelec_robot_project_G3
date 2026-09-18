#include <msp430.h>
#include "./headers/engines.h"


volatile int nbr_front_roue1 = 0;
volatile int nbr_front_roue2 = 0;

#pragma vector=PORT2_VECTOR
__interrupt void octo_coupleur(void)
{
    if((P2IFG & BIT0) == BIT0){
        nbr_front_roue1++;
        P2IES &= ~BIT0;
        if(nbr_front_roue1 == 48) P2IE &= ~BIT0;
        P2IFG &= ~BIT0;

    }

    if((P2IFG & BIT3) == BIT3){
        nbr_front_roue2++;
        P2IES &= ~BIT3;
        if(nbr_front_roue2 == 48) P2IE &= ~BIT3;
        P2IFG &= ~BIT3;
    }
}

void afficher_nbr_front(){
    Aff_Init();
    Aff_Efface();
    Aff_valeur(convert_Hex_Dec(nbr_front_roue1));
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

  BCSCTL1= CALBC1_1MHZ;
  DCOCTL= CALDCO_1MHZ;

  engines_configs();
  octo_coupleur_reading_config();
  __enable_interrupt();
  robot_avancer();
  __delay_cycles(5000000);
  robot_arret();
  afficher_nbr_front();
  while(1);
}
