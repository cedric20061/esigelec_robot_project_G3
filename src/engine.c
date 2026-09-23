#include <msp430.h>
#include "../headers/Afficheur.h"
#include "../headers/engines.h"

// How many steps to spread the braking ramp-down over, and how long to
// wait between each step. Six steps of ~0.3s give a ~1.8s stop instead
// of the previous abrupt full-speed-to-zero cut.
#define BRAKE_STEPS 6
#define BRAKE_STEP_DELAY_CYCLES 300000 // ~0.3s at 1MHz

// How long to hold a pivot-turn action before stopping again
#define TURN_DURATION_CYCLES 500000 // ~0.5s at 1MHz

// Definitions for the shared state declared `extern` in engines.h
volatile char robot_is_moving = 0;
volatile long distance_traveled_mm = 0;

// Clamps a duty-cycle percentage to the valid [0, 100] range.
int clamp_percentage(int percent){
    if(percent < 0)   return 0;
    if(percent > 100) return 100;
    return percent;
}

// Configures the motor direction pins and Timer1 (PWM)
void engines_configs(){

    // Motor direction pins as GPIO outputs (A = left engine, B = right engine)
    P2SEL &= ~(BIT1 | BIT5);
    P2SEL2 &= ~(BIT1 | BIT5);

    P2DIR |= (BIT1 | BIT5);

    P2OUT |= (BIT1 | BIT5); // initial motor direction

    // P2.2 and P2.4 as Timer1 PWM outputs
    P2DIR |= (BIT2 | BIT4);
    P2SEL |= (BIT2 | BIT4);
    P2SEL2 &= ~(BIT2 | BIT4);

    TA1CTL = TASSEL_2 | MC_1 | ID_0; // Timer1: SMCLK, no divider, up mode
    TA1CCTL1 |= OUTMOD_7; // reset/set PWM mode on both compare outputs
    TA1CCTL2 |= OUTMOD_7;

    TA1CCR0 = 10000; // PWM period (100 kHz, under the 250 kHz limit from the datasheet)

    // Both motors stopped at startup
    TA1CCR1 = 0;
    TA1CCR2 = 0;
}

// Configures the two optocouplers as Port 2 interrupt inputs
void optocoupler_config(){
    // P2.0: 1st optocoupler
    P2SEL &= ~BIT0;
    P2SEL2 &= ~BIT0;
    P2DIR &= ~BIT0;

    P2IE |= BIT0;   // enable interrupt on P2.0
    P2IES |= BIT0;  // initial edge selection: 1 = falling edge
    P2IFG &= ~BIT0; // clear any pending flag before enabling

    // P2.3: 2nd optocoupler
    P2SEL &= ~BIT3;
    P2SEL2 &= ~BIT3;
    P2DIR &= ~BIT3;

    P2IE |= BIT3;
    P2IES |= BIT3;
    P2IFG &= ~BIT3; // clear any pending flag before enabling
}

// Timer0 configuration (drives the ~1s control-loop tick)
void timer_set()
{
    TA0CTL = TASSEL_2 | ID_3 | TAIE;
    TA0CCR0 = 62500; // ~every 1s
}

// Starts Timer0 in up/down mode
void timer_start()
{
    TA0CTL |= MC_3;
}

// Stops Timer0
void timer_reset()
{
    TA0CTL &= ~MC_3;
}

// Applies one of the robot's actions (direction + speed)
void set_robot_action(int action)
{
    switch(action)
    {
        case ACTION_FORWARD:
            // Favor acceleration since the caster wheel is at the back
            robot_is_moving = 1;
            P2OUT &= ~BIT1; // left wheel direction
            P2OUT |= BIT5;  // right wheel direction
            TA1CCR1 = PERCENT_CONTROL(95);
            TA1CCR2 = PERCENT_CONTROL(95);
            break;

        case ACTION_RIGHT:
            robot_is_moving = 0;
            P2OUT &= ~BIT1;
            P2OUT &= ~BIT5;
            TA1CCR1 = PERCENT_CONTROL(100);
            TA1CCR2 = PERCENT_CONTROL(60);
            break;

        case ACTION_LEFT:
            robot_is_moving = 0;
            P2OUT |= BIT1;
            P2OUT |= BIT5;
            TA1CCR1 = PERCENT_CONTROL(60);
            TA1CCR2 = PERCENT_CONTROL(100);
            break;

        case ACTION_STOP:
            robot_is_moving = 0;
            P2OUT &= ~BIT1;
            P2OUT |= BIT5;
            TA1CCR1 = PERCENT_CONTROL(0);
            TA1CCR2 = PERCENT_CONTROL(0);
            break;

        case ACTION_BRAKE: {
            // Smooth, progressive braking instead of the old 2-step ramp
            // that still ended in an abrupt jump straight to 0%. That
            // final cut is what caused the harsh stop: even after two
            // -25% steps, the last drop could still be large (e.g. 95%
            // -> 70% -> 45% -> 0% is a 45-point instant cut). Here we
            // ramp linearly from whatever the current speed is down to
            // exactly 0 over BRAKE_STEPS steps, so the last step is
            // always small.
            robot_is_moving = 0;
            P2OUT &= ~BIT1;
            P2OUT |= BIT5;

            int start_percent_left  = clamp_percentage(VALUE_TO_PERCENT(TA1CCR1));
            int start_percent_right = clamp_percentage(VALUE_TO_PERCENT(TA1CCR2));
            int step;

            for(step = 1; step <= BRAKE_STEPS; step++){
                int decrement_left  = (start_percent_left  * step) / BRAKE_STEPS;
                int decrement_right = (start_percent_right * step) / BRAKE_STEPS;
                TA1CCR1 = PERCENT_CONTROL(clamp_percentage(start_percent_left  - decrement_left));
                TA1CCR2 = PERCENT_CONTROL(clamp_percentage(start_percent_right - decrement_right));
                __delay_cycles(BRAKE_STEP_DELAY_CYCLES);
            }

            TA1CCR1 = 0;
            TA1CCR2 = 0;
            break;
        }

        default:
            robot_is_moving = 0;
            P2OUT &= ~BIT1;
            P2OUT |= BIT5;
            TA1CCR1 = PERCENT_CONTROL(0);
            TA1CCR2 = PERCENT_CONTROL(0);
            break;
    }
}

void robot_move_forward()
{
    set_robot_action(ACTION_FORWARD);
}

void robot_turn_left()
{
    set_robot_action(ACTION_STOP);
    set_robot_action(ACTION_LEFT);
    __delay_cycles(TURN_DURATION_CYCLES);
    set_robot_action(ACTION_STOP);
}

void robot_turn_right()
{
    set_robot_action(ACTION_STOP);
    set_robot_action(ACTION_RIGHT);
    __delay_cycles(TURN_DURATION_CYCLES);
    set_robot_action(ACTION_STOP);
}

void robot_stop(){
    set_robot_action(ACTION_STOP);
}

void robot_brake(){
    set_robot_action(ACTION_BRAKE);
}
