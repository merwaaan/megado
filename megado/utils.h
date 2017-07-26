#pragma once

#define FATAL(...) do { fprintf(stderr, "FATAL(in %s): ", __func__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1); } while (0)
