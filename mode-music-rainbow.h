namespace ModeMusicRainbow {

float SMOOTH = 0.3;   // коэффициент плавности анимации VU (по умолчанию 0.5)
#define MAX_COEF 1.8  // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)
float RAINBOW_STEP = 5.00;  // шаг изменения цвета радуги
int hue;

float averageLevel = 50;
float averK = 0.006;
int maxLevel = 100;
int Rlenght, Llenght;

int MAX_CH = 60;  //NUM_LEDS / 2;
float index = (float)255 / MAX_CH;  // коэффициент перевода для палитры

void step(CRGB* leds, int num_leds, unsigned long time_ms) {
  float RsoundLevel = 0;
  int RcurrentLevel = 0;
  float RsoundLevel_f = 0;

  for (byte i = 0; i < 100; i++) {                   // делаем 100 измерений
    RcurrentLevel = analogRead(SOUND_R);             // с правого
    if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;  // ищем максимальное
  }

  // фильтруем по нижнему порогу шумов
  RsoundLevel = map(RsoundLevel, Audio::LOW_PASS, 1023, 0, 500);

  // ограничиваем диапазон
  RsoundLevel = constrain(RsoundLevel, 0, 500);

  // возводим в степень (для большей чёткости работы)
  RsoundLevel = pow(RsoundLevel, EXP);

  // фильтр
  RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);

  // заливаем "подложку", если яркость достаточная
  // if (EMPTY_BRIGHT > 5) {
  //   for (int i = 0; i < num_leds; i++)
  //     leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
  // }

  // если значение выше порога - начинаем самое интересное
  if (RsoundLevel_f > 15) {

    // расчёт общей средней громкости с обоих каналов, фильтрация.
    // Фильтр очень медленный, сделано специально для автогромкости
    averageLevel = (float)(RsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

    // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
    maxLevel = (float)averageLevel * MAX_COEF;

    // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
    Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);

    // ограничиваем до макс. числа светодиодов
    Rlenght = constrain(Rlenght, 0, MAX_CH);

    // animation
    EVERY_N_MILLISECONDS(30) {
      hue = floor((float)hue + RAINBOW_STEP);
    }

    int count = 0;
    for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
      leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
      count++;
    }
    count = 0;
    for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++) {
      leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
      count++;
    }
    
    // if (EMPTY_BRIGHT > 0) {
    //   CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
    //   for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
    //     leds[i] = this_dark;
    //   for (int i = MAX_CH + Llenght; i < num_leds; i++)
    //     leds[i] = this_dark;
    // }
  }
}

void forward() {
  RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, 0.5, 0.5, 20);
  // SMOOTH = smartIncrFloat(SMOOTH, -0.05, 0.05, 1);
}

void backward() {
}

void action() {
}


const struct Modes get_mode() {
  return create_modes_s(step, NULL, forward, backward, action);
}
}