#pragma once
#include "game.h"
#include "boss_muzzles.h"
#include "presentation_config.h"
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
inline Player interpolatedPlayer(const Player &source,float alpha) {
  Player p=source;
  p.x=between(source.prevX,source.x,alpha);
  p.y=between(source.prevY,source.y,alpha);
  p.stride=between(source.prevStride,source.stride,alpha);
  p.anim=between(source.prevAnim,source.anim,alpha);
  p.climbCycle=between(source.prevClimbCycle,source.climbCycle,alpha);
  // A new shot is a discontinuity; do not blend from an old idle fire age.
  p.fireAge=source.fireAge<source.prevFireAge?source.fireAge:between(source.prevFireAge,source.fireAge,alpha);
  return p;
}
struct BossPose {
  float lift = 0, lean = 0, stride = 0, weapon = 0, kick = 0, charge = 0, core = 0, collapse = 0;
};
struct MuzzlePoint {
  float x = 0, y = 0;
  bool vertical = false;
};
inline Enemy interpolatedEnemy(const Enemy &source,float alpha) {
  Enemy e=source;
  e.x=between(source.prevX,source.x,alpha);e.y=between(source.prevY,source.y,alpha);
  e.gait=between(source.prevGait,source.gait,alpha);
  e.fireAge=source.fireAge<source.prevFireAge?source.fireAge:between(source.prevFireAge,source.fireAge,alpha);
  return e;
}
inline int guardFirePhase(const Enemy &e) {
  for(int i=0;i<4;++i)if(e.fireAge<tuning::guardFireAnchors[i].end)return i;
  return 3;
}
inline float guardPoseX(const Enemy &e,int frame) {
  const auto &rig=tuning::guardAction;
  return e.x+e.dir*(tuning::guardPoseRoots[std::clamp(frame,0,15)].x-rig.cellSize/2)*rig.renderSize/rig.cellSize;
}
inline MuzzlePoint guardMuzzle(const Enemy &e) {
  const auto &rig=tuning::guardAction;int phase=guardFirePhase(e);
  const auto &tip=tuning::guardFireAnchors[phase];float scale=rig.renderSize/rig.cellSize;
  float root=tuning::guardPoseRoots[12+phase].x;
  return {e.x+e.dir*(root-tip.x)*scale,e.y-(rig.baseline-tip.y)*scale,false};
}
inline bool guardFlashVisible(const Enemy &e) {
  return e.active && !e.dead && e.flinch<=0 && e.state==2 && e.fireAge<tuning::guardAction.flashDuration;
}
inline float enemyDeathElapsed(const Enemy &e) {
  return std::max(0.f,(e.deathDuration>0?e.deathDuration:tuning::combat.deathDuration)-e.death);
}
inline int guardReactionFrame(const Enemy &e) {
  if(e.dead) {
    float t=enemyDeathElapsed(e)/tuning::guardReactions.collapseDuration;
    for(const auto &pose:tuning::guardDeathTimeline)if(t<pose.end)return int(pose.frame);
    return int(tuning::guardDeathTimeline.back().frame);
  }
  float t=1-e.flinch/std::max(.001f,e.flinchDuration);
  int start=std::clamp(int(e.hitZone),0,2)*4;
  for(int i=0;i<4;++i)if(t<tuning::guardHitTimeline[start+i].end)return int(tuning::guardHitTimeline[start+i].frame);
  return int(tuning::guardHitTimeline[start+3].frame);
}
inline float guardCorpseOpacity(const Enemy &e) {
  if(!e.dead)return 1;
  const auto &rig=tuning::guardReactions;
  return std::clamp(1-(enemyDeathElapsed(e)-rig.collapseDuration-rig.corpseHold)/rig.fadeDuration,0.f,1.f);
}
inline bool guardReactionFlip(const Enemy &e) {return e.dead?e.hitDir<0:e.dir>0;}
inline bool useHeroLocomotion(const Player &p) {
  return tuning::heroLocomotion.enabled>0 && p.presentationScale>1.f &&
         p.grounded && !p.crouch && !p.vehicleHP && p.ladder<0 && p.climbTransition<=0 &&
         p.action<=0 && std::fabs(p.vx)>1;
}
inline int heroLocomotionPhase(const Player &p) {
  return int(p.stride*8)%8;
}
inline bool useHeroAirFire(const Player &p,const Input &input) {
  return tuning::heroLocomotion.enabled>0 && p.presentationScale>1.f &&
         !p.grounded && !p.vehicleHP && p.ladder<0 && p.climbTransition<=0 &&
         p.action<=0 && !input.up && !input.down && (input.shoot || p.shot>0);
}
inline int heroFirePhase(const Player &p) {
  return p.grounded?heroLocomotionPhase(p):(p.vy<0?3:7);
}

// Keep the muzzle flash, local light and projectile origin on one authored
// anchor.  Previously the renderer used a hard-coded horizontal offset while
// gameplay fired from a different point, which made a flash appear beside an
// up/down pose and made the light detach from the weapon.
inline MuzzlePoint muzzlePoint(const Player &p, const Input &input) {
  if (p.vehicleHP)
    return {p.x + p.dir * 22.0f, p.y - 42.0f, false};
  const float scale=p.presentationScale;
  if (input.up)
    return {p.x + p.dir * (std::fabs(p.vx) > 1 ? 6.0f : 0.0f)*scale,
            p.y - (std::fabs(p.vx) > 1 ? 44.0f : 48.0f)*scale, true};
  if (input.down && !p.grounded)
    return {p.x + p.dir * 3.0f*scale, p.y, true};
  if(useHeroLocomotion(p) || useHeroAirFire(p,input)) {
    const auto &rig=tuning::heroLocomotion;
    const auto &m=tuning::heroRunMuzzles[heroFirePhase(p)];
    float factor=rig.sourceScale*rig.renderSize/rig.cellSize;
    return {p.x+p.dir*m.x*factor,p.y-m.height*factor,false};
  }
  return {p.x + p.dir * 19.0f*scale, p.y - (p.crouch ? 14.0f : 27.0f)*scale, false};
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
  if(kind==3)return {between(boss.prevX,boss.x,alpha)+tuning::forgeTitan.muzzleX,
                    between(boss.prevY,boss.y,alpha)+tuning::forgeTitan.muzzleY,false};
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

inline int forgeTitanFrame(const Boss &b,float alpha=1) {
  return int(tuning::forgeTitanPoses[bossFrame(b,3,alpha)].frame);
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
