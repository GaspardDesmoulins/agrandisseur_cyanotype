#ifndef EXPOSURE_STATE_MACHINE_H
#define EXPOSURE_STATE_MACHINE_H

#include "globals.h"

enum ExposureState {
  EXPOSURE_IDLE,
  EXPOSURE_RUNNING,
  EXPOSURE_PAUSED
};

struct ExposureMachine {
  ExposureState state;
  unsigned long exposureDurationMs;
  unsigned long exposureStartMs;
  unsigned long exposureElapsedMs;
  unsigned long lastTickMs;
  bool needsOutput;
  bool requestStart;
  bool requestPause;
};

void exposureInit(ExposureMachine &machine);
void exposureUpdate(ExposureMachine &machine);
void exposureOutput(ExposureMachine &machine);

#endif
