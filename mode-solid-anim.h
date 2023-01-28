namespace ModeSolidAnim {

int current_color;
byte COLOR_SPEED;

unsigned long last_time_ms;

byte LIGHT_COLOR = 0;  // начальный цвет подсветки
byte LIGHT_SAT = 255;

void activate(CRGB* leds, int num_leds) {
  current_color = 0;
  COLOR_SPEED = 100;
}

void step(CRGB* leds, int num_leds, unsigned long time_ms) {
  if (time_ms - last_time_ms > COLOR_SPEED) {
    last_time_ms = time_ms;
    current_color = cycled(current_color + 1, 0, 255);
  }

  for (int i = 0; i < num_leds; i++) {
    leds[i] = CHSV(current_color, LIGHT_SAT, 255);
  }
}

const struct Modes mode = create_modes_s(step, activate);
const struct Modes* mode_ptr = &mode;
}

// byte COLOR_SPEED = 100;

// button up
// LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
