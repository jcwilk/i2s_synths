#ifndef HARDWARE_BRIDGE_FRAME_H
#define HARDWARE_BRIDGE_FRAME_H

#include <stdint.h>
#include "../config/constants.h"

// Host↔device binary frame contract (Phase 0 hardware module bridge).
// Audio paths: stereo interleaved int16, BUFFER_LEN samples per path.

#define BRIDGE_MAGIC 0x31424D48u  // 'H' 'M' 'B' '1' little-endian on wire

#define BRIDGE_CMD_ENTER 0x01u
#define BRIDGE_CMD_EXIT 0x02u
#define BRIDGE_CMD_EXCHANGE 0x03u
#define BRIDGE_CMD_LOOPBACK 0x04u

#define BRIDGE_STATUS_OK 0x0001u
#define BRIDGE_STATUS_ERR_BAD_MAGIC 0x0002u
#define BRIDGE_STATUS_ERR_BAD_CMD 0x0003u
#define BRIDGE_STATUS_ERR_NOT_ACTIVE 0x0004u
#define BRIDGE_STATUS_ERR_SHORT_READ 0x0005u

#define BRIDGE_HEADER_SIZE 6u
#define BRIDGE_AUDIO_SAMPLE_BYTES (BUFFER_LEN * (int)sizeof(int16_t))
#define BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE (4 + BRIDGE_AUDIO_SAMPLE_BYTES * 2 + 8)
#define BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE (4 + BRIDGE_AUDIO_SAMPLE_BYTES * 2 + 2)
#define BRIDGE_EXCHANGE_REQUEST_SIZE (BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE)
#define BRIDGE_EXCHANGE_RESPONSE_SIZE (BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE)
#define BRIDGE_ACK_PAYLOAD_SIZE 6u
#define BRIDGE_ACK_SIZE (BRIDGE_HEADER_SIZE + BRIDGE_ACK_PAYLOAD_SIZE)

typedef struct {
  uint32_t sequence;
  int16_t downstreamIn[BUFFER_LEN];
  int16_t upstreamIn[BUFFER_LEN];
  float primaryControl;
  float secondaryControl;
} BridgeExchangeRequest;

typedef struct {
  uint32_t sequence;
  int16_t downstreamOut[BUFFER_LEN];
  int16_t upstreamOut[BUFFER_LEN];
  uint16_t status;
} BridgeExchangeResponse;

#endif  // HARDWARE_BRIDGE_FRAME_H
