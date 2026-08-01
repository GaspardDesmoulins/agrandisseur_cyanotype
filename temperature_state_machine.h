#ifndef TEMPERATURE_STATE_MACHINE_H
#define TEMPERATURE_STATE_MACHINE_H

#include "globals.h"

enum TemperatureState {
  TEMPERATURE_READING,
  TEMPERATURE_SENSOR_FAULT
};

struct TemperatureMachine {
  TemperatureState state;
  unsigned long lastReadMs;
};

void temperatureInit(TemperatureMachine &machine);
void temperatureUpdate(TemperatureMachine &machine);
void temperatureOutput(TemperatureMachine &machine);

#endif