# Robot Danseur — MSP430G2553

Documentation de l'architecture logicielle : ce que fait chaque fichier, comment fonctionne l'asservissement, et comment fonctionne la chorégraphie.

## Arborescence

```
main.c                   point d'entrée, configuration, ISR (interruptions)
headers/
  engines.h               constantes moteur, actions de bas niveau, état partagé
  actions.h                homologation : seuils, capteur, chrono
  choreography.h            format des mouvements de danse
src/
  engines.c                configuration matérielle + exécution des actions moteur
  actions.c                logique de l'épreuve d'homologation
  choreography.c            logique du concours (danse de 90s)
```

## État partagé entre fichiers

Trois variables globales sont utilisées par plusieurs fichiers :

| Variable | Définie dans | Déclarée `extern` dans | Rôle |
|---|---|---|---|
| `robot_is_moving` | `engines.c` | `engines.h` | vrai pendant un déplacement en ligne droite (active l'asservissement) |
| `distance_traveled_mm` | `engines.c` | `engines.h` | distance cumulée parcourue, en mm |
| `distance_sensor_reading` | `actions.c` | `actions.h` | dernière lecture brute du capteur de distance avant |
| `elapsed_seconds` | `main.c` | `actions.h` | secondes écoulées pendant l'homologation, mis en pause pendant un obstacle |

Règle suivie partout : **une seule définition** de chaque variable (dans un `.c`), et un `extern` dans le header correspondant pour que les autres fichiers la partagent — plutôt que de la déclarer directement dans le header (ce qui donnait à chaque fichier sa propre copie et causait le bug historique mentionné dans `engines.h`).

## `engines.c` / `engines.h` — commande moteur

- `engines_configs()` configure Timer1 en PWM (fréquence, période `TA1CCR0`) et les pins de direction moteur.
- `set_robot_action(action)` est le point d'entrée unique pour piloter les moteurs : il fixe les bits de direction (`P2OUT`) et le rapport cyclique PWM (`TA1CCR1`/`TA1CCR2`) selon l'action demandée (`ACTION_FORWARD`, `ACTION_LEFT`, `ACTION_RIGHT`, `ACTION_STOP`, `ACTION_BRAKE`).
- `clamp_percentage(int percent)` ramène une valeur de rapport cyclique dans `[0, 100]`. Elle prend un `int` (pas un `char`) car les formules de correction de vitesse peuvent dépasser largement `-128..127` — un `char` ferait un dépassement silencieux qui pourrait retomber par hasard dans une plage valide.
- `ACTION_BRAKE` ne coupe pas le PWM d'un coup : il fait descendre le rapport cyclique en `BRAKE_STEPS` paliers, du niveau actuel jusqu'à 0 exactement, pour un arrêt progressif plutôt qu'une saccade.

**Marche arrière / rotation sur place** : pas encore implémentées. Il manque la confirmation de la combinaison de bits `BIT1`/`BIT5` qui inverse chaque roue sur le driver moteur — à tester prudemment (roues en l'air, PWM faible) avant de l'ajouter.

## Asservissement de vitesse (dans `main.c`, ISR `TIMER0_A1_VECTOR`)

Toutes les ~1s (période de Timer0), pendant un déplacement (`robot_is_moving`) :

1. On calcule le **delta** de tics d'encodeur de chaque roue depuis le dernier passage de l'interruption (pas le total depuis le démarrage — comparer les totaux cumulés biaiserait la correction en permanence dès qu'un petit écart s'installe au début).
2. La roue la plus rapide voit son PWM réduit, la plus lente voit son PWM augmenté, proportionnellement à l'écart (`SPEED_CORRECTION_GAIN * écart`).
3. Le résultat passe par `clamp_percentage()` pour rester dans `[0, 100]`.

`SPEED_CORRECTION_GAIN` n'est pas une constante universelle : elle dépend de la période de la boucle de contrôle, de la fréquence PWM et de la résolution de l'encodeur. À retuner à la main en observant si les roues « oscillent » (elles se corrigent trop fort dans un sens puis dans l'autre → réduire le gain).

## Distance parcourue

Calculée de façon incrémentale à chaque tic de l'ISR, à partir de la moyenne des deltas des deux roues :

```
distance_traveled_mm += (delta_moyen_tics * WHEEL_PERIMETER_MM) / ENCODER_SLOTS_PER_REV
```

- Accumulée dans un `long` pour ne pas déborder sur les longs trajets.
- Calcul en **millimètres** plutôt qu'en centimètres : la division entière tronque, et l'erreur relative introduite est plus petite sur une unité plus fine.
- `WHEEL_RADIUS_MM` est une estimation — la mesurer précisément sur le robot réel est plus important que la précision du calcul lui-même pour respecter la tolérance de ±5cm de l'homologation.

## `actions.c` / `actions.h` — épreuve d'homologation

Correspondance avec le règlement du concours :

| Règle | Implémentation |
|---|---|
| 1.30m ± 5cm | `HOMOLOGATION_DISTANCE_MM = 1300`, comparé à `distance_traveled_mm` |
| Obstacle → arrêt puis reprise | `obstacle_detected()` lit l'ADC et compare à `STOP_DISTANCE_THRESHOLD` ; la boucle appelle `robot_stop()` ou `robot_move_forward()` selon l'état à chaque itération |
| Anti-rebond capteur | `obstacle_detected()` n'annonce un obstacle qu'après **deux lectures consécutives** au-dessus du seuil, pour éviter qu'une lecture bruitée isolée fasse clignoter le robot entre arrêt/marche |
| Durée max 10s, pause pendant l'obstacle | `elapsed_seconds` (incrémenté dans l'ISR de `main.c`, seulement quand pas d'obstacle détecté) est comparé à `HOMOLOGATION_TIMEOUT_SECONDS = 10` dans la condition de la boucle `while` |
| Aucune roue ne traverse la ligne centrale | **Non garanti par le logiciel** : aucun capteur latéral n'existe. Seul un bon alignement mécanique de départ et un gain d'asservissement bien calé peuvent limiter la dérive. |

`STOP_DISTANCE_THRESHOLD` (dans `actions.h`) doit être calibré en testant physiquement à 5cm puis à 15cm d'un obstacle, pour choisir une valeur qui déclenche de façon fiable entre les deux.

## `choreography.c` / `choreography.h` — routine du concours (90s)

Conçu pour être modifiable **sans toucher à la logique**, seulement à la table de mouvements.

### Le format

```c
typedef struct {
    ChoreoMove move;        // MOVE_FORWARD, MOVE_CURVE_LEFT, MOVE_CURVE_RIGHT, MOVE_PAUSE, MOVE_BRAKE_FLOURISH
    unsigned int duration_ms;
} ChoreoStep;
```

La chorégraphie elle-même est un tableau `static const ChoreoStep choreography[] = { ... };` — c'est la **seule chose à éditer** pour changer la danse (caler les durées sur le tempo de la musique).

### Deux robots, un seul moteur de danse

`#define ROBOT_ROLE ROBOT_ROLE_FAST` (ou `ROBOT_ROLE_SLOW`) sélectionne, à la compilation, quelle table est utilisée. Les deux robots partagent exactement le même code d'exécution (`run_choreography()`, `apply_move()`, `hold_move_ms()`) — seule la table de mouvements diffère entre les deux builds.

### Ajouter un nouveau mouvement

1. Ajouter une valeur dans `enum ChoreoMove` (`choreography.h`)
2. Ajouter un `case` correspondant dans `apply_move()` (`choreography.c`)
3. L'utiliser dans une table

`run_choreography()` (la boucle qui gère le budget de 90s) n'a jamais besoin d'être modifiée.

### Évitement réactif

Pendant chaque mouvement, `hold_move_ms()` interroge le capteur avant toutes les `OBSTACLE_POLL_MS` (50ms par défaut). Si quelque chose s'approche :

1. `avoid_obstacle()` arrête le robot et tourne brièvement (côté choisi en alternance, via le compteur libre de Timer0 comme source pseudo-aléatoire, pour que deux robots qui se font face n'insistent pas toujours dans le même sens).
2. Le mouvement interrompu **reprend automatiquement** ensuite — aucune étape de la chorégraphie n'est perdue ou sautée.

### Limites connues (matériel actuel)

- **Le capteur ne regarde que devant.** Une collision par le côté ou l'arrière n'est pas détectée. L'alternance de sens en évitement réduit le risque de blocage frontal répété entre les deux robots, mais ne le résout pas complètement.
- **Rien ne détecte les bords de la piste (2m×2m).** Aucun capteur ne regarde le sol ou les côtés. Deux pistes possibles :
  - *Recommandée* : capteurs de réflectance orientés vers le sol (même principe que le capteur de luminosité des phares), détectant une bordure contrastée collée au bord de la piste.
  - *Filet de sécurité logiciel seul* : odométrie (position x/y/cap estimée à partir des deltas d'encodeurs) pour faire revenir le robot vers le centre en approchant d'une limite virtuelle — approximatif car ça dérive dans le temps, faisable avec le matériel actuel si besoin.

## Notes de compilation

- Les chemins d'inclusion (`./headers/...` depuis `main.c`, `../headers/...` depuis `src/*.c`) supposent que `main.c` est à la racine du projet et que `src/` et `headers/` sont des dossiers frères. Garder cette disposition ou adapter les chemins si le projet CCS est organisé différemment.
- `main()` appelle soit `homologation()` soit `run_choreography()` selon la phase (une ligne à commenter/décommenter dans `main.c`).
