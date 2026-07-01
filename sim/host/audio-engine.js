const SAMPLE_RATE = 44100;
const BUFFER_LEN = 512;

export class SimAudioEngine {
  constructor() {
    this.audioContext = null;
    this.workletNode = null;
    this.micStream = null;
    this.micSource = null;
    this.playbackNode = null;
    this.running = false;
    this.micEnabled = false;
    this.onStatus = () => {};
    this.onLevels = () => {};
    this.chainScheduler = null;
  }

  setChainScheduler(scheduler) {
    this.chainScheduler = scheduler;
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
    if (!this.chainScheduler) {
      throw new Error('Chain scheduler not configured');
    }

    this.audioContext = new AudioContext({ sampleRate: SAMPLE_RATE });
    if (this.audioContext.sampleRate !== SAMPLE_RATE) {
      console.warn(
        `AudioContext sample rate is ${this.audioContext.sampleRate} Hz, expected ${SAMPLE_RATE} Hz`,
      );
    }
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

    await this.ensurePlaybackWorklet();
    this.playbackNode = new AudioWorkletNode(this.audioContext, 'module-chain-playback');
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
              while (this.queue.length > 3) {
                this.queue.shift();
              }
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

  handleFirmwareBuffer(floatSamples) {
    if (!this.chainScheduler) {
      return;
    }

    const { playback, levels } = this.chainScheduler.processBuffer(
      floatSamples,
      this.micEnabled,
    );

    if (this.playbackNode) {
      this.playbackNode.port.postMessage({ type: 'playback', samples: playback });
    }

    this.onLevels(levels);
  }
}
