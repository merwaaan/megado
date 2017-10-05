#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../megado/ym2612.h"
#include "../megado/psg.h"

struct GYMHeader {
  char tag[4];
  char song[32];
  char game[32];
  char copyright[32];
  char emulator[32];
  char dumper[32];
  char comment[256];
  uint32_t loop_start;
  uint32_t packed_size;
};

static const uint32_t NTSC_MASTER_FREQUENCY = 53693175;
static const uint32_t SAMPLE_RATE = 44100;

void gym_play(uint8_t *data, uint64_t size) {
  YM2612 ym;
  PSG psg;

  ym2612_initialize(&ym);
  psg_initialize(&psg);

  double remaining_time = 0;
  double time_slice = (double)1 / SAMPLE_RATE;
  uint32_t slice_cycles = time_slice * NTSC_MASTER_FREQUENCY;

  uint64_t pc = 0;
  uint8_t opcode;

  // Init SDL audio
  SDL_Init(SDL_INIT_AUDIO);
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

  while (pc < size) {
    opcode = data[pc++];

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

      // Emulate for that duration in slice of audio samples
      while (remaining_time > 0) {
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

    } break;

      // Write to part I
    case 0x01: {
      uint8_t reg = data[pc++];
      uint8_t value = data[pc++];
      ym2612_write_register(&ym, reg, value, PART_I);
    } break;

      // Write to part II
    case 0x02: {
      uint8_t reg = data[pc++];
      uint8_t value = data[pc++];
      ym2612_write_register(&ym, reg, value, PART_II);
    } break;

      // Write to PSG
    case 0x03: {
      uint8_t value = data[pc++];
      psg_write(&psg, value);
    } break;

    default:
      printf("Invalid opcode: %x\n", opcode);
    }
  }

  printf("Emulation done.  Playing remaining audio\n");

  // Wait for the audio queue to empty before exiting
  while (SDL_GetQueuedAudioSize(audio_device) > 0) {
    sleep(1);
  }

  // Destroy SDL
  SDL_CloseAudioDevice(audio_device);
  SDL_Quit();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: player GYM_FILE");
    exit(1);
  }

  char *gym_file_path = argv[1];
  FILE *gym_file;

  if ((gym_file = fopen(gym_file_path, "rb")) == NULL) {
    perror("Error opening GYM file");
    exit(1);
  }

  // Find out size of file
  struct stat gym_file_stat;
  fstat(fileno(gym_file), &gym_file_stat);

  // Map file to memory
  void *gym = mmap(NULL, gym_file_stat.st_size, PROT_READ, MAP_PRIVATE, fileno(gym_file), 0);
  if (gym == MAP_FAILED) {
    perror("Error mmaping the file:");
    exit(1);
  }

  // If the file begins GYMX, there's a 428 bytes header first
  struct GYMHeader *header = gym;
  uint8_t *gym_data;
  uint64_t gym_data_length;

  if (strncmp(header->tag, "GYMX", 4) == 0) {
    printf("song: %s\n", header->song);
    printf("game: %s\n", header->game);
    printf("copyright: %s\n", header->copyright);
    printf("emulator: %s\n", header->emulator);
    printf("dumper: %s\n", header->dumper);
    printf("comment: %s\n", header->comment);
    printf("loop_start: %u\n", header->loop_start);

    gym_data = gym + sizeof(struct GYMHeader);
    gym_data_length = gym_file_stat.st_size - sizeof(struct GYMHeader);
  } else {
    // Otherwise, the whole file is just data
    gym_data = gym;
    gym_data_length = gym_file_stat.st_size;
  }

  gym_play(gym_data, gym_data_length);

  return 0;
}
