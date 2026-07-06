/**
 * Length-prefixed frame reader for Web Serial ReadableStream (mirrors usb-relay.js).
 */

import { BRIDGE_WIRE_PREFIX_SIZE } from './frame-protocol.js';

const MAX_WIRE_INNER_BYTES = 33000;

export function createWebSerialFrameReader(readable) {
  /** @type {Uint8Array} */
  let buffer = new Uint8Array(0);
  /** @type {{ resolve: (buf: Uint8Array) => void, reject: (err: Error) => void, timer: ReturnType<typeof setTimeout> }[]} */
  const waiters = [];
  let reader = null;
  let readLoopActive = false;

  function concat(a, b) {
    const out = new Uint8Array(a.length + b.length);
    out.set(a, 0);
    out.set(b, a.length);
    return out;
  }

  function tryTake() {
    while (buffer.length >= BRIDGE_WIRE_PREFIX_SIZE) {
      const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
      const wireLen = view.getUint32(0, true);
      if (wireLen === 0 || wireLen > MAX_WIRE_INNER_BYTES) {
        buffer = buffer.subarray(1);
        continue;
      }
      const total = BRIDGE_WIRE_PREFIX_SIZE + wireLen;
      if (buffer.length < total) {
        return null;
      }
      const inner = buffer.subarray(BRIDGE_WIRE_PREFIX_SIZE, total);
      buffer = buffer.subarray(total);
      return inner;
    }
    return null;
  }

  function flushWaiters() {
    let inner = tryTake();
    while (inner && waiters.length > 0) {
      const waiter = waiters.shift();
      if (waiter) {
        clearTimeout(waiter.timer);
        waiter.resolve(inner);
      }
      inner = tryTake();
    }
  }

  function rejectAll(err) {
    const pending = waiters.splice(0);
    for (const waiter of pending) {
      clearTimeout(waiter.timer);
      waiter.reject(err);
    }
  }

  async function pump() {
    if (!reader || readLoopActive) {
      return;
    }
    readLoopActive = true;
    try {
      while (reader) {
        const { value, done } = await reader.read();
        if (done) {
          break;
        }
        if (value && value.length > 0) {
          buffer = concat(buffer, value);
          flushWaiters();
        }
      }
    } catch (err) {
      rejectAll(err instanceof Error ? err : new Error(String(err)));
    } finally {
      readLoopActive = false;
    }
  }

  return {
    async attach(portReadable) {
      reader = portReadable.getReader();
      pump().catch(() => {});
    },
    reset() {
      buffer = new Uint8Array(0);
    },
    readFrame(timeoutMs = 15000) {
      const existing = tryTake();
      if (existing) {
        return Promise.resolve(existing);
      }
      return new Promise((resolve, reject) => {
        const waiter = {
          resolve,
          reject,
          timer: setTimeout(() => {
            const index = waiters.indexOf(waiter);
            if (index >= 0) {
              waiters.splice(index, 1);
            }
            reject(new Error(`Web Serial read timeout (${buffer.length} bytes buffered)`));
          }, timeoutMs),
        };
        waiters.push(waiter);
      });
    },
  };
}
