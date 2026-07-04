#ifndef HARDWARE_BRIDGE_USB_TRANSPORT_H
#define HARDWARE_BRIDGE_USB_TRANSPORT_H

#include <Arduino.h>
#include <string.h>
#include "bridge_frame.h"
#include "offline_neighbor.h"

static OfflineNeighborState g_offline_neighbor_state;
static BridgeExchangeRequest g_bridge_request;
static BridgeExchangeResponse g_bridge_response;

inline bool bridgeReadExact(uint8_t* buffer, size_t length, uint32_t timeoutMs) {
  size_t received = 0;
  const uint32_t deadline = millis() + timeoutMs;
  while (received < length) {
    if ((int32_t)(deadline - millis()) <= 0) {
      return false;
    }
    if (Serial.available() > 0) {
      const int byteValue = Serial.read();
      if (byteValue < 0) {
        continue;
      }
      buffer[received++] = (uint8_t)byteValue;
    } else {
      delay(1);
    }
  }
  return true;
}

inline bool bridgeWriteExact(const uint8_t* buffer, size_t length) {
  const size_t written = Serial.write(buffer, length);
  Serial.flush();
  return written == length;
}

inline bool bridgeParseHeader(const uint8_t* header, uint8_t& commandOut) {
  const uint32_t magic = (uint32_t)header[0]
      | ((uint32_t)header[1] << 8)
      | ((uint32_t)header[2] << 16)
      | ((uint32_t)header[3] << 24);
  if (magic != BRIDGE_MAGIC) {
    return false;
  }
  commandOut = header[4];
  return true;
}

inline void bridgeWriteHeader(uint8_t* header, uint8_t command) {
  header[0] = (uint8_t)(BRIDGE_MAGIC & 0xFFu);
  header[1] = (uint8_t)((BRIDGE_MAGIC >> 8) & 0xFFu);
  header[2] = (uint8_t)((BRIDGE_MAGIC >> 16) & 0xFFu);
  header[3] = (uint8_t)((BRIDGE_MAGIC >> 24) & 0xFFu);
  header[4] = command;
  header[5] = 0u;
}

inline void bridgeWriteAck(uint8_t command, uint32_t sequence, uint16_t status) {
  uint8_t frame[BRIDGE_ACK_SIZE] = {};
  bridgeWriteHeader(frame, command);
  memcpy(frame + BRIDGE_HEADER_SIZE, &sequence, sizeof(sequence));
  memcpy(frame + BRIDGE_HEADER_SIZE + 4, &status, sizeof(status));
  bridgeWriteExact(frame, sizeof(frame));
}

inline bool bridgeReadExchangeRequest(BridgeExchangeRequest& request) {
  uint8_t* payload = reinterpret_cast<uint8_t*>(&request.sequence);
  if (!bridgeReadExact(payload, BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE, 5000)) {
    return false;
  }
  return true;
}

inline void bridgeWriteExchangeResponse(uint8_t command, const BridgeExchangeResponse& response) {
  static uint8_t frame[BRIDGE_EXCHANGE_RESPONSE_SIZE];
  bridgeWriteHeader(frame, command);
  size_t offset = BRIDGE_HEADER_SIZE;
  memcpy(frame + offset, &response.sequence, sizeof(response.sequence));
  offset += sizeof(response.sequence);
  memcpy(frame + offset, response.downstreamOut, sizeof(response.downstreamOut));
  offset += sizeof(response.downstreamOut);
  memcpy(frame + offset, response.upstreamOut, sizeof(response.upstreamOut));
  offset += sizeof(response.upstreamOut);
  memcpy(frame + offset, &response.status, sizeof(response.status));
  bridgeWriteExact(frame, sizeof(frame));
}

inline void bridgeTransportInit() {
  offlineNeighborMakeInitial(g_offline_neighbor_state);
}

inline bool bridgeTransportIsActive() {
  return g_offline_neighbor_state.active;
}

inline void bridgeTransportPoll() {
  if (Serial.available() < (int)BRIDGE_HEADER_SIZE) {
    return;
  }

  uint8_t header[BRIDGE_HEADER_SIZE] = {};
  for (int i = 0; i < (int)BRIDGE_HEADER_SIZE; ++i) {
    header[i] = (uint8_t)Serial.read();
  }

  uint8_t command = 0;
  if (!bridgeParseHeader(header, command)) {
    bridgeWriteAck(command, 0, BRIDGE_STATUS_ERR_BAD_MAGIC);
    return;
  }

  switch (command) {
    case BRIDGE_CMD_ENTER:
      offlineNeighborEnter(g_offline_neighbor_state);
      bridgeWriteAck(BRIDGE_CMD_ENTER, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_EXIT:
      offlineNeighborExit(g_offline_neighbor_state);
      bridgeWriteAck(BRIDGE_CMD_EXIT, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_LOOPBACK: {
      if (!g_offline_neighbor_state.active) {
        offlineNeighborEnter(g_offline_neighbor_state);
      }
      offlineNeighborEnableLoopback(g_offline_neighbor_state);
      memset(&g_bridge_request, 0, sizeof(g_bridge_request));
      if (!bridgeReadExchangeRequest(g_bridge_request)) {
        bridgeWriteAck(BRIDGE_CMD_LOOPBACK, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      offlineNeighborLoadRequest(g_offline_neighbor_state, g_bridge_request);
      offlineNeighborProcessExchange(g_offline_neighbor_state);
      offlineNeighborBuildResponse(g_offline_neighbor_state, g_bridge_response);
      bridgeWriteExchangeResponse(BRIDGE_CMD_LOOPBACK, g_bridge_response);
      offlineNeighborDisableLoopback(g_offline_neighbor_state);
      break;
    }

    case BRIDGE_CMD_EXCHANGE: {
      if (!g_offline_neighbor_state.active) {
        bridgeWriteAck(BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_NOT_ACTIVE);
        return;
      }
      memset(&g_bridge_request, 0, sizeof(g_bridge_request));
      if (!bridgeReadExchangeRequest(g_bridge_request)) {
        bridgeWriteAck(BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      offlineNeighborLoadRequest(g_offline_neighbor_state, g_bridge_request);
      offlineNeighborProcessExchange(g_offline_neighbor_state);
      offlineNeighborBuildResponse(g_offline_neighbor_state, g_bridge_response);
      bridgeWriteExchangeResponse(BRIDGE_CMD_EXCHANGE, g_bridge_response);
      break;
    }

    default:
      bridgeWriteAck(command, 0, BRIDGE_STATUS_ERR_BAD_CMD);
      break;
  }
}

#endif  // HARDWARE_BRIDGE_USB_TRANSPORT_H
