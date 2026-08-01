#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <BitBang_LiquidCrystal_I2C.h>
#include "config.h"

extern LiquidCrystal_I2C lcd;
extern bool boutonPresse;
extern bool uvLedRequested;
extern bool uvLedEnabled;
extern float junctionTempC;
extern uint8_t fanDutyCycle;
extern bool safetyTrip;
extern bool servoControlEnabled;
extern int menuExposedSelection;
extern bool menuExposedAlarmActive;
extern int encoderRotationCount;
extern bool encoderButtonPressedEvent;
extern unsigned long exposureDurationMs;
extern unsigned long exposureElapsedMs;
extern bool exposureActive;
extern float maxJunctionTempC;
extern int servoSweepRadiusDeg;
extern int servoSweepPhaseDeg;

struct SystemState {
  bool encoderChanged;
  bool buttonChanged;
  bool relayChanged;
  bool fanChanged;
  bool safetyChanged;
};

extern SystemState systemState;

#endif
