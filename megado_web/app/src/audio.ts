import workletUrl from "./audio-worklet.ts?worker&url";

export class Audio {
  readonly context = new AudioContext({ latencyHint: "interactive" });
  private node: AudioWorkletNode | null = null;

  get sampleRate(): number {
    return this.context.sampleRate;
  }

  async start(): Promise<void> {
    // Create the audio worklet if needed

    if (this.node === null) {
      await this.context.audioWorklet.addModule(workletUrl);

      this.node = new AudioWorkletNode(this.context, "megado", {
        outputChannelCount: [2],
      });

      this.node.connect(this.context.destination);
    }

    // Resume if not running

    await this.context.resume();
  }

  async suspend(): Promise<void> {
    // Suspend if running

    await this.context.suspend();
  }

  push(samples: Int16Array): void {
    if (this.node === null || samples.length === 0) {
      return;
    }

    this.node.port.postMessage(samples, [samples.buffer]);
  }
}
