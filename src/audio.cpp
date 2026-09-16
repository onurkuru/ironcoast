#include "audio.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace kh {
Audio::Audio(bool openDevice) {
  if (!openDevice) return;
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
  if (device) SDL_LockAudioDevice(device);
  theme = t;
  muted = m;
  paused = p;
  boss = b;
  if (device) SDL_UnlockAudioDevice(device);
}
void Audio::play(Sound s, float distance, int weapon) {
  if (device) SDL_LockAudioDevice(device);
  Voice *v = nullptr;
  for (auto &vv : voices)
    if (vv.remaining <= 0) {
      v = &vv;
      break;
    }
  if (!v) {
    if (device) SDL_UnlockAudioDevice(device);
    return;
  }
  *v = {0, 300, 0, .22f, .1f, .1f, 0, .35f};
  switch (s) {
  case Sound::Reload: case Sound::Empty: case Sound::Discovery: break;
  case Sound::Shot:
    v->freq = 760;
    v->slide = -5200;
    v->remaining = .075f;
    v->wave = 3;
    v->volume = .2f;
    v->noiseMix = .2f;
    break;
  case Sound::Heavy:
    v->freq = 250;
    v->slide = -1250;
    v->remaining = .11f;
    v->wave = 3;
    v->volume = .21f;
    v->noiseMix = .26f;
    break;
  case Sound::Shotgun:
    v->freq = 180;
    v->remaining = .15f;
    v->wave = 3;
    v->volume = .3f;
    v->noiseMix = .56f;
    break;
  case Sound::Rocket:
    v->freq = 110;
    v->remaining = .25f;
    v->slide = 100;
    v->wave = 3;
    v->volume = .33f;
    v->noiseMix = .36f;
    break;
  case Sound::Flame:
    v->freq = 95;
    v->slide = 340;
    v->remaining = .18f;
    v->wave = 3;
    v->volume = .24f;
    v->noiseMix = .34f;
    break;
  case Sound::Laser:
    v->freq = 920;
    v->slide = -2400;
    v->remaining = .12f;
    v->wave = 3;
    v->volume = .2f;
    v->noiseMix = .015f;
    break;
  case Sound::Grenade:
    v->freq = 520;
    v->slide = -850;
    v->remaining = .13f;
    v->wave = 3;
    v->volume = .2f;
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
    v->wave = 3;
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
    v->wave = 3;
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
    v->wave = 3;
    v->volume = .08f;
    v->noiseMix = .46f;
    break;
  case Sound::MetalStep:
    v->freq = 430; v->slide = -900; v->remaining = .07f;
    v->wave = 3; v->volume = .055f; v->noiseMix = .18f;
    break;
  case Sound::WaterStep:
    v->freq = 175; v->slide = -480; v->remaining = .09f;
    v->wave = 3; v->volume = .065f; v->noiseMix = .82f;
    break;
  case Sound::Stomp:
    v->freq = 88;
    v->slide = -230;
    v->remaining = .19f;
    v->wave = 2;
    v->noiseMix = .18f;
    v->volume = .22f;
    break;
  case Sound::BodyLand: {
    const auto &c=tuning::guardImpact;
    v->freq=c.landingFrequency;v->slide=c.landingPitchDrop;v->remaining=c.landingDuration;
    v->wave=2;v->noiseMix=c.landingNoise;
    v->event=4;
    float d=std::max(0.f,distance)/tuning::audio.falloff;
    v->volume=c.landingGain/(1+d*d);
    break;
  }
  }
  if (int(s) <= int(Sound::Laser) || s == Sound::Reload || s == Sound::Empty || s == Sound::Hit) {
    int id=std::clamp(weapon < 0 ? (int(s)<=int(Sound::Laser)?int(s):0) : weapon,0,5);
    const auto &w=tuning::weapons[id];
    v->model=id; v->freq=w.frequency; v->slide=w.pitchDrop;
    v->volume=w.gain; v->remaining=w.fireDuration+tuning::audio.tail;
    v->noiseMix=w.noise; v->filter=w.lowpass; v->decay=w.decay;
    v->distance=std::max(0.f,distance)/tuning::audio.falloff;
    v->volume /= 1+v->distance*v->distance;
    if (s==Sound::Reload) {
      v->event=1; v->freq=w.reloadPitch; v->slide=0;
      v->remaining=w.reload; v->volume=tuning::audio.reloadGain;
    } else if (s==Sound::Empty) {
      v->event=2; v->freq=w.reloadPitch*1.5f; v->slide=0;
      v->remaining=tuning::audio.emptyDuration; v->volume=tuning::audio.emptyGain;
    } else if (s==Sound::Hit) {
      v->event=3;v->remaining=tuning::combat.impactLife;v->volume=tuning::audio.impactGain/(1+v->distance*v->distance);
    }
  }
  if (s==Sound::Discovery) {v->freq=tuning::audio.discoveryFrequency;v->slide=tuning::audio.discoveryPitchRise;v->remaining=tuning::audio.discoveryDuration;v->volume=tuning::audio.discoveryGain;v->wave=0;}
  v->total = v->remaining;
  if (device) SDL_UnlockAudioDevice(device);
}
void Audio::callback(void *user, Uint8 *data, int len) {
  static_cast<Audio *>(user)->mix(reinterpret_cast<int16_t *>(data), len / 2);
}
void Audio::renderOffline(int16_t *output,int count) {
  if(device)throw std::logic_error("Offline audio requires a closed output device");
  if(count<0 || (!output && count))throw std::invalid_argument("Invalid offline audio buffer");
  mix(output,count);
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
    if (!paused && !muted && musicEnabled) {
      int step = int(sample / stepSamples) % 16;
      float f = float(std::fmod(double(sample), double(stepSamples)) / stepSamples),
            t = float(sample) / sampleRate;
      int note = motifs[theme % 6][step];
      float transpose = boss ? 3.0f : 0.0f;
      float hz = 110 * std::pow(2.0f, (note + transpose) / 12.0f);
      float lead = (std::sin(6.283185f * hz * 2 * t) * .72f +
                    std::sin(6.283185f * hz * 4 * t) * .18f +
                    std::sin(6.283185f * hz * 1.005f * t) * .10f) *
                   std::exp(-f * 3.2f) * (boss ? .052f : .042f);
      int accentNote = motifs[theme % 6][(step + 5) % 16];
      float accentHz = 220 * std::pow(2.0f, (accentNote + transpose) / 12.0f);
      float accent = std::sin(6.283185f * accentHz * t) * std::exp(-f * 11.0f) *
                     (step % 4 == 1 ? .018f : .010f);
      int chordNote = motifs[theme % 6][(step / 4) * 4];
      float rootHz = 55 * std::pow(2.0f, (chordNote + transpose) / 12.0f);
      float thirdHz = rootHz * (theme % 2 ? 1.1892f : 1.2599f);
      float fifthHz = rootHz * 1.4983f;
      float padGate = .72f + .28f * std::sin(6.283185f * t / 4.0f);
      float pad = (std::sin(6.283185f * rootHz * t) * .014f +
                   std::sin(6.283185f * thirdHz * t) * .010f +
                   std::sin(6.283185f * fifthHz * t) * .008f) * padGate;
      float bassPhase = std::fmod(rootHz * .5f * t, 1.0f);
      float bassWave = 4.0f * std::fabs(bassPhase - .5f) - 1.0f;
      float bass = bassWave * (boss ? .062f : .048f);
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
      float duck=1;
      for (const auto &v:voices) if(v.model>=0 && v.remaining>0 && v.event==0) {duck=tuning::audio.duck;break;}
      sum = (lead + accent + pad + bass + kick + snare + hat + ghost + tom + alarm)*tuning::audio.musicGain*duck;
      sample++;
    }
    for (auto &v : voices)
      if (v.remaining > 0 && !paused) {
        v.remaining -= dt;
        v.freq = std::max(20.0f, v.freq + v.slide * dt);
        v.phase += v.freq * dt;
        v.phase -= std::floor(v.phase);
        float osc = v.wave == 2   ? n * v.noiseMix + std::sin(v.phase * 6.283185f) * (1.0f - v.noiseMix)
                    : v.wave == 3 ? ((4.0f * std::fabs(v.phase - .5f) - 1.0f) * .72f +
                                     std::sin(v.phase * 12.56637f) * .28f)*(1-v.noiseMix)+n*v.noiseMix
                    : v.wave == 1 ? (v.phase < .5f ? .6f : -.6f)
                                  : std::sin(v.phase * 6.283185f);
        if (v.model>=0) {
          v.age += dt;
          float filter=std::max(tuning::audio.distantLowpass,v.filter/(1+v.distance));
          v.low += filter*(n-v.low);
          float body=std::sin(v.phase*6.283185f);
          float crack=n-v.low;
          float envelope=std::exp(-v.age*v.decay);
          float tailAge=std::max(0.f,v.age-tuning::audio.tailDelay*(1+std::min(2.f,v.distance)));
          if (v.event==1) {
            // Latch, insertion and closure; gas valve / laser charge use a continuous body.
            float f=v.age/v.total;
            float tick=std::exp(-f*70)+std::exp(-std::fabs(f-tuning::audio.reloadLatch)*100)+std::exp(-std::fabs(f-tuning::audio.reloadClose)*120);
            osc=(body*.32f+crack*.68f)*tick;
            if(v.model>=4) osc+=body*std::sin(f*3.14159f)*.2f;
          } else if (v.event==2) osc=(body*.25f+crack*.75f)*std::exp(-v.age*tuning::audio.clickDecay);
          else if(v.event==3) osc=(v.low*.8f+body*.2f)*std::exp(-v.age*tuning::audio.impactDecay);
          else {
            osc=(v.low*v.noiseMix+body*(1-v.noiseMix))*envelope;
            osc+=crack*std::exp(-v.age*tuning::audio.crackDecay)*(.75f/(1+v.distance));
            if(v.age>tuning::audio.tailDelay*(1+std::min(2.f,v.distance)))
              osc+=v.low*std::exp(-tailAge*tuning::audio.tailDecay)*tuning::audio.tailGain;
            if(v.model==4) osc=v.low*(.65f+.35f*std::sin(v.age*tuning::audio.flamePulse))*std::exp(-v.age*tuning::audio.flameDecay);
            if(v.model==5) osc=(body*.8f+std::sin(v.phase*18.84955f)*.2f)*envelope;
          }
        }
        // A short attack and release prevents zipper clicks while retaining
        // the sharp arcade transients at the start of each effect.
        float age = std::max(0.0f, v.total - v.remaining);
        if(v.event==4)osc*=std::exp(-age*tuning::guardImpact.landingDecay);
        float attack = std::min(1.0f, age / tuning::audio.attack);
        float releaseWindow = std::min(tuning::audio.release, v.total * .45f);
        float release = std::min(1.0f, std::max(0.0f, v.remaining) / releaseWindow);
        float env = attack * release;
        sum += osc * v.volume * env;
      }
    out[i] = muted || paused ? 0 : int16_t(std::max(-.95f, std::min(.95f, sum)) * 32760);
  }
}
} // namespace kh
