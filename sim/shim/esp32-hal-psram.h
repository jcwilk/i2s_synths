#ifndef ESP32_HAL_PSRAM_H
#define ESP32_HAL_PSRAM_H

#include <stdlib.h>

inline bool psramFound() { return true; }

inline void* ps_malloc(size_t size) { return malloc(size); }

#endif // ESP32_HAL_PSRAM_H
