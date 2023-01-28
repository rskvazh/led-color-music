namespace ModeStrobe {

unsigned long strobe_timer;

unsigned int period = 2000;  // период вспышек, миллисекунды
#define STROBE_DUTY 0.2      // скважность вспышек (1 - 99) - отношение времени вспышки ко времени темноты

#define MODE_STROBE_NUM_COLOR 2

CRGB colors[MODE_STROBE_NUM_COLOR] = {
  CRGB::White,
  CHSV(247, 255, 255)
};

byte current_color_i = 0;
boolean is_on = false;

#define MODE_STROBE_NUM_PERIODS 9
unsigned int periods[MODE_STROBE_NUM_PERIODS] = {
  1000,
  750,
  500,
  300,
  220,
  140,
  100,
  60,
  40,
};

int current_period_i = 0;

void step(CRGB* leds, int num_leds, unsigned long time_ms) {
  unsigned int light_time = periods[current_period_i] * STROBE_DUTY;

  unsigned int elapsed = time_ms - strobe_timer;
  if (elapsed > periods[current_period_i]) {
    strobe_timer = time_ms;
    is_on = true;
  } else if (elapsed > light_time) {
    is_on = false;
  }

  if (is_on) {
    fill_solid(leds, num_leds, colors[current_color_i]);
  } else {
    fill_solid(leds, num_leds, CRGB::Black);
  }
}

void forward() {
  current_period_i = constrain(current_period_i + 1, 0, MODE_STROBE_NUM_PERIODS - 1);
}

void backward() {
  current_period_i = constrain(current_period_i - 1, 0, MODE_STROBE_NUM_PERIODS - 1);
}

void action() {
  current_color_i = cycled(current_color_i + 1, 0, MODE_STROBE_NUM_COLOR - 1);
}

const struct Modes get_mode() {
  return create_modes_s(step, NULL, forward, backward, action);
}
}