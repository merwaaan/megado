import createModule, { type MainModule } from "./wasm/megado.js";

export enum Button {
  Up = 0x101,
  Down = 0x202,
  Left = 0x4,
  Right = 0x8,
  B = 0x10,
  C = 0x20,
  A = 0x1000,
  Start = 0x2000,
}

export enum Status {
  NoGameLoaded = 0,
  Pause = 1,
  Running = 2,
  Rewinding = 3,
  Quitting = 4,
}

export type Frame = {
  pixels: Uint8Array;
  width: number;
  height: number;
  pitch: number;
};

export class Emulator {
  private constructor(private readonly module: MainModule) {
    module._megado_create();
  }

  static async create(): Promise<Emulator> {
    return new Emulator(await createModule());
  }

  get status(): Status {
    return this.module._megado_status();
  }

  get romName(): string {
    return this.module.UTF8ToString(this.module._megado_rom_name());
  }

  loadRom(data: Uint8Array): boolean {
    const pointer = this.copyInModule(data);
    const loaded = this.module._megado_load_rom(pointer, data.length);
    this.module._free(pointer);

    return Boolean(loaded);
  }

  reset(): void {
    this.module._megado_reset();
  }

  runSamples(count: number, sampleDuration: number): Int16Array {
    const produced = this.module._megado_run_samples(count, sampleDuration);
    const pointer = this.module._megado_samples();

    return this.module.HEAP16.slice(
      pointer >> 1,
      (pointer >> 1) + produced * 2,
    );
  }

  frame(): Frame {
    const pointer = this.module._megado_frame();
    const width = this.module._megado_frame_width();
    const height = this.module._megado_frame_height();
    const pitch = this.module._megado_frame_pitch();

    return {
      pixels: this.module.HEAPU8.subarray(pointer, pointer + pitch * height),
      width,
      height,
      pitch,
    };
  }

  setButton(port: number, button: Button, pressed: boolean): void {
    this.module._megado_set_button(port, button, pressed ? 1 : 0);
  }

  get snapshotVersion(): number {
    return this.module._megado_snapshot_version();
  }

  takeSnapshot(): Uint8Array {
    const pointer = this.module._megado_snapshot_take();
    const size = this.module._megado_snapshot_size();
    const data = this.module.HEAPU8.slice(pointer, pointer + size);
    this.module._free(pointer);

    return data;
  }

  restoreSnapshot(data: Uint8Array): boolean {
    if (data.length !== this.module._megado_snapshot_size()) {
      return false;
    }

    const pointer = this.copyInModule(data);
    this.module._megado_snapshot_restore(pointer);
    this.module._free(pointer);

    return true;
  }

  private copyInModule(data: Uint8Array): number {
    const pointer = this.module._malloc(data.length);
    this.module.HEAPU8.set(data, pointer);

    return pointer;
  }
}
