namespace ModeGradient {

unsigned long last_color_time_ms;

int this_color;

byte period = 1;
float rainbow_step = 0.5;

void activate(CRGB* leds, int num_leds) {
  this_color = 0;
}

void step(CRGB* leds, int num_leds, unsigned long time_ms) {
  if (time_ms - last_color_time_ms > 30) {
    last_color_time_ms = time_ms;

    this_color = cycled(this_color + period, 0, 255);
  }

  float rainbow_steps = this_color;

  for (int i = 0; i < num_leds; i++) {
    leds[i] = CHSV((int)floor(rainbow_steps), 255, 255);

    rainbow_steps = cycled(rainbow_steps + rainbow_step, 0, 255);
  }
}

const struct Modes get_mode() {
  return create_modes_s(step, activate);
}
}

// btn up
// RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, 0.5, 0.5, 10);

// btn left
// RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, -1, -20, 20);