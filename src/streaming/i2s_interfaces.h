#ifndef I2S_INTERFACES_H
#define I2S_INTERFACES_H

#include <Arduino.h>
#include <string.h>
#include <driver/i2s_std.h>
#include <driver/i2s_common.h>
#include "../config/constants.h"

// Upstream/Downstream I2S channel handles
static i2s_chan_handle_t i2s_tx_u = NULL;
static i2s_chan_handle_t i2s_rx_u = NULL;
static i2s_chan_handle_t i2s_tx_d = NULL;
static i2s_chan_handle_t i2s_rx_d = NULL;

// Event counters/flags
static volatile uint32_t i2s_rx_ready_count = 0;
static volatile int tx_buffers_queued = 0;
static volatile int tx_buffers_sent = 0;
static volatile bool tx_underrun = false;
static volatile bool tx_overrun = false;

// Rotating TX buffer state for simple generator
#define SINE_NUM_BUFFERS 4
#define SINE_TARGET_BUFFER_DEPTH 2
static float sine_phase = 0.0f;
static int16_t sine_buffers[SINE_NUM_BUFFERS][BUFFER_LEN];
static int sine_buffer_index = 0;

static bool IRAM_ATTR on_rx_recv_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_data) {
  (void)handle; (void)event; (void)user_data;
  i2s_rx_ready_count++;
  return false;
}

static bool IRAM_ATTR on_tx_sent_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_data) {
  (void)handle; (void)event; (void)user_data;
  tx_buffers_sent++;
  if (tx_buffers_queued > 0) tx_buffers_queued--; else tx_underrun = true;
  return false;
}

static inline void generateSineBuffer(int16_t* buffer, int frames, float frequency, float amplitude) {
  const float sampleRate = (float)SAMPLE_RATE;
  const float phaseIncrement = (2.0f * PI * frequency) / sampleRate;
  for (int i = 0; i < frames; i++) {
    float sample = sinf(sine_phase) * amplitude;
    int16_t s = (int16_t)sample;
    buffer[(i << 1) + 0] = s;
    buffer[(i << 1) + 1] = s;
    sine_phase += phaseIncrement;
    if (sine_phase >= 2.0f * PI) sine_phase -= 2.0f * PI;
  }
}

static inline void refillTxBuffersWithSine() {
  const int framesPerBuf = BUFFER_LEN / 2;
  while (tx_buffers_queued < SINE_TARGET_BUFFER_DEPTH) {
    int16_t* buf = sine_buffers[sine_buffer_index];
    generateSineBuffer(buf, framesPerBuf, 440.0f, 8000.0f);
    size_t bytesWritten = 0;
    esp_err_t res = i2s_channel_write(i2s_tx_u, buf, sizeof(sine_buffers[0]), &bytesWritten, 0);
    if (res == ESP_OK && bytesWritten == sizeof(sine_buffers[0])) {
      tx_buffers_queued++;
      sine_buffer_index = (sine_buffer_index + 1) % SINE_NUM_BUFFERS;
    } else if (res == ESP_ERR_TIMEOUT) {
      break;
    } else {
      tx_overrun = true;
      break;
    }
  }
}

static inline void refillTxBuffersWithSilence() {
  while (tx_buffers_queued < SINE_TARGET_BUFFER_DEPTH) {
    int16_t* sbuf = sine_buffers[sine_buffer_index];
    memset(sbuf, 0, sizeof(sine_buffers[0]));
    size_t bytesWritten = 0;
    esp_err_t res = i2s_channel_write(i2s_tx_u, sbuf, sizeof(sine_buffers[0]), &bytesWritten, 0);
    if (res == ESP_OK && bytesWritten == sizeof(sine_buffers[0])) {
      tx_buffers_queued++;
      sine_buffer_index = (sine_buffer_index + 1) % SINE_NUM_BUFFERS;
    } else if (res == ESP_ERR_TIMEOUT) {
      break;
    } else {
      tx_overrun = true;
      break;
    }
  }
}

static bool setupI2SOverlap(i2s_port_t port,
                            i2s_role_t role,
                            const i2s_std_gpio_config_t& gpio_cfg,
                            const i2s_event_callbacks_t* rx_cbs,
                            const i2s_event_callbacks_t* tx_cbs,
                            i2s_chan_handle_t& out_tx,
                            i2s_chan_handle_t& out_rx,
                            bool preload_tx_zero) {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(port, role);
  chan_cfg.dma_frame_num = (uint32_t)(BUFFER_LEN / 2);
  chan_cfg.dma_desc_num = (uint32_t)I2S_DMA_BUF_COUNT;
  esp_err_t result = i2s_new_channel(&chan_cfg, &out_tx, &out_rx);
  if (result != ESP_OK) {
    Serial.println("I2S channel allocation failed");
    return false;
  }

  const i2s_std_clk_config_t clk_cfg = {
    .sample_rate_hz = SAMPLE_RATE,
    .clk_src = I2S_CLK_SRC_DEFAULT,
    .ext_clk_freq_hz = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
    .bclk_div = 8,
  };
  const i2s_std_slot_config_t slot_cfg = {
    .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT,
    .slot_mode = I2S_SLOT_MODE_STEREO,
    .slot_mask = I2S_STD_SLOT_BOTH,
    .ws_width = 16,
    .ws_pol = false,
    .bit_shift = true,
    .left_align = false,
    .big_endian = false,
    .bit_order_lsb = false,
  };
  const i2s_std_config_t std_cfg = { .clk_cfg = clk_cfg, .slot_cfg = slot_cfg, .gpio_cfg = gpio_cfg };

  result = i2s_channel_init_std_mode(out_tx, &std_cfg);
  if (result != ESP_OK) { Serial.println("I2S TX init failed"); return false; }
  result = i2s_channel_init_std_mode(out_rx, &std_cfg);
  if (result != ESP_OK) { Serial.println("I2S RX init failed"); return false; }

  if (rx_cbs) i2s_channel_register_event_callback(out_rx, rx_cbs, NULL);
  if (tx_cbs) i2s_channel_register_event_callback(out_tx, tx_cbs, NULL);

  if (preload_tx_zero) {
    static int16_t zeroBuf[BUFFER_LEN] = {0};
    size_t bytes_loaded = 0;
    for (uint32_t i = 0; i < chan_cfg.dma_desc_num; i++) {
      i2s_channel_preload_data(out_tx, zeroBuf, sizeof(zeroBuf), &bytes_loaded);
    }
  }

  i2s_channel_enable(out_tx);
  i2s_channel_enable(out_rx);
  return true;
}

static inline void setupI2SU() {
  const i2s_std_gpio_config_t gpio_cfg = {
    .mclk = (gpio_num_t)I2SU_MCK,
    .bclk = (gpio_num_t)I2SU_SCK,
    .ws = (gpio_num_t)I2SU_WS,
    .dout = (gpio_num_t)I2SU_SD_OUT,
    .din = (gpio_num_t)I2SU_SD_IN,
    .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
  };
  const i2s_event_callbacks_t rx_cbs = {
    .on_recv = on_rx_recv_callback,
    .on_recv_q_ovf = NULL,
    .on_sent = NULL,
    .on_send_q_ovf = NULL,
  };
  const i2s_event_callbacks_t tx_cbs = {
    .on_recv = NULL,
    .on_recv_q_ovf = NULL,
    .on_sent = on_tx_sent_callback,
    .on_send_q_ovf = NULL,
  };
  #if ENABLE_GATEWAY
    const i2s_role_t role = I2S_ROLE_MASTER;
  #else
    const i2s_role_t role = I2S_ROLE_SLAVE;
  #endif
  if (!setupI2SOverlap(I2SU_PORT, role, gpio_cfg, &rx_cbs, &tx_cbs, i2s_tx_u, i2s_rx_u, true)) return;
  Serial.printf("I2SU ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SU_MCK, I2SU_SCK, I2SU_WS, I2SU_SD_OUT, I2SU_SD_IN);
}

static inline void setupI2SD() {
  const i2s_std_gpio_config_t gpio_cfg = {
    .mclk = (gpio_num_t)I2SD_MCK,
    .bclk = (gpio_num_t)I2SD_SCK,
    .ws = (gpio_num_t)I2SD_WS,
    .dout = (gpio_num_t)I2SD_SD_OUT,
    .din = (gpio_num_t)I2SD_SD_IN,
    .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
  };
  if (!setupI2SOverlap(I2SD_PORT, I2S_ROLE_MASTER, gpio_cfg, NULL, NULL, i2s_tx_d, i2s_rx_d, true)) return;
  Serial.printf("I2SD ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SD_MCK, I2SD_SCK, I2SD_WS, I2SD_SD_OUT, I2SD_SD_IN);
}

static inline void i2sSetup() {
  setupI2SD();
  setupI2SU();
  #if ENABLE_GATEWAY == 1
    pinMode(POT_PIN_SECONDARY, OUTPUT);
    digitalWrite(POT_PIN_SECONDARY, HIGH);
    delay(100);
    digitalWrite(POT_PIN_SECONDARY, LOW);
    delay(50);
  #endif
}

static inline void i2sLoop(bool inStartupMute) {
  if (inStartupMute) {
    refillTxBuffersWithSilence();
    return;
  }
  refillTxBuffersWithSine();
}

#endif // I2S_INTERFACES_H


