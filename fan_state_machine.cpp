#include "fan_state_machine.h"

void fanInit(FanMachine &machine) {
  machine.state = FAN_OFF;
  machine.needsOutput = true;
}

void fanUpdate(FanMachine &machine) {
  if (safetyTrip) {
    machine.state = FAN_HIGH;
    fanDutyCycle = 255;
    machine.needsOutput = true;
    systemState.fanChanged = true;
    return;
  }

  if (uvLedEnabled) {
    machine.state = FAN_MEDIUM;
    fanDutyCycle = 180;
  }
  else {
    machine.state = FAN_LOW;
    fanDutyCycle = 90;
  }

  machine.needsOutput = true;
  systemState.fanChanged = true;
}

void fanOutput(FanMachine &machine) {
  if (!machine.needsOutput) {
    return;
  }

  analogWrite(PIN_SM_FAN, fanDutyCycle);
  machine.needsOutput = false;
}
