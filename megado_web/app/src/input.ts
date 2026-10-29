import { Button, type Emulator } from "./emulator";

const KEYBOARD: Record<string, [number, Button]> = {
  ArrowUp: [0, Button.Up],
  ArrowDown: [0, Button.Down],
  ArrowLeft: [0, Button.Left],
  ArrowRight: [0, Button.Right],
  Enter: [0, Button.Start],
  KeyQ: [0, Button.A],
  KeyW: [0, Button.B],
  KeyE: [0, Button.C],

  Digit1: [1, Button.Left],
  Digit3: [1, Button.Right],
  Digit5: [1, Button.Up],
  Digit2: [1, Button.Down],
  Digit0: [1, Button.Start],
  Digit7: [1, Button.A],
  Digit8: [1, Button.B],
  Digit9: [1, Button.C],
};

const GAMEPAD: [number, Button][] = [
  [12, Button.Up],
  [13, Button.Down],
  [14, Button.Left],
  [15, Button.Right],
  [9, Button.Start],
  [2, Button.A],
  [0, Button.B],
  [1, Button.C],
];

export class Input {
  private readonly gamepadState = new Map<string, boolean>();

  constructor(private readonly emulator: Emulator) {
    window.addEventListener("keydown", (event) =>
      this.handle_keyboard(event, true),
    );
    window.addEventListener("keyup", (event) =>
      this.handle_keyboard(event, false),
    );
  }

  poll_gamepads(): void {
    for (const pad of navigator.getGamepads()) {
      // Use the two first gamepads

      if (pad === null || pad.index > 1) {
        continue;
      }

      // Poll the buttons

      for (const [index, button] of GAMEPAD) {
        const pressed = pad.buttons[index]?.pressed ?? false;

        const id = `${pad.index}:${index}`;

        if (this.gamepadState.get(id) !== pressed) {
          this.gamepadState.set(id, pressed);

          // Forward to the emulator

          this.emulator.setButton(pad.index, button, pressed);
        }
      }
    }
  }

  private handle_keyboard(event: KeyboardEvent, pressed: boolean): void {
    const mapping = KEYBOARD[event.code];

    if (mapping) {
      event.preventDefault();

      // Forward to the emulator

      if (!event.repeat) {
        this.emulator.setButton(mapping[0], mapping[1], pressed);
      }
    }
  }
}
