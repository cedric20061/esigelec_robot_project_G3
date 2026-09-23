#include <msp430.h>
#include "./headers/engines.h"
#include "./headers/actions.h"
#include "./headers/choreography.h"

#define LIGHT_THRESHOLD 30 // light level threshold to switch on the headlights

// Encoder edge counts for each wheel since startup.
// Only used within this file (by the Port2 and Timer0 interrupts and by
// display_encoder_count()), so they're `static` instead of being
// visible project-wide like before.
static volatile int encoder_ticks_left = 0;
static volatile int encoder_ticks_right = 0;

// Elapsed seconds while the front sensor sees no close obstacle
// (see the TIMER0_A1_VECTOR interrupt below)
volatile int elapsed_seconds = 0;

void display_encoder_count(){
    Aff_Efface();
    Aff_valeur(convert_Hex_Dec(encoder_ticks_left));
}

// Reads the light sensor and drives the LaunchPad LEDs (LED1 & LED2 -> green & red)
void headlight_power(){
    ADC_Demarrer_conversion(2);
    int light_level = ADC_Lire_resultat()/10;
    if(light_level <= LIGHT_THRESHOLD) P1OUT |= (BIT0 | BIT6);
    else P1OUT &= ~(BIT0 | BIT6);
}

void headlight_config(){
    P1SEL &= ~(BIT0 | BIT6);
    P1SEL2 &= ~(BIT0 | BIT6);

    P1DIR |= (BIT0 | BIT6);

    P1OUT &= ~(BIT0 | BIT6);
}

void display_time(int seconds){
  Aff_Efface();
  Aff_valeur(convert_Hex_Dec(seconds));
}

// Interrupt routine for the two optocouplers (wheel encoders)
#pragma vector=PORT2_VECTOR
__interrupt void optocoupler_isr(void)
{
    if((P2IFG & BIT0) == BIT0){
        encoder_ticks_left++;
        P2IES ^= BIT0; // toggle edge so we catch both edges of the slot
        P2IFG &= ~BIT0;
    }

    if((P2IFG & BIT3) == BIT3){
        encoder_ticks_right++;
        P2IES ^= BIT3;
        P2IFG &= ~BIT3;
    }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void speed_regulation_isr(void)
{
    if((TA0CTL & TAIFG == TAIFG)){

        if(robot_is_moving){
            // Ticks accumulated by each wheel SINCE THE LAST TIME this
            // interrupt ran, not since power-on. The previous version
            // compared the lifetime encoder counts, so a difference
            // that built up early on never went away even once both
            // wheels were spinning at exactly the same speed - the
            // correction stayed permanently biased. Comparing deltas
            // over the last control period is what "current speed
            // difference" actually needs to mean.
            static int prev_ticks_left = 0;
            static int prev_ticks_right = 0;

            int delta_left  = encoder_ticks_left  - prev_ticks_left;
            int delta_right = encoder_ticks_right - prev_ticks_right;
            prev_ticks_left  = encoder_ticks_left;
            prev_ticks_right = encoder_ticks_right;

            int speed_error_ticks;
            if(delta_left > delta_right){
                speed_error_ticks = delta_left - delta_right;
                TA1CCR1 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR1) - SPEED_CORRECTION_GAIN * speed_error_ticks));
                TA1CCR2 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR2) + SPEED_CORRECTION_GAIN * speed_error_ticks));
            } else {
                speed_error_ticks = delta_right - delta_left;
                TA1CCR1 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR1) + SPEED_CORRECTION_GAIN * speed_error_ticks));
                TA1CCR2 = PERCENT_CONTROL(clamp_percentage(VALUE_TO_PERCENT(TA1CCR2) - SPEED_CORRECTION_GAIN * speed_error_ticks));
            }

            // Distance is now accumulated incrementally from the tick
            // DELTA (averaging both wheels) instead of being recomputed
            // from scratch as (total_ticks * 11) every tick. The old
            // formula had two problems: it used magic numbers (11, 21)
            // instead of the wheel-geometry macros, and once
            // encoder_ticks_left got large, `encoder_ticks_left * 11`
            // could overflow a 16-bit int well before the robot
            // finished its run.
            int delta_avg_ticks = (delta_left + delta_right) / 2;
            distance_traveled_mm += ((long)delta_avg_ticks * WHEEL_PERIMETER_MM) / ENCODER_SLOTS_PER_REV;
        }

        if(distance_sensor_reading < STOP_DISTANCE_THRESHOLD){
            elapsed_seconds++;
        }

        // display_encoder_count();
        display_time(elapsed_seconds);
        headlight_power();

        TA0CTL &= ~TAIFG;
    }
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;

    robot_is_moving = 0;
    distance_traveled_mm = 0;
    elapsed_seconds = 0;

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
}
