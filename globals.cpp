#include "globals.h"

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS, PIN_SM_LCD_SDA, PIN_SM_LCD_SCL);

bool boutonPresse = false;
bool uvLedRequested = false;
bool uvLedEnabled = false;
float junctionTempC = 25.0f;
bool junctionTempSensorFault = true;
uint8_t fanDutyCycle = 0;
bool safetyTrip = false;
bool servoControlEnabled = false;
int menuExposedSelection = 0;
bool menuExposedAlarmActive = false;
unsigned long exposureDurationMs = 900000UL;
unsigned long exposureElapsedMs = 0UL;
bool exposureActive = false;
float maxJunctionTempC = DEFAULT_MAX_JUNCTION_TEMP_C;
int encoderRotationCount = 0;
bool encoderButtonPressedEvent = false;

SystemState systemState = {false, false, false, false, false};
