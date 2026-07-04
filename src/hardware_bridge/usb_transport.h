#ifndef HARDWARE_BRIDGE_USB_TRANSPORT_H
#define HARDWARE_BRIDGE_USB_TRANSPORT_H

#include <Arduino.h>
#include <string.h>
#include "bridge_frame.h"
#include "offline_neighbor.h"

static OfflineNeighborState g_offline_neighbor_state;

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
  uint8_t payload[BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE] = {};
  if (!bridgeReadExact(payload, sizeof(payload), 5000)) {
    return false;
  }

  size_t offset = 0;
  memcpy(&request.sequence, payload + offset, sizeof(request.sequence));
  offset += sizeof(request.sequence);
  memcpy(request.downstreamIn, payload + offset, sizeof(request.downstreamIn));
  offset += sizeof(request.downstreamIn);
  memcpy(request.upstreamIn, payload + offset, sizeof(request.upstreamIn));
  offset += sizeof(request.upstreamIn);
  memcpy(&request.primaryControl, payload + offset, sizeof(request.primaryControl));
  offset += sizeof(request.primaryControl);
  memcpy(&request.secondaryControl, payload + offset, sizeof(request.secondaryControl));
  return true;
}

inline void bridgeWriteExchangeResponse(uint8_t command, const BridgeExchangeResponse& response) {
  uint8_t frame[BRIDGE_EXCHANGE_RESPONSE_SIZE] = {};
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
  g_offline_neighbor_state = offlineNeighborMakeInitial();
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
      g_offline_neighbor_state = offlineNeighborEnter(g_offline_neighbor_state);
      bridgeWriteAck(BRIDGE_CMD_ENTER, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_EXIT:
      g_offline_neighbor_state = offlineNeighborExit(g_offline_neighbor_state);
      bridgeWriteAck(BRIDGE_CMD_EXIT, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_LOOPBACK: {
      if (!g_offline_neighbor_state.active) {
        g_offline_neighbor_state = offlineNeighborEnter(g_offline_neighbor_state);
      }
      g_offline_neighbor_state = offlineNeighborEnableLoopback(g_offline_neighbor_state);
      BridgeExchangeRequest request = {};
      if (!bridgeReadExchangeRequest(request)) {
        bridgeWriteAck(BRIDGE_CMD_LOOPBACK, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      g_offline_neighbor_state = offlineNeighborLoadRequest(g_offline_neighbor_state, request);
      g_offline_neighbor_state = offlineNeighborProcessExchange(g_offline_neighbor_state);
      const BridgeExchangeResponse loopbackResponse =
          offlineNeighborBuildResponse(g_offline_neighbor_state);
      bridgeWriteExchangeResponse(BRIDGE_CMD_LOOPBACK, loopbackResponse);
      g_offline_neighbor_state = offlineNeighborDisableLoopback(g_offline_neighbor_state);
      break;
    }

    case BRIDGE_CMD_EXCHANGE: {
      if (!g_offline_neighbor_state.active) {
        bridgeWriteAck(BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_NOT_ACTIVE);
        return;
      }
      BridgeExchangeRequest exchangeRequest = {};
      if (!bridgeReadExchangeRequest(exchangeRequest)) {
        bridgeWriteAck(BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      g_offline_neighbor_state = offlineNeighborLoadRequest(g_offline_neighbor_state, exchangeRequest);
      g_offline_neighbor_state = offlineNeighborProcessExchange(g_offline_neighbor_state);
      const BridgeExchangeResponse exchangeResponse =
          offlineNeighborBuildResponse(g_offline_neighbor_state);
      bridgeWriteExchangeResponse(BRIDGE_CMD_EXCHANGE, exchangeResponse);
      break;
    }

    default:
      bridgeWriteAck(command, 0, BRIDGE_STATUS_ERR_BAD_CMD);
      break;
  }
}

#endif  // HARDWARE_BRIDGE_USB_TRANSPORT_H
