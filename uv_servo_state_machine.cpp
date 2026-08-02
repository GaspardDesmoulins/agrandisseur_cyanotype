#include "uv_servo_state_machine.h"

constexpr int SERVO_PAN_NEUTRAL_DEG = 100;
constexpr int SERVO_TILT_NEUTRAL_DEG = 86;
constexpr int SERVO_SWEEP_STEP_DEG = 1;
#define SERVO_PAN_ANGLE_MULTIPLIER 6.0f
constexpr int SERVO_PAN_MAX_OFFSET_DEG = 60;
constexpr int SERVO_MIN_DEG = 0;
constexpr int SERVO_MAX_DEG = 180;

int uvServoMaxSweepRadiusDeg() {
  const int panMaxRadiusDeg = static_cast<int>(SERVO_PAN_MAX_OFFSET_DEG / SERVO_PAN_ANGLE_MULTIPLIER);
  const int tiltNegativeMarginDeg = SERVO_TILT_NEUTRAL_DEG - SERVO_MIN_DEG;
  const int tiltPositiveMarginDeg = SERVO_MAX_DEG - SERVO_TILT_NEUTRAL_DEG;
  const int tiltMaxRadiusDeg = min(tiltNegativeMarginDeg, tiltPositiveMarginDeg);

  return min(panMaxRadiusDeg, tiltMaxRadiusDeg);
}

void uvServoInit(UvServoMachine &machine) {
  machine.state = UV_SERVO_CENTER;
  machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
  machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
  machine.lastSweepMs = millis();
  machine.pwmEnabled = false;
  machine.needsOutput = false;
}

void uvServoUpdate(UvServoMachine &machine) {
  if (safetyTrip || !servoControlEnabled) {
    machine.state = UV_SERVO_CENTER;
    machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
    machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
    machine.needsOutput = machine.pwmEnabled;
    machine.pwmEnabled = false;
    return;
  }

  const unsigned long now = millis();
  if (!machine.pwmEnabled) {
    machine.panServo.attach(PIN_SM_SERVO_PAN);
    machine.tiltServo.attach(PIN_SM_SERVO_TILT);
    machine.pwmEnabled = true;
    machine.lastSweepMs = now;
  }
  else if (now - machine.lastSweepMs >= servoSweepIntervalMs) {
    servoSweepPhaseDeg = (servoSweepPhaseDeg + SERVO_SWEEP_STEP_DEG) % 360;
    machine.lastSweepMs = now;
  }

  const float angleRad = servoSweepPhaseDeg * 0.0174532925f;
  const int panOffset = static_cast<int>(sin(angleRad) * servoSweepRadiusDeg * SERVO_PAN_ANGLE_MULTIPLIER);
  const int tiltOffset = static_cast<int>(cos(angleRad) * servoSweepRadiusDeg);

  machine.state = UV_SERVO_CENTER;
  machine.panAngle = SERVO_PAN_NEUTRAL_DEG + panOffset;
  machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG + tiltOffset;

  machine.needsOutput = true;
}

void uvServoOutput(UvServoMachine &machine) {
  if (!machine.needsOutput) {
    return;
  }

  if (!machine.pwmEnabled) {
    machine.panServo.detach();
    machine.tiltServo.detach();
    machine.needsOutput = false;
    return;
  }

  machine.panServo.write(machine.panAngle);
  machine.tiltServo.write(machine.tiltAngle);
  machine.needsOutput = false;
}
