void valeur_capteurs_env(int x_r, int y_r, int o_r)
{
    int x=x_r, y=y_r;
    int capt_ir; 
    P1IN=0;		//tous les bits a 0
    // switch(o_r)
    // {
    //     case ORI_DROITE :   capt_us=LABY[y-1][x]; capt_ir=LABY[y][x+1]; capt_ligne=LABY[y+1][x]; break;
    //     case ORI_HAUT :     capt_us=LABY[y][x-1]; capt_ir=LABY[y-1][x]; capt_ligne=LABY[y][x+1]; break;
    //     case ORI_GAUCHE :   capt_us=LABY[y+1][x]; capt_ir=LABY[y][x-1]; capt_ligne=LABY[y-1][x]; break;
    //     case ORI_BAS :      capt_us=LABY[y][x+1]; capt_ir=LABY[y+1][x]; capt_ligne=LABY[y][x-1]; break;
    //     default :       break;
    // }

    if(capt_ir){
      engine  
    }
    P1IN |= BIT1;


}

void lecture_capteurs(int *capt_ir)
{
    *capt_ir=0;
    //Capteur de distance à infra-rouge
    if((P1IN & BIT1) == BIT1)     
        *capt_ir=1;
    }




