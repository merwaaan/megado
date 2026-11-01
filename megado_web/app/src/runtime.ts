import { Audio } from "./audio";
import { Emulator, Status } from "./emulator";
import { Input } from "./input";
import { Rewind, SNAPSHOT_INTERVAL, type Snapshot } from "./rewind";
import { loadState, saveState } from "./storage";
import { Video } from "./video";

const MAX_STEP = 0.1;

export class Runtime {
  private readonly audio = new Audio();
  private readonly input: Input;
  private video: Video | null = null;
  private readonly rewind = new Rewind();
  private readonly listeners = new Set<() => void>();

  speed = 1;

  private loaded = false;
  private paused = false;
  private scrubbing = false;
  private pausedBeforeScrub = false;
  private sinceRewind = 0;
  private rewindPausedAt: number | null = null;
  private last = performance.now();

  private constructor(private readonly emulator: Emulator) {
    this.input = new Input(emulator);

    window.addEventListener("pointerdown", () => this.startAudio());
    window.addEventListener("keydown", () => this.startAudio());

    requestAnimationFrame((now) => this.frame(now));
  }

  static async create(): Promise<Runtime> {
    return new Runtime(await Emulator.create());
  }

  get romName(): string {
    return this.emulator.romName;
  }

  get rewindSnapshots(): readonly Snapshot[] {
    return this.rewind.snapshots;
  }

  get rewindOffset(): number {
    return this.rewind.offset;
  }

  get rewindPosition(): number | null {
    return this.rewindPausedAt;
  }

  get rewindVersion(): number {
    return this.rewind.version;
  }

  subscribe = (listener: () => void): (() => void) => {
    this.listeners.add(listener);

    return () => this.listeners.delete(listener);
  };

  attach(canvas: HTMLCanvasElement): void {
    this.video = new Video(canvas);
  }

  async openRom(file: File): Promise<boolean> {
    if (!this.emulator.loadRom(new Uint8Array(await file.arrayBuffer()))) {
      return false;
    }

    this.loaded = true;

    this.clearRewind();
    this.setPaused(false);

    return true;
  }

  setPaused(value: boolean): void {
    if (!this.loaded) {
      return;
    }

    this.paused = value;

    if (!this.paused && this.rewindPausedAt !== null) {
      this.rewind.truncate(this.rewindPausedAt);
      this.rewindPausedAt = null;
      this.sinceRewind = 0;
      this.notify();
    }

    if (this.paused) {
      this.audio.suspend();
    } else {
      this.audio.start();
    }
  }

  reset(): void {
    if (!this.loaded) {
      return;
    }

    this.emulator.reset();
    this.video?.clear();
    this.clearRewind();
  }

  scrub(index: number): void {
    const snapshot = this.rewind.snapshots[index];

    if (!this.loaded || snapshot === undefined) {
      return;
    }

    if (!this.scrubbing) {
      this.scrubbing = true;
      this.pausedBeforeScrub = this.paused;
      this.audio.suspend();
    }

    this.video?.show(snapshot.frame);
  }

  endScrub(index: number): void {
    const snapshot = this.rewind.snapshots[index];

    if (!this.scrubbing) {
      return;
    }

    this.scrubbing = false;

    if (
      snapshot !== undefined &&
      this.emulator.restoreSnapshot(snapshot.state)
    ) {
      this.video?.show(snapshot.frame);

      if (this.paused) {
        this.rewindPausedAt = index;
      } else {
        this.rewind.truncate(index);
        this.sinceRewind = 0;
      }

      this.notify();
    }

    if (!this.pausedBeforeScrub) {
      this.audio.start();
    }
  }

  async hasSave(): Promise<boolean> {
    if (!this.loaded) {
      return false;
    }

    const saved = await loadState(this.emulator.romName);

    return (
      saved !== undefined && saved.version === this.emulator.snapshotVersion
    );
  }

  async save(): Promise<void> {
    if (!this.loaded) {
      return;
    }

    await saveState(this.emulator.romName, {
      version: this.emulator.snapshotVersion,
      date: Date.now(),
      data: this.emulator.takeSnapshot(),
    });
  }

  async load(): Promise<boolean> {
    if (!this.loaded) {
      return false;
    }

    const saved = await loadState(this.emulator.romName);

    if (
      saved !== undefined &&
      saved.version === this.emulator.snapshotVersion &&
      this.emulator.restoreSnapshot(saved.data)
    ) {
      this.video?.draw(this.emulator.frame());
      this.clearRewind();

      return true;
    }

    return false;
  }

  private clearRewind(): void {
    this.rewind.clear();
    this.rewindPausedAt = null;
    this.sinceRewind = 0;
    this.notify();
  }

  private notify(): void {
    for (const listener of this.listeners) {
      listener();
    }
  }

  private record(): void {
    if (this.video === null) {
      return;
    }

    this.rewind.push({
      date: Date.now(),
      state: this.emulator.takeSnapshot(),
      frame: this.video.capture(),
    });

    this.notify();
  }

  private startAudio(): void {
    if (this.loaded && !this.paused && !this.scrubbing) {
      this.audio.start();
    }
  }

  private frame(now: number): void {
    const dt = Math.min((now - this.last) / 1000, MAX_STEP);

    this.last = now;

    this.input.poll_gamepads();

    if (
      this.loaded &&
      !this.paused &&
      !this.scrubbing &&
      this.emulator.status === Status.Running
    ) {
      const rate = this.audio.sampleRate;
      const count = Math.round(dt * rate);

      this.audio.push(this.emulator.runSamples(count, this.speed / rate));
      this.video?.draw(this.emulator.frame());

      this.sinceRewind += (count / rate) * this.speed;

      if (this.sinceRewind >= SNAPSHOT_INTERVAL) {
        this.sinceRewind = 0;
        this.record();
      }
    }

    requestAnimationFrame((now) => this.frame(now));
  }
}
