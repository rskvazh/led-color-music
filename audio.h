#define LOG_OUT 1
#include <FHT.h>  // преобразование Хартли

namespace Audio {

uint16_t LOW_PASS = 100;        // нижний порог шумов режим VU, ручная настройка
#define LOW_PASS_ADD 3          // "добавочная" величина к нижнему порогу, для надёжности (режим VU)
#define LOW_PASS_FREQ_ADD 3     // "добавочная" величина к нижнему порогу, для надёжности (режим частот)
#define FHT_N 64  // ширина спектра х2
uint16_t SPEKTR_LOW_PASS = 40;  // нижний порог шумов режим спектра, ручная настройка
#define EXP 1.4                 // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)

void analyzeAudio() {
  for (int i = 0; i < FHT_N; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample;  // put real data into bins
  }
  fht_window();   // window the data for better frequency response
  fht_reorder();  // reorder the data before doing the fht
  fht_run();      // process the data in the fht
  fht_mag_log();  // take the output of the fht
}

void autoLowPass() {
  // для режима VU
  delay(10);        // ждём инициализации АЦП
  int thisMax = 0;  // максимум
  int thisLevel;
  for (byte i = 0; i < 200; i++) {
    thisLevel = analogRead(SOUND_R);  // делаем 200 измерений
    if (thisLevel > thisMax)          // ищем максимумы
      thisMax = thisLevel;            // запоминаем
    delay(4);                         // ждём 4мс
  }
  LOW_PASS = thisMax + LOW_PASS_ADD;  // нижний порог как максимум тишины + некая величина

  // для режима спектра
  thisMax = 0;
  for (byte i = 0; i < 100; i++) {   // делаем 100 измерений
    analyzeAudio();                  // разбить в спектр
    for (byte j = 2; j < 32; j++) {  // первые 2 канала - хлам
      thisLevel = fht_log_out[j];
      if (thisLevel > thisMax)  // ищем максимумы
        thisMax = thisLevel;    // запоминаем
    }
    delay(4);  // ждём 4мс
  }
  SPEKTR_LOW_PASS = thisMax + LOW_PASS_FREQ_ADD;  // нижний порог как максимум тишины
  // if (EEPROM_LOW_PASS && !AUTO_LOW_PASS) {
  //   EEPROM.updateInt(70, LOW_PASS);
  //   EEPROM.updateInt(72, SPEKTR_LOW_PASS);
  // }
}

void fullLowPass() {
  // digitalWrite(MLED_PIN, MLED_ON);    // включить светодиод
  // FastLED.setBrightness(0);           // погасить ленту
  FastLED.clear();                    // очистить массив пикселей
  FastLED.show();                     // отправить значения на ленту
  delay(500);                         // подождать чутка
  autoLowPass();                      // измерить шумы
  delay(500);                         // подождать
  // FastLED.setBrightness(brightness);  // вернуть яркость
  // digitalWrite(MLED_PIN, !MLED_ON);   // выключить светодиод
}
}