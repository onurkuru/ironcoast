#include "game.h"
#include "animation.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace kh {
static float clamp(float x, float a, float b) { return std::max(a, std::min(x, b)); }
bool overlap(Rect a, Rect b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}
bool segmentRect(float ax, float ay, float bx, float by, Rect r) {
  float lo = 0, hi = 1, dx = bx - ax, dy = by - ay;
  auto slab = [&](float p, float d, float mn, float mx) {
    if (std::fabs(d) < .00001f)
      return p >= mn && p <= mx;
    float a = (mn - p) / d, b = (mx - p) / d;
    if (a > b)
      std::swap(a, b);
    lo = std::max(lo, a);
    hi = std::min(hi, b);
    return lo <= hi;
  };
  return slab(ax, dx, r.x, r.x + r.w) && slab(ay, dy, r.y, r.y + r.h);
}
const Level &Game::level() const { return campaign().at(levelIndex); }
float Game::random() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  return float(randomState & 0xffffff) / float(0x1000000);
}
Rect Game::playerBox() const {
  float h = player.vehicleHP ? 44 : (player.crouch ? 19 : 34), w = player.vehicleHP ? 43 : 18;
  return {player.x - w / 2, player.y - h, w, h};
}
Rect Game::enemyBox(const Enemy &e) const {
  float h = e.kind == 3 ? 26 : (e.kind == 4 ? 27 : 35),
        w = e.kind == 3 ? 30 : (e.kind == 2 ? 25 : 20);
  return {e.x - w / 2, e.y - h, w, h};
}
Rect Game::bossBox() const { return {boss.x - 57, boss.y - 102, 114, 99}; }
float Game::floorAt(float x, float fromY) const {
  float y = 999;
  for (auto &p : level().platforms)
    if (x >= p.box.x && x <= p.box.x + p.box.w && p.box.y >= fromY)
      y = std::min(y, p.box.y);
  return y;
}
bool Game::hazardOn(const Hazard &h) const {
  return h.period <= 0 || std::fmod(time + h.offset, h.period) < h.on;
}
void Game::load(int index, bool keepScore, float startX) {
  const int oldScore = score, oldTotal = totalRescued, oldContinues = continues,
            oldLives = player.lives;
  *this = Game{};
  levelIndex = std::max(0, std::min(index, int(campaign().size() - 1)));
  if (keepScore) {
    score = oldScore;
    totalRescued = oldTotal;
    continues = oldContinues;
    player.lives = std::max(1, oldLives);
  }
  randomState = 12345 + levelIndex * 891;
  player.x = startX;
  player.y = floorAt(startX);
  if (player.y > 500)
    player.y = 232;
  player.prevX = player.x;
  player.prevY = player.y;
  player.inv = 1.5f;
  checkpoint = startX;
  camera = clamp(startX - 120, 0, level().width - W);
  vehicleX = level().vehicleX;
  vehicleAvailable = vehicleX > 0;
  for (auto &s : level().spawns) {
    Enemy e;
    e.x = e.origin = s.x;
    e.y = e.baseY = s.y;
    e.kind = s.kind;
    e.hp = e.maxhp = (s.kind == 0 ? 2 : s.kind == 1 ? 3 : s.kind == 2 ? 7 : s.kind == 3 ? 3 : 9);
    e.timer = .4f + random();
    if (s.x < startX - 70) {
      e.dead = true;
    }
    enemies.push_back(e);
  }
  for (auto &s : level().items) {
    if (s.kind == 7 || s.kind == 8)
      props.push_back({s.x, s.y, s.kind, s.kind == 7 ? 3 : 2, false});
    else
      items.push_back({s.x, s.y, s.kind, s.x < startX - 50, 0});
  }
  boss.x = level().width - 90;
  boss.y = levelIndex == 4 ? 190 : 232;
  boss.moveX = level().width - 195;
  boss.moveY = levelIndex == 4 ? 169 : 232;
  boss.hp = boss.maxhp = 100 + levelIndex * 26;
  boss.timer = boss.duration = 1.5f;
  syncPresentation();
}
void Game::syncPresentation() {
  player.prevX = player.x;
  player.prevY = player.y;
  boss.prevX = boss.x;
  boss.prevY = boss.y;
  prevCamera = camera;
  prevTime = time;
  for (auto &e : enemies) {
    e.prevX = e.x;
    e.prevY = e.y;
  }
  for (auto &p : particles) {
    p.prevX = p.x;
    p.prevY = p.y;
  }
  for (auto &b : bullets) {
    b.px = b.x;
    b.py = b.y;
  }
}
void Game::retry() {
  int c = continues + 1;
  load(levelIndex, true, checkpoint);
  player.lives = 3;
  continues = c;
}
void Game::fire(float x, float y, float vx, float vy, float damage, int kind, bool hostile,
                float life) {
  for (auto &b : bullets)
    if (!b.alive) {
      b = {x,      y,    x,       y,   vx, vy, life, kind == 2 || kind == 3 ? 4.0f : 2.0f,
           damage, kind, hostile, true};
      return;
    }
}
void Game::burst(float x, float y, int kind, int count, float power) {
  for (int i = 0; i < count; i++)
    for (auto &p : particles)
      if (p.life <= 0) {
        float a = random() * 6.28318f, s = (25 + random() * 80) * power;
        p = {x,
             y,
             std::cos(a) * s,
             std::sin(a) * s - 25,
             .25f + random() * .6f,
             0,
             1 + random() * 3,
             kind,
             0};
        if (kind == 1) {
          p.vx *= .12f;
          p.vy = -15 - random() * 10;
          p.life = .6f + random() * .5f;
          p.size = 5 + random() * 8;
        }
        if (kind == 3) {
          p.life = .52f;
          p.size = power;
          p.vx = p.vy = 0;
        }
        p.maxlife = p.life;
        p.prevX = p.x;
        p.prevY = p.y;
        break;
      }
}
void Game::damageEnemy(Enemy &e, float amount, bool explosive, int approach) {
  if (e.dead || !e.active)
    return;
  if (e.kind == 2 && !explosive && e.state != 2 && approach == -e.dir) {
    burst(e.x, e.y - 22, 0, 3);
    sounds.push_back(Sound::Hit);
    return;
  }
  e.hp -= int(std::ceil(amount));
  e.hurt = .09f;
  burst(e.x, e.y - 22, 0, 3);
  if (e.hp <= 0) {
    e.dead = true;
    e.death = .45f;
    score += e.kind == 4 ? 200 : 100;
    kills++;
    burst(e.x, e.y - 18, 0, 10);
    if (e.kind >= 3)
      burst(e.x, e.y - 16, 3, 1, 45);
    sounds.push_back(Sound::Hit);
  }
}
void Game::damageBoss(float amount) {
  if (!boss.active || boss.dead)
    return;
  // Armor reduces damage outside recovery, but the pistol always remains viable.
  boss.hp -= amount * (boss.state == BossState::Recover ? 1.0f : .4f);
  boss.hurt = .08f;
  if (boss.hp <= 0) {
    boss.hp = 0;
    boss.dead = true;
    boss.death = 3.2f;
    score += 2500;
    totalRescued += rescued;
    for (auto &b : bullets)
      if (b.hostile)
        b.alive = false;
    sounds.push_back(Sound::Boss);
  }
}
void Game::explosion(float x, float y, float radius, float damage, bool hostile) {
  burst(x, y, 3, 1, radius * 1.55f);
  burst(x, y, 0, 18, 1.5f);
  burst(x, y, 1, 5);
  shake = std::max(shake, 3.0f);
  sounds.push_back(Sound::Blast);
  if (hostile) {
    if (std::hypot(player.x - x, player.y - 18 - y) < radius)
      hitPlayer();
  } else {
    for (auto &e : enemies)
      if (!e.dead && e.active && std::hypot(e.x - x, e.y - 18 - y) < radius + 14)
        damageEnemy(e, damage, true);
    if (boss.active && !boss.dead && std::hypot(boss.x - x, boss.y - 50 - y) < radius + 65)
      damageBoss(damage);
    for (auto &p : props)
      if (!p.dead && std::hypot(p.x - x, p.y - 12 - y) < radius) {
        p.hp = 0;
      }
  }
}
void Game::hitPlayer() {
  if (player.inv > 0 || debugInvincible || status != Status::Play)
    return;
  if (player.vehicleHP > 0) {
    player.vehicleHP--;
    player.inv = 1.1f;
    shake = 4;
    burst(player.x, player.y - 20, 0, 20);
    sounds.push_back(Sound::Hurt);
    if (player.vehicleHP == 0) {
      explosion(player.x, player.y - 20, 42, 5);
      player.vy = -220;
      vehicleAvailable = false;
    }
    return;
  }
  player.health = std::max(0, player.health - 1);
  player.inv = .82f;
  player.hitFlash = .16f;
  player.vx = -player.dir * 78;
  player.vy = -92;
  shake = std::max(shake, 2.5f);
  burst(player.x, player.y - 22, 0, 9, .75f);
  sounds.push_back(Sound::Hurt);
  // A short damage window gives the player room to read the hit and recover.
  if (player.health > 0)
    return;
  status = Status::Dying;
  deathTimer = 1.0f;
  player.lives--;
  player.action = 0;
  rescued = 0;
  player.vx = -player.dir * 70;
  player.vy = -125;
  shake = 3;
}
void Game::fireBossVolley() {
  const int k = level().bossKind, phase = boss.phase, volley = boss.volleys++;
  float ox = boss.x - 48, oy = boss.y - 51;
  auto aimed = [&](float speed, float spread = 0) {
    float a = std::atan2(player.y - 18 - oy, player.x - ox) + spread;
    fire(ox, oy, std::cos(a) * speed, std::sin(a) * speed, 1, 4, true, 5);
  };
  if (k == 0) {
    if (boss.pattern % 2 == 0) {
      for (int i = 0; i < 3; i++)
        aimed(105 + phase * 8, (i - 1) * .15f);
    } else if (volley == 0) {
      for (int i = 0; i < phase + 1; i++)
        fire(boss.targetX - 20 + i * 40, 42, 0, 155, 1, 6, true, 3);
    }
  } else if (k == 1) {
    if (boss.pattern % 2 == 0)
      fire(ox, oy, -100 - volley * 24, -185, 1, 3, true, 1.2f + volley * .12f);
    else if (volley == 0)
      fire(ox, 216, -150, 0, 1, 4, true, 4);
  } else if (k == 2) {
    for (int i = 0; i < 3; i++)
      aimed(126, (i - 1) * .16f + (volley % 2 ? .07f : -.07f));
  } else if (k == 3) {
    if (boss.pattern % 2 == 0 && volley == 0) {
      for (int i = 0; i < phase + 2; i++)
        fire(boss.targetX - 45 + i * 40, 40, 0, 175, 1, 6, true, 2);
    } else if (boss.pattern % 2 == 1) {
      fire(ox, 214 - volley * 12, -120, 0, 1, 4, true, 4);
    }
  } else if (k == 4) {
    for (int i = 0; i < 8 + phase * 2; i++) {
      float a = 6.283185f * i / (8 + phase * 2) + boss.pattern * .18f + volley * .22f;
      fire(boss.x, boss.y - 50, std::cos(a) * 86, std::sin(a) * 86, 1, 5, true, 4);
    }
  } else {
    for (int i = 0; i < 3; i++)
      aimed(120, (i - 1) * .14f);
    if (boss.pattern % 2 == 1 && volley == 0)
      for (int i = 0; i < 3; i++)
        fire(boss.targetX - 40 + i * 40, 24, 0, 160, 1, 6, true, 2);
  }
  boss.recoil = .18f;
  burst(ox, oy, 2, 3);
  shake = std::max(shake, 1.2f);
  sounds.push_back(k == 4 ? Sound::Laser : Sound::Heavy);
}
void Game::updateBoss(float dt) {
  if (!boss.active)
    return;
  const int k = level().bossKind;
  boss.age += dt;
  boss.stateAge += dt;
  boss.hurt = std::max(0.0f, boss.hurt - dt);
  boss.recoil = std::max(0.0f, boss.recoil - dt);
  boss.impact = std::max(0.0f, boss.impact - dt * 4);
  if (boss.dead) {
    boss.vx *= std::max(0.0f, 1 - dt * 8);
    boss.death -= dt;
    if (int(boss.death * 12) != int((boss.death + dt) * 12) && boss.death > 0) {
      burst(boss.x + (random() - .5f) * 100, boss.y - random() * 95, 3, 1, 45 + random() * 50);
      shake = 4;
      if (int(boss.death * 12) % 4 == 0)
        sounds.push_back(Sound::Blast);
    }
    if (boss.death <= 0 && status == Status::Play) {
      status = Status::Clear;
      clearTimer = 0;
      score += rescued * 500;
    }
    return;
  }
  auto enter = [&](BossState state, float duration) {
    boss.state = state;
    boss.stateAge = 0;
    boss.timer = boss.duration = duration;
    boss.volleys = 0;
    boss.shotTimer = 0;
  };
  if (boss.hp < boss.maxhp * .5f && boss.phase == 1) {
    boss.phase = 2;
    enter(BossState::Overload, 1.15f);
    flash = .12f;
    shake = 4;
    burst(boss.x, boss.y - 55, 0, 18);
    sounds.push_back(Sound::Boss);
    for (auto &b : bullets)
      if (b.hostile)
        b.alive = false;
  }

  // Six locomotion profiles. All movement is acceleration-limited; attacks
  // never teleport the boss and each arena retains a safe corridor at left.
  const float minX = level().width - 305, maxX = level().width - 88;
  float targetX = boss.moveX, targetY = boss.moveY;
  bool moving = boss.state == BossState::Move || boss.state == BossState::Enter;
  float maxSpeed = k == 1 ? 72 : k == 2 ? 52 : k == 4 ? 90 : 62;
  float acceleration = k == 4 ? 130 : 200;
  if (boss.phase == 2)
    maxSpeed *= 1.24f;
  if (k == 4 && boss.state != BossState::Enter) {
    targetX = level().width - 198 + std::sin(boss.age * .82f) * 82;
    targetY = 174 + std::sin(boss.age * 1.64f) * 25;
    moving = true;
  }
  bool charge = k == 1 && boss.pattern % 2 == 1 && boss.state == BossState::Attack;
  if (charge) {
    targetX = minX + 8;
    maxSpeed = boss.phase == 2 ? 172 : 148;
    acceleration = 360;
    moving = true;
  }
  float desiredVX = moving ? clamp((targetX - boss.x) * 2.8f, -maxSpeed, maxSpeed) : 0;
  boss.vx += clamp(desiredVX - boss.vx, -acceleration * dt, acceleration * dt);
  float oldX = boss.x;
  boss.x = clamp(boss.x + boss.vx * dt, minX, maxX);
  boss.vx = (boss.x - oldX) / dt;
  if (k == 4) {
    float desiredVY = clamp((targetY - boss.y) * 2.5f, -48, 48);
    boss.vy += clamp(desiredVY - boss.vy, -100 * dt, 100 * dt);
    boss.y = clamp(boss.y + boss.vy * dt, 142, 205);
  }
  float previousGait = boss.gait;
  boss.gait += std::fabs(boss.x - oldX) / 54.0f;
  bool walker = k == 0 || k == 3 || k == 5;
  if (walker && int(previousGait * 2) != int(boss.gait * 2)) {
    burst(boss.x + (int(boss.gait * 2) % 2 ? 35 : -35), boss.y, 1, 3, .5f);
    boss.impact = .22f;
    sounds.push_back(Sound::Stomp);
  }
  if (k == 1 && std::fabs(boss.vx) > 20 &&
      int(boss.age * 12) != int((boss.age - dt) * 12))
    burst(boss.x + 42, boss.y - 3, 1, 1, .4f);
  if (boss.state == BossState::Recover &&
      int(boss.age * 8) != int((boss.age - dt) * 8))
    burst(boss.x + 15, boss.y - 76, 1, 1, .45f);

  if (boss.state == BossState::Attack) {
    boss.shotTimer -= dt;
    // Hammer/crane impacts occur at the visible contact pose, after windup.
    if ((k == 0 || k == 3) && boss.stateAge >= .18f && boss.volleys == 0) {
      boss.impact = 1;
      shake = std::max(shake, 3.5f);
      burst(boss.x - 48, 232, 0, 12);
      burst(boss.x - 48, 231, 1, 6, .8f);
      sounds.push_back(Sound::Stomp);
    }
    const float firstShot = (k == 0 || k == 3) ? .18f : .06f;
    const bool singleImpact = ((k == 0 || k == 1) && boss.pattern % 2 == 1) ||
                              (k == 3 && boss.pattern % 2 == 0);
    const int maxVolleys = k == 4 ? 2 : singleImpact ? 1 : 2 + boss.phase;
    if (boss.stateAge >= firstShot && boss.shotTimer <= 0 && boss.volleys < maxVolleys) {
      fireBossVolley();
      boss.shotTimer = k == 4 ? .43f : .22f;
    }
    if (charge && overlap(playerBox(), {boss.x - 67, boss.y - 29, 125, 29}))
      hitPlayer();
  }
  boss.timer = std::max(0.0f, boss.duration - boss.stateAge);
  if (boss.timer > 0)
    return;
  if (boss.state == BossState::Move || boss.state == BossState::Enter ||
      boss.state == BossState::Overload) {
    boss.targetX = clamp(player.x, level().width - W + 35, level().width - 100);
    enter(BossState::Windup, boss.phase == 2 ? .72f : .92f);
    sounds.push_back(Sound::Boss);
  } else if (boss.state == BossState::Windup) {
    enter(BossState::Attack, k == 1 && boss.pattern % 2 ? .95f : .82f);
  } else if (boss.state == BossState::Attack) {
    enter(BossState::Recover, boss.phase == 2 ? 1.1f : 1.45f);
  } else {
    boss.pattern++;
    // Alternating destinations avoid endless leftward drift and guarantee
    // readable repositioning between attacks for every grounded boss.
    boss.moveX = level().width - (boss.pattern % 2 ? 278 : 128);
    boss.moveY = 174;
    enter(BossState::Move, boss.phase == 2 ? 1.55f : 1.9f);
  }
}
void Game::update(Input in, float dt) {
  sounds.clear();
  // Save the last fixed-step position so the renderer can interpolate between
  // 60 Hz simulation ticks when the desktop window is refreshed more often.
  syncPresentation();
  time += dt;
  shake = std::max(0.0f, shake - dt * 8);
  flash = std::max(0.0f, flash - dt);
  for (auto &p : particles)
    if (p.life > 0) {
      p.life -= dt;
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      if (p.kind == 0)
        p.vy += 180 * dt;
    }
  for (auto &e : enemies) {
    e.hurt = std::max(0.0f, e.hurt - dt);
    e.death = std::max(0.0f, e.death - dt);
  }
  if (status == Status::GameOver)
    return;
  if (status == Status::Clear) {
    clearTimer += dt;
    return;
  }
  if (status == Status::Dying) {
    deathTimer -= dt;
    player.x += player.vx * dt;
    player.y += player.vy * dt;
    player.vy += 450 * dt;
    if (deathTimer <= 0) {
      if (player.lives <= 0) {
        status = Status::GameOver;
      } else {
        status = Status::Play;
        player.x = checkpoint;
        player.y = floorAt(checkpoint);
        player.vx = player.vy = 0;
        player.inv = 2;
        player.health = player.maxHealth;
        player.hitFlash = 0;
        player.weapon = player.ammo = 0;
        player.grenades = 10;
        player.vehicleHP = 0;
        for (auto &b : bullets)
          if (b.hostile)
            b.alive = false;
        camera = clamp(checkpoint - 100, 0, level().width - W);
        syncPresentation();
      }
    }
    return;
  }
  player.inv = std::max(0.0f, player.inv - dt);
  player.shot = std::max(0.0f, player.shot - dt);
  player.action = std::max(0.0f, player.action - dt);
  player.land = std::max(0.0f, player.land - dt);
  player.recoil = std::max(0.0f, player.recoil - dt);
  player.hitFlash = std::max(0.0f, player.hitFlash - dt);
  player.fireAge += dt;
  const bool wasGrounded = player.grounded;
  player.crouch = in.down && player.grounded && player.vehicleHP == 0;
  player.vx = player.crouch ? 0 : in.move * (player.vehicleHP ? 125.0f : 145.0f);
  if (std::fabs(in.move) > .1f)
    player.dir = in.move > 0 ? 1 : -1;
  // Keep one continuous animation clock so stopping and starting never snaps
  // the sprite back to frame zero.
  player.anim += dt;
  if (in.jump && player.grounded) {
    player.vy = player.vehicleHP ? -320 : -320;
    player.grounded = false;
    burst(player.x, player.y, 1, 2, .3f);
    sounds.push_back(Sound::Jump);
  }
  float oldx = player.x;
  player.x += player.vx * dt;
  for (auto &p : level().platforms)
    if (!p.oneWay && overlap(playerBox(), p.box)) {
      float half = player.vehicleHP ? 21.5f : 9.0f;
      if (oldx <= p.box.x)
        player.x = p.box.x - half;
      else if (oldx >= p.box.x + p.box.w)
        player.x = p.box.x + p.box.w + half;
    }
  float oldy = player.y;
  player.vy += 1000 * dt;
  player.y += player.vy * dt;
  player.grounded = false;
  float hh = player.crouch ? 19 : 34;
  if (player.vehicleHP)
    hh = 44;
  for (auto &p : level().platforms)
    if (player.x + 8 > p.box.x && player.x - 8 < p.box.x + p.box.w) {
      if (player.vy >= 0 && oldy <= p.box.y + .2f && player.y >= p.box.y) {
        player.y = p.box.y;
        player.vy = 0;
        player.grounded = true;
      } else if (!p.oneWay && player.vy < 0 && oldy - hh >= p.box.y + p.box.h &&
                 player.y - hh < p.box.y + p.box.h) {
        player.y = p.box.y + p.box.h + hh;
        player.vy = 0;
      }
    }
  if (!wasGrounded && player.grounded) {
    player.land = .16f;
    burst(player.x, player.y, 1, 4, .45f);
  }
  player.x = clamp(player.x, 12, level().width - 18);
  if (player.grounded)
    player.stride += std::fabs(player.x - oldx) / 88.0f;
  if (player.y > H + 85) {
    player.inv = 0;
    if (debugInvincible) {
      player.x = checkpoint;
      player.y = floorAt(checkpoint);
      player.vy = 0;
    } else
      hitPlayer();
  }
  for (float cp : level().checkpoints)
    if (player.x >= cp && checkpoint < cp && !boss.active)
      checkpoint = cp;
  if (player.x > level().width - 510 && !boss.active) {
    boss.active = true;
    checkpoint = level().width - 440;
    sounds.push_back(Sound::Boss);
  }
  if (boss.active) {
    player.x = std::max(player.x, level().width - W + 12);
    camera = level().width - W;
    if (std::fabs(camera - prevCamera) > 32)
      prevCamera = camera;
  } else {
    float target = clamp(player.x - 155, 0, level().width - W);
    camera += (target - camera) * std::min(1.0f, dt * 7);
  }
  if (in.interact) {
    if (player.vehicleHP > 0) {
      float exitX = player.x - player.dir * 30;
      float ground = floorAt(exitX, player.y - 2);
      if (ground < player.y + 12) {
        vehicleX = player.x;
        vehicleAvailable = true;
        player.vehicleHP = 0;
        player.x = exitX;
        player.inv = .5f;
      }
    } else if (vehicleAvailable && std::fabs(player.x - vehicleX) < 48) {
      player.vehicleHP = 3;
      vehicleAvailable = false;
      sounds.push_back(Sound::Pickup);
    }
  }
  if (in.shoot && player.shot <= 0) {
    bool melee = false;
    if (!in.up && !player.vehicleHP)
      for (auto &e : enemies)
        if (!e.dead && e.active && e.kind < 3 && std::fabs(e.x - player.x) < 28 &&
            std::fabs(e.y - player.y) < 22) {
          damageEnemy(e, 4, true);
          melee = true;
          player.action = .42f;
          player.actionKind = 1;
          player.shot = .42f;
          player.recoil = .1f;
          burst(player.x + player.dir * 18, player.y - 20, 2, 5);
          break;
        }
    if (!melee) {
      player.fireAge = 0;
      int w = player.vehicleHP ? 1 : player.weapon;
      float vx = player.dir * 500.0f, vy = 0, ox = player.x + player.dir * 19,
            oy = player.y - (player.crouch ? 14 : 27);
      if (in.up) {
        vx = 0;
        vy = -500;
        ox = player.x + player.dir * 4;
        oy = player.y - 44;
      } else if (in.down && !player.grounded) {
        vx = 0;
        vy = 500;
        ox = player.x;
        oy = player.y + 2;
      }
      if (player.vehicleHP) {
        oy = player.y - 36;
        ox = player.x + player.dir * 31;
      }
      if (w == 0) {
        fire(ox, oy, vx, vy, 1, 0);
        player.shot = .18f;
        player.recoil = .08f;
        sounds.push_back(Sound::Shot);
      }
      if (w == 1) {
        fire(ox, oy, vx, vy, 1.1f, 1);
        player.shot = .082f;
        player.recoil = .1f;
        sounds.push_back(Sound::Heavy);
      }
      if (w == 2) {
        float a = std::atan2(vy, vx);
        for (int i = -2; i <= 2; i++)
          fire(ox, oy, std::cos(a + i * .065f) * 430, std::sin(a + i * .065f) * 430, 1.5f, 1, false,
               .34f);
        player.shot = .48f;
        player.recoil = .14f;
        sounds.push_back(Sound::Shotgun);
        shake = 1;
      }
      if (w == 3) {
        fire(ox, oy, vx * .5f, vy * .5f, 9, 2, false, 2.4f);
        player.shot = .7f;
        player.recoil = .16f;
        sounds.push_back(Sound::Rocket);
        shake = 1.5f;
      }
      if (w == 4) {
        // Flame Shot trades range for a forgiving, close-range damage cone.
        float a = std::atan2(vy, vx);
        for (int i = -1; i <= 1; i++)
          fire(ox, oy, std::cos(a + i * .105f) * 330, std::sin(a + i * .105f) * 330, 2.4f, 7,
               false, .42f);
        player.shot = .24f;
        player.recoil = .11f;
        sounds.push_back(Sound::Flame);
        shake = .5f;
      }
      if (w == 5) {
        // Laser is a fast precision shot for exposed boss cores and turrets.
        fire(ox, oy, vx * 1.9f, vy * 1.9f, 3.5f, 8, false, .24f);
        player.shot = .32f;
        player.recoil = .12f;
        sounds.push_back(Sound::Laser);
        shake = .8f;
      }
      burst(ox, oy, 2, 2);
      if (w > 0 && !player.vehicleHP) {
        player.ammo--;
        if (player.ammo <= 0)
          player.weapon = 0;
      }
    }
  }
  if (in.grenade && player.grenades > 0) {
    player.grenades--;
    player.action = .45f;
    player.actionKind = 2;
    player.recoil = .12f;
    float v = player.vehicleHP ? 250 : 155;
    fire(player.x + player.dir * 10, player.y - 26, player.dir * v, -220, 8, 3, false, .95f);
    sounds.push_back(Sound::Grenade);
  }
  for (auto &e : enemies) {
    if (e.dead) {
      continue;
    }
    if (!e.active && e.x < camera + W + 30 && e.x > camera - 70)
      e.active = true;
    if (!e.active || e.x < camera - 180 || e.x > camera + W + 130)
      continue;
    float distance = std::fabs(player.x - e.x);
    e.dir = player.x < e.x ? -1 : 1;
    e.timer -= dt;
    if (e.kind == 3) {
      e.y = e.baseY + std::sin(time * 2 + e.origin) * 9;
      if (distance > 120)
        e.x += e.dir * 22 * dt;
    } else {
      if (e.kind != 4 && e.state == 0 && distance > 110 && std::fabs(e.x - e.origin) < 90) {
        float nx = e.x + e.dir * (e.kind == 2 ? 18 : 26) * dt;
        if (floorAt(nx, e.y - 2) < e.y + 20)
          e.x = nx;
      }
      float floor = floorAt(e.x, e.y - 1);
      if (e.y < floor) {
        e.vy += 800 * dt;
        e.y = std::min(floor, e.y + e.vy * dt);
      } else
        e.vy = 0;
    }
    if (e.timer <= 0) {
      if (e.state == 0) {
        e.state = 1;
        e.timer = .5f;
      } else if (e.state == 1) {
        e.state = 2;
        e.timer = .3f;
        if (distance < 27 && e.kind < 3 && std::fabs(e.y - player.y) < 26)
          hitPlayer();
        else if (e.kind == 1)
          fire(e.x, e.y - 30, e.dir * (65 + distance * .25f), -205, 1, 3, true, 1.25f);
        else {
          float ox = e.x + e.dir * 15, oy = e.y - (e.kind == 4 ? 16 : 24),
                a = std::atan2(player.y - 20 - oy, player.x - ox);
          float speed = e.kind == 4 ? 135 : 100;
          fire(ox, oy, std::cos(a) * speed, std::sin(a) * speed, 1, 4, true, 4);
          if (e.kind == 4)
            fire(ox, oy, std::cos(a + .13f) * speed, std::sin(a + .13f) * speed, 1, 4, true, 4);
          burst(ox, oy, 2, 2);
        }
      } else {
        e.state = 0;
        e.timer = .9f + random() * .9f - (levelIndex * .06f);
      }
    }
  }
  for (auto &b : bullets) {
    if (!b.alive) {
      continue;
    }
    b.px = b.x;
    b.py = b.y;
    b.life -= dt;
    if (b.kind == 3) {
      b.vy += 480 * dt;
    }
    b.x += b.vx * dt;
    b.y += b.vy * dt;
    if (b.kind == 2 && int(time * 35) != int((time - dt) * 35))
      burst(b.x, b.y, 1, 1);
    bool collision = false;
    for (auto &p : level().platforms) {
      if (p.oneWay && b.kind != 3)
        continue;
      if (segmentRect(b.px, b.py, b.x, b.y, p.box)) {
        if (b.kind == 3) {
          b.y = p.box.y - 3;
          b.vy = -std::fabs(b.vy) * .35f;
          b.vx *= .72f;
        } else
          collision = true;
        break;
      }
    }
    if (b.kind == 3) {
      if (b.life <= 0) {
        b.alive = false;
        explosion(b.x, b.y, 43, b.damage, b.hostile);
      }
      continue;
    }
    if (!collision && b.hostile) {
      if (segmentRect(b.px, b.py, b.x, b.y, playerBox())) {
        hitPlayer();
        collision = true;
      }
    } else if (!b.hostile && !collision) {
      for (auto &e : enemies)
        if (!e.dead && e.active && segmentRect(b.px, b.py, b.x, b.y, enemyBox(e))) {
          if (b.kind != 2)
            damageEnemy(e, b.damage, false, b.vx > 0 ? 1 : b.vx < 0 ? -1 : 0);
          collision = true;
          break;
        }
      if (!collision && boss.active && !boss.dead && segmentRect(b.px, b.py, b.x, b.y, bossBox())) {
        if (b.kind != 2)
          damageBoss(b.damage);
        burst(b.x, b.y, 0, 3);
        collision = true;
      }
      if (!collision)
        for (auto &p : props)
          if (!p.dead && segmentRect(b.px, b.py, b.x, b.y, {p.x - 13, p.y - 25, 26, 25})) {
            p.hp -= int(std::ceil(b.damage));
            collision = true;
            break;
          }
    }
    if (collision || b.life <= 0 || b.x < camera - 140 || b.x > camera + W + 160 || b.y < -100 ||
        b.y > H + 100) {
      b.alive = false;
      if (b.kind == 2)
        explosion(b.x, b.y, 45, b.damage, b.hostile);
      else if (collision)
        burst(b.x, b.y, 0, 3);
    }
  }
  // Mark before exploding so chained barrels cannot recursively trigger themselves.
  for (auto &p : props)
    if (!p.dead && p.hp <= 0) {
      p.dead = true;
      score += 50;
      if (p.kind == 8)
        explosion(p.x, p.y - 12, 65, 7);
      else {
        burst(p.x, p.y - 12, 0, 14);
        items.push_back({p.x, p.y - 13, levelIndex % 3 + 1, false, 0});
      }
    }
  for (auto &i : items) {
    i.anim += dt;
    if (i.used)
      continue;
    if (std::fabs(player.x - i.x) < 23 && std::fabs(player.y - 17 - i.y) < 35) {
      i.used = true;
      i.anim = 0;
      if (i.kind == 0) {
        rescued++;
        score += 200;
        sounds.push_back(Sound::Rescue);
        player.grenades += 2;
      } else if (i.kind <= 3) {
        player.weapon = i.kind;
        player.ammo = i.kind == 1 ? 180 : i.kind == 2 ? 24 : 14;
        sounds.push_back(Sound::Pickup);
      } else if (i.kind == 6 || i.kind == 9) {
        player.weapon = i.kind == 6 ? 4 : 5;
        player.ammo = i.kind == 6 ? 80 : 36;
        sounds.push_back(Sound::Pickup);
      } else if (i.kind == 4) {
        player.grenades = std::min(30, player.grenades + 5);
        sounds.push_back(Sound::Pickup);
      } else if (i.kind == 5) {
        score += 500;
        sounds.push_back(Sound::Pickup);
      }
      burst(i.x, i.y, 2, 8);
    }
  }
  for (auto &h : level().hazards)
    if (hazardOn(h) && overlap(playerBox(), {h.x, h.y, h.w, h.h}))
      hitPlayer();
  updateBoss(dt);
}
} // namespace kh
