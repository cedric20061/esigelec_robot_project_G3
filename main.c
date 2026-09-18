#include <msp430.h>


// Actif à 1
// LED1: P2.5
// LED2: P2.4
// LED3: P2.2
// LED4: P2.1

// Actif à 0
// SW1: P2.6
// SW2: P2.7
//  S2: P1.3

// Étapes:
// - 1: PxSEL
// - 2: I/O: PxDIR
// - 3: REN + OUT

int main(void)
{
    volatile unsigned int i;
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
    
    
    // Mode selection
    P1SEL &= ~(BIT3);
    P1SEL2 &= ~(BIT3);
    P2SEL &= ~(BIT1 | BIT2 | BIT4 | BIT5 );
    P2SEL2 &= ~(BIT1 | BIT2 | BIT4 | BIT5 );

    // IO direction
    P1DIR &= ~(BIT3);        // S2
    P2DIR &= ~(BIT6 | BIT7); // SW1-2
    P2DIR |= (BIT1 | BIT2 | BIT4 | BIT5); // LED1-4

    // pull res
    P1REN |= BIT3 ;         // S2
    P2REN |= (BIT6 | BIT7); // SW1-2
    // pull-up
    P1OUT |= BIT3 ;         // S2
    P2OUT |= (BIT6 | BIT7); // SW1-2

    // Default LED value
    P2OUT &= ~(BIT1 | BIT2 | BIT4 | BIT5); // LED1-4
    
    // P1DIR |= 0x01;                            // Set P1.0 to output direction

    volatile int result = 0;

    while(1)
    {
      if ((P1IN & BIT1) == 1)
      {
        P2OUT |= (BIT5|BIT4); /// Ici nous allons appeler les fonctions GAUCHE OU DROITE
        P2OUT &= ~(BIT2); // Ici nous la fonction Avancer est ajoutée on passe a 0 
      }
      else
      {
        P2OUT &= ~(BIT5|BIT4); /// Ici nous allons appler les fonctions GAUCHE OU DROITE ce qui passela sorie en stop 0
        P2OUT |= (BIT2); /// Ici nous avançons en passant le la sortie a 1
      }
  }
}