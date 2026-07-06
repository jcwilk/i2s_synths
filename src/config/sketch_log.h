#ifndef CONFIG_SKETCH_LOG_H
#define CONFIG_SKETCH_LOG_H

#include <Arduino.h>

#ifndef BRIDGE_USE_SERIAL_FOR_AUDIO
#define BRIDGE_USE_SERIAL_FOR_AUDIO 0
#endif

#if BRIDGE_USE_SERIAL_FOR_AUDIO
#define SKETCH_LOG_PRINTLN(s) ((void)0)
#define SKETCH_LOG_PRINTF(...) ((void)0)
#else
#define SKETCH_LOG_PRINTLN(s) Serial.println(s)
#define SKETCH_LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#endif

#endif  // CONFIG_SKETCH_LOG_H
