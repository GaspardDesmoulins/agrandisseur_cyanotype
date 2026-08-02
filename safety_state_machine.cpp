#include "safety_state_machine.h"

constexpr float SAFETY_RESET_HYSTERESIS_C = 5.0f;

void safetyInit(SafetyMachine &machine) {
  machine.state = SAFETY_OK;
  machine.needsOutput = true;
}

void safetyUpdate(SafetyMachine &machine) {
  const float resetTemperatureC = maxJunctionTempC - SAFETY_RESET_HYSTERESIS_C;
  const bool trip = junctionTempSensorFault ||
                    junctionTempC >= maxJunctionTempC ||
                    (machine.state == SAFETY_TRIP && junctionTempC > resetTemperatureC);

  if (trip) {
    machine.state = SAFETY_TRIP;
    safetyTrip = true;
    machine.needsOutput = true;
    systemState.safetyChanged = true;
  }
  else {
    machine.state = SAFETY_OK;
    safetyTrip = false;
    machine.needsOutput = true;
    systemState.safetyChanged = true;
  }
}

void safetyOutput(SafetyMachine &machine) {
  if (!machine.needsOutput) {
    return;
  }

  // La machine de sécurité n'a pas de sortie matérielle dédiée.
  // Elle publie uniquement un état de sécurité observé par les autres machines.
  machine.needsOutput = false;
}
