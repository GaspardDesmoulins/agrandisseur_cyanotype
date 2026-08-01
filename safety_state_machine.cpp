#include "safety_state_machine.h"

void safetyInit(SafetyMachine &machine) {
  machine.state = SAFETY_OK;
  machine.needsOutput = true;
}

void safetyUpdate(SafetyMachine &machine) {
  const bool trip = junctionTempSensorFault || junctionTempC >= maxJunctionTempC;

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
