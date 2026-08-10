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
		MENU_ITEM_SWEEP_RADIUS,
		MENU_ITEM_SWEEP_INTERVAL,
		MENU_ITEM_MANUAL_PAN,
		MENU_ITEM_MANUAL_TILT,
		MENU_ITEM_PRESET_INDEX,
		MENU_ITEM_SAVE_PRESET,
		MENU_ITEM_MAX_TEMPERATURE
	};

	uint8_t menuItemCount()
	{
		return 6;
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

		if (uvServoMachine.exposureMode == SERVO_MODE_SWEEP)
		{
			if (index == 4)
			{
				return MENU_ITEM_SWEEP_RADIUS;
			}
			if (index == 5)
			{
				return MENU_ITEM_SWEEP_INTERVAL;
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
		}
		else
		{
			if (index == 4)
			{
				return MENU_ITEM_PRESET_INDEX;
			}
			if (index == 5)
			{
				return MENU_ITEM_SAVE_PRESET;
			}
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
			char remainingTime[6];
			formatRemainingTime(remainingTime, sizeof(remainingTime));
			snprintf(line, sizeof(line), "%cDuree:%s", cursor, remainingTime);
			break;
		}
		case MENU_ITEM_SERVO_MODE:
		{
			snprintf(line, sizeof(line), "%cMode:%s", cursor, uvServoModeLabel(uvServoMachine.exposureMode));
			break;
		}
		case MENU_ITEM_SWEEP_RADIUS:
		{
			snprintf(line, sizeof(line), "%cRayon:%d deg", cursor, uvServoMachine.sweepRadiusDeg);
			break;
		}
		case MENU_ITEM_SWEEP_INTERVAL:
		{
			snprintf(line, sizeof(line), "%cVitesse:%lu ms", cursor, uvServoMachine.sweepIntervalMs);
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
			const UvServoPosition &preset = uvServoMachine.presets[uvServoMachine.selectedPreset];
			snprintf(line, sizeof(line), "%cPreset:%u/%u P%dT%d", cursor, uvServoMachine.selectedPreset + 1, SERVO_PRESET_COUNT, preset.panAngle, preset.tiltAngle);
			break;
		}
		case MENU_ITEM_SAVE_PRESET:
		{
			snprintf(line, sizeof(line), "%cSauver P%u:%d/%d", cursor, uvServoMachine.selectedPreset + 1, uvServoMachine.manualPosition.panAngle, uvServoMachine.manualPosition.tiltAngle);
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
		char remainingTime[6];
		formatRemainingTime(remainingTime, sizeof(remainingTime));
		snprintf(line, sizeof(line), "EXPO:%s REST:%s", exposureActive ? "ON " : "OFF", remainingTime);
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
		else if (uvServoMachine.exposureMode == SERVO_MODE_SWEEP)
		{
			renderMenuItem(machine, 2, MENU_ITEM_SWEEP_RADIUS, machine.selectedItem == 4, editing);
			renderMenuItem(machine, 3, MENU_ITEM_SWEEP_INTERVAL, machine.selectedItem == 5, editing);
		}
		else if (uvServoMachine.exposureMode == SERVO_MODE_MANUAL)
		{
			renderMenuItem(machine, 2, MENU_ITEM_MANUAL_PAN, machine.selectedItem == 4, editing);
			renderMenuItem(machine, 3, MENU_ITEM_MANUAL_TILT, machine.selectedItem == 5, editing);
		}
		else
		{
			renderMenuItem(machine, 2, MENU_ITEM_PRESET_INDEX, machine.selectedItem == 4, editing);
			renderMenuItem(machine, 3, MENU_ITEM_SAVE_PRESET, machine.selectedItem == 5, editing);
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
				exposureDurationMs = constrain(adjustedDurationMs, 5000L, 3600000L);
				exposureElapsedMs = 0UL;
			}
			break;
		case MENU_ITEM_SERVO_MODE:
			uvServoChangeMode(uvServoMachine, direction);
			break;
		case MENU_ITEM_SWEEP_RADIUS:
			uvServoAdjustSweepRadius(uvServoMachine, direction);
			break;
		case MENU_ITEM_SWEEP_INTERVAL:
			uvServoAdjustSweepInterval(uvServoMachine, direction);
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
		case MENU_ITEM_MAX_TEMPERATURE:
			maxJunctionTempC = constrain(maxJunctionTempC + (direction > 0 ? 1.0f : -1.0f), 30.0f, 90.0f);
			break;
		case MENU_ITEM_EXPOSURE_ACTION:
		case MENU_ITEM_SAVE_PRESET:
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
	const bool shouldRefresh = machine.needsRefresh || machine.forceRefresh || (now - machine.lastRefreshMs >= 1000UL);
	if (shouldRefresh)
	{
		buildDisplayTarget(machine);
		machine.lastRefreshMs = now;
		machine.needsRefresh = false;
		machine.forceRefresh = false;
	}

	outputNextChangedCharacter(machine);
}
