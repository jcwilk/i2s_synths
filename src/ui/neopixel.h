#ifndef UI_NEOPIXEL_H
#define UI_NEOPIXEL_H

#include <Arduino.h>
#include <math.h>

// Public modes for time-based neopixel effects
#ifndef NEOPIXEL_MODE_HOLD
#define NEOPIXEL_MODE_HOLD 0
#endif
#ifndef NEOPIXEL_MODE_LINEAR
#define NEOPIXEL_MODE_LINEAR 1
#endif
#ifndef NEOPIXEL_MODE_QUADRATIC
#define NEOPIXEL_MODE_QUADRATIC 2
#endif

#ifndef STATUS_NEOPIXEL_PIN
#define STATUS_NEOPIXEL_PIN 21
#endif
#ifndef STATUS_NEOPIXEL_BRIGHTNESS
#define STATUS_NEOPIXEL_BRIGHTNESS 25
#endif

// Module-local state
static volatile bool np_effect_active = false;
static uint8_t np_effect_r = 0;
static uint8_t np_effect_g = 0;
static uint8_t np_effect_b = 0;
static uint32_t np_effect_duration_ms = 0;
static uint32_t np_effect_elapsed_ms = 0;
static int np_effect_mode = NEOPIXEL_MODE_HOLD;

// Many ESP32 boards wire WS2812 as GRB. Provide a helper to avoid channel confusion.
inline void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(STATUS_NEOPIXEL_PIN, g, r, b);
}

inline void reportError(const char* message) {
  Serial.printf("ERROR: %s\n", message);
  bool ledOn = false;
  unsigned long lastToggleMs = 0;
  for (;;) {
    unsigned long now = millis();
    if (now - lastToggleMs >= 500) {
      lastToggleMs = now;
      ledOn = !ledOn;
      if (ledOn) {
        setNeoPixelColor(STATUS_NEOPIXEL_BRIGHTNESS, 0, 0);
      } else {
        setNeoPixelColor(0, 0, 0);
      }
    }
    delay(10);
  }
}

inline void neopixelSetTimedColor(uint8_t r, uint8_t g, uint8_t b, uint32_t durationMs, int mode) {
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

inline void neopixelUpdate(uint32_t deltaMs) {
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

#endif // UI_NEOPIXEL_H


