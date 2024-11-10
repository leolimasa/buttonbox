
#define DEBUG
#define JOYSTICK_BUTTON_HOLD_MS 50
// #define DEBUG true
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
  int jstkCcwBtn;
  int jstkCwBtn;
  int jstkSwBtn;
  unsigned long lastCwJstkBtnTime;
  unsigned long lastCcwJstkBtnTime;
  bool jstkCcwPressed;
  bool jstkCwPressed;

  Knob(uint8_t pinA, uint8_t pinB, int pinSwitch, int jstkCcwBtn, int jstkCwBtn, int jstkSwBtn) 
    : encoder(pinA, pinB)
    , pinSwitch(pinSwitch)
    , oldPosition(-999)
    , position(0)
    , lastDirection(NO_DIRECTION)
    , direction(NO_DIRECTION)
    , isButtonPressed(false)
    , jstkCcwBtn(jstkCcwBtn)
    , jstkCwBtn(jstkCwBtn)
    , jstkSwBtn(jstkSwBtn)
    , jstkCcwPressed(false)
    , jstkCwPressed(false)
    {}
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

void updateJoystickButtons(struct Knob *knob) {
  unsigned long currentTime = millis();
  KnobDirection direction = getKnobStepDirection(knob);

  // clockwise press
  if (direction == CLOCKWISE) {
    knob->lastCwJstkBtnTime = currentTime;
    #ifdef DEBUG
    Serial.print("clockwise ");
    #endif
    if (!knob->jstkCwPressed) {
      knob->jstkCwPressed = true;
      Joystick.setButton(knob->jstkCwBtn, true);
      #ifdef DEBUG
      Serial.print("press");
      #endif
    }

    #ifdef DEBUG
    Serial.println();
    #endif
  }

  // clockwise release
  if (currentTime - knob->lastCwJstkBtnTime > JOYSTICK_BUTTON_HOLD_MS && direction != CLOCKWISE && knob->jstkCwPressed) {
    Joystick.setButton(knob->jstkCwBtn, false);
    knob->jstkCwPressed = false;
    #ifdef DEBUG
    Serial.println("clockwise release");
    #endif
  }

  // counterclockwise press
  if (direction == COUNTERCLOCKWISE) {
    knob->lastCcwJstkBtnTime = currentTime;
    #ifdef DEBUG
    Serial.print("counter-clockwise ");
    #endif
    if (!knob->jstkCcwPressed) {
      knob->jstkCcwPressed = true;
      Joystick.setButton(knob->jstkCcwBtn, true);
      #ifdef DEBUG
      Serial.print("press");
      #endif
    }

    #ifdef DEBUG
    Serial.println();
    #endif
  }

  // counterclockwise release
  if (currentTime - knob->lastCcwJstkBtnTime > JOYSTICK_BUTTON_HOLD_MS && direction != COUNTERCLOCKWISE && knob->jstkCcwPressed) {
    Joystick.setButton(knob->jstkCcwBtn, false);
    knob->jstkCcwPressed = false;
    #ifdef DEBUG
    Serial.println("counter-clockwise release");
    #endif
  }

  // button press
  if (getKnobButtonEvent(knob) == BUTTON_PRESSED) {
    Joystick.setButton(knob->jstkSwBtn, true);
    #ifdef DEBUG
    Serial.println("button pressed");
    #endif
  } 

  // button release
  if (getKnobButtonEvent(knob) == BUTTON_RELEASED) {
    Joystick.setButton(knob->jstkSwBtn, false);
    #ifdef DEBUG
    Serial.println("button released");
    #endif
  }
}


constexpr int knobsLen = 1;
Knob knobs[knobsLen] = {
  Knob(11, 9, 18, 0, 1, 2)
};

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  Joystick.begin();

  for (int i = 0; i < knobsLen; i++) {
    setupKnob(&knobs[i]);
  }
}

void loop() {
  for (int i = 0; i < knobsLen; i++) {
    Knob *knob = &knobs[i];
    updateKnob(knob);
    updateJoystickButtons(knob);
  }
}
