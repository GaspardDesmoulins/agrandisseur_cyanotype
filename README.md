# Agrandisseur cyanotype

Contrôleur Arduino en cours de développement pour intégrer un petit projecteur UV dans un agrandisseur KROKUS 44. Son objectif est de réaliser des agrandissements de négatifs argentiques sur papier sensibilisé au cyanotype, afin de produire des tirages sans procédé de chambre noire traditionnel.

Le firmware pilote l'exposition UV, son balayage mécanique, le refroidissement et l'interface opérateur.

## Fonctions actuelles

- Pilotage d'un projecteur UV par relais.
- Minuterie d'exposition réglable de 5 secondes à 60 minutes, initialisée à 15 minutes. Le réglage progresse par pas de 5 secondes sous une minute, de 30 secondes entre 1 et 5 minutes, de 2 minutes entre 5 et 15 minutes, puis de 5 minutes au-delà.
- Déplacement circulaire du projecteur avec deux servomoteurs (panoramique et inclinaison) pendant l'exposition pour répartir la lumière.
- Ventilation à trois niveaux : faible hors exposition, moyenne pendant l'exposition et maximale en cas de sécurité thermique.
- Interface utilisateur avec encodeur rotatif KY-040 et écran LCD I2C de 20 x 4 caractères.
- Coupure du relais UV lorsque la température de jonction atteint le seuil configuré.

## Utilisation de l'interface

Un appui sur le bouton de l'encodeur bascule entre le mode sélection (`STBY`) et le mode édition (`EDIT`).

En mode sélection, la rotation choisit l'un des quatre éléments :

1. État de l'exposition : démarrer ou mettre en pause la minuterie.
2. Durée d'exposition restante.
3. Rayon du balayage des servomoteurs.
4. Température maximale de jonction.

En mode édition, la rotation modifie l'élément choisi. Le minuteur démarre ou se met en pause en tournant l'encodeur lorsque l'élément « État » est sélectionné.

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
- [uv_servo_state_machine.cpp](uv_servo_state_machine.cpp) pilote le balayage du projecteur.
- [temperature_state_machine.cpp](temperature_state_machine.cpp) relève le thermocouple via la MAX6675 toutes les 250 ms.
- [safety_state_machine.cpp](safety_state_machine.cpp) publie l'état de sécurité thermique.

Les données communes sont déclarées dans [globals.h](globals.h) et la configuration matérielle dans [config.h](config.h). À chaque tour de boucle, toutes les fonctions `update()` calculent les nouveaux états avant que les fonctions `output()` appliquent les sorties matérielles.

## Dépendances

- Arduino AVR core, avec la cible Arduino Duemilanove (`arduino:avr:diecimila`).
- Bibliothèque `BitBang_LiquidCrystal_I2C` pour l'écran LCD.

## État du projet

La température de jonction est relevée par un thermocouple avec une MAX6675. Si le thermocouple est déconnecté ou si la sonde ne répond pas, le relais UV reste coupé, le ventilateur passe au maximum et l'écran affiche `ERR` pour la température de jonction.

Le projet est en développement. Les valeurs d'exposition, la géométrie du balayage, le câblage et les protections doivent être testés avec le projecteur UV réellement installé dans le KROKUS 44 avant tout usage prolongé.
