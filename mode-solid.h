namespace ModeSolid {

#define MODE_SOLID_NUM_HUES 9

byte hues[MODE_SOLID_NUM_HUES] = {
  247,  // розовый интим
  215,  // фиолетовый
  186,  // сине-фиолетовый
  159,  // синий
  142,  // голубой
  123,  // бирюзовый
  89,   // лайм
  26,   // warm yellow
  5     // красный апельсин
};

byte current_hue_i = 0;

void step(CRGB* leds, int num_leds, unsigned long time_ms) {
  fill_solid(leds, num_leds, CHSV(hues[current_hue_i], 255, 255));
}

void forward() {
  current_hue_i = cycled(current_hue_i + 1, 0, MODE_SOLID_NUM_HUES - 1);
  Serial.println(hues[current_hue_i]);
}

void backward() {
  current_hue_i = cycled(current_hue_i - 1, 0, MODE_SOLID_NUM_HUES - 1);
  Serial.println(hues[current_hue_i]);
}

const struct Modes get_mode() {
  return create_modes_s(step, NULL, forward, backward);
}
}