#include "relay_state_machine.h"

void relayInit(RelayMachine &machine) {
  machine.state = RELAY_OFF;
  machine.requestPending = false;
  machine.needsOutput = true;
}

void relayUpdate(RelayMachine &machine) {
  if (safetyTrip) {
    machine.state = RELAY_ERROR;
    uvLedEnabled = false;
    machine.requestPending = false;
    machine.needsOutput = true;
    systemState.relayChanged = true;
    return;
  }

  if (uvLedRequested && !uvLedEnabled) {
    machine.state = RELAY_ON;
    uvLedEnabled = true;
    machine.requestPending = true;
    machine.needsOutput = true;
    systemState.relayChanged = true;
  }
  else if (!uvLedRequested && uvLedEnabled) {
    machine.state = RELAY_OFF;
    uvLedEnabled = false;
    machine.requestPending = true;
    machine.needsOutput = true;
    systemState.relayChanged = true;
  }
  else {
    machine.state = uvLedEnabled ? RELAY_ON : RELAY_OFF;
  }
}

void relayOutput(RelayMachine &machine) {
  if (!machine.needsOutput) {
    return;
  }

  digitalWrite(PIN_SM_RELAY, (machine.state == RELAY_ON) ? HIGH : LOW);
  machine.needsOutput = false;
}
