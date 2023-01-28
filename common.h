#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

typedef void (*ModeActivateFunction)(CRGB *leds, int num_leds);
typedef void (*ModeStepFunction)(CRGB *leds, int num_leds, unsigned long time_ms);
typedef void (*ModeForwardFunction)();
typedef void (*ModeBackwardFunction)();

struct Modes {
  ModeActivateFunction activate;
  ModeStepFunction step;
  ModeForwardFunction forward;
  ModeBackwardFunction backward;
};

const Modes create_modes_s(ModeStepFunction step, ModeActivateFunction activate = NULL, ModeForwardFunction forward = NULL, ModeBackwardFunction backward = NULL) {
  const struct Modes mode = {
    .activate = activate,
    .step = step,
    .forward = forward,
    .backward = backward
  };
  return mode;
}

#define cycled(x, a, b) ((x) < (a) ? (b) : ((x) > (b) ? (a) : (x)))

int smartIncr(int value, int incr_step, int mininmum, int maximum) {
  int val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

float smartIncrFloat(float value, float incr_step, float mininmum, float maximum) {
  float val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}