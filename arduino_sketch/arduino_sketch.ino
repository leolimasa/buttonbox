
enum KnobDirection {
  CLOCKWISE,
  COUNTERCLOCKWISE,
  NO_DIRECTION
};

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

struct Knob {
  Encoder encoder;
  int pinSwitch;
  long oldPosition;
  int position;
  KnobDirection direction;
  KnobDirection lastDirection;
  unsigned long lastDirectionChangeTime;

  Knob(uint8_t pinA, uint8_t pinB, int pinSwitch) 
    : encoder(pinA, pinB), pinSwitch(pinSwitch), oldPosition(-999), position(0), lastDirection(NO_DIRECTION), direction(NO_DIRECTION) {}
};

KnobDirection getKnobDirection(struct Knob *knob) {
  if (knob->position > knob->oldPosition) {
    return CLOCKWISE;
  } else if (knob->position < knob->oldPosition) {
    return COUNTERCLOCKWISE;
  }
  return NO_DIRECTION;
}

void updateKnob(struct Knob *knob) {
  knob->position = knob->encoder.read();

  knob->direction = getKnobDirection(knob);

  // If the direction changed too fast, ignore the change
  if (knob->direction != knob->lastDirection) {
    unsigned long currentTime = millis();
    if (currentTime - knob->lastDirectionChangeTime < 70) {
      knob->direction = NO_DIRECTION;
    }
  }

  if (knob->direction != NO_DIRECTION) {
    knob->lastDirection = knob->direction;
    knob->lastDirectionChangeTime = millis();
  }

  knob->oldPosition = knob->position;
}


constexpr int knobsLen = 1;
Knob knobs[knobsLen] = {
  Knob(11, 9, 18)
};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < knobsLen; i++) {
    Knob *knob = &knobs[i];
    updateKnob(knob);

    if (knob->direction != NO_DIRECTION && knob->position % 4 == 0) {
      Serial.print("Direction: ");
      Serial.println(knob->direction == CLOCKWISE ? "CLOCKWISE" : "COUNTERCLOCKWISE");
    }
  }
}
