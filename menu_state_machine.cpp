#include "menu_state_machine.h"
#include "exposure_state_machine.h"
#include "uv_servo_state_machine.h"

extern ExposureMachine exposureMachine;
extern UvServoMachine uvServoMachine;

namespace
{

	constexpr uint8_t MENU_ITEMS_PER_PAGE = 2;

	enum MenuItem
	{
		MENU_ITEM_EXPOSURE_ACTION,
		MENU_ITEM_EXPOSURE_DURATION,
		MENU_ITEM_SERVO_MODE,
		MENU_ITEM_ELLIPSE_PAN_MAX,
		MENU_ITEM_ELLIPSE_TILT_MAX,
		MENU_ITEM_ELLIPSE_INTERVAL,
		MENU_ITEM_ELLIPSE_STATUS,
		MENU_ITEM_MANUAL_PAN,
		MENU_ITEM_MANUAL_TILT,
		MENU_ITEM_PRESET_INDEX,
		MENU_ITEM_PRESET_DURATION,
		MENU_ITEM_SAVE_PRESET,
		MENU_ITEM_SCENARIO_STATUS,
		MENU_ITEM_MAX_TEMPERATURE
	};

	uint8_t menuItemCount()
	{
		if (uvServoMachine.exposureMode == SERVO_MODE_ELLIPSE)
		{
			return 8;
		}

		if (uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			return 9;
		}

		return uvServoMachine.exposureMode == SERVO_MODE_SCENARIO ? 5 : 4;
	}

	MenuItem menuItemForIndex(uint8_t index)
	{
		if (index == 0)
		{
			return MENU_ITEM_EXPOSURE_ACTION;
		}
		if (index == 1)
		{
			return MENU_ITEM_EXPOSURE_DURATION;
		}
		if (index == 2)
		{
			return MENU_ITEM_SERVO_MODE;
		}
		if (index == 3)
		{
			return MENU_ITEM_MAX_TEMPERATURE;
		}

		if (uvServoMachine.exposureMode == SERVO_MODE_ELLIPSE)
		{
			if (index == 4)
			{
				return MENU_ITEM_ELLIPSE_PAN_MAX;
			}
			if (index == 5)
			{
				return MENU_ITEM_ELLIPSE_TILT_MAX;
			}
			if (index == 6)
			{
				return MENU_ITEM_ELLIPSE_INTERVAL;
			}
			if (index == 7)
			{
				return MENU_ITEM_ELLIPSE_STATUS;
			}
		}
		else if (uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			if (index == 4)
			{
				return MENU_ITEM_MANUAL_PAN;
			}
			if (index == 5)
			{
				return MENU_ITEM_MANUAL_TILT;
			}
			if (index == 6)
			{
				return MENU_ITEM_PRESET_INDEX;
			}
			if (index == 7)
			{
				return MENU_ITEM_PRESET_DURATION;
			}
			if (index == 8)
			{
				return MENU_ITEM_SAVE_PRESET;
			}
		}
		else if (uvServoMachine.exposureMode == SERVO_MODE_SCENARIO && index == 4)
		{
			return MENU_ITEM_SCENARIO_STATUS;
		}

		return MENU_ITEM_EXPOSURE_ACTION;
	}

	void setDisplayLine(MenuMachine &machine, uint8_t row, const char *text)
	{
		for (uint8_t column = 0; column < LCD_COLS; ++column)
		{
			machine.displayTarget[row][column] = ' ';
		}

		for (uint8_t column = 0; column < LCD_COLS && text[column] != '\0'; ++column)
		{
			machine.displayTarget[row][column] = text[column];
		}
	}

	void formatRemainingTime(char *buffer, size_t bufferSize)
	{
		const unsigned long remainingMs = (exposureDurationMs > exposureElapsedMs) ? (exposureDurationMs - exposureElapsedMs) : 0UL;
		const unsigned int remainingHours = remainingMs / 3600000UL;
		const unsigned int remainingMinutes = (remainingMs % 3600000UL) / 60000UL;
		const unsigned int remainingSeconds = (remainingMs % 60000UL) / 1000UL;

		if (remainingHours > 0)
		{
			snprintf(buffer, bufferSize, "%02u:%02u:%02u", remainingHours, remainingMinutes, remainingSeconds);
		}
		else
		{
			snprintf(buffer, bufferSize, "%02u:%02u", remainingMinutes, remainingSeconds);
		}
	}

	void formatScenarioRemainingTime(char *buffer, size_t bufferSize)
	{
		const UvServoPreset &preset = uvServoMachine.presets[uvServoMachine.scenarioPreset];
		const unsigned long durationMs = static_cast<unsigned long>(preset.durationSeconds) * 1000UL;
		const unsigned long elapsedMs = uvServoMachine.pwmEnabled ? millis() - uvServoMachine.lastScenarioMs : 0UL;
		const unsigned long remainingMs = durationMs > elapsedMs ? durationMs - elapsedMs : 0UL;
		const unsigned int remainingMinutes = remainingMs / 60000UL;
		const unsigned int remainingSeconds = (remainingMs % 60000UL) / 1000UL;

		snprintf(buffer, bufferSize, "%02u:%02u", remainingMinutes, remainingSeconds);
	}

	void renderMenuItem(MenuMachine &machine, uint8_t row, MenuItem item, bool selected, bool editing)
	{
		char line[LCD_COLS + 1];
		const char cursor = selected ? (editing ? '*' : '>') : ' ';

		switch (item)
		{
		case MENU_ITEM_EXPOSURE_ACTION:
		{
			snprintf(line, sizeof(line), "%cExposition:%s", cursor, exposureActive ? "PAUSE" : "START");
			break;
		}
		case MENU_ITEM_EXPOSURE_DURATION:
		{
			char remainingTime[9];
			formatRemainingTime(remainingTime, sizeof(remainingTime));
			snprintf(line, sizeof(line), "%cDuree:%s", cursor, remainingTime);
			break;
		}
		case MENU_ITEM_SERVO_MODE:
		{
			snprintf(line, sizeof(line), "%cMode:%s", cursor, uvServoModeLabel(uvServoMachine.exposureMode));
			break;
		}
		case MENU_ITEM_ELLIPSE_PAN_MAX:
		{
			snprintf(line, sizeof(line), "%cPan max:%d deg", cursor, uvServoMachine.ellipsePanMaxAngleDeg);
			break;
		}
		case MENU_ITEM_ELLIPSE_TILT_MAX:
		{
			snprintf(line, sizeof(line), "%cTilt max:%d deg", cursor, uvServoMachine.ellipseTiltMaxAngleDeg);
			break;
		}
		case MENU_ITEM_ELLIPSE_INTERVAL:
		{
			snprintf(line, sizeof(line), "%cVitesse:%lu ms", cursor, uvServoMachine.ellipseIntervalMs);
			break;
		}
		case MENU_ITEM_MANUAL_PAN:
		{
			snprintf(line, sizeof(line), "%cPan:%d deg", cursor, uvServoMachine.manualPosition.panAngle);
			break;
		}
		case MENU_ITEM_MANUAL_TILT:
		{
			snprintf(line, sizeof(line), "%cTilt:%d deg", cursor, uvServoMachine.manualPosition.tiltAngle);
			break;
		}
		case MENU_ITEM_PRESET_INDEX:
		{
			const UvServoPreset &preset = uvServoMachine.presets[uvServoMachine.selectedPreset];
			snprintf(line, sizeof(line), "%cPreset:%u/%u P%dT%d", cursor, uvServoMachine.selectedPreset + 1, SERVO_PRESET_COUNT, preset.panAngle, preset.tiltAngle);
			break;
		}
		case MENU_ITEM_PRESET_DURATION:
		{
			snprintf(line, sizeof(line), "%cDuree P%u:%us", cursor, uvServoMachine.selectedPreset + 1, uvServoMachine.presets[uvServoMachine.selectedPreset].durationSeconds);
			break;
		}
		case MENU_ITEM_SAVE_PRESET:
		{
			snprintf(line, sizeof(line), "%cSauver P%u", cursor, uvServoMachine.selectedPreset + 1);
			break;
		}
		case MENU_ITEM_MAX_TEMPERATURE:
		{
			snprintf(line, sizeof(line), "%cTmax:%d C", cursor, static_cast<int>(maxJunctionTempC));
			break;
		}
		}

		setDisplayLine(machine, row, line);
	}

	void buildDisplayTarget(MenuMachine &machine)
	{
		char line[LCD_COLS + 1];
		char remainingTime[9];
		formatRemainingTime(remainingTime, sizeof(remainingTime));
		if (exposureActive)
		{
			snprintf(line, sizeof(line), "EXPO:%s", remainingTime);
		}
		else
		{
			snprintf(line, sizeof(line), "EXPO: OFF");
		}
		setDisplayLine(machine, 0, line);

		if (junctionTempSensorFault)
		{
			snprintf(line, sizeof(line), "Tj:ERR/%dC SEC:%s", static_cast<int>(maxJunctionTempC), safetyTrip ? "KO" : "OK");
		}
		else
		{
			snprintf(line, sizeof(line), "Tj:%d/%dC SEC:%s", static_cast<int>(junctionTempC), static_cast<int>(maxJunctionTempC), safetyTrip ? "KO" : "OK");
		}
		setDisplayLine(machine, 1, line);

		const uint8_t pageIndex = machine.selectedItem / MENU_ITEMS_PER_PAGE;
		const bool editing = machine.state == MENU_EDIT;
		if (pageIndex == 0)
		{
			renderMenuItem(machine, 2, MENU_ITEM_EXPOSURE_ACTION, machine.selectedItem == 0, editing);
			renderMenuItem(machine, 3, MENU_ITEM_EXPOSURE_DURATION, machine.selectedItem == 1, editing);
		}
		else if (pageIndex == 1)
		{
			renderMenuItem(machine, 2, MENU_ITEM_SERVO_MODE, machine.selectedItem == 2, editing);
			renderMenuItem(machine, 3, MENU_ITEM_MAX_TEMPERATURE, machine.selectedItem == 3, editing);
		}
		else if (pageIndex == 2 && uvServoMachine.exposureMode == SERVO_MODE_ELLIPSE)
		{
			renderMenuItem(machine, 2, MENU_ITEM_ELLIPSE_PAN_MAX, machine.selectedItem == 4, editing);
			renderMenuItem(machine, 3, MENU_ITEM_ELLIPSE_TILT_MAX, machine.selectedItem == 5, editing);
		}
		else if (pageIndex == 2 && uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			renderMenuItem(machine, 2, MENU_ITEM_MANUAL_PAN, machine.selectedItem == 4, editing);
			renderMenuItem(machine, 3, MENU_ITEM_MANUAL_TILT, machine.selectedItem == 5, editing);
		}
		else if (pageIndex == 2 && uvServoMachine.exposureMode == SERVO_MODE_SCENARIO)
		{
			char scenarioRemainingTime[6];
			formatScenarioRemainingTime(scenarioRemainingTime, sizeof(scenarioRemainingTime));
			snprintf(line, sizeof(line), "%cScenario P:%u/%u", editing ? '*' : '>', uvServoMachine.scenarioPreset + 1, SERVO_PRESET_COUNT);
			setDisplayLine(machine, 2, line);
			snprintf(line, sizeof(line), "R:%s P:%d T:%d", scenarioRemainingTime, uvServoMachine.panAngle, uvServoMachine.tiltAngle);
			setDisplayLine(machine, 3, line);
		}
		else if (pageIndex == 3 && uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			renderMenuItem(machine, 2, MENU_ITEM_PRESET_INDEX, machine.selectedItem == 6, editing);
			renderMenuItem(machine, 3, MENU_ITEM_PRESET_DURATION, machine.selectedItem == 7, editing);
		}
		else if (uvServoMachine.exposureMode == SERVO_MODE_ELLIPSE)
		{
			renderMenuItem(machine, 2, MENU_ITEM_ELLIPSE_INTERVAL, machine.selectedItem == 6, editing);
			const char cursor = machine.selectedItem == 7 ? '>' : ' ';
			snprintf(line, sizeof(line), "%cActuel P:%d T:%d", cursor, uvServoMachine.panAngle, uvServoMachine.tiltAngle);
			setDisplayLine(machine, 3, line);
		}
		else if (pageIndex == 4 && uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			renderMenuItem(machine, 2, MENU_ITEM_SAVE_PRESET, machine.selectedItem == 8, editing);
			snprintf(line, sizeof(line), "P:%d T:%d %us", uvServoMachine.manualPosition.panAngle, uvServoMachine.manualPosition.tiltAngle, uvServoMachine.presets[uvServoMachine.selectedPreset].durationSeconds);
			setDisplayLine(machine, 3, line);
		}
	}

	void outputNextChangedCharacter(MenuMachine &machine)
	{
		for (uint8_t row = 0; row < LCD_ROWS; ++row)
		{
			for (uint8_t column = 0; column < LCD_COLS; ++column)
			{
				if (machine.displayTarget[row][column] != machine.displayRendered[row][column])
				{
					lcd.setCursor(column, row);
					lcd.write(machine.displayTarget[row][column]);
					machine.displayRendered[row][column] = machine.displayTarget[row][column];
					return;
				}
			}
		}
	}

	void applyMenuAdjustment(MenuItem item, int direction)
	{
		switch (item)
		{
		case MENU_ITEM_EXPOSURE_DURATION:
			if (!exposureActive)
			{
				unsigned long adjustmentMs = 5000UL;
				if ((direction > 0 && exposureDurationMs >= 900000UL) || (direction < 0 && exposureDurationMs > 900000UL))
				{
					adjustmentMs = 300000UL;
				}
				else if ((direction > 0 && exposureDurationMs >= 300000UL) || (direction < 0 && exposureDurationMs > 300000UL))
				{
					adjustmentMs = 120000UL;
				}
				else if ((direction > 0 && exposureDurationMs >= 60000UL) || (direction < 0 && exposureDurationMs > 60000UL))
				{
					adjustmentMs = 30000UL;
				}

				const long adjustedDurationMs = static_cast<long>(exposureDurationMs) + (direction > 0 ? static_cast<long>(adjustmentMs) : -static_cast<long>(adjustmentMs));
				exposureDurationMs = constrain(adjustedDurationMs, 5000L, static_cast<long>(MAX_EXPOSURE_DURATION_MS));
				exposureElapsedMs = 0UL;
			}
			break;
		case MENU_ITEM_SERVO_MODE:
			uvServoChangeMode(uvServoMachine, direction);
			break;
		case MENU_ITEM_ELLIPSE_PAN_MAX:
			uvServoAdjustEllipsePanMaxAngle(uvServoMachine, direction);
			break;
		case MENU_ITEM_ELLIPSE_TILT_MAX:
			uvServoAdjustEllipseTiltMaxAngle(uvServoMachine, direction);
			break;
		case MENU_ITEM_ELLIPSE_INTERVAL:
			uvServoAdjustEllipseInterval(uvServoMachine, direction);
			break;
		case MENU_ITEM_ELLIPSE_STATUS:
			break;
		case MENU_ITEM_MANUAL_PAN:
			uvServoAdjustManualPan(uvServoMachine, direction);
			break;
		case MENU_ITEM_MANUAL_TILT:
			uvServoAdjustManualTilt(uvServoMachine, direction);
			break;
		case MENU_ITEM_PRESET_INDEX:
			uvServoSelectPreset(uvServoMachine, direction);
			break;
		case MENU_ITEM_PRESET_DURATION:
			uvServoAdjustPresetDuration(uvServoMachine, direction);
			break;
		case MENU_ITEM_MAX_TEMPERATURE:
			maxJunctionTempC = constrain(maxJunctionTempC + (direction > 0 ? 1.0f : -1.0f), 30.0f, 90.0f);
			break;
		case MENU_ITEM_EXPOSURE_ACTION:
		case MENU_ITEM_SAVE_PRESET:
		case MENU_ITEM_SCENARIO_STATUS:
			break;
		}
	}
}

void menuInit(MenuMachine &machine)
{
	machine.state = MENU_STANDBY;
	machine.selectedItem = 0;
	machine.currentPage = 0;
	machine.pageCount = 0;
	machine.needsRefresh = true;
	machine.editing = false;
	machine.forceRefresh = true;
	machine.lastRefreshMs = 0UL;
	for (uint8_t row = 0; row < LCD_ROWS; ++row)
	{
		for (uint8_t column = 0; column < LCD_COLS; ++column)
		{
			machine.displayTarget[row][column] = ' ';
			machine.displayRendered[row][column] = ' ';
		}
	}
}

void menuUpdate(MenuMachine &machine)
{
	menuExposedAlarmActive = safetyTrip;

	const uint8_t itemCount = menuItemCount();
	if (machine.selectedItem >= itemCount)
	{
		machine.selectedItem = itemCount - 1;
	}

	const bool inputChanged = encoderButtonPressedEvent || (encoderRotationCount != 0);

	if (encoderButtonPressedEvent)
	{
		const MenuItem selectedItem = menuItemForIndex(machine.selectedItem);
		if (selectedItem == MENU_ITEM_EXPOSURE_ACTION)
		{
			if (exposureActive)
			{
				exposureMachine.requestPause = true;
			}
			else
			{
				exposureMachine.requestStart = true;
			}
			machine.editing = false;
			machine.state = MENU_STANDBY;
		}
		else if (selectedItem == MENU_ITEM_SAVE_PRESET)
		{
			uvServoSaveManualPositionToPreset(uvServoMachine);
			machine.editing = false;
			machine.state = MENU_STANDBY;
		}
		else if (selectedItem == MENU_ITEM_SCENARIO_STATUS || selectedItem == MENU_ITEM_ELLIPSE_STATUS)
		{
			machine.editing = false;
			machine.state = MENU_STANDBY;
		}
		else
		{
			machine.editing = !machine.editing;
			machine.state = machine.editing ? MENU_EDIT : MENU_STANDBY;
		}
		encoderButtonPressedEvent = false;
		machine.forceRefresh = true;
	}

	if (machine.state == MENU_STANDBY)
	{
		if (encoderRotationCount != 0)
		{
			int nextItem = machine.selectedItem;
			const int step = (encoderRotationCount > 0 ? 1 : -1);

			nextItem = (nextItem + step + itemCount) % itemCount;

			machine.selectedItem = static_cast<uint8_t>(nextItem);
			encoderRotationCount = 0;
			machine.forceRefresh = true;
		}
	}
	else if (machine.state == MENU_EDIT)
	{
		if (encoderRotationCount != 0)
		{
			applyMenuAdjustment(menuItemForIndex(machine.selectedItem), encoderRotationCount > 0 ? 1 : -1);
			encoderRotationCount = 0;
			machine.forceRefresh = true;
		}
	}

	if (inputChanged)
	{
		machine.forceRefresh = true;
	}

	machine.currentPage = machine.selectedItem / MENU_ITEMS_PER_PAGE;
	machine.pageCount = (menuItemCount() + MENU_ITEMS_PER_PAGE - 1) / MENU_ITEMS_PER_PAGE;
	menuExposedSelection = machine.selectedItem;
	machine.needsRefresh = inputChanged;
}

void menuOutput(MenuMachine &machine)
{
	const unsigned long now = millis();
	const bool ellipseStatusVisible = uvServoMachine.exposureMode == SERVO_MODE_ELLIPSE && machine.selectedItem / MENU_ITEMS_PER_PAGE == 3;
	const bool shouldRefresh = machine.needsRefresh || machine.forceRefresh || ellipseStatusVisible || (now - machine.lastRefreshMs >= 1000UL);
	if (shouldRefresh)
	{
		buildDisplayTarget(machine);
		machine.lastRefreshMs = now;
		machine.needsRefresh = false;
		machine.forceRefresh = false;
	}

	outputNextChangedCharacter(machine);
}
