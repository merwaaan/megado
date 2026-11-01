import { Box, Text } from "@mantine/core";
import { useElementSize } from "@mantine/hooks";
import {
  useEffect,
  useRef,
  useState,
  useSyncExternalStore,
  type PointerEvent,
} from "react";
import { RECORDED_SNAPSHOTS, SNAPSHOT_INTERVAL } from "./rewind";
import type { Runtime } from "./runtime";

const HEIGHT = 64;
const THUMB_WIDTH = (HEIGHT * 320) / 224;

type Props = {
  runtime: Runtime;
  disabled: boolean;
};

// Filmstrip of the recorded snapshots, spanning the whole rewind duration.
// Dragging along it shows past frames, releasing rewinds the emulator there.
export function Timeline({ runtime, disabled }: Props) {
  const { ref, width } = useElementSize();

  // Re-render whenever snapshots are updated

  useSyncExternalStore(runtime.subscribe, () => runtime.rewindVersion);

  // Index being dragged to, null when not dragging

  const [dragged, setDragged] = useState<number | null>(null);

  const snapshots = runtime.rewindSnapshots;

  const last = snapshots.length - 1;

  const enabled = !disabled && snapshots.length >= 2;

  // The strip always shows RECORDED_SNAPSHOTS, even before the history is full.
  // A snapshot occupies `scale` pixels and cells keep a fixed width.
  // Each cell covers `stride` snapshots and shows the frame of the first one.

  const offset = runtime.rewindOffset;

  const count = Math.max(Math.floor(width / THUMB_WIDTH), 1);

  const stride = Math.ceil(RECORDED_SNAPSHOTS / count);

  const scale = width / RECORDED_SNAPSHOTS;

  // Cells start on snapshot numbers that are multiples of the stride, so a cell keeps showing the same frame and slides left as recording goes on.
  // The first cell can start before the oldest recorded snapshot.
  // It is then partly off the strip and keeps the frame it already displays.

  const thumbs: number[] = [];

  for (
    let number = Math.floor(offset / stride) * stride;
    number < offset + snapshots.length;
    number += stride
  ) {
    thumbs.push(number);
  }

  // Index the playhead is held at: being dragged, or released while paused
  // (the later snapshots are then kept until resuming). Otherwise, the
  // playhead follows the newest snapshot.

  const held = dragged ?? runtime.rewindPosition;

  const current = held ?? last;

  const position = (current + 0.5) / RECORDED_SNAPSHOTS;

  // Convert the pointer position to a snapshot index, clamped to the recorded ones
  // (the right part of the strip is empty until the history is full)

  function indexAt(event: PointerEvent<HTMLDivElement>): number {
    const bounds = event.currentTarget.getBoundingClientRect();
    const fraction = (event.clientX - bounds.left) / bounds.width;

    return Math.min(
      Math.max(Math.floor(fraction * RECORDED_SNAPSHOTS), 0),
      last,
    );
  }

  // Pressing starts scrubbing.
  // The emulator stops and the frame at the pointer is displayed.
  // The pointer is captured so that the drag keeps working when leaving the strip.

  function down(event: PointerEvent<HTMLDivElement>): void {
    if (!enabled) {
      return;
    }

    event.currentTarget.setPointerCapture(event.pointerId);

    const index = indexAt(event);
    setDragged(index);
    runtime.scrub(index);
  }

  function move(event: PointerEvent<HTMLDivElement>): void {
    if (dragged === null) {
      return;
    }

    const index = indexAt(event);

    if (index !== dragged) {
      setDragged(index);
      runtime.scrub(index);
    }
  }

  // Releasing restores the emulator at the snapshot under the pointer

  function up(event: PointerEvent<HTMLDivElement>): void {
    if (dragged === null) {
      return;
    }

    const index = indexAt(event);
    setDragged(null);
    runtime.endScrub(index);
  }

  return (
    <Box
      ref={ref}
      w="100%"
      h={HEIGHT}
      pos="relative"
      onPointerDown={down}
      onPointerMove={move}
      onPointerUp={up}
      style={{
        cursor: enabled ? "pointer" : "default",
        opacity: enabled ? 1 : 0.4,
        touchAction: "none",
        userSelect: "none",
      }}
    >
      {/* Cells, clipped so that the first one can slide off the left edge */}

      <Box
        pos="absolute"
        inset={0}
        style={{
          overflow: "hidden",
          background: "var(--mantine-color-dark-8)",
          borderRadius: 4,
        }}
      >
        {thumbs.map((number) => {
          const index = number - offset;

          const size = stride * scale;

          // The cell is keyed by snapshot number so that React keeps its canvas while it moves.
          // The rightmost cell is only revealed up to the latest snapshot.

          return (
            <Cell
              key={number}
              frame={snapshots[index]?.frame}
              left={index * scale}
              size={size}
              visibleSize={Math.min((snapshots.length - index) * scale, size)}
            />
          );
        })}
      </Box>

      {/* Playhead, with the game time separating it from the latest snapshot */}

      {enabled && (
        <Box
          pos="absolute"
          top={-4}
          bottom={-4}
          w={2}
          bg="blue.6"
          style={{
            left: `calc(${position * 100}% - 1px)`,
            pointerEvents: "none",
          }}
        >
          {held !== null && (
            <Text
              size="xs"
              px={4}
              bg="blue.6"
              c="white"
              pos="absolute"
              top={-20}
              style={{
                transform: "translateX(-50%)",
                borderRadius: 4,
                whiteSpace: "nowrap",
              }}
            >
              {((last - held) * SNAPSHOT_INTERVAL).toFixed(1)} seconds ago
            </Text>
          )}
        </Box>
      )}
    </Box>
  );
}

// A cell of the strip.
// The outer div is `visibleSize` pixels wide and clips the `size` pixels wide canvas.

type CellProps = {
  frame: ImageData | undefined;
  left: number;
  size: number;
  visibleSize: number;
};

function Cell({ frame, left, visibleSize: visible, size }: CellProps) {
  const canvas = useRef<HTMLCanvasElement>(null);

  // Draw the frame when it changes.
  // An undefined frame means its snapshot was dropped and we keep what is already drawn.

  useEffect(() => {
    const context = canvas.current?.getContext("2d");

    if (context && frame) {
      context.canvas.width = frame.width;
      context.canvas.height = frame.height;
      context.putImageData(frame, 0, 0);
    }
  }, [frame]);

  return (
    <div
      style={{
        position: "absolute",
        left,
        width: Math.max(visible, 0),
        height: "100%",
        overflow: "hidden",
      }}
    >
      <canvas
        ref={canvas}
        style={{
          display: "block",
          width: size,
          height: "100%",
          objectFit: "cover",
          imageRendering: "pixelated",
          background: "black",
        }}
      />
    </div>
  );
}
