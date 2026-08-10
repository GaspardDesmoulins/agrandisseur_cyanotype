#include "globals.h"
#include "menu_state_machine.h"
#include "encoder_state_machine.h"
#include "relay_state_machine.h"
#include "safety_state_machine.h"
#include "fan_state_machine.h"
#include "uv_servo_state_machine.h"
#include "exposure_state_machine.h"
#include "temperature_state_machine.h"

MenuMachine menuMachine;
EncoderMachine encoderMachine;
RelayMachine relayMachine;
SafetyMachine safetyMachine;
FanMachine fanMachine;
UvServoMachine uvServoMachine;
ExposureMachine exposureMachine;
TemperatureMachine temperatureMachine;

void setup()
{
	pinMode(PIN_SM_ENCODER_CLK, INPUT_PULLUP);
	pinMode(PIN_SM_ENCODER_DT, INPUT_PULLUP);
	pinMode(PIN_SM_ENCODER_SW, INPUT_PULLUP);
	pinMode(PIN_SM_RELAY, OUTPUT);
	pinMode(PIN_SM_FAN, OUTPUT);

	lcd.begin();
	lcd.clear();

	menuInit(menuMachine);
	encoderInit(encoderMachine);
	relayInit(relayMachine);
	safetyInit(safetyMachine);
	fanInit(fanMachine);
	uvServoInit(uvServoMachine);
	exposureInit(exposureMachine);
	temperatureInit(temperatureMachine);
}

void loop()
{
	encoderUpdate(encoderMachine);
	temperatureUpdate(temperatureMachine);
	safetyUpdate(safetyMachine);
	relayUpdate(relayMachine);
	fanUpdate(fanMachine);
	uvServoUpdate(uvServoMachine);
	exposureUpdate(exposureMachine);
	menuUpdate(menuMachine);

	encoderOutput(encoderMachine);
	safetyOutput(safetyMachine);
	relayOutput(relayMachine);
	fanOutput(fanMachine);
	uvServoOutput(uvServoMachine);
	exposureOutput(exposureMachine);
	temperatureOutput(temperatureMachine);
	menuOutput(menuMachine);
}
