export type Snapshot = {
  date: number;
  state: Uint8Array;
  frame: ImageData;
};

export const REWIND_DURATION = 30; // seconds
export const SNAPSHOTS_PER_SECOND = 10;

export const RECORDED_SNAPSHOTS = REWIND_DURATION * SNAPSHOTS_PER_SECOND;
export const SNAPSHOT_INTERVAL = 1 / SNAPSHOTS_PER_SECOND;

export class Rewind {
  #snapshots: Snapshot[] = [];

  get snapshots(): readonly Snapshot[] {
    return this.#snapshots;
  }

  // Current version, bumped when the rewind is modified.
  // Allows external observers to detect changes.
  #version = 0;

  get version(): number {
    return this.#version;
  }

  // Number of snapshots that have been dropped due to overflow.
  // Used to calculate the current rewind position.
  #offset = 0;

  get offset(): number {
    return this.#offset;
  }

  push(snapshot: Snapshot): void {
    this.#snapshots.push(snapshot);

    if (this.#snapshots.length > RECORDED_SNAPSHOTS) {
      this.#snapshots.shift();
      this.#offset++;
    }

    this.#version++;
  }

  truncate(index: number): void {
    this.#snapshots.length = index + 1;
    this.#version++;
  }

  clear(): void {
    this.#snapshots = [];
    this.#offset = 0;
    this.#version++;
  }
}
