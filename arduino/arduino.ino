
#define DEBUG
#define JOYSTICK_BUTTON_HOLD_MS 50
// #define DEBUG true
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include "protocol.pb.h"

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
    , isButtonPressed(false)
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

void sendSerialMessage(uint8_t *message, uint8_t len) {
  // Write the 4 byte magic number to identify the start of a message
  Serial.write(0x3d);
  Serial.write(0x01);
  Serial.write(0x6f);
  Serial.write(0x31);

  // Write the message length
  Serial.write(len);

  // Write the message
  for (int i = 0; i < len; i++) {
    Serial.write(message[i]);
  }
}

void sendMessage(protocol_Message *message) {
  uint8_t buffer[256];
  size_t messageLength;
  bool status;

  // Create a stream that will write to our buffer.
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  // Encode message
  status = pb_encode(&stream, protocol_Message_fields, message);
  messageLength = stream.bytes_written;

  if (!status) {
    #ifdef DEBUG
    Serial.println("Encoding failed");
    #endif
    return;
  }

  sendSerialMessage(buffer, messageLength);
}

void sendRotaryChange(int32_t index, protocol_Direction direction) {
  protocol_RotaryChange rotaryChange = protocol_RotaryChange_init_default;
  rotaryChange.has_index = true;
  rotaryChange.index = index;
  rotaryChange.has_direction = true;
  rotaryChange.direction = direction;

  protocol_Message message = protocol_Message_init_default;
  message.which_payload = protocol_Message_rotary_change_tag;
  message.payload.rotary_change = rotaryChange;

  sendMessage(&message);
}

void sendButtonChange(int32_t index, bool pressed) {
  protocol_ButtonChange buttonChange = protocol_ButtonChange_init_default;
  buttonChange.has_index = true;
  buttonChange.index = index;
  buttonChange.has_pressed = true;
  buttonChange.pressed = pressed;

  protocol_Message message = protocol_Message_init_default;
  message.which_payload = protocol_Message_button_change_tag;
  message.payload.button_change = buttonChange;

  sendMessage(&message);
}

void sendMessages(int index, struct Knob *knob) {
  KnobDirection direction = getKnobStepDirection(knob);

  // clockwise press
  if (direction == CLOCKWISE) {
    sendRotaryChange(index, protocol_Direction_CLOCKWISE);
  }

  // counterclockwise press
  if (direction == COUNTERCLOCKWISE) {
    sendRotaryChange(index, protocol_Direction_COUNTERCLOCKWISE);
  }

  // button press
  if (getKnobButtonEvent(knob) == BUTTON_PRESSED) {
    sendButtonChange(index, true);
  }

  // button release
  if (getKnobButtonEvent(knob) == BUTTON_RELEASED) {
    sendButtonChange(index, false);
  }
}


// void updateJoystickButtons(struct Knob *knob) {
//   unsigned long currentTime = millis();
//   KnobDirection direction = getKnobStepDirection(knob);
//
//   // clockwise press
//   if (direction == CLOCKWISE) {
//     knob->lastCwJstkBtnTime = currentTime;
//     #ifdef DEBUG
//     Serial.print("clockwise ");
//     #endif
//     if (!knob->jstkCwPressed) {
//       knob->jstkCwPressed = true;
//       Joystick.setButton(knob->jstkCwBtn, true);
//       #ifdef DEBUG
//       Serial.print("press");
//       #endif
//     }
//
//     #ifdef DEBUG
//     Serial.println();
//     #endif
//   }
//
//   // clockwise release
//   if (currentTime - knob->lastCwJstkBtnTime > JOYSTICK_BUTTON_HOLD_MS && direction != CLOCKWISE && knob->jstkCwPressed) {
//     Joystick.setButton(knob->jstkCwBtn, false);
//     knob->jstkCwPressed = false;
//     #ifdef DEBUG
//     Serial.println("clockwise release");
//     #endif
//   }
//
//   // counterclockwise press
//   if (direction == COUNTERCLOCKWISE) {
//     knob->lastCcwJstkBtnTime = currentTime;
//     #ifdef DEBUG
//     Serial.print("counter-clockwise ");
//     #endif
//     if (!knob->jstkCcwPressed) {
//       knob->jstkCcwPressed = true;
//       Joystick.setButton(knob->jstkCcwBtn, true);
//       #ifdef DEBUG
//       Serial.print("press");
//       #endif
//     }
//
//     #ifdef DEBUG
//     Serial.println();
//     #endif
//   }
//
//   // counterclockwise release
//   if (currentTime - knob->lastCcwJstkBtnTime > JOYSTICK_BUTTON_HOLD_MS && direction != COUNTERCLOCKWISE && knob->jstkCcwPressed) {
//     Joystick.setButton(knob->jstkCcwBtn, false);
//     knob->jstkCcwPressed = false;
//     #ifdef DEBUG
//     Serial.println("counter-clockwise release");
//     #endif
//   }
//
//   // button press
//   if (getKnobButtonEvent(knob) == BUTTON_PRESSED) {
//     Joystick.setButton(knob->jstkSwBtn, true);
//     #ifdef DEBUG
//     Serial.println("button pressed");
//     #endif
//   } 
//
//   // button release
//   if (getKnobButtonEvent(knob) == BUTTON_RELEASED) {
//     Joystick.setButton(knob->jstkSwBtn, false);
//     #ifdef DEBUG
//     Serial.println("button released");
//     #endif
//   }
// }


constexpr int knobsLen = 1;
Knob knobs[knobsLen] = {
  Knob(11, 9, 18)
};

void setup() {
  Serial.begin(115200);

  //Joystick.begin();

  for (int i = 0; i < knobsLen; i++) {
    setupKnob(&knobs[i]);
  }
}

void loop() {
  for (int i = 0; i < knobsLen; i++) {
    Knob *knob = &knobs[i];
    updateKnob(knob);
    sendMessages(i, knob);
  }
}
