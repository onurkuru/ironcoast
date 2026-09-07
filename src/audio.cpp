#include "audio.h"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace kh {
Audio::Audio() {
  SDL_AudioSpec want{}, have{};
  // 32 kHz keeps the low-frequency arcade punch while giving laser and
  // metallic transients enough headroom on both desktop and Vita audio.
  want.freq = 32000;
  want.format = AUDIO_S16SYS;
  want.channels = 1;
  want.samples = 512;
  want.callback = callback;
  want.userdata = this;
  device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  if (device) {
    sampleRate = float(have.freq);
    SDL_PauseAudioDevice(device, 0);
  }
}
Audio::~Audio() {
  if (device)
    SDL_CloseAudioDevice(device);
}
void Audio::settings(int t, bool m, bool p, bool b) {
  if (!device)
    return;
  SDL_LockAudioDevice(device);
  theme = t;
  muted = m;
  paused = p;
  boss = b;
  SDL_UnlockAudioDevice(device);
}
void Audio::play(Sound s) {
  if (!device)
    return;
  SDL_LockAudioDevice(device);
  Voice *v = nullptr;
  for (auto &vv : voices)
    if (vv.remaining <= 0) {
      v = &vv;
      break;
    }
  if (!v) {
    SDL_UnlockAudioDevice(device);
    return;
  }
  *v = {0, 300, 0, .22f, .1f, .1f, 0, .35f};
  switch (s) {
  case Sound::Shot:
    v->freq = 700;
    v->slide = -6000;
    v->remaining = .055f;
    v->wave = 2;
    v->noiseMix = .28f;
    break;
  case Sound::Heavy:
    v->freq = 330;
    v->slide = -1800;
    v->remaining = .07f;
    v->wave = 2;
    v->volume = .16f;
    v->noiseMix = .22f;
    break;
  case Sound::Shotgun:
    v->freq = 180;
    v->remaining = .15f;
    v->wave = 2;
    v->volume = .3f;
    v->noiseMix = .62f;
    break;
  case Sound::Rocket:
    v->freq = 110;
    v->remaining = .25f;
    v->slide = 100;
    v->wave = 2;
    v->noiseMix = .42f;
    break;
  case Sound::Flame:
    v->freq = 95;
    v->slide = 340;
    v->remaining = .18f;
    v->wave = 2;
    v->volume = .22f;
    v->noiseMix = .46f;
    break;
  case Sound::Laser:
    v->freq = 920;
    v->slide = -2400;
    v->remaining = .12f;
    v->volume = .18f;
    v->noiseMix = .025f;
    break;
  case Sound::Grenade:
    v->freq = 520;
    v->slide = -850;
    v->remaining = .13f;
    break;
  case Sound::Blast:
    v->freq = 70;
    v->slide = -100;
    v->wave = 2;
    v->remaining = .42f;
    v->volume = .35f;
    v->noiseMix = .78f;
    break;
  case Sound::Hit:
    v->freq = 250;
    v->wave = 2;
    v->remaining = .07f;
    v->noiseMix = .38f;
    break;
  case Sound::Jump:
    v->freq = 270;
    v->slide = 2100;
    v->remaining = .12f;
    v->volume = .12f;
    break;
  case Sound::Pickup:
    v->freq = 850;
    v->slide = 1600;
    v->remaining = .19f;
    v->volume = .16f;
    break;
  case Sound::Rescue:
    v->freq = 600;
    v->slide = 1200;
    v->remaining = .3f;
    v->volume = .17f;
    break;
  case Sound::Hurt:
    v->freq = 280;
    v->slide = -650;
    v->remaining = .35f;
    v->volume = .28f;
    break;
  case Sound::Boss:
    v->freq = 85;
    v->remaining = .45f;
    v->wave = 1;
    v->volume = .16f;
    break;
  case Sound::Step:
    v->freq = 120;
    v->remaining = .025f;
    v->wave = 2;
    v->volume = .08f;
    v->noiseMix = .58f;
    break;
  }
  v->total = v->remaining;
  SDL_UnlockAudioDevice(device);
}
void Audio::callback(void *user, Uint8 *data, int len) {
  static_cast<Audio *>(user)->mix(reinterpret_cast<int16_t *>(data), len / 2);
}
void Audio::mix(int16_t *out, int count) {
  static const int motifs[6][16] = {{0, 7, 12, 7, 3, 10, 15, 10, 5, 12, 17, 12, 7, 14, 19, 14},
                                    {0, 3, 7, 10, 0, 5, 7, 12, 3, 7, 10, 14, 5, 7, 12, 10},
                                    {0, 7, 0, 12, 3, 10, 3, 15, 5, 12, 5, 17, 7, 14, 10, 7},
                                    {0, 0, 7, 3, 0, 12, 7, 3, 5, 5, 12, 8, 7, 14, 10, 7},
                                    {0, 7, 10, 14, 12, 7, 3, 10, 5, 12, 15, 19, 14, 10, 7, 3},
                                    {0, 12, 7, 3, 10, 7, 15, 12, 5, 17, 12, 8, 7, 19, 14, 10}};
  float bpm = boss ? 142 : 112 + theme * 3, stepSamples = sampleRate * 60 / (bpm * 4),
        dt = 1 / sampleRate;
  for (int i = 0; i < count; i++) {
    noise ^= noise << 13;
    noise ^= noise >> 17;
    noise ^= noise << 5;
    float n = float(noise & 65535) / 32768 - 1;
    float sum = 0;
    if (!paused && !muted) {
      int step = int(sample / stepSamples) % 16;
      float f = float(std::fmod(double(sample), double(stepSamples)) / stepSamples),
            t = float(sample) / sampleRate;
      int note = motifs[theme % 6][step];
      float transpose = boss ? 3.0f : 0.0f;
      float hz = 110 * std::pow(2.0f, (note + transpose) / 12.0f);
      float lead = std::sin(6.283185f * hz * 2 * t) * std::exp(-f * 4) * (boss ? .045f : .036f);
      int chordNote = motifs[theme % 6][(step / 4) * 4];
      float rootHz = 55 * std::pow(2.0f, (chordNote + transpose) / 12.0f);
      float thirdHz = rootHz * (theme % 2 ? 1.1892f : 1.2599f);
      float fifthHz = rootHz * 1.4983f;
      float padGate = .72f + .28f * std::sin(6.283185f * t / 4.0f);
      float pad = (std::sin(6.283185f * rootHz * t) * .014f +
                   std::sin(6.283185f * thirdHz * t) * .010f +
                   std::sin(6.283185f * fifthHz * t) * .008f) * padGate;
      float bass = (2 / std::acos(-1.0f)) * std::asin(std::sin(6.283185f * rootHz * .5f * t)) *
                   (boss ? .064f : .05f);
      float kick = step % 4 == 0
                       ? std::sin(6.283185f * (65 - 35 * f) * f * .14f) * std::exp(-f * 12) * .12f
                       : 0;
      float snare = step % 8 == 4 ? n * std::exp(-f * 18) * (boss ? .095f : .075f) : 0;
      float hat = n * std::exp(-f * 40) * (step % 2 ? .018f : .027f);
      float ghost = step % 4 == 3 ? n * std::exp(-f * 34) * .012f : 0;
      float tom = boss && step % 8 == 6 ? std::sin(6.283185f * (145 - 35 * f) * t) *
                                           std::exp(-f * 15) * .035f
                                       : 0;
      float alarm = boss ? std::sin(6.283185f * (190 + 28 * std::sin(t * .8f)) * t) *
                               std::exp(-f * 3) * .012f
                         : 0;
      sum = lead + pad + bass + kick + snare + hat + ghost + tom + alarm;
      sample++;
    }
    for (auto &v : voices)
      if (v.remaining > 0 && !paused) {
        v.remaining -= dt;
        v.freq = std::max(20.0f, v.freq + v.slide * dt);
        v.phase += v.freq * dt;
        v.phase -= std::floor(v.phase);
        float osc = v.wave == 2   ? n * v.noiseMix + std::sin(v.phase * 6.283185f) * (1.0f - v.noiseMix)
                    : v.wave == 1 ? (v.phase < .5f ? .6f : -.6f)
                                  : std::sin(v.phase * 6.283185f);
        // A short attack and release prevents zipper clicks while retaining
        // the sharp arcade transients at the start of each effect.
        float age = std::max(0.0f, v.total - v.remaining);
        float attack = std::min(1.0f, age / .004f);
        float releaseWindow = std::max(.008f, std::min(.035f, v.total * .45f));
        float release = std::min(1.0f, std::max(0.0f, v.remaining) / releaseWindow);
        float env = attack * release;
        sum += osc * v.volume * env;
      }
    out[i] = muted || paused ? 0 : int16_t(std::max(-.95f, std::min(.95f, sum)) * 32760);
  }
}
} // namespace kh
