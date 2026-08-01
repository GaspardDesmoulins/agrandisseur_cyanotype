#ifndef SAFETY_STATE_MACHINE_H
#define SAFETY_STATE_MACHINE_H

#include "globals.h"

enum SafetyState {
  SAFETY_OK,
  SAFETY_TRIP
};

struct SafetyMachine {
  SafetyState state;
  bool needsOutput;
};

void safetyInit(SafetyMachine &machine);
void safetyUpdate(SafetyMachine &machine);
void safetyOutput(SafetyMachine &machine);

#endif
