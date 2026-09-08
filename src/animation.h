#pragma once
#include "game.h"
#include "boss_muzzles.h"
#include <algorithm>
#include <cmath>

namespace kh {
inline float between(float previous, float current, float alpha) {
  return previous + (current - previous) * std::clamp(alpha, 0.0f, 1.0f);
}
inline float ease(float t) {
  t = std::clamp(t, 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}
struct BossPose {
  float lift = 0, lean = 0, stride = 0, weapon = 0, kick = 0, charge = 0, core = 0, collapse = 0;
};
struct MuzzlePoint {
  float x = 0, y = 0;
  bool vertical = false;
};

// Keep the muzzle flash, local light and projectile origin on one authored
// anchor.  Previously the renderer used a hard-coded horizontal offset while
// gameplay fired from a different point, which made a flash appear beside an
// up/down pose and made the light detach from the weapon.
inline MuzzlePoint muzzlePoint(const Player &p, const Input &input) {
  if (p.vehicleHP)
    return {p.x + p.dir * 22.0f, p.y - 42.0f, false};
  if (input.up)
    return {p.x + p.dir * (std::fabs(p.vx) > 1 ? 6.0f : 0.0f),
            p.y - (std::fabs(p.vx) > 1 ? 44.0f : 48.0f), true};
  if (input.down && !p.grounded)
    return {p.x + p.dir * 3.0f, p.y, true};
  return {p.x + p.dir * 19.0f, p.y - (p.crouch ? 14.0f : 27.0f), false};
}

inline int workerFrame(bool rescued, float age) {
  age = std::max(0.0f, age);
  if (!rescued) return 40 + int(age * 2) % 2;
  if (age < .2f) return 42;
  if (age < .4f) return 43;
  if (age < .85f) return 44 + int((age - .4f) * 8) % 2;
  return 46 + int((age - .85f) * 10) % 2;
}

inline MuzzlePoint bossMuzzlePoint(const Boss &boss, int kind, float alpha = 1) {
  return {between(boss.prevX, boss.x, alpha) + BOSS_MUZZLES[kind][0],
          between(boss.prevY, boss.y, alpha) + BOSS_MUZZLES[kind][1], false};
}

inline int bossFrame(const Boss &b, int kind, float alpha = 1) {
  float lag = DT * (1 - std::clamp(alpha, 0.0f, 1.0f));
  float elapsed = std::max(0.0f, b.stateAge - lag);
  float progress = std::clamp(elapsed / std::max(.01f, b.duration), 0.0f, 1.0f);
  if (b.dead)
    return 12 + std::clamp(int((1 - (b.death + lag) / 3.2f) * 4), 0, 3);
  if (b.state == BossState::Windup)
    return 4 + std::min(2, int(progress * 3));
  if (b.state == BossState::Attack)
    return elapsed + .00001f < ((kind == 0 || kind == 3) ? .18f : .06f) ? 6 : 7;
  if (b.state == BossState::Recover)
    return 8 + std::min(3, int(progress * 4));
  if (b.state == BossState::Overload) return 10;
  float cycle = kind == 4 ? std::max(0.0f, b.age - lag) * 3
                         : std::max(0.0f, b.gait - std::fabs(b.vx) * lag / 54);
  return int(cycle * 4) % 4;
}

// Animation is derived from combat time, never from the render frame counter.
// This same pose drives all pieces of the rig at any display refresh rate.
inline BossPose bossPose(const Boss &b, int kind, float alpha = 1) {
  constexpr float tau = 6.2831853f;
  BossPose p;
  float lag = DT * (1 - std::clamp(alpha, 0.0f, 1.0f));
  float age = std::max(0.0f, b.age - lag);
  float gait = b.gait - std::fabs(b.vx) * lag / 54.0f;
  float walking = std::min(1.0f, std::fabs(b.vx) / 35.0f);
  p.stride = std::sin(gait * tau) * walking;
  p.lift = -std::fabs(std::sin(gait * tau)) * walking * 3;
  p.lean = std::clamp(b.vx / 75.0f, -1.0f, 1.0f) * 2;
  p.kick = std::clamp(b.recoil / .18f, 0.0f, 1.0f);
  p.lift += b.impact * 6;
  float progress = ease((b.stateAge - lag) / std::max(.01f, b.duration));
  const float anticipation = kind == 3 ? 32.0f : kind == 0 ? 16.0f : 8.0f;
  if (b.state == BossState::Windup) {
    p.charge = progress;
    p.weapon = -progress * anticipation;
    p.lift += progress * 3;
  } else if (b.state == BossState::Overload) {
    p.charge = progress;
  } else if (b.state == BossState::Attack) {
    float contact = kind == 3 ? 15.0f : 5.0f;
    float elapsed = std::max(0.0f, b.stateAge - lag);
    p.weapon = elapsed < .18f
                   ? -anticipation + (anticipation + contact) * ease(elapsed / .18f)
                   : contact * (1 - ease((elapsed - .18f) / std::max(.01f, b.duration - .18f)));
    p.lift += 3 * (1 - ease(elapsed / .18f));
    p.lean -= p.kick * 3;
  } else if (b.state == BossState::Recover) {
    p.core = std::sin(progress * 3.141593f);
  }
  if (kind == 4) {
    p.lean = std::sin(age * 1.7f) * 4;
    p.stride = std::sin(age * 4) * .7f;
  }
  if (b.dead) {
    p.collapse = ease((3.2f - b.death) / 3.2f);
    p.lift = p.collapse * 25;
    p.lean = p.collapse * 12;
    p.stride = std::sin(age * 18) * (1 - p.collapse);
    p.weapon = p.collapse * 35;
    p.charge = p.core = 0;
  }
  return p;
}
} // namespace kh
