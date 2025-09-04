#ifndef INPUT_POTS_H
#define INPUT_POTS_H

#include <Arduino.h>

#ifndef ADC_MAX
#define ADC_MAX 4095
#endif
#ifndef EMA_ALPHA
#define EMA_ALPHA 0.05f
#endif
#ifndef DEAD_ZONE_LOW
#define DEAD_ZONE_LOW 0.02f
#endif
#ifndef DEAD_ZONE_HIGH
#define DEAD_ZONE_HIGH 0.98f
#endif

// Smooth a potentiometer reading with EMA, apply dead-zone mapping to [0..1]
inline float readPotWithSmoothingAndDeadZone(int pin, float& emaState) {
  float raw = 1.0f - ((float)analogRead(pin) / (float)ADC_MAX);
  emaState = (EMA_ALPHA * raw) + ((1.0f - EMA_ALPHA) * emaState);

  if (emaState < DEAD_ZONE_LOW) {
    return 0.0f;
  }
  if (emaState > DEAD_ZONE_HIGH) {
    return 1.0f;
  }
  return (emaState - DEAD_ZONE_LOW) / (DEAD_ZONE_HIGH - DEAD_ZONE_LOW);
}

#endif // INPUT_POTS_H


