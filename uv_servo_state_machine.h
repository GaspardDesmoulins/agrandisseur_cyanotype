#ifndef UV_SERVO_STATE_MACHINE_H
#define UV_SERVO_STATE_MACHINE_H

#include "globals.h"
#include <Servo.h>

enum UvServoState {
  UV_SERVO_CENTER,
  UV_SERVO_LEFT,
  UV_SERVO_RIGHT,
  UV_SERVO_UP,
  UV_SERVO_DOWN
};

struct UvServoMachine {
  UvServoState state;
  Servo panServo;
  Servo tiltServo;
  int panAngle;
  int tiltAngle;
  bool needsOutput;
};

void uvServoInit(UvServoMachine &machine);
void uvServoUpdate(UvServoMachine &machine);
void uvServoOutput(UvServoMachine &machine);

#endif
