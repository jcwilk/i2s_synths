#!/usr/bin/env node
/**
 * Phase 0 hardware module bridge offline harness.
 *
 * Modes:
 *   generate-vectors          Build test vectors + WASM golden files
 *   reference <vector.json>   Print reference-only run summary
 *   compare-capture <vector> <capture.json> [golden.json]
 *   run-usb <vector.json> --port /dev/ttyACM0
 *   loopback-usb --port /dev/ttyACM0
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { compareCaptureToReference, formatComparisonReport } from './compare.js';
import { int16ArrayFromJson, runReferenceSequence } from './reference-runner.js';
import {
  closeSerialPort,
  enterOfflineMode,
  exitOfflineMode,
  openSerialPort,
  runLoopbackSelfTest,
  exchangePeriod,
} from './serial-transport.js';
import {
  closeAudioPort,
  enterUsbNeighborMode,
  exitUsbNeighborMode,
  exchangeUsbPeriod,
  openAudioPort,
} from '../hardware-realtime/usb-transport.js';
import { BUFFER_LEN } from './frame-protocol.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

function usage() {
  console.log(`Usage:
  node sim/hardware-offline/harness.mjs generate-vectors
  node sim/hardware-offline/harness.mjs reference <vector.json>
  node sim/hardware-offline/harness.mjs compare-capture <vector.json> <capture.json> [golden.json]
  node sim/hardware-offline/harness.mjs run-usb <vector.json> --port <path>
  node sim/hardware-offline/harness.mjs loopback-usb --port <path>`);
}

function loadJson(filePath) {
  return JSON.parse(fs.readFileSync(filePath, 'utf8'));
}

function resolveGoldenPath(vectorPath, explicitGolden) {
  if (explicitGolden) {
    return explicitGolden;
  }
  const sibling = vectorPath.replace(/\.json$/, '.golden.json');
  if (fs.existsSync(sibling)) {
    return sibling;
  }
  throw new Error(`golden file not found for ${vectorPath}`);
}

function vectorPeriodToTransport(periodSpec, vector) {
  return {
    downstreamIn: int16ArrayFromJson(periodSpec.downstreamIn),
    upstreamIn: int16ArrayFromJson(periodSpec.upstreamIn),
    primary: periodSpec.primary ?? vector.defaultPrimary ?? 0,
    secondary: periodSpec.secondary ?? vector.defaultSecondary ?? 0,
  };
}

async function cmdReference(vectorPath) {
  const vector = loadJson(vectorPath);
  const reference = await runReferenceSequence(vector.moduleKind, vector);
  console.log(`Reference generated: ${vector.moduleKind}, periods=${reference.length}`);
}

async function cmdCompareCapture(vectorPath, capturePath, goldenPath) {
  const vector = loadJson(vectorPath);
  const captureDoc = loadJson(capturePath);
  const goldenDoc = loadJson(goldenPath ?? resolveGoldenPath(vectorPath));
  const compareFromPeriod = vector.compareFromPeriod ?? vector.warmupPeriods ?? 0;
  const result = compareCaptureToReference({
    moduleKind: vector.moduleKind,
    capture: captureDoc.capture,
    reference: goldenDoc.reference,
    compareFromPeriod,
  });
  console.log(formatComparisonReport(result));
  process.exit(result.pass ? 0 : 1);
}

async function cmdRunUsb(vectorPath, portPath) {
  const vector = loadJson(vectorPath);
  const goldenDoc = loadJson(resolveGoldenPath(vectorPath));
  const capture = [];
  const port = await openAudioPort(portPath);

  try {
    await enterUsbNeighborMode(port);
    for (let period = 0; period < vector.periods.length; period++) {
      const periodSpec = vectorPeriodToTransport(vector.periods[period], vector);
      const response = await exchangeUsbPeriod(port, periodSpec, period);
      capture.push({
        downstreamOut: Array.from(response.downstreamOut),
        upstreamOut: Array.from(response.upstreamOut),
      });
    }
    await exitUsbNeighborMode(port);
  } finally {
    await closeAudioPort(port);
  }

  const compareFromPeriod = vector.compareFromPeriod ?? vector.warmupPeriods ?? 0;
  const result = compareCaptureToReference({
    moduleKind: vector.moduleKind,
    capture,
    reference: goldenDoc.reference,
    compareFromPeriod,
  });
  console.log(formatComparisonReport(result));

  const capturePath = vectorPath.replace(/\.json$/, '.capture.json');
  fs.writeFileSync(
    capturePath,
    `${JSON.stringify({ moduleKind: vector.moduleKind, capture }, null, 2)}\n`,
  );
  console.log(`Wrote capture ${capturePath}`);
  process.exit(result.pass ? 0 : 1);
}

async function cmdLoopbackUsb(portPath) {
  const port = await openSerialPort(portPath);
  try {
    await enterOfflineMode(port);
    const periodSpec = {
      downstreamIn: Int16Array.from({ length: BUFFER_LEN }, (_, i) => i * 10),
      upstreamIn: Int16Array.from({ length: BUFFER_LEN }, (_, i) => 1000 + i),
      primary: 0.42,
      secondary: 0.17,
    };
    const response = await runLoopbackSelfTest(port, periodSpec, 0);
    let pass = true;
    for (let i = 0; i < BUFFER_LEN; i++) {
      if (response.downstreamOut[i] !== periodSpec.downstreamIn[i]) {
        pass = false;
        break;
      }
    }
    for (let i = 0; i < BUFFER_LEN; i++) {
      if (response.upstreamOut[i] !== periodSpec.upstreamIn[i]) {
        pass = false;
        break;
      }
    }
    console.log(pass ? 'PASS loopback-usb: device echoed inputs' : 'FAIL loopback-usb: echo mismatch');
    await exitOfflineMode(port);
    process.exit(pass ? 0 : 1);
  } finally {
    await closeSerialPort(port);
  }
}

async function main() {
  const [command, ...args] = process.argv.slice(2);
  if (!command) {
    usage();
    process.exit(1);
  }

  if (command === 'generate-vectors') {
    const { spawn } = await import('node:child_process');
    await new Promise((resolve, reject) => {
      const child = spawn(process.execPath, ['sim/hardware-offline/generate-vectors.mjs'], {
        cwd: path.join(__dirname, '..', '..'),
        stdio: 'inherit',
      });
      child.on('exit', (code) => (code === 0 ? resolve() : reject(new Error(`generate-vectors exit ${code}`))));
    });
    return;
  }

  if (command === 'reference') {
    await cmdReference(args[0]);
    return;
  }

  if (command === 'compare-capture') {
    await cmdCompareCapture(args[0], args[1], args[2]);
    return;
  }

  if (command === 'run-usb') {
    const vectorPath = args[0];
    const portIndex = args.indexOf('--port');
    const portPath = portIndex >= 0 ? args[portIndex + 1] : process.env.FIRMWARE_PORT;
    if (!vectorPath || !portPath) {
      usage();
      process.exit(1);
    }
    await cmdRunUsb(vectorPath, portPath);
    return;
  }

  if (command === 'loopback-usb') {
    const portIndex = args.indexOf('--port');
    const portPath = portIndex >= 0 ? args[portIndex + 1] : process.env.FIRMWARE_PORT;
    if (!portPath) {
      usage();
      process.exit(1);
    }
    await cmdLoopbackUsb(portPath);
    return;
  }

  usage();
  process.exit(1);
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
