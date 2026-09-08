#include "game.h"
#include "animation.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace kh;

static unsigned long long frames = 0;
static void require(bool ok, const std::string &message) {
  if (!ok) throw std::runtime_error(message);
}
static void finite(const Game &g) {
  for (float value : {g.player.x, g.player.y, g.player.vx, g.player.vy, g.camera,
                      g.boss.x, g.boss.y, g.boss.hp, g.time})
    require(std::isfinite(value), "non-finite gameplay state");
  require(g.camera >= 0 && g.camera <= g.level().width - W + .01f, "camera escaped level");
  require(g.items.size() <= g.level().items.size() + g.props.size(), "unbounded pickup growth");
  require(g.enemies.size() == g.level().spawns.size(), "unexpected enemy vector growth");
  require(g.player.health >= 0 && g.player.health <= g.player.maxHealth, "invalid player health");
  for (const auto &b : g.bullets)
    if (b.alive) require(std::isfinite(b.x) && std::isfinite(b.y) && std::isfinite(b.life), "invalid projectile");
  for (const auto &p : g.particles)
    if (p.life > 0) require(std::isfinite(p.x) && std::isfinite(p.y) && p.maxlife > 0, "invalid particle");
}
static void tick(Game &g, Input input = {}) {
  g.update(input);
  ++frames;
  finite(g);
}

// Reach the arena through normal movement and kill the boss with real bullets.
// Invulnerability isolates route/AI progression from this bot's dodging skill;
// teleports, debug boss damage and stage skipping are not used.
static void campaignRun(bool vehicle) {
  Game g;
  for (int stage = 0; stage < 6; ++stage) {
    g.load(stage, stage > 0);
    g.debugInvincible = true;
    int falls = 0, arenaFrames = 0;
    float farthest = g.player.x;
    for (int n = 0; n < 36000 && g.status != Status::Clear; ++n) {
      Input in;
      in.shoot = true;
      in.move = g.boss.active ? 0 : 1;
      if (!g.boss.active) {
        in.jump = g.player.grounded && g.floorAt(g.player.x + 18, g.player.y - 2) > g.player.y + 18;
        in.interact = vehicle && g.vehicleAvailable && std::fabs(g.player.x - g.vehicleX) < 24;
      } else {
        ++arenaFrames;
        in.jump = stage == 4 && g.player.grounded;
        in.grenade = arenaFrames % 90 == 1;
      }
      float before = g.player.x;
      tick(g, in);
      if (before - g.player.x > 50) ++falls;
      farthest = std::max(farthest, g.player.x);
      require(falls == 0, "route fell/respawned stage " + std::to_string(stage + 1) +
                           " vehicle=" + std::to_string(vehicle) + " x=" + std::to_string(before));
    }
    require(g.status == Status::Clear, "route/boss timeout stage " + std::to_string(stage + 1) +
             " vehicle=" + std::to_string(vehicle) + " x=" + std::to_string(farthest) +
             " boss_hp=" + std::to_string(g.boss.hp));
    std::cout << "CLEAR stage=" << stage + 1 << " vehicle=" << vehicle
              << " seconds=" << g.time << " score=" << g.score << '\n';
  }
}

static void restarts() {
  for (int stage = 0; stage < 6; ++stage) {
    auto checkpoints = campaign()[stage].checkpoints;
    checkpoints.push_back(40);
    checkpoints.push_back(campaign()[stage].width - 440);
    for (float cp : checkpoints) {
      Game g;
      g.load(stage, false, cp);
      for (int life = 0; life < 3; ++life) {
        g.player.inv = 0;
        g.player.health = 1;
        g.hitPlayer();
        for (int n = 0; n < 70; ++n) tick(g);
        require(g.status == (life == 2 ? Status::GameOver : Status::Play), "death state stuck");
        if (life < 2) require(g.player.y < H, "unsafe checkpoint respawn");
      }
      g.retry();
      require(g.status == Status::Play && g.player.lives == 3 && g.player.y < H, "retry stuck");
      for (int n = 0; n < 5; ++n) tick(g);
    }
  }
}

static void rescuePersistence() {
  for (int stage = 0; stage < 6; ++stage) {
    Game g;
    g.load(stage);
    auto worker = std::find_if(g.items.begin(), g.items.end(), [](const Item &i) { return i.kind == 0; });
    require(worker != g.items.end(), "missing worker fixture");
    g.player.x = worker->x;
    g.player.y = worker->y + 17;
    tick(g);
    require(g.rescued == 1 && worker->used, "worker not rescued");
    g.player.inv = 0;
    g.player.health = 1;
    g.hitPlayer();
    require(g.rescued == 1, "life loss erased rescued count while leaving the worker unavailable");
    for (int n = 0; n < 70; ++n) tick(g);
    require(g.rescued == 1 && worker->used, "respawn lost worker state");
    const float wx = worker->x, wy = worker->y;
    g.player.inv = 0;
    g.player.health = g.player.lives = 1;
    g.hitPlayer();
    for (int n = 0; n < 70; ++n) tick(g);
    require(g.status == Status::GameOver, "game-over fixture did not complete");
    g.retry();
    require(g.rescued == 1, "checkpoint retry lost previous rescues");
    g.player.x = wx;
    g.player.y = wy + 17;
    tick(g);
    require(g.rescued == 1, "checkpoint retry allowed duplicate rescue rewards");
  }
}

static void fuzz() {
  for (int stage = 0; stage < 6; ++stage)
    for (int seed = 0; seed < 8; ++seed) {
      std::mt19937 rng(1921 + seed * 31 + stage);
      Game g;
      const auto &level = campaign()[stage];
      std::vector<float> starts;
      for (const auto &p : level.platforms)
        if (!p.oneWay) starts.push_back(p.box.x + std::min(70.0f, p.box.w / 2));
      g.load(stage, false, seed == 7 ? level.width - 440 : starts[seed % starts.size()]);
      int retries = 0;
      for (int n = 0; n < 18000; ++n) {
        if (g.status == Status::GameOver) { g.retry(); ++retries; }
        if (g.status == Status::Clear) g.load(stage);
        Input in;
        in.move = int(rng() % 3) - 1;
        in.jump = rng() % 13 == 0;
        in.shoot = rng() % 4 != 0;
        in.grenade = rng() % 97 == 0;
        in.up = rng() % 11 == 0;
        in.down = rng() % 17 == 0;
        in.interact = rng() % 71 == 0;
        tick(g, in);
      }
      std::cout << "FUZZ stage=" << stage + 1 << " seed=" << seed << " retries=" << retries << '\n';
    }
}
int main() {
  std::cout << std::unitbuf;
  try {
    rescuePersistence();
    campaignRun(false);
    campaignRun(true);
    restarts();
    fuzz();
    std::cout << "PASS stability simulation frames=" << frames << '\n';
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
