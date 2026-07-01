const SAMPLE_RATE = 44100;
const BUFFER_LEN = 512;

export class SimAudioEngine {
  constructor() {
    this.audioContext = null;
    this.workletNode = null;
    this.micStream = null;
    this.micSource = null;
    this.playbackNode = null;
    this.playbackBuffer = null;
    this.playbackReadIndex = 0;
    this.running = false;
    this.micEnabled = false;
    this.onStatus = () => {};
    this.wasmBindings = null;
    this.prevDownstreamOut = null;
  }

  setWasmBindings(bindings) {
    this.wasmBindings = bindings;
    this.prevDownstreamOut = new Int16Array(BUFFER_LEN);
    if (this.workletNode) {
      this.workletNode.port.postMessage({
        type: 'config',
        bufferLen: BUFFER_LEN,
      });
    }
  }

  async start() {
    if (this.running) {
      return;
    }
    if (!this.wasmBindings) {
      throw new Error('WASM module not loaded');
    }

    this.audioContext = new AudioContext({ sampleRate: SAMPLE_RATE });
    await this.audioContext.audioWorklet.addModule('./module-chain-worklet.js');

    this.workletNode = new AudioWorkletNode(this.audioContext, 'module-chain-processor', {
      processorOptions: { bufferLen: BUFFER_LEN },
      numberOfInputs: 1,
      numberOfOutputs: 1,
      outputChannelCount: [2],
    });

    this.workletNode.port.onmessage = (event) => {
      if (event.data.type === 'buffer') {
        this.handleFirmwareBuffer(event.data.samples);
      }
    };

    this.playbackBuffer = new Float32Array(BUFFER_LEN);
    this.playbackReadIndex = 0;

    this.playbackNode = new AudioWorkletNode(this.audioContext, 'module-chain-playback');
    await this.ensurePlaybackWorklet();

    this.workletNode.connect(this.playbackNode);
    this.playbackNode.connect(this.audioContext.destination);

    await this.ensureMicGraph();

    this.running = true;
    this.workletNode.port.postMessage({ type: 'running', running: true });
    this.onStatus('Audio running at 44100 Hz');
  }

  async ensurePlaybackWorklet() {
    const playbackCode = `
      class ModuleChainPlayback extends AudioWorkletProcessor {
        constructor() {
          super();
          this.queue = [];
          this.readIndex = 0;
          this.current = null;
          this.port.onmessage = (event) => {
            if (event.data.type === 'playback') {
              this.queue.push(event.data.samples);
            }
          };
        }
        process(inputs, outputs) {
          const outL = outputs[0][0];
          const outR = outputs[0][1] || outputs[0][0];
          for (let i = 0; i < outL.length; i++) {
            if (!this.current || this.readIndex >= this.current.length) {
              this.current = this.queue.shift() || null;
              this.readIndex = 0;
            }
            if (this.current) {
              outL[i] = this.current[this.readIndex++];
              outR[i] = this.current[this.readIndex++];
            } else {
              outL[i] = 0;
              outR[i] = 0;
            }
          }
          return true;
        }
      }
      registerProcessor('module-chain-playback', ModuleChainPlayback);
    `;
    const blob = new Blob([playbackCode], { type: 'application/javascript' });
    const url = URL.createObjectURL(blob);
    await this.audioContext.audioWorklet.addModule(url);
    URL.revokeObjectURL(url);
  }

  async ensureMicGraph() {
    if (this.micSource) {
      return;
    }
    try {
      this.micStream = await navigator.mediaDevices.getUserMedia({
        audio: {
          channelCount: 2,
          sampleRate: SAMPLE_RATE,
          echoCancellation: false,
          noiseSuppression: false,
          autoGainControl: false,
        },
        video: false,
      });
      this.micSource = this.audioContext.createMediaStreamSource(this.micStream);
      this.micSource.connect(this.workletNode);
      this.onStatus('Microphone capture available');
    } catch (err) {
      this.onStatus(`Microphone unavailable: ${err.message}`);
      this.micEnabled = false;
    }
  }

  async stop() {
    if (!this.running) {
      return;
    }
    this.running = false;
    if (this.workletNode) {
      this.workletNode.port.postMessage({ type: 'running', running: false });
    }
    if (this.micStream) {
      this.micStream.getTracks().forEach((t) => t.stop());
      this.micStream = null;
    }
    if (this.audioContext) {
      await this.audioContext.close();
      this.audioContext = null;
    }
    this.workletNode = null;
    this.micSource = null;
    this.playbackNode = null;
    this.onStatus('Audio stopped');
  }

  setMicEnabled(enabled) {
    this.micEnabled = enabled;
    if (this.workletNode) {
      this.workletNode.port.postMessage({ type: 'mic', enabled });
    }
  }

  floatToInt16(sample) {
    const clamped = Math.max(-1, Math.min(1, sample));
    return clamped < 0 ? Math.round(clamped * 32768) : Math.round(clamped * 32767);
  }

  int16ToFloat(sample) {
    return sample / (sample < 0 ? 32768 : 32767);
  }

  handleFirmwareBuffer(floatSamples) {
    const { module, heap16, ptrs, potsPtr, heapF32 } = this.wasmBindings;
    const {
      upstreamInPtr,
      upstreamOutPtr,
      downstreamInPtr,
      downstreamOutPtr,
    } = ptrs;

    const downstreamBase = downstreamInPtr >> 1;
    const upstreamInBase = upstreamInPtr >> 1;
    const downstreamOutBase = downstreamOutPtr >> 1;
    const upstreamOutBase = upstreamOutPtr >> 1;

    for (let i = 0; i < BUFFER_LEN; i++) {
      heap16[downstreamBase + i] = this.floatToInt16(floatSamples[i]);
      heap16[upstreamInBase + i] = this.prevDownstreamOut[i];
    }

    module._sim_process_upstream();
    module._sim_process_downstream();

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.prevDownstreamOut[i] = heap16[downstreamOutBase + i];
    }

    const playback = new Float32Array(BUFFER_LEN);
    for (let i = 0; i < BUFFER_LEN; i++) {
      playback[i] = this.int16ToFloat(heap16[upstreamOutBase + i]);
    }

    if (this.playbackNode) {
      this.playbackNode.port.postMessage({ type: 'playback', samples: playback });
    }

    void potsPtr;
    void heapF32;
  }

  updatePots(primary01, secondary01) {
    if (!this.wasmBindings) {
      return;
    }
    const { potsPtr, heapF32 } = this.wasmBindings;
    const base = potsPtr >> 2;
    heapF32[base + 2] = primary01;
    heapF32[base + 5] = secondary01;
  }
}
