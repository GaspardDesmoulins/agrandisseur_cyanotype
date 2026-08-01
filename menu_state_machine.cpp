#include "menu_state_machine.h"
#include "exposure_state_machine.h"

extern ExposureMachine exposureMachine;

void menuInit(MenuMachine &machine) {
  machine.state = MENU_STANDBY;
  machine.selectedItem = 0;
  machine.editItem = 0;
  machine.needsRefresh = true;
  machine.editing = false;
  machine.forceRefresh = true;
  machine.lastRefreshMs = 0UL;
  machine.lastTimerRefreshMs = 0UL;
}

void menuUpdate(MenuMachine &machine) {
  if (safetyTrip) {
    menuExposedAlarmActive = true;
  }
  else {
    menuExposedAlarmActive = false;
  }

  const bool inputChanged = encoderButtonPressedEvent || (encoderRotationCount != 0);

  if (encoderButtonPressedEvent) {
    machine.editing = !machine.editing;
    machine.state = machine.editing ? MENU_EDIT : MENU_STANDBY;
    encoderButtonPressedEvent = false;
    machine.forceRefresh = true;
  }

  if (machine.state == MENU_STANDBY) {
    // In standby, the encoder moves the selection cursor between menu items.
    // The editable order is: exposure state, exposure time, radius, Tmax.
    if (encoderRotationCount != 0) {
      int nextItem = machine.selectedItem;
      const int step = (encoderRotationCount > 0 ? 1 : -1);
      const int itemCount = 4;

      nextItem = (nextItem + step + itemCount) % itemCount;

      machine.selectedItem = static_cast<uint8_t>(nextItem);
      encoderRotationCount = 0;
      machine.forceRefresh = true;
    }
  }
  else if (machine.state == MENU_EDIT) {
    // In edit mode, a non-zero value means the user turned the encoder.
    // We process that pending change once and then reset the counter so the same
    // turn is not applied again on the next loop iteration.
    if (encoderRotationCount != 0) {
      if (machine.selectedItem == 0) {
        // Item 0 starts or pauses the exposure timer.
        if (!exposureActive) {
          exposureMachine.requestStart = true;
          exposureActive = true;
        }
        else {
          exposureMachine.requestPause = true;
          exposureActive = false;
        }
      }
      else if (machine.selectedItem == 1) {
        // Item 1 adjusts the remaining exposure time, but only when the timer is idle.
        if (!exposureActive) {
          const bool increaseDuration = encoderRotationCount > 0;
          unsigned long adjustmentMs = 5000UL;

          if ((increaseDuration && exposureDurationMs >= 900000UL) || (!increaseDuration && exposureDurationMs > 900000UL)) {
            adjustmentMs = 300000UL;
          }
          else if ((increaseDuration && exposureDurationMs >= 300000UL) || (!increaseDuration && exposureDurationMs > 300000UL)) {
            adjustmentMs = 120000UL;
          }
          else if ((increaseDuration && exposureDurationMs >= 60000UL) || (!increaseDuration && exposureDurationMs > 60000UL)) {
            adjustmentMs = 30000UL;
          }

          const long adjustedDurationMs = static_cast<long>(exposureDurationMs) + (increaseDuration ? static_cast<long>(adjustmentMs) : -static_cast<long>(adjustmentMs));
          exposureDurationMs = constrain(adjustedDurationMs, 5000L, 3600000L);
          exposureElapsedMs = 0UL;
        }
      }
      else if (machine.selectedItem == 2) {
        // Adjust the servo sweep radius.
        servoSweepRadiusDeg = constrain(servoSweepRadiusDeg + (encoderRotationCount > 0 ? 1 : -1), 1, 20);
      }
      else if (machine.selectedItem == 3) {
        // Adjust the maximum junction temperature threshold.
        maxJunctionTempC = constrain(maxJunctionTempC + (encoderRotationCount > 0 ? 1.0f : -1.0f), 30.0f, 90.0f);
      }
      encoderRotationCount = 0;
      machine.forceRefresh = true;
    }
  }

  if (inputChanged) {
    machine.forceRefresh = true;
  }

  menuExposedSelection = machine.selectedItem;
  machine.needsRefresh = true;
}

void menuOutput(MenuMachine &machine) {
  if (!machine.needsRefresh) {
    return;
  }

  const unsigned long now = millis();
  const bool shouldRenderFullMenu = machine.forceRefresh || (now - machine.lastRefreshMs >= 1000UL);
  const bool shouldRefreshTimer = machine.forceRefresh || (now - machine.lastTimerRefreshMs >= 100UL);
  if (!shouldRenderFullMenu && !shouldRefreshTimer) {
    return;
  }

  const bool editing = machine.state == MENU_EDIT;
  const bool isExposureActionSelected = machine.selectedItem == 0;
  const bool isExposureTimeRemainingSelected = machine.selectedItem == 1;
  const bool isRadiusItemSelected = machine.selectedItem == 2;
  const bool isMaxTempItemSelected = machine.selectedItem == 3;

  if (shouldRenderFullMenu) {
    lcd.setCursor(0, 0);
    lcd.print(editing ? "[EDIT]" : "[STBY]");
    lcd.print(" ");

    const unsigned long remainingMs = (exposureDurationMs > exposureElapsedMs) ? (exposureDurationMs - exposureElapsedMs) : 0UL;
    const unsigned int remainingMinutes = remainingMs / 60000UL;
    const unsigned int remainingSeconds = (remainingMs % 60000UL) / 1000UL;

    lcd.setCursor(15, 0);
    if (remainingMinutes < 10U) {
      lcd.print('0');
    }
    lcd.print(remainingMinutes);
    lcd.print(':');
    if (remainingSeconds < 10U) {
      lcd.print('0');
    }
    lcd.print(remainingSeconds);

    lcd.setCursor(0, 1);
    lcd.print(isExposureActionSelected ? ">" : " ");
    lcd.print("Etat:");
    lcd.print(exposureActive ? "RUN  " : "PAUSE");

    lcd.setCursor(0, 2);
    lcd.print(isRadiusItemSelected ? ">" : " ");
    lcd.print("Rayon:");
    lcd.print(servoSweepRadiusDeg);
    lcd.setCursor(10, 2);
    lcd.print(safetyTrip ? "SAFETY:KO!" : "SAFETY:OK ");

    lcd.setCursor(0, 3);
    lcd.print(isMaxTempItemSelected ? ">" : " ");
    lcd.print("Tj/Tmax: ");
    if (junctionTempSensorFault) {
      lcd.print("ERR/");
    }
    else {
      lcd.print(static_cast<int>(junctionTempC));
      lcd.print((char)223);
      lcd.print("C/");
    }
    lcd.print(static_cast<int>(maxJunctionTempC));
    lcd.print((char)223);
    lcd.print("C");

    machine.lastRefreshMs = now;
  }

  if (shouldRefreshTimer) {
    const unsigned long remainingMs = (exposureDurationMs > exposureElapsedMs) ? (exposureDurationMs - exposureElapsedMs) : 0UL;
    const unsigned int remainingMinutes = remainingMs / 60000UL;
    const unsigned int remainingSeconds = (remainingMs % 60000UL) / 1000UL;

    lcd.setCursor(6, 0);
    lcd.print(isExposureTimeRemainingSelected ? ">" : " ");
    lcd.print("RESTANT:");
    if (remainingMinutes < 10U) {
      lcd.print('0');
    }
    lcd.print(remainingMinutes);
    lcd.print(':');
    if (remainingSeconds < 10U) {
      lcd.print('0');
    }
    lcd.print(remainingSeconds);

    machine.lastTimerRefreshMs = now;
  }

  machine.needsRefresh = false;
  machine.forceRefresh = false;
}
