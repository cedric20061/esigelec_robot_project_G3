#include <msp430.h>
#include "../headers/Afficheur.h"
#include "../headers/engines.h"

#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3
#define ACTION_FREINER  4

extern volatile char is_moving;

void engines_configs(){
    P2SEL &= ~(BIT1 | BIT5); // engine directions (A --> left engine and B--> Right engine)
    P2SEL2 &= ~(BIT1 | BIT5); // engine directions (A --> left engine and B--> Right engine)
    P2DIR |= (BIT1 | BIT5); // define engines directions to out

    P2OUT |= (BIT1 | BIT5);	// Initialement à l'arrêt

    P2DIR |= (BIT2 | BIT4);
    P2SEL |= (BIT2 | BIT4);
    P2SEL2 &= ~(BIT2 | BIT4);

    TA1CTL =TASSEL_2 | MC_1 | ID_0; // configuration des signaux PWM envoyé aux moteur pour leur activation 
    TA1CCTL1 |= OUTMOD_7;
    TA1CCTL2 |= OUTMOD_7;

    TA1CCR0 = 10000; // fréquence de 100kHz < 250kHz (voir doc)

    TA1CCR1 = 0; //Robot à l'arrêt
    TA1CCR2 = 0; //Robot à l'arrêt
}

void octo_coupleur_reading_config(){
    P2SEL &= ~BIT0;
    P2SEL2 &= ~BIT0;
    P2DIR &= ~BIT0;          
    P2IE |= BIT0;
    P2IES |= BIT0;

    P2SEL &= ~BIT3;
    P2SEL2 &= ~BIT3;
    P2DIR &= ~BIT3;          
    P2IE |= BIT3;
    P2IES |= BIT3;
}

void timer_set()
{
    TA0CTL = TASSEL_2 | ID_3 | TAIE;
    TA0CCR0 = 62500; // toute les 1s
}

void timer_start()
{
    TA0CTL |= MC_3;
}

void timer_reset()
{
    TA0CTL &= ~MC_3;
}


void action_robot(int action)
{
    char i =0;
    switch(action)
    {
        case ACTION_AVANCE : // On favorise l'accélération car la roue folle est à l'arrière
            is_moving = 1;
            P2OUT &= ~BIT1;	//sens arrière roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(95); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(95); //activation roue droite
            break;
        case ACTION_DROITE :
            is_moving = 0;
            P2OUT &= ~BIT1;	//sens avant roue gauche (a completer)
            P2OUT &= ~BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(100); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(60); //activation roue droite
            
            break;
        case ACTION_GAUCHE :
            is_moving = 0;
            P2OUT |= BIT1;	//sens avant roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(60); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(100); //activation roue droite
            break;
        case ACTION_ARRET :
            is_moving = 0;
            P2OUT &= ~BIT1;	//sens arrière roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(0); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(0); //activation roue droite
            break;
        case ACTION_FREINER :
            is_moving = 0;
            P2OUT &= ~BIT1;	//sens arrière roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            for(i=0; i<2; i++){
                TA1CCR1 = PERCENT_CONTROL(VALUE_TO_PERCENT(TA1CCR1) - 25);
                TA1CCR2 = PERCENT_CONTROL(VALUE_TO_PERCENT(TA1CCR2) - 25);
                __delay_cycles(1000000);
            }
            TA1CCR1 = PERCENT_CONTROL(0); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(0); //activation roue droite
            break;
		default :
            is_moving = 0;
            P2OUT &= ~BIT1;	//sens arrière roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(0); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(0); //activation roue droite
            break;
    }
}

void
robot_avancer()
{
    action_robot(ACTION_AVANCE);        //positionner les bits du controle moteur
}

void
robot_tourner_gauche()
{
    action_robot(ACTION_ARRET);
    action_robot(ACTION_GAUCHE);        //positionner les bits du controle moteur
    __delay_cycles(500000);
    action_robot(ACTION_ARRET);
}

void
robot_tourner_droite()
{
    action_robot(ACTION_ARRET);
    action_robot(ACTION_DROITE);        //positionner les bits du controle moteur
    __delay_cycles(500000);
    action_robot(ACTION_ARRET);
}

void robot_arret(){
    action_robot(ACTION_ARRET);
}

void robot_freiner(){
    action_robot(ACTION_FREINER);
}