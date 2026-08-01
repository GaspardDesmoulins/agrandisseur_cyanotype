#ifndef MENU_STATE_MACHINE_H
#define MENU_STATE_MACHINE_H

#include "globals.h"

enum MenuState {
  MENU_STANDBY,
  MENU_EDIT
};

struct MenuMachine {
  MenuState state;
  uint8_t selectedItem;
  uint8_t editItem;
  bool needsRefresh;
  bool editing;
  bool forceRefresh;
  unsigned long lastRefreshMs;
  unsigned long lastTimerRefreshMs;
};

void menuInit(MenuMachine &machine);
void menuUpdate(MenuMachine &machine);
void menuOutput(MenuMachine &machine);

#endif
