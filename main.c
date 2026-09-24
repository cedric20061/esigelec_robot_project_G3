#include "./headers/choreography.h"
#include "./headers/engines.h"
#include "./headers/homologation.h"
#include <msp430.h>

#define SEUIL_LUM 30 //lifgt threshold to switch on the headlights
#define LIGHT_THRESHOLD 30 // light level threshold to switch on the headlights

#define IS_DANCING 1
// Encoder edge counts for each wheel since startup.
extern volatile int encoder_ticks_left;
extern volatile int encoder_ticks_right;

// Elapsed seconds while the front sensor sees no close obstacle
// (see the TIMER0_A1_VECTOR interrupt below)
extern volatile int distance_sensor_reading;
extern volatile int elapsed_seconds;
extern volatile char robot_is_moving;

//read the light sensor and control the launchpad LEDs (led1 & 2 --> green &red)
void headlight_power(){
    ADC_Demarrer_conversion(2);
    int lum = ADC_Lire_resultat()/10;
    if(lum <= SEUIL_LUM) P1OUT |= (BIT0 | BIT6);
    else P1OUT &= ~(BIT0 | BIT6);
}

void headlight_config(){
    P1SEL &= ~(BIT0 | BIT6);
    P1SEL2 &= ~(BIT0 | BIT6);

    P1DIR |= (BIT0 | BIT6);

    P1OUT &= ~(BIT0 | BIT6);
}



void display_encoder_count() {
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(encoder_ticks_left));
}

void display_time(int seconds) {
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(seconds));
}

// Interrupt routine for the two optocouplers (wheel encoders)
#pragma vector = PORT2_VECTOR
__interrupt void optocoupler_isr(void) {

    if((P2IFG & BIT3) == BIT3){
        encoder_ticks_right++;
        P2IES ^= BIT3;
        P2IFG &= ~BIT3;
    }

    if ((P2IFG & BIT0) == BIT0) {
        encoder_ticks_left++;
        P2IES ^= BIT0; // toggle edge so we catch both edges of the slot
        P2IFG &= ~BIT0;
    }

}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void speed_regulation_isr(void) {
  if ((TA0CTL & TAIFG) == TAIFG) {
        if(IS_DANCING){
            if(elapsed_seconds <= CHOREO_DURATION_S){
                elapsed_seconds++;
            }
        }else {
            if((distance_sensor_reading < STOP_DISTANCE_THRESHOLD) &&
                (encoder_ticks_left*5 <= HOMOLOGATION_DISTANCE_MM)
            ){
                elapsed_seconds++;
            }
        }
        // display_encoder_count();
        display_time(elapsed_seconds);
        headlight_power();
        TA0CTL &= ~TAIFG;
    }
}

int main(void) {
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    robot_is_moving = 0;
    distance_sensor_reading =0;
    elapsed_seconds = 0;
    encoder_ticks_left = 0;
    encoder_ticks_right = 0;

    engines_configs();
    headlight_config();
    optocoupler_config();
    timer_set();
    timer_start();
    ADC_init();
    Aff_Init();
    __enable_interrupt();

    if(IS_DANCING){
        run_choreography();
    }else {
        // Chore parameters: 
        //      FAST: PERCENT_PWM_FAST_CHORE
        //            ROBOT_ROLE = ROBOT_ROLE_FAST
        //      SLOW: PERCENT_PWM_SLOW_CHORE
        //            ROBOT_ROLE = ROBOT_ROLE_SLOW
        //            TA1CCR0 = 20000
        homologation();
    }
}
