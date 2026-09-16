#pragma once
#include "game.h"
#include <SDL.h>
#include <array>
namespace kh {
class Audio {
  friend struct AudioAudit;
  struct Voice {
    float phase = 0, freq = 0, slide = 0, volume = 0, remaining = 0, total = 0;
    int wave = 0;
    float noiseMix = .35f;
    float low = 0, filter = .5f, distance = 0, age = 0, decay = 20;
    int model = -1, event = 0;
  };
  SDL_AudioDeviceID device = 0;
  std::array<Voice, 24> voices{};
  float sampleRate = 32000;
  uint64_t sample = 0;
  uint32_t noise = 123;
  int theme = 0;
  bool musicEnabled = true;
  bool muted = false, paused = false, boss = false;
  static void callback(void *, Uint8 *, int);
  void mix(int16_t *, int);

public:
  explicit Audio(bool openDevice = true);
  ~Audio();
  void play(Sound, float distance = 0, int weapon = -1);
  void settings(int, bool, bool, bool);
  // Offline capture uses the same mixer as SDL, with no competing callback.
  void renderOffline(int16_t *,int);
};
} // namespace kh
