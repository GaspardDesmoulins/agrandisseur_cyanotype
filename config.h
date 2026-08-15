#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pins du module encodeur KY-040
constexpr uint8_t PIN_SM_ENCODER_CLK = 4;
constexpr uint8_t PIN_SM_ENCODER_DT = 2;
constexpr uint8_t PIN_SM_ENCODER_SW = 3;

// Pins des machines de sortie
constexpr uint8_t PIN_SM_RELAY = 5;
constexpr uint8_t PIN_SM_FAN = 6;
constexpr uint8_t PIN_SM_SERVO_PAN = 10;
constexpr uint8_t PIN_SM_SERVO_TILT = 11;

// Pins du convertisseur thermocouple MAX6675
constexpr uint8_t PIN_SM_MAX6675_SO = 12;
constexpr uint8_t PIN_SM_MAX6675_CS = 7;
constexpr uint8_t PIN_SM_MAX6675_SCK = 13;

// Pins de l'afficheur LCD
constexpr uint8_t PIN_SM_LCD_SDA = 8;
constexpr uint8_t PIN_SM_LCD_SCL = 9;

// Aliases conservés pour compatibilité avec le code existant
constexpr uint8_t PIN_CLK = PIN_SM_ENCODER_CLK;
constexpr uint8_t PIN_DT = PIN_SM_ENCODER_DT;
constexpr uint8_t PIN_SW = PIN_SM_ENCODER_SW;
constexpr uint8_t PIN_RELAY = PIN_SM_RELAY;
constexpr uint8_t PIN_FAN = PIN_SM_FAN;
constexpr uint8_t PIN_SERVO_PAN = PIN_SM_SERVO_PAN;
constexpr uint8_t PIN_SERVO_TILT = PIN_SM_SERVO_TILT;
constexpr uint8_t PIN_MAX6675_SO = PIN_SM_MAX6675_SO;
constexpr uint8_t PIN_MAX6675_CS = PIN_SM_MAX6675_CS;
constexpr uint8_t PIN_MAX6675_SCK = PIN_SM_MAX6675_SCK;
constexpr uint8_t PIN_LCD_SDA = PIN_SM_LCD_SDA;
constexpr uint8_t PIN_LCD_SCL = PIN_SM_LCD_SCL;

constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;
constexpr uint8_t LCD_I2C_ADDR = 0x27;

constexpr float DEFAULT_MAX_JUNCTION_TEMP_C = 50.0f;
constexpr float WARNING_TEMP_C = 45.0f;

constexpr int SERVO_PAN_NEUTRAL_DEG = 100;
constexpr int SERVO_TILT_NEUTRAL_DEG = 86;
constexpr int SERVO_PAN_MIN_DEG = 40;
constexpr int SERVO_PAN_MAX_DEG = 160;
constexpr int SERVO_TILT_MIN_DEG = 60;
constexpr int SERVO_TILT_MAX_DEG = 115;
constexpr int DEFAULT_SERVO_PAN_MAX_ANGLE_DEG = 50;
constexpr int DEFAULT_SERVO_TILT_MAX_ANGLE_DEG = 20;
constexpr int SERVO_ELLIPSE_STEP_DEG = 1;
constexpr unsigned long DEFAULT_SERVO_ELLIPSE_INTERVAL_MS = 40UL;
constexpr unsigned long MIN_SERVO_ELLIPSE_INTERVAL_MS = 10UL;
constexpr unsigned long MAX_SERVO_ELLIPSE_INTERVAL_MS = 500UL;
constexpr unsigned long SERVO_ELLIPSE_INTERVAL_STEP_MS = 10UL;
constexpr uint8_t SERVO_PRESET_COUNT = 5;

#endif
