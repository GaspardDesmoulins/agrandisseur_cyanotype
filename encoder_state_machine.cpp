#include "encoder_state_machine.h"

static const int8_t transitionTable[16] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

uint8_t readEncoderState() {
  uint8_t clk = digitalRead(PIN_SM_ENCODER_CLK);
  uint8_t dt = digitalRead(PIN_SM_ENCODER_DT);
  return (clk << 1) | dt;
}

void encoderInit(EncoderMachine &machine) {
  machine.state = ENCODER_IDLE;
  machine.previousState = readEncoderState();
  machine.accumulator = 0;
  machine.lastButtonChangeMs = millis();
  machine.lastEncoderChangeMs = millis();
  machine.buttonPressed = false;
  machine.buttonChanged = false;
  machine.buttonDebounced = false;
  machine.buttonEventConsumed = true;
  machine.rotationCount = 0;
  machine.rotationCountConsumed = true;
}

void encoderUpdate(EncoderMachine &machine) {
  uint8_t currentState = readEncoderState();
  const bool buttonState = (digitalRead(PIN_SM_ENCODER_SW) == LOW);
  const unsigned long now = millis();

  if (buttonState != machine.buttonPressed) {
    machine.buttonPressed = buttonState;
    machine.buttonChanged = true;
    machine.lastButtonChangeMs = now;
  }

  if (machine.buttonChanged && (now - machine.lastButtonChangeMs) >= 30UL) {
    machine.buttonDebounced = machine.buttonPressed;
    machine.buttonChanged = false;

    if (machine.buttonDebounced && machine.buttonEventConsumed) {
      encoderButtonPressedEvent = true;
      machine.buttonEventConsumed = false;
    }
    else if (!machine.buttonDebounced) {
      machine.buttonEventConsumed = true;
    }
  }

  if (currentState != machine.previousState) {
    if (now - machine.lastEncoderChangeMs >= 2UL) {
      uint8_t index = (machine.previousState << 2) | currentState;
      int8_t movement = transitionTable[index];
      machine.accumulator += movement;
      machine.previousState = currentState;
      machine.state = ENCODER_MOVING;
      machine.lastEncoderChangeMs = now;

      if (machine.accumulator >= 4) {
        machine.accumulator = 0;
        machine.rotationCount++;
        encoderRotationCount++;
        machine.rotationCountConsumed = false;
        systemState.encoderChanged = true;
      }
      else if (machine.accumulator <= -4) {
        machine.accumulator = 0;
        machine.rotationCount--;
        encoderRotationCount--;
        machine.rotationCountConsumed = false;
        systemState.encoderChanged = true;
      }
    }
  }
  else {
    machine.state = ENCODER_IDLE;
  }
}

void encoderOutput(EncoderMachine &machine) {
  (void)machine;
}
