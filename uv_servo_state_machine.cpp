#include "uv_servo_state_machine.h"

namespace
{

	int clampPanAngle(int angle)
	{
		return constrain(angle, SERVO_PAN_MIN_DEG, SERVO_PAN_MAX_DEG);
	}

	int clampTiltAngle(int angle)
	{
		return constrain(angle, SERVO_TILT_MIN_DEG, SERVO_TILT_MAX_DEG);
	}

}

int uvServoMaxSweepRadiusDeg()
{
	const int panNegativeMarginDeg = SERVO_PAN_NEUTRAL_DEG - SERVO_PAN_MIN_DEG;
	const int panPositiveMarginDeg = SERVO_PAN_MAX_DEG - SERVO_PAN_NEUTRAL_DEG;
	const int panMaxRadiusDeg = static_cast<int>(min(panNegativeMarginDeg, panPositiveMarginDeg) / SERVO_PAN_ANGLE_MULTIPLIER);
	const int tiltNegativeMarginDeg = SERVO_TILT_NEUTRAL_DEG - SERVO_TILT_MIN_DEG;
	const int tiltPositiveMarginDeg = SERVO_TILT_MAX_DEG - SERVO_TILT_NEUTRAL_DEG;
	const int tiltMaxRadiusDeg = min(tiltNegativeMarginDeg, tiltPositiveMarginDeg);

	return min(panMaxRadiusDeg, tiltMaxRadiusDeg);
}

const char *uvServoModeLabel(ServoExposureMode mode)
{
	switch (mode)
	{
	case SERVO_MODE_MANUAL:
		return "MANUEL";
	case SERVO_MODE_PRESET:
		return "PRESET";
	case SERVO_MODE_SWEEP:
	default:
		return "SWEEP";
	}
}

void uvServoChangeMode(UvServoMachine &machine, int direction)
{
	const int modeCount = 3;
	const int nextMode = (static_cast<int>(machine.exposureMode) + direction + modeCount) % modeCount;
	machine.exposureMode = static_cast<ServoExposureMode>(nextMode);
	machine.lastSweepMs = millis();
}

void uvServoAdjustSweepRadius(UvServoMachine &machine, int direction)
{
	machine.sweepRadiusDeg = constrain(machine.sweepRadiusDeg + direction, 1, uvServoMaxSweepRadiusDeg());
}

void uvServoAdjustSweepInterval(UvServoMachine &machine, int direction)
{
	const long adjustedIntervalMs = static_cast<long>(machine.sweepIntervalMs) + direction * static_cast<long>(SERVO_SWEEP_INTERVAL_STEP_MS);
	machine.sweepIntervalMs = static_cast<unsigned long>(constrain(adjustedIntervalMs, static_cast<long>(MIN_SERVO_SWEEP_INTERVAL_MS), static_cast<long>(MAX_SERVO_SWEEP_INTERVAL_MS)));
}

void uvServoAdjustManualPan(UvServoMachine &machine, int direction)
{
	machine.manualPosition.panAngle = clampPanAngle(machine.manualPosition.panAngle + direction);
}

void uvServoAdjustManualTilt(UvServoMachine &machine, int direction)
{
	machine.manualPosition.tiltAngle = clampTiltAngle(machine.manualPosition.tiltAngle + direction);
}

void uvServoSelectPreset(UvServoMachine &machine, int direction)
{
	const int nextPreset = (static_cast<int>(machine.selectedPreset) + direction + SERVO_PRESET_COUNT) % SERVO_PRESET_COUNT;
	machine.selectedPreset = static_cast<uint8_t>(nextPreset);
}

void uvServoSaveManualPositionToPreset(UvServoMachine &machine)
{
	machine.presets[machine.selectedPreset] = machine.manualPosition;
}

void uvServoInit(UvServoMachine &machine)
{
	machine.state = UV_SERVO_CENTER;
	machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
	machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
	machine.exposureMode = SERVO_MODE_SWEEP;
	machine.sweepRadiusDeg = DEFAULT_SERVO_SWEEP_RADIUS_DEG;
	machine.sweepPhaseDeg = 0;
	machine.sweepIntervalMs = DEFAULT_SERVO_SWEEP_INTERVAL_MS;
	machine.manualPosition = {SERVO_PAN_NEUTRAL_DEG, SERVO_TILT_NEUTRAL_DEG};
	for (uint8_t presetIndex = 0; presetIndex < SERVO_PRESET_COUNT; ++presetIndex)
	{
		machine.presets[presetIndex] = machine.manualPosition;
	}
	machine.selectedPreset = 0;
	machine.lastSweepMs = millis();
	machine.pwmEnabled = false;
	machine.needsOutput = false;
}

void uvServoUpdate(UvServoMachine &machine)
{
	if (safetyTrip || !servoControlEnabled)
	{
		machine.state = UV_SERVO_CENTER;
		machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
		machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
		machine.needsOutput = machine.pwmEnabled;
		machine.pwmEnabled = false;
		return;
	}

	const unsigned long now = millis();
	if (!machine.pwmEnabled)
	{
		machine.panServo.attach(PIN_SM_SERVO_PAN);
		machine.tiltServo.attach(PIN_SM_SERVO_TILT);
		machine.pwmEnabled = true;
		machine.lastSweepMs = now;
	}
	else if (machine.exposureMode == SERVO_MODE_SWEEP && now - machine.lastSweepMs >= machine.sweepIntervalMs)
	{
		machine.sweepPhaseDeg = (machine.sweepPhaseDeg + SERVO_SWEEP_STEP_DEG) % 360;
		machine.lastSweepMs = now;
	}

	machine.state = UV_SERVO_CENTER;
	if (machine.exposureMode == SERVO_MODE_SWEEP)
	{
		const float angleRad = machine.sweepPhaseDeg * 0.0174532925f;
		const int panOffset = static_cast<int>(sin(angleRad) * machine.sweepRadiusDeg * SERVO_PAN_ANGLE_MULTIPLIER);
		const int tiltOffset = static_cast<int>(cos(angleRad) * machine.sweepRadiusDeg);
		machine.panAngle = clampPanAngle(SERVO_PAN_NEUTRAL_DEG + panOffset);
		machine.tiltAngle = clampTiltAngle(SERVO_TILT_NEUTRAL_DEG + tiltOffset);
	}
	else if (machine.exposureMode == SERVO_MODE_MANUAL)
	{
		machine.panAngle = clampPanAngle(machine.manualPosition.panAngle);
		machine.tiltAngle = clampTiltAngle(machine.manualPosition.tiltAngle);
	}
	else
	{
		const UvServoPosition &preset = machine.presets[machine.selectedPreset];
		machine.panAngle = clampPanAngle(preset.panAngle);
		machine.tiltAngle = clampTiltAngle(preset.tiltAngle);
	}

	machine.needsOutput = true;
}

void uvServoOutput(UvServoMachine &machine)
{
	if (!machine.needsOutput)
	{
		return;
	}

	if (!machine.pwmEnabled)
	{
		machine.panServo.detach();
		machine.tiltServo.detach();
		machine.needsOutput = false;
		return;
	}

	machine.panServo.write(machine.panAngle);
	machine.tiltServo.write(machine.tiltAngle);
	machine.needsOutput = false;
}
