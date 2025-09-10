#ifndef I2S_OUTPUT_H
#define I2S_OUTPUT_H

#include <stdbool.h>
#include <driver/i2s_common.h>
#include <stdlib.h>
#include "../config/constants.h"
#include "../ui/neopixel.h" // reportError

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

// Mailbox of monotonic totals updated only by ISR callbacks
typedef struct {
  volatile uint32_t tx_sent_total;
} I2SOutputIsrTotals;

typedef struct {
  int tx_buffers_queued;
  int tx_buffers_sent;
  bool tx_underrun;
  bool tx_overrun;
  uint32_t tx_sent_seen; // snapshot of ISR total
  I2SOutputIsrTotals* isr_totals; // heap-allocated, never freed
  i2s_chan_handle_t tx_handle;    // bound TX handle for internal use
} I2SOutputState;

typedef enum {
  I2S_OUTPUT_WRITE_OK = 0,
  I2S_OUTPUT_WRITE_TIMEOUT = 1,
  I2S_OUTPUT_WRITE_ERROR = 2,
} I2SOutputWriteResult;

typedef struct {
  I2SOutputState state;
  I2SOutputWriteResult result;
} I2SOutputWriteOutcome;

// Internal ISR adapter: increments mailbox counter
static inline void i2s_output_isr_on_tx_sent(I2SOutputIsrTotals* t) {
  t->tx_sent_total++;
}

// TX sent callback (registered by this module)
static bool IRAM_ATTR i2s_output_tx_sent_cb(i2s_chan_handle_t handle, i2s_event_data_t* event, void* user_data) {
  (void)handle; (void)event;
  I2SOutputIsrTotals* totals = (I2SOutputIsrTotals*)user_data;
  if (totals) i2s_output_isr_on_tx_sent(totals);
  return false;
}

static inline I2SOutputState i2s_output_make_initial(i2s_chan_handle_t tx_handle) {
  I2SOutputState s;
  s.tx_buffers_queued = 0;
  s.tx_buffers_sent = 0;
  s.tx_underrun = false;
  s.tx_overrun = false;
  s.tx_sent_seen = 0;
  s.isr_totals = NULL;
  s.tx_handle = tx_handle;
  return s;
}

// Register callbacks and enable the TX channel. Must be called before first enable.
static inline I2SOutputState i2s_output_finalize(I2SOutputState s) {
  if (!s.tx_handle) {
    reportError("i2s_output_finalize: tx_handle not set");
    return s;
  }
  if (!s.isr_totals) {
    I2SOutputIsrTotals* totals = (I2SOutputIsrTotals*)malloc(sizeof(I2SOutputIsrTotals));
    if (!totals) {
      reportError("i2s_output_finalize: totals alloc failed");
      return s;
    }
    totals->tx_sent_total = 0;
    s.isr_totals = totals;
  }
  const i2s_event_callbacks_t tx_cbs = {
    .on_recv = NULL,
    .on_recv_q_ovf = NULL,
    .on_sent = i2s_output_tx_sent_cb,
    .on_send_q_ovf = NULL,
  };
  esp_err_t cb_res = i2s_channel_register_event_callback(s.tx_handle, &tx_cbs, (void*)s.isr_totals);
  if (cb_res != ESP_OK) {
    reportError("i2s_output_finalize: register callback failed");
    return s;
  }
  esp_err_t en_res = i2s_channel_enable(s.tx_handle);
  if (en_res != ESP_OK) {
    reportError("i2s_output_finalize: enable failed or already enabled");
    return s;
  }
  return s;
}

static inline I2SOutputState i2s_output_clear_flags(I2SOutputState s) {
  s.tx_underrun = false;
  s.tx_overrun = false;
  return s;
}

// Called when a TX buffer has been successfully queued for sending
static inline I2SOutputState i2s_output_on_tx_queue_success(I2SOutputState s) {
  s.tx_buffers_queued++;
  return s;
}

// Called when a TX queue attempt has failed due to a non-timeout error
static inline I2SOutputState i2s_output_on_tx_queue_error(I2SOutputState s) {
  s.tx_overrun = true;
  return s;
}

// Called from on_tx_sent callback when a TX buffer has been sent
static inline I2SOutputState i2s_output_on_tx_sent(I2SOutputState s) {
  s.tx_buffers_sent++;
  if (s.tx_buffers_queued > 0) {
    s.tx_buffers_queued--;
  } else {
    s.tx_underrun = true;
  }
  return s;
}

// Apply monotonic total (from ISR mailbox) to state
static inline I2SOutputState i2s_output_apply_tx_sent_total(I2SOutputState s, uint32_t tx_sent_total) {
  uint32_t delta = (uint32_t)(tx_sent_total - s.tx_sent_seen); // wrap-safe for unsigned
  s.tx_sent_seen = tx_sent_total;
  while (delta--) {
    s = i2s_output_on_tx_sent(s);
  }
  return s;
}

// Consume ISR mailbox totals into functional state
static inline I2SOutputState i2s_output_sync_from_isr(I2SOutputState s) {
  if (!s.isr_totals) return s;
  return i2s_output_apply_tx_sent_total(s, s.isr_totals->tx_sent_total);
}

// Wrapper around i2s_channel_write that updates state and reports outcome
static inline I2SOutputWriteOutcome i2s_output_write(I2SOutputState s,
                                                     i2s_chan_handle_t handle,
                                                     const void* buffer,
                                                     size_t bytes) {
  I2SOutputWriteOutcome out;
  out.state = s;
  out.result = I2S_OUTPUT_WRITE_ERROR;
  if (!handle || !buffer || bytes == 0) {
    return out;
  }
  size_t bytesWritten = 0;
  esp_err_t res = i2s_channel_write(handle, buffer, bytes, &bytesWritten, 0);
  if (res == ESP_OK && bytesWritten == bytes) {
    out.state = i2s_output_on_tx_queue_success(out.state);
    out.result = I2S_OUTPUT_WRITE_OK;
    return out;
  }
  if (res == ESP_ERR_TIMEOUT) {
    out.result = I2S_OUTPUT_WRITE_TIMEOUT;
    return out;
  }
  out.state = i2s_output_on_tx_queue_error(out.state);
  out.result = I2S_OUTPUT_WRITE_ERROR;
  return out;
}

// Query: return true if there is room to queue another buffer
static inline bool i2s_output_can_queue(const I2SOutputState& s) {
  return s.tx_buffers_queued < (int)I2S_DMA_BUF_COUNT - 1; // -1 because we can't queue all the way full at the same time
}

#endif // I2S_OUTPUT_H
