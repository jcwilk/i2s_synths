#ifndef DRIVER_I2S_STD_H
#define DRIVER_I2S_STD_H

#include "i2s_common.h"

#define I2S_MCLK_MULTIPLE_256 256

typedef enum {
  I2S_DATA_BIT_WIDTH_16BIT = 16,
} i2s_data_bit_width_t;

typedef enum {
  I2S_SLOT_BIT_WIDTH_16BIT = 16,
} i2s_slot_bit_width_t;

typedef enum {
  I2S_SLOT_MODE_STEREO = 0,
} i2s_slot_mode_t;

typedef enum {
  I2S_STD_SLOT_BOTH = 0,
} i2s_std_slot_mask_t;

typedef void* i2s_chan_handle_t;
typedef int gpio_num_t;
typedef int esp_err_t;

#define ESP_OK 0

#endif // DRIVER_I2S_STD_H
