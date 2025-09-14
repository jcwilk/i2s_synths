#ifndef MODULE_PASSTHROUGH_H
#define MODULE_PASSTHROUGH_H

#include <Arduino.h>
#include <string.h>
#include "../input/pots.h"

inline void moduleSetup() {
  Serial.println("Passthrough module active");
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength,
                                 DualPotsState /*pots_state*/) {
  if (outputBuffer && inputBuffer && samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength,
                               DualPotsState /*pots_state*/) {
  if (outputBuffer && inputBuffer && samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

#endif // MODULE_PASSTHROUGH_H


