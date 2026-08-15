#include "uv_servo_state_machine.h"
#include <EEPROM.h>

namespace
{
	constexpr uint16_t SERVO_PRESET_STORAGE_MAGIC = 0x4359;
	constexpr uint8_t SERVO_PRESET_STORAGE_VERSION = 1;
	constexpr int SERVO_PRESET_EEPROM_ADDRESS = 0;

	struct StoredServoPresets
	{
		uint16_t magic;
		uint8_t version;
		UvServoPosition presets[SERVO_PRESET_COUNT];
	};

	int clampPanAngle(int angle)
	{
		return constrain(angle, SERVO_PAN_MIN_DEG, SERVO_PAN_MAX_DEG);
	}

	int clampTiltAngle(int angle)
	{
		return constrain(angle, SERVO_TILT_MIN_DEG, SERVO_TILT_MAX_DEG);
	}

	bool isValidPresetPosition(const UvServoPosition &position)
	{
		return position.panAngle >= SERVO_PAN_MIN_DEG && position.panAngle <= SERVO_PAN_MAX_DEG && position.tiltAngle >= SERVO_TILT_MIN_DEG && position.tiltAngle <= SERVO_TILT_MAX_DEG;
	}

	void savePresetsToEeprom(const UvServoMachine &machine)
	{
		StoredServoPresets storedPresets;
		storedPresets.magic = SERVO_PRESET_STORAGE_MAGIC;
		storedPresets.version = SERVO_PRESET_STORAGE_VERSION;
		for (uint8_t presetIndex = 0; presetIndex < SERVO_PRESET_COUNT; ++presetIndex)
		{
			storedPresets.presets[presetIndex] = machine.presets[presetIndex];
		}

		EEPROM.put(SERVO_PRESET_EEPROM_ADDRESS, storedPresets);
	}

	void loadPresetsFromEeprom(UvServoMachine &machine)
	{
		StoredServoPresets storedPresets;
		EEPROM.get(SERVO_PRESET_EEPROM_ADDRESS, storedPresets);

		const bool storageIsCurrent = storedPresets.magic == SERVO_PRESET_STORAGE_MAGIC && storedPresets.version == SERVO_PRESET_STORAGE_VERSION;
		bool saveDefaults = !storageIsCurrent;

		for (uint8_t presetIndex = 0; presetIndex < SERVO_PRESET_COUNT; ++presetIndex)
		{
			const UvServoPosition defaultPosition = {SERVO_PAN_NEUTRAL_DEG, SERVO_TILT_NEUTRAL_DEG};
			if (storageIsCurrent && isValidPresetPosition(storedPresets.presets[presetIndex]))
			{
				machine.presets[presetIndex] = storedPresets.presets[presetIndex];
			}
			else
			{
				machine.presets[presetIndex] = defaultPosition;
				saveDefaults = true;
			}
		}

		if (saveDefaults)
		{
			savePresetsToEeprom(machine);
		}
	}

}

int uvServoMaxPanOffsetDeg()
{
	const int panNegativeMarginDeg = SERVO_PAN_NEUTRAL_DEG - SERVO_PAN_MIN_DEG;
	const int panPositiveMarginDeg = SERVO_PAN_MAX_DEG - SERVO_PAN_NEUTRAL_DEG;
	return min(panNegativeMarginDeg, panPositiveMarginDeg);
}

int uvServoMaxTiltOffsetDeg()
{
	const int tiltNegativeMarginDeg = SERVO_TILT_NEUTRAL_DEG - SERVO_TILT_MIN_DEG;
	const int tiltPositiveMarginDeg = SERVO_TILT_MAX_DEG - SERVO_TILT_NEUTRAL_DEG;
	return min(tiltNegativeMarginDeg, tiltPositiveMarginDeg);
}

const char *uvServoModeLabel(ServoExposureMode mode)
{
	switch (mode)
	{
	case SERVO_MODE_MANUAL:
		return "MANUEL";
	case SERVO_MODE_PRESET:
		return "PRESET";
	case SERVO_MODE_ELLIPSE:
	default:
		return "ELLIPSE";
	}
}

void uvServoChangeMode(UvServoMachine &machine, int direction)
{
	const int modeCount = 3;
	const int nextMode = (static_cast<int>(machine.exposureMode) + direction + modeCount) % modeCount;
	machine.exposureMode = static_cast<ServoExposureMode>(nextMode);
	machine.lastEllipseMs = millis();
}

void uvServoAdjustEllipsePanMaxAngle(UvServoMachine &machine, int direction)
{
	machine.ellipsePanMaxAngleDeg = constrain(machine.ellipsePanMaxAngleDeg + direction, 1, uvServoMaxPanOffsetDeg());
}

void uvServoAdjustEllipseTiltMaxAngle(UvServoMachine &machine, int direction)
{
	machine.ellipseTiltMaxAngleDeg = constrain(machine.ellipseTiltMaxAngleDeg + direction, 1, uvServoMaxTiltOffsetDeg());
}

void uvServoAdjustEllipseInterval(UvServoMachine &machine, int direction)
{
	const long adjustedIntervalMs = static_cast<long>(machine.ellipseIntervalMs) + direction * static_cast<long>(SERVO_ELLIPSE_INTERVAL_STEP_MS);
	machine.ellipseIntervalMs = static_cast<unsigned long>(constrain(adjustedIntervalMs, static_cast<long>(MIN_SERVO_ELLIPSE_INTERVAL_MS), static_cast<long>(MAX_SERVO_ELLIPSE_INTERVAL_MS)));
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
	savePresetsToEeprom(machine);
}

void uvServoInit(UvServoMachine &machine)
{
	machine.state = UV_SERVO_CENTER;
	machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
	machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
	machine.exposureMode = SERVO_MODE_ELLIPSE;
	machine.ellipsePanMaxAngleDeg = DEFAULT_SERVO_PAN_MAX_ANGLE_DEG;
	machine.ellipseTiltMaxAngleDeg = DEFAULT_SERVO_TILT_MAX_ANGLE_DEG;
	machine.ellipsePhaseDeg = 0;
	machine.ellipseIntervalMs = DEFAULT_SERVO_ELLIPSE_INTERVAL_MS;
	machine.manualPosition = {SERVO_PAN_NEUTRAL_DEG, SERVO_TILT_NEUTRAL_DEG};
	loadPresetsFromEeprom(machine);
	machine.selectedPreset = 0;
	machine.lastEllipseMs = millis();
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
		machine.lastEllipseMs = now;
	}
	else if (machine.exposureMode == SERVO_MODE_ELLIPSE && now - machine.lastEllipseMs >= machine.ellipseIntervalMs)
	{
		machine.ellipsePhaseDeg = (machine.ellipsePhaseDeg + SERVO_ELLIPSE_STEP_DEG) % 360;
		machine.lastEllipseMs = now;
	}

	machine.state = UV_SERVO_CENTER;
	if (machine.exposureMode == SERVO_MODE_ELLIPSE)
	{
		const float angleRad = machine.ellipsePhaseDeg * 0.0174532925f;
		const int panOffset = static_cast<int>(sin(angleRad) * machine.ellipsePanMaxAngleDeg);
		const int tiltOffset = static_cast<int>(cos(angleRad) * machine.ellipseTiltMaxAngleDeg);
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
