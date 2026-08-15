#ifndef UV_SERVO_STATE_MACHINE_H
#define UV_SERVO_STATE_MACHINE_H

#include "globals.h"
#include <Servo.h>

enum ServoExposureMode
{
	SERVO_MODE_ELLIPSE,
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
	int ellipsePanMaxAngleDeg;
	int ellipseTiltMaxAngleDeg;
	int ellipsePhaseDeg;
	unsigned long ellipseIntervalMs;
	UvServoPosition manualPosition;
	UvServoPosition presets[SERVO_PRESET_COUNT];
	uint8_t selectedPreset;
	unsigned long lastEllipseMs;
	bool pwmEnabled;
	bool needsOutput;
};

void uvServoInit(UvServoMachine &machine);
void uvServoUpdate(UvServoMachine &machine);
void uvServoOutput(UvServoMachine &machine);
int uvServoMaxPanOffsetDeg();
int uvServoMaxTiltOffsetDeg();
const char *uvServoModeLabel(ServoExposureMode mode);
void uvServoChangeMode(UvServoMachine &machine, int direction);
void uvServoAdjustEllipsePanMaxAngle(UvServoMachine &machine, int direction);
void uvServoAdjustEllipseTiltMaxAngle(UvServoMachine &machine, int direction);
void uvServoAdjustEllipseInterval(UvServoMachine &machine, int direction);
void uvServoAdjustManualPan(UvServoMachine &machine, int direction);
void uvServoAdjustManualTilt(UvServoMachine &machine, int direction);
void uvServoSelectPreset(UvServoMachine &machine, int direction);
void uvServoSaveManualPositionToPreset(UvServoMachine &machine);

#endif
