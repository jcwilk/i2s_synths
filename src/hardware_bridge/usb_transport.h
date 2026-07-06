#ifndef HARDWARE_BRIDGE_USB_TRANSPORT_H
#define HARDWARE_BRIDGE_USB_TRANSPORT_H

#include <Arduino.h>
#include <string.h>
#include "bridge_audio.h"
#include "bridge_frame.h"
#include "offline_neighbor.h"
#include "usb_neighbor.h"

static OfflineNeighborState g_offline_neighbor_state;
static UsbNeighborState g_usb_neighbor_state;
static BridgeExchangeRequest g_bridge_request;
static BridgeExchangeResponse g_bridge_response;

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

inline void bridgeWriteAckOn(Stream& stream, uint8_t command, uint32_t sequence, uint16_t status) {
  uint8_t frame[BRIDGE_ACK_SIZE] = {};
  bridgeWriteHeader(frame, command);
  memcpy(frame + BRIDGE_HEADER_SIZE, &sequence, sizeof(sequence));
  memcpy(frame + BRIDGE_HEADER_SIZE + 4, &status, sizeof(status));
  bridgeWriteExact(stream, frame, sizeof(frame));
}

inline void bridgeWriteUsbAck(uint8_t command, uint32_t sequence, uint16_t status) {
  uint8_t frame[BRIDGE_ACK_SIZE] = {};
  bridgeWriteHeader(frame, command);
  memcpy(frame + BRIDGE_HEADER_SIZE, &sequence, sizeof(sequence));
  memcpy(frame + BRIDGE_HEADER_SIZE + 4, &status, sizeof(status));
  bridgeWriteLengthPrefixed(bridgeAudioStream(), frame, sizeof(frame));
}

inline bool bridgeReadExchangeRequestFrom(Stream& stream, BridgeExchangeRequest& request) {
  uint8_t* payload = reinterpret_cast<uint8_t*>(&request.sequence);
  return bridgeReadExact(stream, payload, BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE, 5000);
}

inline void bridgeWriteExchangeResponseOn(Stream& stream,
                                          uint8_t command,
                                          const BridgeExchangeResponse& response) {
  static uint8_t frame[BRIDGE_EXCHANGE_RESPONSE_OFFLINE_SIZE];
  bridgeWriteHeader(frame, command);
  size_t offset = BRIDGE_HEADER_SIZE;
  memcpy(frame + offset, &response.sequence, sizeof(response.sequence));
  offset += sizeof(response.sequence);
  memcpy(frame + offset, response.downstreamOut, sizeof(response.downstreamOut));
  offset += sizeof(response.downstreamOut);
  memcpy(frame + offset, response.upstreamOut, sizeof(response.upstreamOut));
  offset += sizeof(response.upstreamOut);
  memcpy(frame + offset, &response.status, sizeof(response.status));
  bridgeWriteExact(stream, frame, sizeof(frame));
}

inline void bridgeWriteUsbExchangeResponse(uint8_t command, const BridgeExchangeResponse& response) {
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
  offset += sizeof(response.status);
  memcpy(frame + offset, &response.timestampUs, sizeof(response.timestampUs));
  offset += sizeof(response.timestampUs);
  memcpy(frame + offset, &response.primaryTelemetry, sizeof(response.primaryTelemetry));
  offset += sizeof(response.primaryTelemetry);
  memcpy(frame + offset, &response.secondaryTelemetry, sizeof(response.secondaryTelemetry));
  offset += sizeof(response.secondaryTelemetry);
  memcpy(frame + offset, &response.processingUs, sizeof(response.processingUs));
  bridgeWriteLengthPrefixed(bridgeAudioStream(), frame, sizeof(frame));
}

inline void bridgeTransportInit() {
  offlineNeighborMakeInitial(g_offline_neighbor_state);
  usbNeighborMakeInitial(g_usb_neighbor_state);
  bridgeAudioInit();
}

inline bool bridgeTransportIsActive() {
  return g_offline_neighbor_state.active || g_usb_neighbor_state.active;
}

inline void bridgeHandleOfflineCommand(uint8_t command) {
  switch (command) {
    case BRIDGE_CMD_ENTER:
      offlineNeighborEnter(g_offline_neighbor_state);
      bridgeWriteAckOn(Serial, BRIDGE_CMD_ENTER, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_EXIT:
      offlineNeighborExit(g_offline_neighbor_state);
      bridgeWriteAckOn(Serial, BRIDGE_CMD_EXIT, 0, g_offline_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_LOOPBACK: {
      if (!g_offline_neighbor_state.active) {
        offlineNeighborEnter(g_offline_neighbor_state);
      }
      offlineNeighborEnableLoopback(g_offline_neighbor_state);
      memset(&g_bridge_request, 0, sizeof(g_bridge_request));
      if (!bridgeReadExchangeRequestFrom(Serial, g_bridge_request)) {
        bridgeWriteAckOn(Serial, BRIDGE_CMD_LOOPBACK, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      offlineNeighborLoadRequest(g_offline_neighbor_state, g_bridge_request);
      offlineNeighborProcessExchange(g_offline_neighbor_state);
      offlineNeighborBuildResponse(g_offline_neighbor_state, g_bridge_response);
      g_bridge_response.timestampUs = 0;
      bridgeWriteExchangeResponseOn(Serial, BRIDGE_CMD_LOOPBACK, g_bridge_response);
      offlineNeighborDisableLoopback(g_offline_neighbor_state);
      break;
    }

    case BRIDGE_CMD_EXCHANGE: {
      if (!g_offline_neighbor_state.active) {
        bridgeWriteAckOn(Serial, BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_NOT_ACTIVE);
        return;
      }
      memset(&g_bridge_request, 0, sizeof(g_bridge_request));
      if (!bridgeReadExchangeRequestFrom(Serial, g_bridge_request)) {
        bridgeWriteAckOn(Serial, BRIDGE_CMD_EXCHANGE, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      offlineNeighborLoadRequest(g_offline_neighbor_state, g_bridge_request);
      offlineNeighborProcessExchange(g_offline_neighbor_state);
      offlineNeighborBuildResponse(g_offline_neighbor_state, g_bridge_response);
      g_bridge_response.timestampUs = 0;
      bridgeWriteExchangeResponseOn(Serial, BRIDGE_CMD_EXCHANGE, g_bridge_response);
      break;
    }

    default:
      bridgeWriteAckOn(Serial, command, 0, BRIDGE_STATUS_ERR_BAD_CMD);
      break;
  }
}

inline void bridgePollOfflineSerial() {
  if (Serial.available() < (int)BRIDGE_HEADER_SIZE) {
    return;
  }
  if (Serial.peek() != (int)(BRIDGE_MAGIC & 0xFFu)) {
    return;
  }

  uint8_t header[BRIDGE_HEADER_SIZE] = {};
  for (int i = 0; i < (int)BRIDGE_HEADER_SIZE; ++i) {
    header[i] = (uint8_t)Serial.read();
  }

  uint8_t command = 0;
  if (!bridgeParseHeader(header, command)) {
    bridgeWriteAckOn(Serial, command, 0, BRIDGE_STATUS_ERR_BAD_MAGIC);
    return;
  }

  bridgeHandleOfflineCommand(command);
}

inline void bridgeHandleUsbWirePayload(const uint8_t* payload, size_t payloadLen) {
  if (payloadLen < BRIDGE_HEADER_SIZE) {
    bridgeWriteUsbAck(0, 0, BRIDGE_STATUS_ERR_BAD_LENGTH);
    return;
  }

  uint8_t command = 0;
  if (!bridgeParseHeader(payload, command)) {
    bridgeWriteUsbAck(command, 0, BRIDGE_STATUS_ERR_BAD_MAGIC);
    return;
  }

  switch (command) {
    case BRIDGE_CMD_ENTER_USB: {
      uint8_t enterMode = BRIDGE_ENTER_MODE_INJECTED;
      if (payloadLen > BRIDGE_HEADER_SIZE) {
        enterMode = payload[5];
      } else if (payloadLen == BRIDGE_HEADER_SIZE) {
        enterMode = payload[5];
      }
      usbNeighborEnter(g_usb_neighbor_state, enterMode);
      bridgeWriteUsbAck(BRIDGE_CMD_ENTER_USB, 0, g_usb_neighbor_state.lastStatus);
      break;
    }

    case BRIDGE_CMD_EXIT_USB:
      usbNeighborExit(g_usb_neighbor_state);
      bridgeWriteUsbAck(BRIDGE_CMD_EXIT_USB, 0, g_usb_neighbor_state.lastStatus);
      break;

    case BRIDGE_CMD_LOOPBACK_USB: {
      if (!g_usb_neighbor_state.active) {
        usbNeighborEnter(g_usb_neighbor_state);
      }
      if (g_usb_neighbor_state.processing) {
        bridgeWriteUsbAck(BRIDGE_CMD_LOOPBACK_USB, 0, BRIDGE_STATUS_OVERRUN);
        return;
      }
      if (payloadLen < BRIDGE_EXCHANGE_REQUEST_SIZE) {
        bridgeWriteUsbAck(BRIDGE_CMD_LOOPBACK_USB, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      usbNeighborEnableLoopback(g_usb_neighbor_state);
      memcpy(&g_bridge_request.sequence, payload + BRIDGE_HEADER_SIZE, BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE);
      usbNeighborLoadRequest(g_usb_neighbor_state, g_bridge_request);
      usbNeighborProcessExchange(g_usb_neighbor_state);
      usbNeighborBuildResponse(g_usb_neighbor_state, g_bridge_response);
      bridgeWriteUsbExchangeResponse(BRIDGE_CMD_LOOPBACK_USB, g_bridge_response);
      usbNeighborDisableLoopback(g_usb_neighbor_state);
      break;
    }

    case BRIDGE_CMD_EXCHANGE_USB: {
      if (!g_usb_neighbor_state.active) {
        bridgeWriteUsbAck(BRIDGE_CMD_EXCHANGE_USB, 0, BRIDGE_STATUS_ERR_NOT_ACTIVE);
        return;
      }
      if (payloadLen < BRIDGE_EXCHANGE_REQUEST_SIZE) {
        bridgeWriteUsbAck(BRIDGE_CMD_EXCHANGE_USB, 0, BRIDGE_STATUS_ERR_SHORT_READ);
        return;
      }
      memcpy(&g_bridge_request.sequence, payload + BRIDGE_HEADER_SIZE, BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE);
      usbNeighborLoadRequest(g_usb_neighbor_state, g_bridge_request);
      usbNeighborProcessExchange(g_usb_neighbor_state);
      usbNeighborBuildResponse(g_usb_neighbor_state, g_bridge_response);
      bridgeWriteUsbExchangeResponse(BRIDGE_CMD_EXCHANGE_USB, g_bridge_response);
      break;
    }

    case BRIDGE_CMD_QUERY_MODULE:
      bridgeWriteUsbAck(BRIDGE_CMD_QUERY_MODULE, (uint32_t)ACTIVE_MODULE, BRIDGE_STATUS_OK);
      break;

    default:
      bridgeWriteUsbAck(command, 0, BRIDGE_STATUS_ERR_BAD_CMD);
      break;
  }
}

inline void bridgePollUsbRealtime() {
  Stream& audio = bridgeAudioStream();
  if (audio.available() < (int)BRIDGE_WIRE_PREFIX_SIZE) {
    return;
  }

  static uint8_t wirePayload[BRIDGE_USB_MAX_WIRE_PAYLOAD];
  size_t payloadLen = 0;
  if (!bridgeReadLengthPrefixed(audio, wirePayload, sizeof(wirePayload), payloadLen, 5000)) {
    return;
  }
  bridgeHandleUsbWirePayload(wirePayload, payloadLen);
}

inline void bridgeTransportPoll() {
#if !BRIDGE_USE_SERIAL_FOR_AUDIO
  bridgePollOfflineSerial();
#endif
  if (g_usb_neighbor_state.active) {
    for (int pass = 0; pass < 64; ++pass) {
      const int avail = bridgeAudioStream().available();
      if (avail < (int)BRIDGE_WIRE_PREFIX_SIZE) {
        break;
      }
      bridgePollUsbRealtime();
    }
    return;
  }
  bridgePollUsbRealtime();
}

inline bool bridgeUsbNeighborUsesPhysicalAdc() {
  return usbNeighborUsesPhysicalAdc(g_usb_neighbor_state);
}

inline void bridgeUsbNeighborSetPhysicalPots(const DualPotsState& pots) {
  usbNeighborSetPhysicalPots(g_usb_neighbor_state, pots);
}

inline bool bridgeUsbNeighborIsActive() {
  return g_usb_neighbor_state.active;
}

inline void bridgeTransportRunUsbRealtimeLoop() {
  while (g_usb_neighbor_state.active) {
    bridgePollUsbRealtime();
#if !BRIDGE_USE_SERIAL_FOR_AUDIO
    bridgePollOfflineSerial();
#endif
    delay(1);
  }
}

inline void bridgeTransportServiceActive() {
  if (g_usb_neighbor_state.active) {
    bridgeTransportRunUsbRealtimeLoop();
    return;
  }
  for (int pass = 0; pass < 64; ++pass) {
    bridgeTransportPoll();
    if (!bridgeTransportIsActive()) {
      break;
    }
    if (Serial.available() < (int)BRIDGE_HEADER_SIZE
        && bridgeAudioStream().available() < (int)BRIDGE_WIRE_PREFIX_SIZE) {
      break;
    }
  }
}

#endif  // HARDWARE_BRIDGE_USB_TRANSPORT_H
