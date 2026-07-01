class ModuleChainProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    this.bufferLen = options.processorOptions.bufferLen;
    this.accumulator = new Float32Array(this.bufferLen);
    this.accumIndex = 0;
    this.micEnabled = false;
    this.running = false;

    this.port.onmessage = (event) => {
      const msg = event.data;
      if (msg.type === 'config') {
        this.bufferLen = msg.bufferLen;
        this.accumulator = new Float32Array(this.bufferLen);
        this.accumIndex = 0;
      } else if (msg.type === 'mic') {
        this.micEnabled = !!msg.enabled;
      } else if (msg.type === 'running') {
        this.running = !!msg.running;
        if (!this.running) {
          this.accumIndex = 0;
        }
      }
    };
  }

  downmixInput(inputs) {
    const input = inputs[0];
    if (!input || input.length === 0) {
      return { left: 0, right: 0 };
    }
    const ch0 = input[0];
    const ch1 = input[1] || ch0;
    if (!ch0 || ch0.length === 0) {
      return { left: 0, right: 0 };
    }
    return { left: ch0[0] || 0, right: (ch1 && ch1[0]) || ch0[0] || 0 };
  }

  process(inputs, outputs) {
    const output = outputs[0];
    if (!output || output.length === 0) {
      return true;
    }

    const outL = output[0];
    const outR = output[1] || output[0];
    const frames = outL.length;

    if (!this.running) {
      for (let i = 0; i < frames; i++) {
        outL[i] = 0;
        outR[i] = 0;
      }
      return true;
    }

    const mic = this.downmixInput(inputs);

    for (let i = 0; i < frames; i++) {
      const left = this.micEnabled ? mic.left : 0;
      const right = this.micEnabled ? mic.right : 0;
      this.accumulator[this.accumIndex++] = left;
      this.accumulator[this.accumIndex++] = right;

      if (this.accumIndex >= this.bufferLen) {
        this.port.postMessage({
          type: 'buffer',
          samples: this.accumulator.slice(0, this.bufferLen),
        });
        this.accumIndex = 0;
      }

      outL[i] = 0;
      outR[i] = 0;
    }

    return true;
  }
}

registerProcessor('module-chain-processor', ModuleChainProcessor);
