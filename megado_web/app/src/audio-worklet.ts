declare const sampleRate: number;

declare class AudioWorkletProcessor {
  readonly port: MessagePort;
}

declare function registerProcessor(
  name: string,
  processor: new () => AudioWorkletProcessor,
): void;

class MegadoProcessor extends AudioWorkletProcessor {
  private readonly left = new Float32Array(sampleRate);
  private readonly right = new Float32Array(sampleRate);
  private readIndex = 0;
  private writeIndex = 0;
  private buffered = 0;

  constructor() {
    super();

    this.port.onmessage = (event: MessageEvent<Int16Array>) =>
      this.push(event.data);
  }

  private push(samples: Int16Array): void {
    const capacity = this.left.length;
    const count = samples.length / 2;

    for (let i = 0; i < count; ++i) {
      if (this.buffered === capacity) {
        this.readIndex = (this.readIndex + 1) % capacity;
        --this.buffered;
      }

      this.left[this.writeIndex] = samples[i * 2] / 32768;
      this.right[this.writeIndex] = samples[i * 2 + 1] / 32768;

      this.writeIndex = (this.writeIndex + 1) % capacity;

      ++this.buffered;
    }
  }

  process(_inputs: Float32Array[][], outputs: Float32Array[][]): boolean {
    const output = outputs[0];

    const left = output[0];
    const right = output[1] ?? output[0];

    const capacity = this.left.length;

    for (let i = 0; i < left.length; ++i) {
      if (this.buffered > 0) {
        left[i] = this.left[this.readIndex];
        right[i] = this.right[this.readIndex];

        this.readIndex = (this.readIndex + 1) % capacity;

        --this.buffered;
      } else {
        left[i] = 0;
        right[i] = 0;
      }
    }

    return true;
  }
}

registerProcessor("megado", MegadoProcessor);

export {};
