#ifndef I2S_INPUT_H
#define I2S_INPUT_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <driver/i2s_common.h>
#include "../config/constants.h"
#include "../ui/neopixel.h" // for reportError

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

typedef struct {
  bool rx_overrun;            // sticky flag if we detect an RX error condition
  bool rx_error;              // sticky flag for non-timeout read errors
  i2s_chan_handle_t rx_handle; // bound RX handle for internal use
} I2SInputState;

typedef enum {
  I2S_INPUT_READ_OK = 0,
  I2S_INPUT_READ_TIMEOUT = 1,
  I2S_INPUT_READ_ERROR = 2,
} I2SInputReadResult;

typedef struct {
  I2SInputState state;
  I2SInputReadResult result;
  size_t bytes_read; // number of bytes delivered to caller on success
} I2SInputReadOutcome;

static inline I2SInputState i2s_input_make_initial(i2s_chan_handle_t rx_handle) {
  I2SInputState s;
  s.rx_overrun = false;
  s.rx_error = false;
  s.rx_handle = rx_handle;
  return s;
}

static inline I2SInputState i2s_input_clear_flags(I2SInputState s) {
  s.rx_overrun = false;
  s.rx_error = false;
  return s;
}

static inline I2SInputReadOutcome i2s_input_read(I2SInputState s,
                                                 void* dest,
                                                 size_t bytes) {
  I2SInputReadOutcome out;
  out.state = s;
  out.result = I2S_INPUT_READ_ERROR;
  out.bytes_read = 0;
  if (!dest || bytes == 0) {
    s.rx_error = true;
    out.state = s;
    reportError("i2s_input_read: invalid dest or bytes");
    return out; // unreachable, reportError does not return
  }
  // Use bound handle from state
  i2s_chan_handle_t rx = s.rx_handle;
  if (!rx) {
    s.rx_error = true;
    out.state = s;
    reportError("i2s_input_read: rx_handle not set");
    return out;
  }
  const size_t full_bytes = sizeof(int16_t) * BUFFER_LEN;
  if (bytes < full_bytes) {
    s.rx_error = true;
    out.state = s;
    reportError("i2s_input_read: bytes smaller than full buffer size");
    return out;
  }
  // Try direct non-blocking read into caller buffer
  size_t br = 0;
  esp_err_t r = i2s_channel_read(rx, dest, full_bytes, &br, 0);
  if (r == ESP_OK && br == full_bytes) {
    out.state = s;
    out.result = I2S_INPUT_READ_OK;
    out.bytes_read = full_bytes;
    return out;
  }
  if (r == ESP_ERR_TIMEOUT) {
    out.state = s;
    out.result = I2S_INPUT_READ_TIMEOUT;
    return out;
  }
  s.rx_error = true;
  out.state = s;
  out.result = I2S_INPUT_READ_ERROR;
  return out;
}

#endif // I2S_INPUT_H


