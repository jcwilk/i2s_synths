#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

typedef bool boolean;
typedef uint8_t byte;

inline void delay(unsigned long) {}

class HardwareSerial {
public:
  void begin(unsigned long) {}
  void println(const char* msg) { fputs(msg, stdout); fputc('\n', stdout); }
  void print(const char* msg) { fputs(msg, stdout); }
  void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
};

static HardwareSerial Serial;

inline int analogRead(int) { return 0; }
inline void analogSetAttenuation(int) {}
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}

#endif // ARDUINO_H
