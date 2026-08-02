#include "fan_state_machine.h"

constexpr unsigned long FAN_IDLE_TIMEOUT_MS = 30000UL;
constexpr float FAN_STOP_TEMPERATURE_C = 35.0f;

void fanInit(FanMachine &machine) {
  machine.state = FAN_OFF;
  machine.lastUvActiveMs = millis() - FAN_IDLE_TIMEOUT_MS;
  machine.needsOutput = true;
}

void fanUpdate(FanMachine &machine) {
  const unsigned long now = millis();

  if (safetyTrip) {
    machine.state = FAN_HIGH;
    fanDutyCycle = 255;
    machine.needsOutput = true;
    systemState.fanChanged = true;
    return;
  }

  if (uvLedEnabled) {
    machine.lastUvActiveMs = now;
    machine.state = FAN_MEDIUM;
    fanDutyCycle = 180;
  }
  else if (!junctionTempSensorFault && junctionTempC < FAN_STOP_TEMPERATURE_C && now - machine.lastUvActiveMs >= FAN_IDLE_TIMEOUT_MS) {
    machine.state = FAN_OFF;
    fanDutyCycle = 0;
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
