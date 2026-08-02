#ifndef FAN_STATE_MACHINE_H
#define FAN_STATE_MACHINE_H

#include "globals.h"

enum FanState {
  FAN_OFF,
  FAN_LOW,
  FAN_MEDIUM,
  FAN_HIGH
};

struct FanMachine {
  FanState state;
  unsigned long lastUvActiveMs;
  bool needsOutput;
};

void fanInit(FanMachine &machine);
void fanUpdate(FanMachine &machine);
void fanOutput(FanMachine &machine);

#endif
