<<<<<<< HEAD
#include "./headers/choreography.h"
#include "./headers/engines.h"
#include "./headers/homologation.h"
=======
>>>>>>> origin/master
#include <msp430.h>
#include "./headers/engines.h"
#include "./headers/actions.h"

#define SEUIL_LUM 30 //lifgt threshold to switch on the headlights

//store the number of encoder edges detected for each wheel
volatile int nbr_front_roue1 = 0;
volatile int nbr_front_roue2 = 0;

extern volatile char distance_parcourue; // only goes til 127 --> should shange this one too en int 
extern volatile char is_moving;
extern volatile int capt;
volatile int sec = 0;

//store the difference between the two encoder counters
volatile char error = 0;

void afficheTime(char sec){
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(sec));
}

//define the interrupt routine for the two optocouplers
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

//keep a pwm %age inside the valid range
int limit(char val){
  //check whether the percentage is outside the 0–100% range
    if(val<0 || val>100) {
        return 50;
    }
    return val;
}

//read the light sensor and control the launchpad LEDs (led1 & 2 --> green &red)
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

<<<<<<< HEAD
#define LIGHT_THRESHOLD 30 // light level threshold to switch on the headlights

// Encoder edge counts for each wheel since startup.
// Only used within this file (by the Port2 and Timer0 interrupts and by
// display_encoder_count(), so they're `static` instead of being
// visible project-wide like before.
extern volatile int encoder_ticks_left;
extern volatile int encoder_ticks_right;


// Elapsed seconds while the front sensor sees no close obstacle
// (see the TIMER0_A1_VECTOR interrupt below)
extern volatile int distance_sensor_reading;
extern volatile int elapsed_seconds;

extern volatile char robot_is_moving;

void display_encoder_count() {
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(encoder_ticks_left));
}


// Reads the light sensor and drives the LaunchPad LEDs (LED1 & LED2 -> green
// & red)
void headlight_power() {
  ADC_Demarrer_conversion(2);
  int light_level = ADC_Lire_resultat() / 10;
  if (light_level <= LIGHT_THRESHOLD)
    P1OUT |= (BIT0 | BIT6);
  else
    P1OUT &= ~(BIT0 | BIT6);
}

void headlight_config() {
  P1SEL &= ~(BIT0 | BIT6);
  P1SEL2 &= ~(BIT0 | BIT6);

  P1DIR |= (BIT0 | BIT6);

  P1OUT &= ~(BIT0 | BIT6);
}

void display_time(int seconds) {
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(seconds));
}

// Interrupt routine for the two optocouplers (wheel encoders)
#pragma vector = PORT2_VECTOR
__interrupt void optocoupler_isr(void) {
  if ((P2IFG & BIT0) == BIT0) {
    encoder_ticks_left++;
    P2IES ^= BIT0; // toggle edge so we catch both edges of the slot
    P2IFG &= ~BIT0;
  }

    if((P2IFG & BIT3) == BIT3){
        encoder_ticks_right++;
        total_encoder_tick++;
        P2IES ^= BIT3;
        P2IFG &= ~BIT3;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void speed_regulation_isr(void) {
  if ((TA0CTL & TAIFG) == TAIFG) {

        if(robot_is_moving){
            static int prev_ticks_left = 0;
            static int prev_ticks_right = 0;

            int delta_left  = encoder_ticks_left  - prev_ticks_left;
            int delta_right = encoder_ticks_right - prev_ticks_right;
            prev_ticks_left  = encoder_ticks_left;
            prev_ticks_right = encoder_ticks_right;

            int speed_error_ticks;
            if(encoder_ticks_left > encoder_ticks_right){
                speed_error_ticks = encoder_ticks_left - encoder_ticks_right;
                TA1CCR1 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR1) - SPEED_CORRECTION_GAIN * speed_error_ticks));
                TA1CCR2 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR2) + SPEED_CORRECTION_GAIN * speed_error_ticks));
            } else {
                speed_error_ticks = encoder_ticks_right - encoder_ticks_left;
                TA1CCR1 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR1) + SPEED_CORRECTION_GAIN * speed_error_ticks));
                TA1CCR2 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR2) - SPEED_CORRECTION_GAIN * speed_error_ticks));
            }   
        }

        if((distance_sensor_reading < STOP_DISTANCE_THRESHOLD) && (encoder_ticks_left*5 <= HOMOLOGATION_DISTANCE_MM)){
            elapsed_seconds++;
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
  total_encoder_tick =0;

  engines_configs();
  headlight_config();
  optocoupler_config();
  timer_set();
  timer_start();
  ADC_init();
  Aff_Init();
  __enable_interrupt();
  
  homologation();
  // run_choreography();
=======
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
>>>>>>> origin/master
}
