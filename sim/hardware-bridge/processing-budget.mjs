#!/usr/bin/env node
/**
 * Measure ESP32 per-exchange processing time (Phase 3 task 6.4).
 */
import os from 'node:os';
import { PERIOD_MS, BUFFER_LEN } from './frame-protocol.js';
import {
  closeAudioPort,
  enterUsbNeighborMode,
  exitUsbNeighborMode,
  exchangeUsbPeriod,
  openAudioPort,
} from '../hardware-realtime/usb-transport.js';

const BUDGET_FRACTION = 0.7;
const PERIOD_US = PERIOD_MS * 1000;
const BUDGET_US = PERIOD_US * BUDGET_FRACTION;
const SAMPLES = 200;

async function main() {
  const portPath = process.env.FIRMWARE_PORT ?? process.argv[2];
  if (!portPath) {
    console.error('Usage: FIRMWARE_PORT=/dev/ttyACM0 node sim/hardware-bridge/processing-budget.mjs');
    process.exit(1);
  }

  const port = await openAudioPort(portPath);
  const processingUs = [];

  try {
    await enterUsbNeighborMode(port);
    for (let seq = 0; seq < SAMPLES; seq++) {
      const pattern = {
        downstreamIn: new Int16Array(BUFFER_LEN),
        upstreamIn: new Int16Array(BUFFER_LEN),
        primary: 0.5,
        secondary: 0.5,
      };
      const response = await exchangeUsbPeriod(port, pattern, seq);
      if (response.processingUs !== undefined && response.processingUs > 0) {
        processingUs.push(response.processingUs);
      }
      await new Promise((r) => setTimeout(r, PERIOD_MS));
    }
    await exitUsbNeighborMode(port);
  } finally {
    await closeAudioPort(port);
  }

  processingUs.sort((a, b) => a - b);
  const worst = processingUs[processingUs.length - 1] ?? 0;
  const p99 = processingUs[Math.floor(processingUs.length * 0.99)] ?? 0;
  const pass = worst < BUDGET_US;

  const summary = {
    platform: `${os.type()} ${os.release()}`,
    port: portPath,
    bufferPeriodMs: PERIOD_MS,
    budgetUs: BUDGET_US,
    samples: processingUs.length,
    processingUs: { worst, p99 },
    pass,
  };

  console.log(JSON.stringify(summary, null, 2));
  process.exit(pass ? 0 : 1);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
