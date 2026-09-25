## Robot Danseur – MSP430G2553 – Groupe 3

## Membres
- Lucas BRISSAUD  
- Nathanel DJEUTCHEU  
- Amadou SOW  
- Gerald GUIDI
- Safietou DIOP  

## Résumé du projet
Le robot doit :
- parcourir 1,30 m en ligne droite, s'arrêter devant un obstacle, puis reprendre (homologation) 
- exécuter une chorégraphie de 90 s avec des déplacements et des figures, en évitant les obstacles de façon réactive (ajouter un deuxièeme robot à condition que les deux ne se touchent pas)

Le code est découpé en trois parties :
- gestion des moteurs et de l'asservissement 
- épreuve d'homologation 
- chorégraphie du concours

## Fichiers principaux
### `main.c`
Point d'entrée du programme. Configure les timers, les interruptions et l'asservissement de vitesse. Lance soit `homologation()`, soit `run_choreography()` selon la phase et le robot (SLOW OU FAST)

### `engines.c` / `engines.h`
Gère les moteurs :
- configuration du PWM et des pins de direction 
- fonctions pour avancer, tourner, s'arrêter, freiner 
- calcul de la distance parcourue à partir des encodeurs
Contient aussi les variables partagées `robot_is_moving` et `distance_traveled_mm`

### `homologation.c` / `homologation.h`
Implémente l'épreuve d'homologation :
- lecture du capteur de distance avant 
- détection d'obstacle avec anti-rebond 
- arrêt et reprise du robot 
- gestion du chrono (10 s max, pause pendant l'obstacle)
Contient les variables partagées `distance_sensor_reading` et `elapsed_seconds`

### `choreography.c` / `choreography.h`
Gère la danse de 90 s :
- définition du format des mouvements (`ChoreoStep` avec type et durée) 
- table de mouvements modifiable sans changer la logique 
- exécution de la chorégraphie avec évitement réactif d'obstacles

Permet de compiler deux versions (rôles FAST / SLOW) avec des tables de mouvements différentes, mais le même code d'exécution

## Variables partagées
Chaque variable est définie une seule fois dans un `.c` et déclarée `extern` dans le header correspondant :

 Variable --> Définie dans -->  Rôle 
- `robot_is_moving` --> `engines.c` --> vrai pendant un déplacement (asservissement actif) 
- `distance_traveled_mm` --> `engines.c` --> distance totale parcourue, en mm 
- `distance_sensor_reading` --> `actions.c` --> dernière lecture du capteur de distance avant 
- `elapsed_seconds` --> `main.c` --> secondes écoulées pendant l'homologation (pause pendant un obstacle) 

## Notes de compilation
Pour changer de phase, changer la constante IS_DANCING (1 pour la chorégraphie et 0 pour l'homologation) dans `main.c` et faire attention à modifier les parametre pour le robot correspondant:

### Parameters --> Homologation pour que ça marche :
STOP_DISTANCE_TRESHOLD = 0x150 -> homologation.h
SPEED_CORRECTION_GAIN = 2 --> engines.h
PERCENT_PWM_SLOW_CHORE 94 --->engines.c
 
### Parameters --> chorégraphie:
CHOREO_OBSTACLE_TRESHOLD = 0x050 -->choregraphie.h
FAST: PERCENT_PWM_FAST_CHORE
ROBOT_ROLE = ROBOT_ROLE_FAST
SLOW: PERCENT_PWM_SLOW_CHORE
ROBOT_ROLE = ROBOT_ROLE_SLOW
TA1CCR0 = 20000
ID_3