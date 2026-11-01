import {
  AppShell,
  Button,
  Divider,
  FileButton,
  Group,
  Slider,
  Stack,
  Text,
  Title,
} from "@mantine/core";
import {
  IconMaximize,
  IconPlayerPauseFilled,
  IconPlayerPlayFilled,
  IconRefresh,
  IconWindowMaximize,
} from "@tabler/icons-react";
import { useFullscreenElement, useMergedRef } from "@mantine/hooks";
import { notifications } from "@mantine/notifications";
import { useEffect, useRef, useState } from "react";
import type { Runtime } from "./runtime";
import { Timeline } from "./timeline";

type Scale = 1 | 2 | 3 | "fill";

type Props = {
  runtime: Runtime;
};

export function App({ runtime }: Props) {
  const canvas = useRef<HTMLCanvasElement>(null);
  const fullscreen = useFullscreenElement<HTMLCanvasElement>();
  const canvasRef = useMergedRef(canvas, fullscreen.ref);

  const [romName, setRomName] = useState<string | null>(null);
  const [paused, setPaused] = useState(false);
  const [hasSave, setHasSave] = useState(false);
  const [speed, setSpeed] = useState(1);
  const [scale, setScale] = useState<Scale>(2);

  useEffect(() => {
    if (canvas.current !== null) {
      runtime.attach(canvas.current);
    }
  }, [runtime]);

  async function openRom(file: File | null): Promise<void> {
    if (file === null) {
      return;
    }

    if (!(await runtime.openRom(file))) {
      notifications.show({ message: "Could not load ROM", color: "red" });
      return;
    }

    setRomName(runtime.romName || file.name);
    setPaused(false);
    setHasSave(await runtime.hasSave());
  }

  function togglePause(): void {
    runtime.setPaused(!paused);
    setPaused(!paused);
  }

  function reset(): void {
    runtime.reset();
  }

  function changeSpeed(value: number): void {
    runtime.speed = value;
    setSpeed(value);
  }

  async function save(): Promise<void> {
    await runtime.save();
    setHasSave(true);
    notifications.show({ message: "State saved", color: "green" });
  }

  async function load(): Promise<void> {
    if (await runtime.load()) {
      notifications.show({ message: "State loaded", color: "green" });
    } else {
      notifications.show({ message: "Could not load state", color: "red" });
    }
  }

  const loaded = romName !== null;

  return (
    <AppShell header={{ height: 60 }} footer={{ height: 84 }} padding="md">
      <AppShell.Header>
        <Group h="100%" px="md" justify="space-between">
          <Group>
            <Title order={3}>Megado</Title>
            {loaded && <Text c="dimmed">{romName}</Text>}
          </Group>

          <Group gap="md">
            <FileButton onChange={openRom} accept=".md,.gen,.zip">
              {(props) => <Button {...props}>Open ROM</Button>}
            </FileButton>

            <Divider orientation="vertical" />

            <Button.Group>
              <Button
                variant="default"
                disabled={!loaded}
                onClick={togglePause}
                leftSection={
                  paused ? (
                    <IconPlayerPlayFilled size={16} />
                  ) : (
                    <IconPlayerPauseFilled size={16} />
                  )
                }
              >
                {paused ? "Resume" : "Pause"}
              </Button>

              <Button
                variant="default"
                disabled={!loaded}
                onClick={reset}
                leftSection={<IconRefresh size={16} />}
              >
                Reset
              </Button>
            </Button.Group>

            <Divider orientation="vertical" />

            <Group gap="xs">
              <Text size="sm">Speed</Text>

              <Slider
                w={160}
                min={0.2}
                max={3}
                step={0.1}
                value={speed}
                onChange={changeSpeed}
                label={(value) => `${value.toFixed(1)}x`}
              />
            </Group>

            <Divider orientation="vertical" />

            <Button.Group>
              <Button variant="default" disabled={!loaded} onClick={save}>
                Save
              </Button>

              <Button
                variant="default"
                disabled={!loaded || !hasSave}
                onClick={load}
              >
                Load
              </Button>
            </Button.Group>
          </Group>
        </Group>
      </AppShell.Header>
      <AppShell.Main>
        <Stack
          align="center"
          justify="center"
          gap="xs"
          mih="calc(100dvh - 144px - 2 * var(--mantine-spacing-md))"
          h={
            scale === "fill"
              ? "calc(100dvh - 144px - 2 * var(--mantine-spacing-md))"
              : undefined
          }
        >
          <canvas
            ref={canvasRef}
            width={320}
            height={224}
            style={
              scale === "fill"
                ? {
                    flex: 1,
                    minHeight: 0,
                    width: "100%",
                    objectFit: "contain",
                    imageRendering: "pixelated",
                    background: "black",
                  }
                : {
                    zoom: fullscreen.fullscreen ? 1 : scale,
                    imageRendering: "pixelated",
                    background: "black",
                  }
            }
          />

          <Button.Group>
            {([1, 2, 3] as const).map((value) => (
              <Button
                key={value}
                size="xs"
                variant={scale === value ? "filled" : "default"}
                onClick={() => setScale(value)}
              >
                {value}x
              </Button>
            ))}

            <Button
              size="xs"
              variant={scale === "fill" ? "filled" : "default"}
              onClick={() => setScale("fill")}
              title="Fill window"
            >
              <IconWindowMaximize size={16} />
            </Button>

            <Button
              size="xs"
              variant="default"
              onClick={fullscreen.toggle}
              title="Fill screen"
            >
              <IconMaximize size={16} />
            </Button>
          </Button.Group>
        </Stack>
      </AppShell.Main>

      <AppShell.Footer>
        <Group h="100%" px="md">
          <Timeline runtime={runtime} disabled={!loaded} />
        </Group>
      </AppShell.Footer>
    </AppShell>
  );
}
