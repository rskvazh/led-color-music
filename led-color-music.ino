/*
  Base on 2.10 version of https://github.com/AlexGyver/ColorMusic
  More info: https://alexgyver.ru/colormusic/
*/

// Sketch uses 19088 bytes (62%) of program storage space. Maximum is 30720 bytes.
// Global variables use 1215 bytes (59%) of dynamic memory, leaving 833 bytes for local variables. Maximum is 2048 bytes.

// #define FASTLED_ALLOW_INTERRUPTS 1
#include "FastLED.h"

#include "common.h"
#include "Remote.h"

#define SOUND_R A2       // аналоговый пин вход аудио, правый канал
#define SOUND_R_FREQ A3  // аналоговый пин вход аудио для режима с частотами (через кондер)

#include "audio.h"

#include "mode-gradient.h"
#include "mode-solid-anim.h"
#include "mode-solid.h"
#include "mode-strobe.h"
#include "mode-music-rainbow.h"

#define DEFAULT_MODE ModeMusicRainbow::get_mode()

const byte OLD_MODE_2 = 13;
const byte OLD_MODE_3 = 14;
const byte OLD_MODE_4 = 15;
const byte OLD_MODE_6 = 17;
const byte OLD_MODE_7 = 18;
const byte OLD_MODE_8 = 19;

// ----- настройки ленты
// #define NUM_LEDS 467 // Production
#define NUM_LEDS 120

byte brightness = 255;  // яркость по умолчанию (0 - 255)

// ----- пины подключения
#define MLED_PIN 13  // пин светодиода режимов
#define MLED_ON HIGH
#define LED_PIN 12  // пин DI светодиодной ленты

#define POT_GND A0  // пин земля для потенциометра
#define IR_PIN 2    // пин ИК приёмника

// ----- отрисовка
#define MAIN_LOOP 5  // период основного цикла отрисовки (по умолчанию 5)

// ----- сигнал
#define POTENT 1                // 1 - используем потенциометр, 0 - используется внутренний источник опорного напряжения 1.1 В
byte EMPTY_BRIGHT = 0;          // яркость "не горящих" светодиодов (0 - 255)
#define EMPTY_COLOR HUE_PURPLE  // цвет "не горящих" светодиодов. Будет чёрный, если яркость 0

// ----- режим цветомузыки
float SMOOTH_FREQ = 0.8;       // коэффициент плавности анимации частот (по умолчанию 0.8)
float MAX_COEF_FREQ = 1.2;     // коэффициент порога для "вспышки" цветомузыки (по умолчанию 1.5)
#define SMOOTH_STEP 20         // шаг уменьшения яркости в режиме цветомузыки (чем больше, тем быстрее гаснет)
#define LOW_COLOR HUE_RED      // цвет низких частот
#define MID_COLOR HUE_GREEN    // цвет средних
#define HIGH_COLOR HUE_YELLOW  // цвет высоких


// ----- режим бегущих частот
byte RUNNING_SPEED = 11;

// ----- режим анализатора спектра
byte HUE_START = 0;
byte HUE_STEP = 5;
#define LIGHT_SMOOTH 2

/*
  Цвета для HSV
  HUE_RED
  HUE_ORANGE
  HUE_YELLOW
  HUE_GREEN
  HUE_AQUA
  HUE_BLUE
  HUE_PURPLE
  HUE_PINK
*/

// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------
#define STRIPE NUM_LEDS / 5
float freq_to_stripe = NUM_LEDS / 40;  // /2 так как симметрия, и /20 так как 20 частот

CRGB leds[NUM_LEDS];

#include "IRLremote.h"
CHashIR IRLremote;
uint32_t IRdata;

unsigned long main_timer, running_timer, color_timer;

boolean lowFlag;
int colorMusic[3];
float colorMusic_f[3], colorMusic_aver[3];
boolean colorMusicFlash[3];
byte currentMode = 255;
int thisBright[3];

volatile boolean ir_flag;
boolean ONstate = true;
int8_t freq_strobe_mode;
int8_t light_mode = 2;
int freq_max;
float freq_max_f;
int freq_f[32];
boolean running_flag[3], eeprom_flag;

// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.clear(true);

  pinMode(MLED_PIN, OUTPUT);         //Режим пина для светодиода режима на выход
  digitalWrite(MLED_PIN, !MLED_ON);  //Выключение светодиода режима

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);

  IRLremote.begin(IR_PIN);

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
  // в данной схеме GND берётся из А0 для удобства подключения
  if (POTENT) analogReference(EXTERNAL);
  else
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif

  // жуткая магия, меняем частоту оцифровки до 18 кГц
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) частота дискретизации будет 19.2 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  Audio::autoLowPass();

  changeMode(DEFAULT_MODE);
}

void loop() {
  remoteTick();  // опрос ИК пульта

  if (ONstate) {
    mainLoop();  // главный цикл обработки и отрисовки
  }
}

Modes current_mode_s;

void changeMode(Modes newMode) {
  current_mode_s = newMode;
  if (current_mode_s.activate != NULL) {
    current_mode_s.activate(leds, NUM_LEDS);
  }
}

void mainLoop() {
  unsigned long time_ms = millis();

  if (time_ms - main_timer > MAIN_LOOP) {
    current_mode_s.step(leds, NUM_LEDS, time_ms);

    // 3-5 режим - цветомузыка
    if (currentMode == OLD_MODE_2 || currentMode == OLD_MODE_3 || currentMode == OLD_MODE_4 || currentMode == OLD_MODE_7 || currentMode == OLD_MODE_8) {
      // Audio::analyzeAudio();
      // colorMusic[0] = 0;
      // colorMusic[1] = 0;
      // colorMusic[2] = 0;
      // for (int i = 0; i < 32; i++) {
      //   if (fht_log_out[i] < Audio::SPEKTR_LOW_PASS) fht_log_out[i] = 0;
      // }
      // // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
      // for (byte i = 2; i < 6; i++) {
      //   if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
      // }
      // // средние частоты, выборка с 6 по 10 тон
      // for (byte i = 6; i < 11; i++) {
      //   if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
      // }
      // // высокие частоты, выборка с 11 по 31 тон
      // for (byte i = 11; i < 32; i++) {
      //   if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
      // }
      // freq_max = 0;
      // for (byte i = 0; i < 30; i++) {
      //   if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
      //   if (freq_max < 5) freq_max = 5;

      //   if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
      //   if (freq_f[i] > 0) freq_f[i] -= LIGHT_SMOOTH;
      //   else freq_f[i] = 0;
      // }
      // freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
      // for (byte i = 0; i < 3; i++) {
      //   colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);        // общая фильтрация
      //   colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);  // локальная
      //   if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
      //     thisBright[i] = 255;
      //     colorMusicFlash[i] = true;
      //     running_flag[i] = true;
      //   } else colorMusicFlash[i] = false;
      //   if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
      //   if (thisBright[i] < EMPTY_BRIGHT) {
      //     thisBright[i] = EMPTY_BRIGHT;
      //     running_flag[i] = false;
      //   }
      // }
      // animation();
    }

    if (!IRLremote.receiving()) {
      FastLED.show();
    }

    // if (currentMode != OLD_MODE_7)  // 7 режиму не нужна очистка!!!
    // FastLED.clear();              // очистить массив пикселей

    main_timer = millis();  // сбросить таймер
  }
}

void animation() {
  // согласно режиму
  switch (currentMode) {
    case OLD_MODE_2:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < STRIPE) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < STRIPE * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
        else if (i < STRIPE * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
      }
      break;
    case OLD_MODE_3:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < NUM_LEDS / 3) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < NUM_LEDS * 2 / 3) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < NUM_LEDS) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
      }
      break;
    case OLD_MODE_4:
      switch (freq_strobe_mode) {
        case 0:
          if (colorMusicFlash[2]) HIGHS();
          else if (colorMusicFlash[1]) MIDS();
          else if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
        case 1:
          if (colorMusicFlash[2]) HIGHS();
          else SILENCE();
          break;
        case 2:
          if (colorMusicFlash[1]) MIDS();
          else SILENCE();
          break;
        case 3:
          if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
      }
      break;
    case OLD_MODE_7:
      switch (freq_strobe_mode) {
        case 0:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 1:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 2:
          if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 3:
          if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
      }
      leds[(NUM_LEDS / 2) - 1] = leds[NUM_LEDS / 2];
      if (millis() - running_timer > RUNNING_SPEED) {
        running_timer = millis();
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {
          leds[i] = leds[i + 1];
          leds[NUM_LEDS - i - 1] = leds[i];
        }
      }
      break;
    case OLD_MODE_8:
      byte HUEindex = HUE_START;
      for (int i = 0; i < NUM_LEDS / 2; i++) {
        byte this_bright = map(freq_f[(int)floor((NUM_LEDS / 2 - i) / freq_to_stripe)], 0, freq_max_f, 0, 255);
        this_bright = constrain(this_bright, 0, 255);
        leds[i] = CHSV(HUEindex, 255, this_bright);
        leds[NUM_LEDS - i - 1] = leds[i];
        HUEindex += HUE_STEP;
        if (HUEindex > 255) HUEindex = 0;
      }
      break;
  }
}

void HIGHS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
}
void MIDS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
}
void LOWS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
}
void SILENCE() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
}

void remoteTick() {
  if (IRLremote.available()) {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;
  }
  if (ir_flag) {
    switch (IRdata) {
      // режимы
      case BUTT_1:
        changeMode(ModeGradient::get_mode());
        break;
      case BUTT_2:
        changeMode(ModeSolidAnim::get_mode());
        break;
      case BUTT_3:
        changeMode(ModeSolid::get_mode());
        break;
      case BUTT_4:
        changeMode(ModeStrobe::get_mode());
        break;
      case BUTT_5:
        changeMode(ModeMusicRainbow::get_mode());
        break;
      case BUTT_6:

        break;
      case BUTT_7:

        break;
      case BUTT_8:

        break;
      case BUTT_9:

        break;
      case BUTT_0:
        Audio::fullLowPass();
        break;
      case BUTT_STAR:
        ONstate = !ONstate;
        FastLED.clear();
        FastLED.show();
        break;
      case BUTT_HASH:
        switch (currentMode) {
          case OLD_MODE_4:
          case OLD_MODE_7:
            if (++freq_strobe_mode > 3) freq_strobe_mode = 0;
            break;
          case OLD_MODE_6:
            if (++light_mode > 2) light_mode = 0;
            break;
        }
        break;
      case BUTT_OK:
        if (current_mode_s.action) {
          current_mode_s.action();
        }
        break;
      case BUTT_UP:
        brightness = smartIncr(brightness, 17, 0, 255);
        FastLED.setBrightness(brightness);
        break;

        switch (currentMode) {
          case OLD_MODE_2:
          case OLD_MODE_3:
          case OLD_MODE_4:
            MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0, 5);
            break;
          case OLD_MODE_7:
            MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0.0, 10);
            break;
          case OLD_MODE_8:
            HUE_START = smartIncr(HUE_START, 10, 0, 255);
            break;
        }
        break;
      case BUTT_DOWN:
        brightness = smartIncr(brightness, -17, 0, 255);
        FastLED.setBrightness(brightness);
        break;

        switch (currentMode) {
          case OLD_MODE_2:
          case OLD_MODE_3:
          case OLD_MODE_4:
            MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0, 5);
            break;
          case OLD_MODE_7:
            MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0.0, 10);
            break;
          case OLD_MODE_8:
            HUE_START = smartIncr(HUE_START, -10, 0, 255);
            break;
        }
        break;
      case BUTT_LEFT:
        if (current_mode_s.backward != NULL) {
          current_mode_s.backward();
        }

        switch (currentMode) {
          case OLD_MODE_2:
          case OLD_MODE_3:
          case OLD_MODE_4:
            SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, -0.05, 0.05, 1);
            break;
          case OLD_MODE_7:
            RUNNING_SPEED = smartIncr(RUNNING_SPEED, -10, 1, 255);
            break;
          case OLD_MODE_8:
            HUE_STEP = smartIncr(HUE_STEP, -1, 1, 255);
            break;
        }

        break;
      case BUTT_RIGHT:
        if (current_mode_s.forward != NULL) {
          current_mode_s.forward();
        }

        switch (currentMode) {
          case OLD_MODE_2:
          case OLD_MODE_3:
          case OLD_MODE_4:
            SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, 0.05, 0.05, 1);
            break;
          case OLD_MODE_7:
            RUNNING_SPEED = smartIncr(RUNNING_SPEED, 10, 1, 255);
            break;
          case OLD_MODE_8:
            HUE_STEP = smartIncr(HUE_STEP, 1, 1, 255);
            break;
        }
        break;
    }
    ir_flag = false;
  }
}
