#ifndef ENCODER_STATE_MACHINE_H
#define ENCODER_STATE_MACHINE_H

#include "globals.h"

enum EncoderState {
  ENCODER_IDLE,
  ENCODER_MOVING
};

struct EncoderMachine {
  EncoderState state;
  uint8_t previousState;
  int8_t accumulator;
  unsigned long lastButtonChangeMs;
  unsigned long lastEncoderChangeMs;
  bool buttonPressed;
  bool buttonChanged;
  bool buttonDebounced;
  bool buttonEventConsumed;
  int rotationCount;
  bool rotationCountConsumed;
};

void encoderInit(EncoderMachine &machine);
void encoderUpdate(EncoderMachine &machine);
void encoderOutput(EncoderMachine &machine);

#endif
