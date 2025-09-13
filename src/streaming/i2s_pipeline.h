#ifndef I2S_PIPELINE_H
#define I2S_PIPELINE_H

#include <stdlib.h>
#include <string.h>
#include "../config/constants.h"
#include "../ui/neopixel.h" // reportError
#include "i2s_input.h"
#include "i2s_output.h"

typedef void (*I2SPipelineProcessFn)(int16_t* in_stereo_interleaved,
                                     int16_t* out_stereo_interleaved,
                                     int samples);

typedef struct {
  I2SInputState input_state;
  I2SOutputState output_state;
  int16_t* last_input_buffer;    // heap-allocated, size = BUFFER_LEN samples (stereo interleaved)
  bool has_last_input;
  int16_t* work_output_buffer;   // heap-allocated, size = BUFFER_LEN samples
  I2SPipelineProcessFn process_callback;
} I2SPipelineState;

static inline void i2s_pipeline_error(const char* message) {
  Serial.printf("I2S pipeline error: %s\n", message);
}

static inline I2SPipelineState i2s_pipeline_make_initial(I2SInputState input_state,
                                                        I2SOutputState output_state,
                                                        I2SPipelineProcessFn process_callback) {
  I2SPipelineState pipeline_state;
  pipeline_state.input_state = input_state;
  pipeline_state.output_state = output_state;
  pipeline_state.last_input_buffer = (int16_t*)malloc(sizeof(int16_t) * BUFFER_LEN);
  pipeline_state.has_last_input = false;
  pipeline_state.work_output_buffer = (int16_t*)malloc(sizeof(int16_t) * BUFFER_LEN);
  pipeline_state.process_callback = process_callback;
  if (!pipeline_state.last_input_buffer || !pipeline_state.work_output_buffer) {
    reportError("i2s_pipeline_make_initial: allocation failed");
  }
  return pipeline_state;
}

// Default passthrough if no process_fn provided
static inline void i2s_pipeline_default_process(int16_t* in_buf,
                                                int16_t* out_buf,
                                                int samples) {
  if (!in_buf || !out_buf || samples <= 0) return;
  memcpy(out_buf, in_buf, sizeof(int16_t) * (size_t)samples);
}

static inline I2SPipelineState i2s_pipeline_make_initial_with_passthrough(I2SInputState input_state,
                                                                         I2SOutputState output_state) {
  return i2s_pipeline_make_initial(input_state, output_state, i2s_pipeline_default_process);
}

// Helper: sync output state with ISR mailbox
static inline I2SPipelineState i2s_pipeline_sync_output(I2SPipelineState pipeline_state) {
  pipeline_state.output_state = i2s_output_sync_from_isr(pipeline_state.output_state);
  return pipeline_state;
}

// Helper: attempt to read one full input buffer if none is staged
static inline I2SPipelineState i2s_pipeline_try_read(I2SPipelineState pipeline_state) {
  const size_t full_bytes = sizeof(int16_t) * BUFFER_LEN;
  if (pipeline_state.has_last_input) return pipeline_state;
  I2SInputReadOutcome read_outcome = i2s_input_read(pipeline_state.input_state,
                                                    pipeline_state.last_input_buffer,
                                                    full_bytes);
  pipeline_state.input_state = read_outcome.state;
  if (read_outcome.result == I2S_INPUT_READ_OK) {
    if (read_outcome.bytes_read == full_bytes) {
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

// Single simple error reporter; call and return early from the caller

static inline I2SPipelineState i2s_pipeline_process(I2SPipelineState pipeline_state) {
  const size_t full_bytes = sizeof(int16_t) * BUFFER_LEN;
  const int samples_per_buf = BUFFER_LEN;
  pipeline_state = i2s_pipeline_try_read(pipeline_state);

  if (!pipeline_state.has_last_input) {
    return pipeline_state;
  }

  pipeline_state = i2s_pipeline_sync_output(pipeline_state);
  // Listener: flash orange on overflow/underrun event
  bool ovf = false;
  pipeline_state.output_state = i2s_output_poll_overflow_event(pipeline_state.output_state, &ovf);
  if (ovf) {
    // Keep brightness consistent with other flashes
    neopixelSetTimedColor(25, 8, 0, 150, NEOPIXEL_MODE_HOLD);
  }
  if (!i2s_output_can_queue(pipeline_state.output_state)) {
    return pipeline_state;
  }

  pipeline_state.process_callback(
    pipeline_state.last_input_buffer,
    pipeline_state.work_output_buffer,
    samples_per_buf
  );

  I2SOutputWriteOutcome write_outcome = i2s_output_write(pipeline_state.output_state,
                                                         pipeline_state.output_state.tx_handle,
                                                         (const void*)pipeline_state.work_output_buffer,
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

#endif // I2S_PIPELINE_H


