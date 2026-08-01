# agrandisseur_cyanotype

Ce projet Arduino est organisé autour de plusieurs machines à états.

## Structure
- Le sketch principal est [Ecran-module-KY-040.ino](Ecran-module-KY-040.ino)
- Les machines à états sont réparties dans des fichiers séparés
- Les variables partagées sont centralisées dans [globals.h](globals.h)

## Convention de nommage
- Les états sont nommés avec le préfixe de la machine, par exemple `MENU_HOME`, `RELAY_ON`, `FAN_HIGH`.
- Les variables exposées aux autres machines portent le suffixe `Exposed` dans [globals.h](globals.h), par exemple `menuExposedSelection`.
- Les pins matériels utilisent le préfixe `PIN_SM_` dans [config.h](config.h), par exemple `PIN_SM_RELAY` ou `PIN_SM_FAN`.
- Les aliases historiques `PIN_RELAY`, `PIN_FAN`, etc. sont conservés uniquement pour compatibilité avec le code existant.

## Rôle des fonctions
- `update()` observe les entrées et les états des autres machines, puis calcule le nouvel état interne.
- `output()` ne manipule que les sorties liées à sa propre machine.
- La machine du menu est la seule à exposer des variables partagées utiles aux autres machines.

## Boucle de fonctionnement
1. Les fonctions `update()` de chaque machine sont appelées dans un premier bloc.
2. Les fonctions `output()` de chaque machine sont appelées dans un second bloc.

Cette organisation permet de séparer la logique d’état de l’action sur les sorties.
