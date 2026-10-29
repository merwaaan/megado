import { Audio } from "./audio";
import { Emulator, Status } from "./emulator";
import { Input } from "./input";
import { loadSnapshot, saveSnapshot } from "./storage";
import { Video } from "./video";

const MAX_STEP = 0.1;

const romInput = document.getElementById("rom") as HTMLInputElement;

romInput.addEventListener("change", () => {
  const file = romInput.files?.[0];

  if (file !== undefined) {
    openRom(file);
  }
});

const pauseButton = document.getElementById("pause") as HTMLButtonElement;

pauseButton.addEventListener("click", () => {
  if (loaded) {
    setPaused(!paused);
  }
});

const saveButton = document.getElementById("save") as HTMLButtonElement;
saveButton.addEventListener("click", save);

const loadButton = document.getElementById("load") as HTMLButtonElement;
loadButton.addEventListener("click", load);

const canvas = document.getElementById("screen") as HTMLCanvasElement;

const emulator = await Emulator.create();
const video = new Video(canvas);
const audio = new Audio();
const input = new Input(emulator);

let loaded = false;
let paused = false;
let last = performance.now();

async function openRom(file: File): Promise<void> {
  emulator.loadRom(new Uint8Array(await file.arrayBuffer()));

  loaded = true;

  setPaused(false);
}

function setPaused(value: boolean): void {
  paused = value;

  pauseButton.textContent = paused ? "Resume" : "Pause";

  if (paused) {
    audio.suspend();
  } else {
    audio.start();
  }
}

async function save(): Promise<void> {
  if (!loaded) {
    return;
  }

  await saveSnapshot(emulator.romName, {
    version: emulator.snapshotVersion,
    date: Date.now(),
    data: emulator.takeSnapshot(),
  });
}

async function load(): Promise<void> {
  if (!loaded) {
    return;
  }

  const saved = await loadSnapshot(emulator.romName);

  if (
    saved !== undefined &&
    saved.version === emulator.snapshotVersion &&
    emulator.restoreSnapshot(saved.data)
  ) {
    video.draw(emulator.frame());
  }
}

function startAudio(): void {
  if (loaded && !paused) {
    audio.start();
  }
}

window.addEventListener("pointerdown", startAudio);
window.addEventListener("keydown", startAudio);

function frame(now: number): void {
  const dt = Math.min((now - last) / 1000, MAX_STEP);

  last = now;

  input.poll_gamepads();

  if (loaded && !paused && emulator.status === Status.Running) {
    const rate = audio.sampleRate;
    const count = Math.round(dt * rate);

    audio.push(emulator.runSamples(count, 1 / rate));
    video.draw(emulator.frame());
  }

  requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
