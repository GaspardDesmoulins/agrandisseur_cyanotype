#include "exposure_state_machine.h"

constexpr unsigned long EXPOSURE_STEP_MS = 1000UL;
constexpr unsigned long DEFAULT_EXPOSURE_MS = 900000UL;
constexpr unsigned long MIN_EXPOSURE_MS = 5000UL;
constexpr unsigned long MAX_EXPOSURE_MS = 3600000UL;

void exposureInit(ExposureMachine &machine) {
  machine.state = EXPOSURE_IDLE;
  machine.exposureDurationMs = DEFAULT_EXPOSURE_MS;
  machine.exposureStartMs = 0;
  machine.exposureElapsedMs = 0;
  machine.lastTickMs = millis();
  machine.needsOutput = true;
  machine.requestStart = false;
  machine.requestPause = false;
}

void exposureUpdate(ExposureMachine &machine) {
  const unsigned long now = millis();

  // Keep the shared globals synchronized with the state machine so the LCD and
  // the rest of the system all use the same timer values.
  machine.exposureDurationMs = exposureDurationMs;
  machine.exposureElapsedMs = exposureElapsedMs;

  if (safetyTrip) {
    if (machine.state == EXPOSURE_RUNNING) {
      machine.state = EXPOSURE_SAFETY_PAUSED;
      machine.exposureElapsedMs += now - machine.lastTickMs;
      machine.lastTickMs = now;
      machine.needsOutput = true;
    }

    uvLedRequested = false;
    servoControlEnabled = false;
    exposureActive = false;
    exposureElapsedMs = machine.exposureElapsedMs;
    exposureDurationMs = machine.exposureDurationMs;
    return;
  }

  if (machine.state == EXPOSURE_SAFETY_PAUSED) {
    machine.state = EXPOSURE_RUNNING;
    machine.lastTickMs = now;
    machine.needsOutput = true;
    uvLedRequested = true;
    servoControlEnabled = true;
    exposureActive = true;
    exposureElapsedMs = machine.exposureElapsedMs;
    exposureDurationMs = machine.exposureDurationMs;
    return;
  }

  if (machine.requestStart) {
    machine.requestStart = false;
    machine.state = EXPOSURE_RUNNING;
    machine.exposureStartMs = now;
    machine.exposureElapsedMs = 0;
    machine.lastTickMs = now;
    machine.needsOutput = true;
    uvLedRequested = true;
    servoControlEnabled = true;
    exposureActive = true;
    exposureElapsedMs = machine.exposureElapsedMs;
    exposureDurationMs = machine.exposureDurationMs;
    return;
  }

  if (machine.requestPause) {
    machine.requestPause = false;
    if (machine.state == EXPOSURE_RUNNING) {
      machine.state = EXPOSURE_PAUSED;
      machine.exposureElapsedMs += now - machine.lastTickMs;
      machine.lastTickMs = now;
      uvLedRequested = false;
      servoControlEnabled = false;
    }
    machine.needsOutput = true;
    exposureActive = (machine.state == EXPOSURE_RUNNING);
    exposureElapsedMs = machine.exposureElapsedMs;
    exposureDurationMs = machine.exposureDurationMs;
    return;
  }

  if (machine.state == EXPOSURE_RUNNING) {
    const unsigned long elapsedSinceLastTick = now - machine.lastTickMs;
    machine.exposureElapsedMs += elapsedSinceLastTick;
    machine.lastTickMs = now;

    if (machine.exposureElapsedMs >= machine.exposureDurationMs) {
      machine.state = EXPOSURE_IDLE;
      machine.exposureElapsedMs = machine.exposureDurationMs;
      uvLedRequested = false;
      servoControlEnabled = false;
      machine.needsOutput = true;
      exposureActive = false;
      exposureElapsedMs = machine.exposureElapsedMs;
      exposureDurationMs = machine.exposureDurationMs;
      return;
    }
  }

  if (machine.state == EXPOSURE_IDLE) {
    machine.exposureElapsedMs = 0;
    exposureActive = false;
  }
  else if (machine.state == EXPOSURE_PAUSED) {
    exposureActive = false;
  }

  exposureElapsedMs = machine.exposureElapsedMs;
  exposureDurationMs = machine.exposureDurationMs;
  machine.needsOutput = true;
}

void exposureOutput(ExposureMachine &machine) {
  if (!machine.needsOutput) {
    return;
  }

  if (machine.state == EXPOSURE_RUNNING) {
    if (machine.exposureElapsedMs >= machine.exposureDurationMs) {
      machine.exposureElapsedMs = machine.exposureDurationMs;
    }
  }

  machine.needsOutput = false;
}
