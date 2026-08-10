#ifndef UV_SERVO_STATE_MACHINE_H
#define UV_SERVO_STATE_MACHINE_H

#include "globals.h"
#include <Servo.h>

enum ServoExposureMode
{
	SERVO_MODE_SWEEP,
	SERVO_MODE_MANUAL,
	SERVO_MODE_PRESET
};

enum UvServoState
{
	UV_SERVO_CENTER,
	UV_SERVO_LEFT,
	UV_SERVO_RIGHT,
	UV_SERVO_UP,
	UV_SERVO_DOWN
};

struct UvServoPosition
{
	int panAngle;
	int tiltAngle;
};

struct UvServoMachine
{
	UvServoState state;
	Servo panServo;
	Servo tiltServo;
	int panAngle;
	int tiltAngle;
	ServoExposureMode exposureMode;
	int sweepRadiusDeg;
	int sweepPhaseDeg;
	unsigned long sweepIntervalMs;
	UvServoPosition manualPosition;
	UvServoPosition presets[SERVO_PRESET_COUNT];
	uint8_t selectedPreset;
	unsigned long lastSweepMs;
	bool pwmEnabled;
	bool needsOutput;
};

void uvServoInit(UvServoMachine &machine);
void uvServoUpdate(UvServoMachine &machine);
void uvServoOutput(UvServoMachine &machine);
int uvServoMaxSweepRadiusDeg();
const char *uvServoModeLabel(ServoExposureMode mode);
void uvServoChangeMode(UvServoMachine &machine, int direction);
void uvServoAdjustSweepRadius(UvServoMachine &machine, int direction);
void uvServoAdjustSweepInterval(UvServoMachine &machine, int direction);
void uvServoAdjustManualPan(UvServoMachine &machine, int direction);
void uvServoAdjustManualTilt(UvServoMachine &machine, int direction);
void uvServoSelectPreset(UvServoMachine &machine, int direction);
void uvServoSaveManualPositionToPreset(UvServoMachine &machine);

#endif
