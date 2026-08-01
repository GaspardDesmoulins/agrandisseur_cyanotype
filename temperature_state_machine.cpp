#include "temperature_state_machine.h"

constexpr unsigned long MAX6675_SAMPLE_INTERVAL_MS = 250UL;
constexpr uint16_t MAX6675_THERMOCOUPLE_OPEN_MASK = 0x0004;
constexpr float MAX6675_TEMPERATURE_STEP_C = 0.25f;

uint16_t readMax6675Raw() {
  uint16_t rawValue = 0;

  digitalWrite(PIN_SM_MAX6675_CS, LOW);
  delayMicroseconds(1);

  for (uint8_t bitIndex = 0; bitIndex < 16; ++bitIndex) {
    digitalWrite(PIN_SM_MAX6675_SCK, LOW);
    delayMicroseconds(1);
    rawValue = static_cast<uint16_t>((rawValue << 1) | digitalRead(PIN_SM_MAX6675_SO));
    digitalWrite(PIN_SM_MAX6675_SCK, HIGH);
    delayMicroseconds(1);
  }

  digitalWrite(PIN_SM_MAX6675_CS, HIGH);
  return rawValue;
}

void temperatureInit(TemperatureMachine &machine) {
  pinMode(PIN_SM_MAX6675_SO, INPUT);
  pinMode(PIN_SM_MAX6675_CS, OUTPUT);
  pinMode(PIN_SM_MAX6675_SCK, OUTPUT);
  digitalWrite(PIN_SM_MAX6675_CS, HIGH);
  digitalWrite(PIN_SM_MAX6675_SCK, HIGH);

  machine.state = TEMPERATURE_SENSOR_FAULT;
  machine.lastReadMs = millis() - MAX6675_SAMPLE_INTERVAL_MS;
}

void temperatureUpdate(TemperatureMachine &machine) {
  const unsigned long now = millis();
  if (now - machine.lastReadMs < MAX6675_SAMPLE_INTERVAL_MS) {
    return;
  }

  machine.lastReadMs = now;
  const uint16_t rawValue = readMax6675Raw();

  if ((rawValue & MAX6675_THERMOCOUPLE_OPEN_MASK) != 0U) {
    machine.state = TEMPERATURE_SENSOR_FAULT;
    junctionTempSensorFault = true;
    return;
  }

  machine.state = TEMPERATURE_READING;
  junctionTempC = static_cast<float>(rawValue >> 3) * MAX6675_TEMPERATURE_STEP_C;
  junctionTempSensorFault = false;
}

void temperatureOutput(TemperatureMachine &machine) {
  (void)machine;
}