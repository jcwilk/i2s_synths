#ifndef HARDWARE_BRIDGE_AUDIO_H
#define HARDWARE_BRIDGE_AUDIO_H

#include <Arduino.h>
#include "bridge_frame.h"

#if defined(ARDUINO_USB_CDC_ON_BOOT) && ARDUINO_USB_CDC_ON_BOOT
#include "USB.h"
#include "USBCDC.h"
#define BRIDGE_DUAL_CDC 1
static USBCDC BridgeAudioPort;
#else
#define BRIDGE_DUAL_CDC 0
#endif

#ifndef BRIDGE_AUDIO_EXCLUSIVE
#define BRIDGE_AUDIO_EXCLUSIVE 0
#endif

inline Stream& bridgeAudioStream() {
#if BRIDGE_DUAL_CDC
  return BridgeAudioPort;
#else
  return Serial;
#endif
}

inline void bridgeAudioInit() {
#if BRIDGE_DUAL_CDC
  BridgeAudioPort.setRxBufferSize(32768);
  BridgeAudioPort.begin(115200);
  USB.begin();
#endif
}

inline bool bridgeLogEnabled() {
#if BRIDGE_AUDIO_EXCLUSIVE
  return false;
#else
  return true;
#endif
}

inline void bridgeLog(const char* message) {
  if (bridgeLogEnabled()) {
    Serial.println(message);
  }
}

inline bool bridgeReadExact(Stream& stream, uint8_t* buffer, size_t length, uint32_t timeoutMs) {
  size_t received = 0;
  const uint32_t deadline = millis() + timeoutMs;
  while (received < length) {
    const int avail = stream.available();
    if (avail > 0) {
      const size_t toRead = (size_t)avail;
      const size_t remaining = length - received;
      const size_t chunk = toRead < remaining ? toRead : remaining;
      const size_t n = stream.readBytes(buffer + received, chunk);
      received += n;
      continue;
    }
    if ((int32_t)(deadline - millis()) <= 0) {
      return false;
    }
  }
  return true;
}

inline bool bridgeWriteExact(Stream& stream, const uint8_t* buffer, size_t length) {
  if (stream.write(buffer, length) != length) {
    return false;
  }
  stream.flush();
  return true;
}

inline bool bridgeReadLengthPrefixed(Stream& stream,
                                     uint8_t* payload,
                                     size_t maxPayload,
                                     size_t& payloadLen,
                                     uint32_t timeoutMs) {
  uint8_t prefix[BRIDGE_WIRE_PREFIX_SIZE] = {};
  if (!bridgeReadExact(stream, prefix, sizeof(prefix), timeoutMs)) {
    return false;
  }
  const uint32_t wireLen = (uint32_t)prefix[0]
      | ((uint32_t)prefix[1] << 8)
      | ((uint32_t)prefix[2] << 16)
      | ((uint32_t)prefix[3] << 24);
  if (wireLen == 0 || wireLen > maxPayload) {
    return false;
  }
  if (!bridgeReadExact(stream, payload, wireLen, timeoutMs)) {
    return false;
  }
  payloadLen = wireLen;
  return true;
}

inline bool bridgeWriteLengthPrefixed(Stream& stream, const uint8_t* payload, size_t payloadLen) {
  if (payloadLen > BRIDGE_USB_MAX_WIRE_PAYLOAD) {
    return false;
  }
  static uint8_t wire[BRIDGE_WIRE_PREFIX_SIZE + BRIDGE_USB_MAX_WIRE_PAYLOAD];
  wire[0] = (uint8_t)(payloadLen & 0xFFu);
  wire[1] = (uint8_t)((payloadLen >> 8) & 0xFFu);
  wire[2] = (uint8_t)((payloadLen >> 16) & 0xFFu);
  wire[3] = (uint8_t)((payloadLen >> 24) & 0xFFu);
  memcpy(wire + BRIDGE_WIRE_PREFIX_SIZE, payload, payloadLen);
  return bridgeWriteExact(stream, wire, BRIDGE_WIRE_PREFIX_SIZE + payloadLen);
}

#endif  // HARDWARE_BRIDGE_AUDIO_H
