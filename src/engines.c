#include <msp430.h>
#include "../headers/Afficheur.h"
#include "../headers/engines.h"

#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3
//-----------------------------------------------------------------------------------
//avancer d'une cellule (n'affecte que le parametre de position du robot dans le simulateur d'environnement)
//entrees
//  o_r         orientation robot
//sorties
//  x_r, y_r    coordonnees robot


// CONST WE HAVE TO REDEFINE --> NO LONGER WORKING IN A SIMULATED ENVIRONMENT:
/*#define DIM_LABY	    16
#define X_SORTIE        DIM_LABY-2
#define Y_SORTIE        DIM_LABY-1

//constantes pour simulateur d'environnement
#define ORI_DROITE      1
#define ORI_HAUT        2
#define ORI_GAUCHE      3
#define ORI_BAS         4
#define ACTION_ARRET    0
#define ACTION_GAUCHE   1
#define ACTION_AVANCE   2
#define ACTION_DROITE   3
*/

#define PERCENT_CONTROLE(percent) (percent*(TA1CCR0/100))

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


void action_robot(int action)
{
    switch(action)
    {
        case ACTION_AVANCE : // On favorise l'accélération car la roue folle est à l'arrière
            TA1CCR1 = PERCENT_CONTROLE(50); //activation roue gauche
            TA1CCR2 = PERCENT_CONTROLE(50); //activation roue droite
            P2OUT &= ~BIT1;	//sens avant roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            break;
        case ACTION_DROITE :
            TA1CCR1 = 2500; //activation roue gauche
            TA1CCR2 = 2500; //activation roue droite
            P2OUT |= BIT1;	//sens avant roue gauche (a completer)
            P2OUT &= ~BIT5;	//sens avant roue droite (a completer)
            break;
        case ACTION_GAUCHE :
            TA1CCR1 = 2500; //activation roue gauche
            TA1CCR2 = 2500; //activation roue droite
            P2OUT &= ~BIT1;	//sens avant roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite (a completer)
            break;
        case ACTION_ARRET :
            TA1CCR1 = 0; //activation roue gauche
            TA1CCR2 = 0; //activation roue droite
            P2OUT |= BIT1;	//sens avant roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite
            break;
		default :
            TA1CCR1 = 0; //activation roue gauche
            TA1CCR2 = 0; //activation roue droite
            P2OUT |= BIT1;	//sens avant roue gauche (a completer)
            P2OUT |= BIT5;	//sens avant roue droite
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
    action_robot(ACTION_GAUCHE);        //positionner les bits du controle moteur
	//a completer
}
//quart de tour a droite (n'affecte que le parametre de direction du robot dans le simulateur d'environnement)
//entrees
//  o_r     orientation robotvoid
void
robot_tourner_droite()
{
    action_robot(ACTION_DROITE);        //positionner les bits du controle moteur
}

void robot_arret(){
    action_robot(ACTION_ARRET);
}