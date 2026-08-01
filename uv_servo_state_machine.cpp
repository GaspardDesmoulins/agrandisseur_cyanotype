#include "uv_servo_state_machine.h"

constexpr int SERVO_PAN_NEUTRAL_DEG = 100;
constexpr int SERVO_TILT_NEUTRAL_DEG = 97;
constexpr int SERVO_SWEEP_STEP_DEG = 10;

void uvServoInit(UvServoMachine &machine) {
  machine.state = UV_SERVO_CENTER;
  machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
  machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
  machine.needsOutput = true;

  machine.panServo.attach(PIN_SM_SERVO_PAN);
  machine.tiltServo.attach(PIN_SM_SERVO_TILT);
}

void uvServoUpdate(UvServoMachine &machine) {
  if (safetyTrip || !exposureActive) {
    machine.state = UV_SERVO_CENTER;
    machine.panAngle = SERVO_PAN_NEUTRAL_DEG;
    machine.tiltAngle = SERVO_TILT_NEUTRAL_DEG;
    machine.needsOutput = true;
    return;
  }

  servoSweepPhaseDeg = (servoSweepPhaseDeg + SERVO_SWEEP_STEP_DEG) % 360;

  const float angleRad = servoSweepPhaseDeg * 0.0174532925f;
  const int panOffset = static_cast<int>(sin(angleRad) * servoSweepRadiusDeg);
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

  machine.panServo.write(machine.panAngle);
  machine.tiltServo.write(machine.tiltAngle);
  machine.needsOutput = false;
}
