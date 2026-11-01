import "@mantine/core/styles.css";
import "@mantine/notifications/styles.css";

import { MantineProvider } from "@mantine/core";
import { Notifications } from "@mantine/notifications";
import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { App } from "./app";
import { Runtime } from "./runtime";

const runtime = await Runtime.create();

createRoot(document.getElementById("root") as HTMLElement).render(
  <StrictMode>
    <MantineProvider defaultColorScheme="dark">
      <Notifications position="bottom-right" />
      <App runtime={runtime} />
    </MantineProvider>
  </StrictMode>,
);
