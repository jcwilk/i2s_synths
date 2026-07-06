#ifndef I2S_PIPELINE_H
#define I2S_PIPELINE_H

#include <stdlib.h>
#include <string.h>
#include "../config/constants.h"
#include "../ui/neopixel.h" // reportError
#include "../input/pots.h"
#include "i2s_input.h"
#include "i2s_output.h"

typedef void (*I2SPipelineProcessFn)(int16_t* in_mono,
                                     int16_t* out_mono,
                                     int samples,
                                     DualPotsState pots_state);

typedef struct {
  I2SInputState input_state;
  I2SOutputState output_state;
  int16_t* wire_input_buffer;    // stereo interleaved I2S wire, I2S_WIRE_SAMPLES
  int16_t* wire_output_buffer;   // stereo interleaved I2S wire, I2S_WIRE_SAMPLES
  int16_t* mono_input_buffer;    // mono module path, BUFFER_LEN
  int16_t* mono_output_buffer;   // mono module path, BUFFER_LEN
  bool has_last_input;
  I2SPipelineProcessFn process_callback;
  bool has_started;
} I2SPipelineState;

static inline void i2s_pipeline_error(const char* message) {
  Serial.printf("I2S pipeline error: %s\n", message);
}

static inline void i2s_wire_to_mono(const int16_t* wire, int16_t* mono, int mono_len) {
  for (int i = 0; i < mono_len; ++i) {
    mono[i] = wire[i * 2];
  }
}

static inline void i2s_mono_to_wire(const int16_t* mono, int16_t* wire, int mono_len) {
  for (int i = 0; i < mono_len; ++i) {
    wire[i * 2] = mono[i];
    wire[i * 2 + 1] = mono[i];
  }
}

static inline I2SPipelineState i2s_pipeline_make_initial(I2SInputState input_state,
                                                        I2SOutputState output_state,
                                                        I2SPipelineProcessFn process_callback) {
  I2SPipelineState pipeline_state;
  pipeline_state.input_state = input_state;
  pipeline_state.output_state = output_state;
  pipeline_state.wire_input_buffer = (int16_t*)malloc(sizeof(int16_t) * I2S_WIRE_SAMPLES);
  pipeline_state.wire_output_buffer = (int16_t*)malloc(sizeof(int16_t) * I2S_WIRE_SAMPLES);
  pipeline_state.mono_input_buffer = (int16_t*)malloc(sizeof(int16_t) * BUFFER_LEN);
  pipeline_state.mono_output_buffer = (int16_t*)malloc(sizeof(int16_t) * BUFFER_LEN);
  pipeline_state.has_last_input = false;
  pipeline_state.process_callback = process_callback;
  pipeline_state.has_started = false;
  if (!pipeline_state.wire_input_buffer || !pipeline_state.wire_output_buffer
      || !pipeline_state.mono_input_buffer || !pipeline_state.mono_output_buffer) {
    reportError("i2s_pipeline_make_initial: allocation failed");
  }
  return pipeline_state;
}

static inline void i2s_pipeline_default_process(int16_t* in_buf,
                                                int16_t* out_buf,
                                                int samples,
                                                DualPotsState /*pots_state*/) {
  if (!in_buf || !out_buf || samples <= 0) return;
  memcpy(out_buf, in_buf, sizeof(int16_t) * (size_t)samples);
}

static inline I2SPipelineState i2s_pipeline_make_initial_with_passthrough(I2SInputState input_state,
                                                                         I2SOutputState output_state) {
  return i2s_pipeline_make_initial(input_state, output_state, i2s_pipeline_default_process);
}

static inline I2SPipelineState i2s_pipeline_sync_output(I2SPipelineState pipeline_state) {
  pipeline_state.output_state = i2s_output_sync_from_isr(pipeline_state.output_state);
  return pipeline_state;
}

static inline I2SPipelineState i2s_pipeline_try_read(I2SPipelineState pipeline_state) {
  const size_t full_bytes = sizeof(int16_t) * I2S_WIRE_SAMPLES;
  if (pipeline_state.has_last_input) return pipeline_state;
  I2SInputReadOutcome read_outcome = i2s_input_read(pipeline_state.input_state,
                                                    pipeline_state.wire_input_buffer,
                                                    full_bytes);
  pipeline_state.input_state = read_outcome.state;
  if (read_outcome.result == I2S_INPUT_READ_OK) {
    if (read_outcome.bytes_read == full_bytes) {
      i2s_wire_to_mono(pipeline_state.wire_input_buffer,
                       pipeline_state.mono_input_buffer,
                       BUFFER_LEN);
      pipeline_state.has_last_input = true;
    } else {
      i2s_pipeline_error("partial read");
    }
    return pipeline_state;
  }
  if (read_outcome.result == I2S_INPUT_READ_ERROR) {
    i2s_pipeline_error("read error");
    return pipeline_state;
  }
  return pipeline_state;
}

static inline I2SPipelineState i2s_pipeline_process(I2SPipelineState pipeline_state,
                                                   DualPotsState pots_state) {
  const size_t full_bytes = sizeof(int16_t) * I2S_WIRE_SAMPLES;
  if (!pipeline_state.has_started) {
    pipeline_state = i2s_pipeline_sync_output(pipeline_state);
    pipeline_state.output_state = i2s_output_preload_silence(pipeline_state.output_state);
    pipeline_state.output_state = i2s_output_clear_flags(pipeline_state.output_state);
    pipeline_state.has_started = true;
  }
  pipeline_state = i2s_pipeline_try_read(pipeline_state);

  if (!pipeline_state.has_last_input) {
    return pipeline_state;
  }

  pipeline_state = i2s_pipeline_sync_output(pipeline_state);
  bool ovf = false;
  pipeline_state.output_state = i2s_output_poll_overflow_event(pipeline_state.output_state, &ovf);
  if (ovf) {
    neopixelSetTimedColor(25, 8, 0, 150, NEOPIXEL_MODE_HOLD);
  }
  if (!i2s_output_can_queue(pipeline_state.output_state)) {
    return pipeline_state;
  }

  pipeline_state.process_callback(
    pipeline_state.mono_input_buffer,
    pipeline_state.mono_output_buffer,
    BUFFER_LEN,
    pots_state
  );

  i2s_mono_to_wire(pipeline_state.mono_output_buffer,
                   pipeline_state.wire_output_buffer,
                   BUFFER_LEN);

  I2SOutputWriteOutcome write_outcome = i2s_output_write(pipeline_state.output_state,
                                                         pipeline_state.output_state.tx_handle,
                                                         (const void*)pipeline_state.wire_output_buffer,
                                                         full_bytes);
  pipeline_state.output_state = write_outcome.state;
  if (write_outcome.result == I2S_OUTPUT_WRITE_OK) {
    pipeline_state.has_last_input = false;
    return pipeline_state;
  }
  if (write_outcome.result == I2S_OUTPUT_WRITE_ERROR) {
    i2s_pipeline_error("write error");
    return pipeline_state;
  }

  return pipeline_state;
}

static inline I2SPipelineState i2s_pipeline_process_muted(I2SPipelineState pipeline_state) {
  if (!pipeline_state.input_state.rx_handle || !pipeline_state.wire_input_buffer) {
    reportError("i2s_pipeline_process_muted: missing RX handle or input buffer");
    return pipeline_state;
  }
  I2SInputReadOutcome read_outcome = i2s_input_read(pipeline_state.input_state,
                                                    (void*)pipeline_state.wire_input_buffer,
                                                    sizeof(int16_t) * I2S_WIRE_SAMPLES);
  pipeline_state.input_state = read_outcome.state;
  pipeline_state.has_last_input = false;
  pipeline_state = i2s_pipeline_sync_output(pipeline_state);
  bool ignored = false;
  pipeline_state.output_state = i2s_output_poll_overflow_event(pipeline_state.output_state, &ignored);
  return pipeline_state;
}

#endif // I2S_PIPELINE_H
