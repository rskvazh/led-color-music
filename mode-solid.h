// case 1:
//   if (millis() - color_timer > COLOR_SPEED) {
//     color_timer = millis();
//     if (++this_color > 255) this_color = 0;
//   }
//   for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, LIGHT_SAT, 255);
//   break;

namespace ModeSolid {

byte LIGHT_COLOR = 0;  // начальный цвет подсветки
byte LIGHT_SAT = 255;

void step_f(CRGB *leds, int num_leds, unsigned long elapsed) {
  for (int i = 0; i < num_leds; i++) {
    leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, 255);
  }
}

}

// byte COLOR_SPEED = 100;

// button up
// LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);

const struct Modes mode_s_solid = {
  ModeSolid::step_f
};
