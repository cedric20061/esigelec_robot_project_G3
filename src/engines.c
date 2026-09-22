#include <msp430.h>
#include "../headers/Afficheur.h"
#include "../headers/engines.h"

#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3

extern volatile char is_moving;

void engines_configs(){
    P2SEL &= ~(BIT1 | BIT5); // engine directions (A --> left engine and B--> Right engine)
    P2SEL2 &= ~(BIT1 | BIT5); // engine directions (A --> left engine and B--> Right engine)
    P2DIR |= (BIT1 | BIT5); // define engines directions to out

    P2OUT |= (BIT1 | BIT5);	// Initialement à l'arrêt

    P2DIR |= (BIT2 | BIT4);
    P2SEL |= (BIT2 | BIT4);
    P2SEL2 &= ~(BIT2 | BIT4);

    TA1CTL =TASSEL_2 | MC_1 | ID_2; // configuration des signaux PWM envoyé aux moteur pour leur activation 
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
    TA0CTL = TASSEL_2 | ID_2 | TAIE;
    TA0CCR0 = 50000; // toute les 100ms
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
    switch(action)
    {
        case ACTION_AVANCE : // On favorise l'accélération car la roue folle est à l'arrière
            is_moving = 1;
            P2OUT &= ~BIT1;	//sens arrière roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            TA1CCR1 = PERCENT_CONTROL(50); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROL(50); //activation roue droite
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
//quart de tour a gauche (n'affecte que le parametre de direction du robot dans le simulateur d'environnement)
//entrees
//  o_r     orientation robot
void
robot_tourner_gauche()
{
    action_robot(ACTION_ARRET);
    action_robot(ACTION_GAUCHE);        //positionner les bits du controle moteur
    __delay_cycles(500000);
    action_robot(ACTION_ARRET);
}
//quart de tour a droite (n'affecte que le parametre de direction du robot dans le simulateur d'environnement)
//entrees
//  o_r     orientation robotvoid
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