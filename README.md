# Agrandisseur cyanotype

Contrôleur Arduino en cours de développement pour intégrer un petit projecteur UV dans un agrandisseur KROKUS 44. Son objectif est de réaliser des agrandissements de négatifs argentiques sur papier sensibilisé au cyanotype, afin de produire des tirages sans procédé de chambre noire traditionnel.

Le firmware pilote l'exposition UV, son balayage mécanique, le refroidissement et l'interface opérateur.

## Fonctions actuelles

- Pilotage d'un projecteur UV par relais.
- Minuterie d'exposition réglable de 5 secondes à 24 heures, initialisée à 15 minutes. Le réglage progresse par pas de 5 secondes sous une minute, de 30 secondes entre 1 et 5 minutes, de 2 minutes entre 5 et 15 minutes, puis de 5 minutes au-delà.
- Trois modes de positionnement du projecteur pendant l'exposition : ellipse réglable, angles panoramique/inclinaison manuels avec gestion de préréglages, ou scénario de préréglages. Les PWM des servos sont coupés hors exposition.
- Ventilation à trois niveaux : faible hors exposition, moyenne pendant l'exposition et maximale en cas de sécurité thermique.
- Interface utilisateur avec encodeur rotatif KY-040 et écran LCD I2C de 20 x 4 caractères.
- Coupure du relais UV lorsque la température de jonction atteint le seuil configuré.

## Utilisation de l'interface

La ligne 0 affiche `EXPO: OFF` hors exposition et le temps restant pendant l'exposition. La ligne 1 affiche la température et l'état de sécurité. Les lignes 2 et 3 présentent une page de deux options.

En mode sélection, la rotation déplace le curseur et change automatiquement de page toutes les deux options. Un appui ouvre ou ferme le mode édition (`EDIT`) ; la rotation modifie alors la valeur sélectionnée. Les actions `Exposition` et `Sauver preset` s'exécutent directement par appui.

Les premières pages donnent accès à l'exposition, sa durée et au mode servo. Les options suivantes dépendent du mode choisi :

1. `ELLIPSE` : amplitudes maximales `Pan` et `Tilt`, vitesse du mouvement, puis affichage en direct des angles appliqués.
2. `MANUEL` : réglage des angles `Pan` et `Tilt`, choix d'un des cinq préréglages, réglage de sa durée et sauvegarde de la position manuelle. Le choix d'un préréglage charge ses angles dans la position manuelle pour pouvoir les ajuster avant de les enregistrer.
3. `SCENARIO` : lecture cyclique des cinq préréglages, en démarrant au premier lors de chaque exposition et en passant au suivant après sa durée sauvegardée.

La température maximale de jonction reste accessible sur la dernière page. Les cinq préréglages et leurs durées sont enregistrés dans l'EEPROM interne lors de l'action `Sauver preset` et sont donc conservés après extinction. Une signature et une version permettent de réinitialiser automatiquement cette zone lors de la première utilisation ou après un changement de format.

## Câblage actuellement défini

La carte cible est une **Arduino Duemilanove**.

| Fonction | Broche Arduino |
| --- | ---: |
| Encodeur KY-040 : CLK | 4 |
| Encodeur KY-040 : DT | 2 |
| Encodeur KY-040 : bouton | 3 |
| Relais du projecteur UV | 5 |
| Ventilateur (PWM) | 6 |
| Servo panoramique | 10 |
| Servo d'inclinaison | 11 |
| LCD I2C : SDA | 8 |
| LCD I2C : SCL | 9 |
| MAX6675 : SO | 12 |
| MAX6675 : CS | 7 |
| MAX6675 : SCK | 13 |

L'adresse LCD configurée est `0x27`.

## Architecture du firmware

Le sketch principal est [agrandisseur_cyanotype.ino](agrandisseur_cyanotype.ino). Chaque fonction est regroupée dans une machine à états dédiée :

- [encoder_state_machine.cpp](encoder_state_machine.cpp) lit et filtre l'encodeur KY-040.
- [menu_state_machine.cpp](menu_state_machine.cpp) gère le menu et l'affichage LCD.
- [exposure_state_machine.cpp](exposure_state_machine.cpp) gère le minuteur et les demandes d'exposition.
- [relay_state_machine.cpp](relay_state_machine.cpp) commande le relais UV.
- [fan_state_machine.cpp](fan_state_machine.cpp) ajuste la ventilation.
- [uv_servo_state_machine.cpp](uv_servo_state_machine.cpp) pilote le mouvement elliptique du projecteur.
- [temperature_state_machine.cpp](temperature_state_machine.cpp) relève le thermocouple via la MAX6675 toutes les 250 ms.
- [safety_state_machine.cpp](safety_state_machine.cpp) publie l'état de sécurité thermique.

Les données communes sont déclarées dans [globals.h](globals.h) et la configuration matérielle dans [config.h](config.h). À chaque tour de boucle, toutes les fonctions `update()` calculent les nouveaux états avant que les fonctions `output()` appliquent les sorties matérielles.

### Cycle des machines à états

Chaque sous-système possède une structure qui conserve son état propre et reçoit successivement un appel à `update()` puis à `output()`. La première phase lit les entrées et les données partagées, applique les transitions d'état et prépare les demandes de sortie. La seconde phase propage les changements vers le matériel lorsqu'ils sont nécessaires.

Le sketch exécute d'abord toutes les mises à jour, dans un ordre où les entrées et la sécurité sont disponibles avant les actionneurs. Il exécute ensuite toutes les sorties. Une demande issue du menu, telle que `requestStart` ou `requestPause`, est ainsi consommée par la machine d'exposition au tour suivant, sans que le menu commande directement le relais ou les servos.

Ce découpage garde les responsabilités séparées : le menu produit des intentions, la machine d'exposition autorise ou suspend l'exposition, la sécurité peut l'interrompre, et les machines de relais, ventilation et servos appliquent leur propre comportement. Les données transverses nécessaires à ces décisions sont regroupées dans [globals.h](globals.h), tandis que les contraintes mécaniques et matérielles restent dans [config.h](config.h).

### Rendu différentiel de l'écran LCD

La machine de menu maintient deux tampons de `20 x 4` caractères : `displayTarget` contient l'écran souhaité et `displayRendered` mémorise le dernier caractère envoyé au LCD. `buildDisplayTarget()` reconstruit d'abord les quatre lignes à partir de l'état courant, sans écrire sur le bus I2C.

`outputNextChangedCharacter()` compare ensuite les deux tampons dans l'ordre de lecture de l'écran. À chaque passage dans la boucle, elle écrit au plus un caractère différent et met à jour `displayRendered`. Les rafraîchissements sont donc progressifs : cela évite un effacement complet de l'écran, limite les transferts I2C du LCD bit-bang et laisse la boucle disponible pour l'encodeur, la sécurité et les servos.

Le tampon cible est reconstruit lors d'une interaction, d'un rafraîchissement forcé ou au moins une fois par seconde. Sur la page d'état du mode `ELLIPSE`, il est reconstruit à chaque tour de boucle afin de suivre les angles appliqués en mouvement ; seuls les caractères ayant effectivement changé sont néanmoins transmis.

## Dépendances

- Arduino AVR core, avec la cible Arduino Duemilanove (`arduino:avr:diecimila`).
- Bibliothèque `BitBang_LiquidCrystal_I2C` pour l'écran LCD.

## Compilation

Depuis le dossier du projet, la commande suivante compile le sketch avec le compilateur embarqué dans l'IDE Arduino sous Windows :

```powershell
& "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile --fqbn arduino:avr:diecimila .
```

Le dossier contenant les exécutables Arduino peut également être trouvé à l'emplacement:
"D:\Program Files (x86)\Arduino\Arduino IDE\"

Les limites mécaniques, valeurs par défaut de l'ellipse et nombre de préréglages sont regroupés dans [config.h](config.h). La vitesse de l'ellipse est réglable de 10 à 500 ms par pas de 10 ms.

## État du projet

La température de jonction est relevée par un thermocouple avec une MAX6675. Si le thermocouple est déconnecté ou si la sonde ne répond pas, le relais UV reste coupé, le ventilateur passe au maximum et l'écran affiche `ERR` pour la température de jonction.

Le projet est en développement. Les valeurs d'exposition, la géométrie du balayage, le câblage et les protections doivent être testés avec le projecteur UV réellement installé dans le KROKUS 44 avant tout usage prolongé.
