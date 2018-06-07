#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gym.h"

static const uint32_t NTSC_MASTER_FREQUENCY = 53693175;
static const uint32_t SAMPLE_RATE = 44100;

static bool g_playing = false;
YM2612 *gym_ym2612;
PSG *gym_psg;

void gym_stop() {
  g_playing = false;
}

void gym_play(struct GYM gym) {
  g_playing = true;

  YM2612 ym;
  PSG psg;

  ym2612_initialize(&ym);
  psg_initialize(&psg);

  gym_ym2612 = &ym;
  gym_psg = &psg;

  uint8_t dac_buffer[1024];
  uint16_t dac_count = 0;

  double remaining_time = 0;
  double time_slice = (double)1 / SAMPLE_RATE;
  uint32_t slice_cycles = time_slice * NTSC_MASTER_FREQUENCY;

  uint64_t pc = 0;
  uint8_t opcode;

  SDL_AudioSpec want, have;

  SDL_memset(&want, 0, sizeof(want));
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 512;
  want.callback = NULL; // Use SDL_QueueAudio instead

  SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

  if (audio_device == 0) {
    fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_PauseAudioDevice(audio_device, 0);

  while (g_playing && pc < gym.size) {
    opcode = gym.data[pc++];

    switch (opcode) {
      // Sleep for 1/60th of a second
      // @Correctness: The GYM spec does not say if this is an exact 1/60th, or
      // one frame of the NTSC Genesis (which is at 59.9Hz)
      // @Correctness: the doc also states the header indicates NTSC or PAL, but
      // there's no corresponding attribute.
    case 0x00: {
      // Compute number of master cycles for the sleep period
      double sleep_period = (double)1/60;
      remaining_time += sleep_period;

      int dac_index = 0;

      int audio_samples_count = SAMPLE_RATE / 60;
      double samples_per_dac = (double)dac_count / audio_samples_count;
      double samples_this_dac = 0;

      // Emulate for that duration in slice of audio samples
      while (remaining_time > 0) {

        // If we have some DAC samples, we need to feed them to the YM2612
        if (dac_count > 0 && dac_index < dac_count) {
          // Write the DAC samples evenly throughout the frame
          samples_this_dac += samples_per_dac;
          if (samples_this_dac > 1) {
            ym2612_write_register(&ym, 0x2a, dac_buffer[dac_index++], PART_I);
            samples_this_dac -= 1;
          }
        }

        ym2612_run_cycles(&ym, slice_cycles);
        psg_run_cycles(&psg, slice_cycles);

        // Sample the audio units
        // @Temporary: use left channel for PSG and right channel for YM2612
        int16_t sample[2] = {psg_mix(&psg), ym2612_mix(&ym)};
        // Queue samples to audio device
        if (SDL_QueueAudio(audio_device, sample, 2 * sizeof(int16_t)) != 0) {
          fprintf(stderr, "Failed to queue audio: %s", SDL_GetError());
        }

        remaining_time -= time_slice;
      }

      // All DAC samples should have been flushed this frame, so reset
      dac_count = 0;

    } break;

      // Write to part I
    case 0x01: {
      uint8_t reg = gym.data[pc++];
      uint8_t value = gym.data[pc++];

      // Multiple DAC writes often happen between two video frames, so we have
      // to save them into a buffer in order to feed them to the YM2612 between
      // audio samples.  Otherwise, only the last write will be taken into
      // account by the YM2612.
      if (reg == 0x2a) {
        if (dac_count >= 1024) {
          printf("Warning: DAC buffer overflow\n");
        } else {
          dac_buffer[dac_count++] = value;
        }
      } else {
        ym2612_write_register(&ym, reg, value, PART_I);
      }

    } break;

      // Write to part II
    case 0x02: {
      uint8_t reg = gym.data[pc++];
      uint8_t value = gym.data[pc++];
      ym2612_write_register(&ym, reg, value, PART_II);
    } break;

      // Write to PSG
    case 0x03: {
      uint8_t value = gym.data[pc++];
      psg_write(&psg, value);
    } break;

    default:
      printf("Invalid opcode: %x\n", opcode);
    }
  }

  printf("Emulation done.  Playing remaining audio\n");

  // Wait for the audio queue to empty before exiting
  while (g_playing && SDL_GetQueuedAudioSize(audio_device) > 0) {
    usleep(100);
  }

  // Destroy SDL
  SDL_CloseAudioDevice(audio_device);

  // Release pointers
  gym_ym2612 = NULL;
  gym_psg = NULL;
}

struct GYM read_gym(FILE *gym_file) {
  // Find out size of file
  struct stat gym_file_stat;
  fstat(fileno(gym_file), &gym_file_stat);

  // Map file to memory
  void *gym = mmap(NULL, gym_file_stat.st_size, PROT_READ, MAP_PRIVATE, fileno(gym_file), 0);
  if (gym == MAP_FAILED) {
    perror("Error mmaping the file:");
    exit(1);
  }

  // If the file begins with GYMX, there's a 428 bytes header first
  struct GYMHeader *header = gym;
  struct GYM ret;

  if (strncmp(header->tag, "GYMX", 4) == 0) {
    ret.header = header;
    ret.data = gym + sizeof(struct GYMHeader);
    ret.size = gym_file_stat.st_size - sizeof(struct GYMHeader);
  } else {
    // Otherwise, the whole file is just data
    ret.header = NULL;
    ret.data = gym;
    ret.size = gym_file_stat.st_size;
  }

  return ret;
}
