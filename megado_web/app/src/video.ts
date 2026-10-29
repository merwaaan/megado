import type { Frame } from "./emulator";

export class Video {
  private readonly context: CanvasRenderingContext2D;
  private image: ImageData;

  constructor(private readonly canvas: HTMLCanvasElement) {
    const context = canvas.getContext("2d");

    if (context === null) {
      throw new Error("2D canvas is not supported");
    }

    this.context = context;
    this.image = context.createImageData(canvas.width, canvas.height);
  }

  draw(frame: Frame): void {
    // Resize if needed

    if (
      frame.width !== this.canvas.width ||
      frame.height !== this.canvas.height
    ) {
      this.canvas.width = frame.width;
      this.canvas.height = frame.height;
      this.image = this.context.createImageData(frame.width, frame.height);
    }

    // Blit

    const source = frame.pixels;
    const target = this.image.data;

    for (let y = 0; y < frame.height; ++y) {
      let from = y * frame.pitch;
      let to = y * frame.width * 4;

      for (let x = 0; x < frame.width; ++x) {
        target[to] = source[from];
        target[to + 1] = source[from + 1];
        target[to + 2] = source[from + 2];
        target[to + 3] = 255;
        from += 3;
        to += 4;
      }
    }

    this.context.putImageData(this.image, 0, 0);
  }
}
