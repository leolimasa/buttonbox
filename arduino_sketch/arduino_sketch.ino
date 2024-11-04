

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

// from https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

enum KnobDirection {
  CLOCKWISE,
  COUNTERCLOCKWISE,
  NO_DIRECTION
};

enum KnobButtonEvent {
  BUTTON_PRESSED,
  BUTTON_RELEASED,
  NO_BUTTON_EVENT
};

Joystick_ Joystick;

struct Knob {
  Encoder encoder;
  int pinSwitch;
  long oldPosition;
  int position;
  KnobDirection direction;
  KnobDirection lastDirection;
  unsigned long lastDirectionChangeTime;
  bool isButtonPressed;
  bool lastButtonState;
  bool buttonChanged;
  unsigned long lastButtonChangeTime;

  Knob(uint8_t pinA, uint8_t pinB, int pinSwitch) 
    : encoder(pinA, pinB)
    , pinSwitch(pinSwitch)
    , oldPosition(-999)
    , position(0)
    , lastDirection(NO_DIRECTION)
    , direction(NO_DIRECTION)
    , isButtonPressed(false) {}
};

KnobDirection getKnobDirection(struct Knob *knob) {
  if (knob->position > knob->oldPosition) {
    return CLOCKWISE;
  } else if (knob->position < knob->oldPosition) {
    return COUNTERCLOCKWISE;
  }
  return NO_DIRECTION;
}

KnobDirection getKnobStepDirection(struct Knob *knob) {
  if (knob->direction != NO_DIRECTION && knob->position % 4 == 0) {
    return knob->direction;
  }
  return NO_DIRECTION;
}

KnobButtonEvent getKnobButtonEvent(struct Knob *knob) {
  if (knob->isButtonPressed && knob->buttonChanged) {
    return BUTTON_PRESSED;
  } else if (!knob->isButtonPressed && knob->buttonChanged) {
    return BUTTON_RELEASED;
  }
  return NO_BUTTON_EVENT;
}

void setupKnob(struct Knob *knob) {
  pinMode(knob->pinSwitch, INPUT_PULLUP);
}

void updateKnob(struct Knob *knob) {
  unsigned long currentTime = millis();

  knob->position = knob->encoder.read();
  knob->direction = getKnobDirection(knob);

  // If the direction changed too fast, ignore the change
  if (knob->direction != knob->lastDirection) {
    if (currentTime - knob->lastDirectionChangeTime < 70) {
      knob->direction = NO_DIRECTION;
    }
  }

  // Update last direction, but only if there was movement
  if (knob->direction != NO_DIRECTION) {
    knob->lastDirection = knob->direction;
    knob->lastDirectionChangeTime = millis();
  }

  // Update button state, but only if the debounce timer has elapsed
  bool isButtonPressed = digitalRead(knob->pinSwitch) == LOW;
  if (currentTime - knob->lastButtonChangeTime > 50) {
    knob->isButtonPressed = isButtonPressed;
    knob->lastButtonChangeTime = currentTime;
  }

  // Detect whether the button state has changed
  if (knob->isButtonPressed != knob->lastButtonState) {
    knob->buttonChanged = true;
  } else {
    knob->buttonChanged = false;
  }

  knob->lastButtonState = knob->isButtonPressed;
  knob->oldPosition = knob->position;
}


constexpr int knobsLen = 1;
Knob knobs[knobsLen] = {
  Knob(11, 9, 18)
};

void setup() {
  Serial.begin(115200);
  Joystick.begin();

  for (int i = 0; i < knobsLen; i++) {
    setupKnob(&knobs[i]);
  }
}

void loop() {
  for (int i = 0; i < knobsLen; i++) {
    Knob *knob = &knobs[i];
    updateKnob(knob);

    KnobDirection stepDirection = getKnobStepDirection(knob);
    if (stepDirection == CLOCKWISE) {
      Joystick.setButton(1, true);
      Serial.println("clockwise");
    } else if (stepDirection == COUNTERCLOCKWISE) {
      Joystick.setButton(2, true);
      Serial.println("counterclockwise");
    }


    KnobButtonEvent buttonEvent = getKnobButtonEvent(knob);
    if (buttonEvent == BUTTON_PRESSED) {
      Joystick.setButton(0, true);
      Serial.println("Button pressed");
    } else if (buttonEvent == BUTTON_RELEASED) {
      Joystick.setButton(0, false);
      Serial.println("Button released");
    }
  }
}
