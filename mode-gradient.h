namespace ModeGradient {

int this_color;
float rainbow_steps;

byte period = 1;   // RAINBOW_PERIOD
float step = 0.5;  // RAINBOW_STEP_2

void step_f(CRGB *leds, int num_leds, unsigned long elapsed) {
  if (elapsed > 30) {
    this_color += period;
    if (this_color > 255) this_color = 0;
    if (this_color < 0) this_color = 255;
  }

  rainbow_steps = this_color;

  for (int i = 0; i < num_leds; i++) {
    leds[i] = CHSV((int)floor(rainbow_steps), 255, 255);
    rainbow_steps += step;
    if (rainbow_steps > 255) rainbow_steps = 0;
    if (rainbow_steps < 0) rainbow_steps = 255;
  }
}

}

// btn up
// RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, 0.5, 0.5, 10);

// btn left
// RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, -1, -20, 20);

const struct Modes mode_s_gradient = {
  ModeGradient::step_f
};
