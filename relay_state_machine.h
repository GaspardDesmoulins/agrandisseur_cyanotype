#ifndef RELAY_STATE_MACHINE_H
#define RELAY_STATE_MACHINE_H

#include "globals.h"

enum RelayState {
  RELAY_OFF,
  RELAY_ON,
  RELAY_ERROR
};

struct RelayMachine {
  RelayState state;
  bool requestPending;
  bool needsOutput;
};

void relayInit(RelayMachine &machine);
void relayUpdate(RelayMachine &machine);
void relayOutput(RelayMachine &machine);

#endif
