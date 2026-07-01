#ifndef DRIVER_I2S_COMMON_H
#define DRIVER_I2S_COMMON_H

typedef int i2s_port_t;

#define I2S_NUM_0 0
#define I2S_NUM_1 1

typedef enum {
  I2S_ROLE_MASTER = 0,
  I2S_ROLE_SLAVE = 1,
} i2s_role_t;

typedef enum {
  I2S_CLK_SRC_DEFAULT = 0,
} i2s_clock_src_t;

#endif // DRIVER_I2S_COMMON_H
