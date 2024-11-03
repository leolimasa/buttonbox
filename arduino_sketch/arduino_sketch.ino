
struct KnobValues {
  int a;
  int b;
  int sw;
};

enum KnobDirection {
  CLOCKWISE,
  COUNTERCLOCKWISE,
  UNKNOWN,
};

struct Knob {
  int pin_a;
  int pin_b;
  int pin_switch;
  struct KnobValues last;
  struct KnobValues current;
  enum KnobDirection direction;
  bool direction_changed;
};

struct Knob knobs[1];

void read_inputs(struct Knob *knob, struct KnobValues *knob_values) {
  knob_values->a = digitalRead(knob->pin_a);
  knob_values->b = digitalRead(knob->pin_b);
  knob_values->sw = digitalRead(knob->pin_switch);
}

void setup_knob(struct Knob *knob) {
  pinMode(knob->pin_a, INPUT_PULLUP);
  pinMode(knob->pin_b, INPUT_PULLUP);
  pinMode(knob->pin_switch, INPUT_PULLUP);

  read_inputs(knob, &knob->last);
  read_inputs(knob, &knob->current);
  knob->direction = UNKNOWN;
  knob->direction_changed = false;

  Serial.print("Setup complete\n");
}

void update_knob(struct Knob *knob) {
  knob->last = knob->current;
  read_inputs(knob, &knob->current);

  bool a_high_to_low = knob->last.a == HIGH && knob->current.a == LOW;

  if (a_high_to_low) {
    if (knob->current.b == LOW) {
      knob->direction = COUNTERCLOCKWISE;
    } else {
      knob->direction = CLOCKWISE;
    }
  } else {
    knob->direction = UNKNOWN;
  }
}

void print_pin_status(struct Knob *knob) {
  Serial.print(knob->last.a);
  Serial.print(knob->last.b);
  Serial.print(knob->last.sw);
  Serial.print(" ");
  Serial.print(knob->current.a);
  Serial.print(knob->current.b);
  Serial.print(knob->current.sw);
  Serial.print("\n");
}

void print_knob_events(struct Knob *knob) {
  switch (knob->direction) {
    case CLOCKWISE:
      print_pin_status(knob);
      Serial.print("CLOCKWISE\n");
      break;
    case COUNTERCLOCKWISE:
      print_pin_status(knob);
      Serial.print("COUNTERCLOCKWISE\n");
      break;
    case UNKNOWN:
      break;
  }

  knob->direction_changed = false;
}

void setup() {
  Serial.begin(115200);

  knobs[0].pin_a = 9;
  knobs[0].pin_b = 11;
  knobs[0].pin_switch = 18;

  setup_knob(&knobs[0]);
}

void loop() {

  Knob *knob = &knobs[0];
  update_knob(knob);
  print_knob_events(knob);
}
