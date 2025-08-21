// Error indication via onboard NeoPixel (21): blocks and blinks red at 1 Hz
#include <math.h>
#define STATUS_NEOPIXEL_PIN 21
#define STATUS_NEOPIXEL_BRIGHTNESS 25  // ~1/10 of full brightness

// Many ESP32 boards wire WS2812 as GRB. Provide a helper to avoid channel confusion.
inline void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b) {
  // Map RGB -> GRB for neopixelWrite
  neopixelWrite(STATUS_NEOPIXEL_PIN, g, r, b);
}

void reportError(const char* message) {
  Serial.printf("ERROR: %s\n", message);
  bool ledOn = false;
  unsigned long lastToggleMs = 0;
  for (;;) {
    unsigned long now = millis();
    if (now - lastToggleMs >= 500) { // toggle every 500ms -> 1 Hz blink
      lastToggleMs = now;
      ledOn = !ledOn;
      if (ledOn) {
        setNeoPixelColor(STATUS_NEOPIXEL_BRIGHTNESS, 0, 0); // dim red
      } else {
        setNeoPixelColor(0, 0, 0); // off
      }
    }
    delay(10);
  }
}

static volatile bool np_effect_active = false;
static uint8_t np_effect_r = 0;
static uint8_t np_effect_g = 0;
static uint8_t np_effect_b = 0;
static uint32_t np_effect_duration_ms = 0;
static uint32_t np_effect_elapsed_ms = 0;
static int np_effect_mode = NEOPIXEL_MODE_HOLD;

void neopixelSetTimedColor(uint8_t r, uint8_t g, uint8_t b, uint32_t durationMs, int mode) {
  // Cancel any prior effect and start new one immediately
  if (durationMs == 0) {
    setNeoPixelColor(0, 0, 0);
    np_effect_active = false;
    np_effect_elapsed_ms = 0;
    np_effect_duration_ms = 0;
    return;
  }

  np_effect_r = r;
  np_effect_g = g;
  np_effect_b = b;
  np_effect_duration_ms = durationMs;
  np_effect_elapsed_ms = 0;
  np_effect_mode = mode;
  np_effect_active = true;

  // Apply immediately at full intensity at start
  setNeoPixelColor(r, g, b);
}

void neopixelUpdate(uint32_t deltaMs) {
  if (!np_effect_active) {
    return;
  }
  if (deltaMs == 0) {
    return;
  }

  uint32_t newElapsed = np_effect_elapsed_ms + deltaMs;
  if (newElapsed >= np_effect_duration_ms) {
    setNeoPixelColor(0, 0, 0);
    np_effect_active = false;
    np_effect_elapsed_ms = np_effect_duration_ms;
    return;
  }

  np_effect_elapsed_ms = newElapsed;

  float progress = (float)np_effect_elapsed_ms / (float)np_effect_duration_ms; // [0,1)
  if (progress < 0.0f) progress = 0.0f;
  if (progress > 1.0f) progress = 1.0f;

  float factor = 1.0f;
  switch (np_effect_mode) {
    case NEOPIXEL_MODE_HOLD:
      factor = 1.0f;
      break;
    case NEOPIXEL_MODE_LINEAR:
      factor = 1.0f - progress;
      break;
    case NEOPIXEL_MODE_QUADRATIC: {
      float lin = 1.0f - progress;
      factor = lin * lin;
      break;
    }
    default:
      factor = 1.0f;
      break;
  }

  if (factor <= 0.0f) {
    setNeoPixelColor(0, 0, 0);
    return;
  }

  uint8_t rr = (uint8_t)fminf(255.0f, fmaxf(0.0f, roundf((float)np_effect_r * factor)));
  uint8_t gg = (uint8_t)fminf(255.0f, fmaxf(0.0f, roundf((float)np_effect_g * factor)));
  uint8_t bb = (uint8_t)fminf(255.0f, fmaxf(0.0f, roundf((float)np_effect_b * factor)));
  setNeoPixelColor(rr, gg, bb);
}

