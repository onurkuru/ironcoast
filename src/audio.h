#pragma once
#include "game.h"
#include <SDL.h>
#include <array>
namespace kh {
class Audio {
  struct Voice {
    float phase = 0, freq = 0, slide = 0, volume = 0, remaining = 0, total = 0;
    int wave = 0;
    float noiseMix = .35f;
  };
  SDL_AudioDeviceID device = 0;
  std::array<Voice, 24> voices{};
  float sampleRate = 32000;
  uint64_t sample = 0;
  uint32_t noise = 123;
  int theme = 0;
  bool muted = false, paused = false, boss = false;
  static void callback(void *, Uint8 *, int);
  void mix(int16_t *, int);

public:
  Audio();
  ~Audio();
  void play(Sound);
  void settings(int, bool, bool, bool);
};
} // namespace kh
