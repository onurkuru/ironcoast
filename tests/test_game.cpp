#include "game.h"
#include "animation.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
using namespace kh;
static int checks = 0;
static void check(bool ok, const char *msg) {
  checks++;
  if (!ok) {
    std::cerr << "FAIL: " << msg << '\n';
    std::exit(1);
  }
}
static void ticks(Game &g, Input in, int n) {
  for (int i = 0; i < n; i++)
    g.update(in);
}
int main() {
  auto &levels = campaign();
  check(levels.size() == 6, "six complete stages");
  std::set<std::string> names;
  for (int index = 0; index < 6; index++) {
    const auto &l = levels[index];
    names.insert(l.name);
    check(l.bossKind == index, "distinct boss pattern");
    check(!l.brief1.empty() && !l.ending.empty(), "story before and after stage");
    int captives = 0;
    for (auto &i : l.items)
      if (i.kind == 0)
        captives++;
    check(captives == 3, "three rescues per stage");
    Game g;
    g.load(index);
    check(g.player.y == 232, "safe stage spawn");
    for (auto &e : l.spawns)
      if (e.kind != 3)
        check(std::fabs(g.floorAt(e.x, e.y - 1) - e.y) < 1, "enemies stand on actual platform");
    std::vector<bool> reached(l.platforms.size());
    reached[0] = true;
    for (size_t pass = 0; pass < l.platforms.size(); pass++)
      for (size_t a = 0; a < l.platforms.size(); a++)
        if (reached[a])
          for (size_t b = 0; b < l.platforms.size(); b++) {
            auto p = l.platforms[a].box, q = l.platforms[b].box;
            float height = p.y - q.y;
            if (height > 51.2f)
              continue;
            float time = (320 + std::sqrt(320 * 320 - 2000 * height)) / 1000;
            float gap = std::max(0.0f, std::max(q.x - p.x - p.w, p.x - q.x - q.w));
            if (gap + 6 < 145 * time)
              reached[b] = true;
          }
    for (size_t b = 0; b < reached.size(); b++)
      check(reached[b], "authored platform reachable by jump graph");
    for (auto &i : l.items)
      if (i.kind == 0) {
        bool support = false;
        for (size_t p = 0; p < l.platforms.size(); p++) {
          auto b = l.platforms[p].box;
          if (reached[p] && i.x >= b.x && i.x <= b.x + b.w && std::fabs(i.y - (b.y - 17)) < 15)
            support = true;
        }
        check(support, "rescue has reachable support");
      }
    for (auto &h : l.hazards)
      check(h.period <= 0 || h.on < h.period, "timed hazard has a safe window");
    g.boss.active = true;
    g.player.x = l.width - 390;
    g.player.y = 232;
    g.debugInvincible = true;
    g.camera = l.width - W;
    for (int phase = 0; phase < 2; phase++) {
      if (phase)
        g.boss.hp = g.boss.maxhp * .4f;
      ticks(g, {}, 600);
      bool fired = false;
      for (auto &b : g.bullets)
        if (b.alive && b.hostile)
          fired = true;
      check(g.boss.pattern > 0, "boss executes attacks");
      (void)fired;
    }
    g.boss.state = BossState::Recover;
    g.damageBoss(10000);
    ticks(g, {}, 210);
    check(g.status == Status::Clear, "boss death leads to mission completion");
    std::cout << "PASS stage " << index + 1
              << " geometry, story, hazards, boss phases and completion\n";
  }
  check(names.size() == 6, "unique authored stages");
  // Regression: bosses previously stayed fixed or teleported on attack. Verify
  // actual arena movement, full combat cycles and telegraphs across both phases.
  for (int index = 0; index < 6; index++) {
    for (int phase = 1; phase <= 2; phase++) {
      Game arena;
      arena.load(index, false, levels[index].width - 395);
      arena.enemies.clear();
      arena.props.clear();
      arena.items.clear();
      arena.debugInvincible = true;
      arena.boss.active = true;
      if (phase == 2)
        arena.boss.hp = arena.boss.maxhp * .4f;
      float minX = arena.boss.x, maxX = minX, minY = arena.boss.y, maxY = minY;
      int visited = 0, warnings = 0, shots = 0;
      float warningAge = 0;
      float minWeapon = 0, maxWeapon = 0, minStride = 0, maxStride = 0;
      for (int tick = 0; tick < 1800; tick++) {
        Boss previous = arena.boss;
        if (previous.state == BossState::Windup)
          warningAge += DT;
        arena.update({});
        const auto &b = arena.boss;
        minX = std::min(minX, b.x);
        maxX = std::max(maxX, b.x);
        minY = std::min(minY, b.y);
        maxY = std::max(maxY, b.y);
        visited |= 1 << int(b.state);
        check(std::fabs(b.x - previous.x) < 3.1f && std::fabs(b.y - previous.y) < 1,
              "boss locomotion has no teleport frames");
        check(b.x >= levels[index].width - 305 && b.x <= levels[index].width - 88,
              "boss remains in arena with left escape corridor");
        if (b.state == BossState::Attack && previous.state != BossState::Attack) {
          check(previous.state == BossState::Windup && warningAge >= .7f,
                "attack follows readable preparation window");
          warningAge = 0;
          warnings++;
        }
        if (b.volleys > previous.volleys) {
          check(b.state == BossState::Attack, "volley occurs during the attack pose");
          bool projectile = false;
          for (auto &bullet : arena.bullets)
            projectile |= bullet.alive && bullet.hostile;
          check(projectile, "boss attack actually emits projectiles");
          shots++;
        }
        auto pose = bossPose(b, index);
        minWeapon = std::min(minWeapon, pose.weapon);
        maxWeapon = std::max(maxWeapon, pose.weapon);
        minStride = std::min(minStride, pose.stride);
        maxStride = std::max(maxStride, pose.stride);
        check(std::isfinite(pose.lift) && std::isfinite(pose.weapon), "boss pose stays finite");
      }
      check(maxX - minX > 85, "every boss visibly traverses the arena");
      if (index == 4)
        check(maxY - minY > 32, "flying boss traverses vertically as well");
      for (auto state : {BossState::Move, BossState::Windup, BossState::Attack, BossState::Recover})
        check(visited & (1 << int(state)), "boss completes every combat animation state");
      check(warnings >= 3 && shots >= 3, "boss repeats complete attack cycles");
      check(maxWeapon - minWeapon > 7, "weapon articulates through preparation and contact");
      if (index == 0 || index == 3 || index == 5)
        check(maxStride - minStride > 1.1f, "walking rigs alternate both feet");
      arena.boss.state = BossState::Recover;
      float hp = arena.boss.hp;
      arena.damageBoss(1);
      float openDamage = hp - arena.boss.hp;
      arena.boss.state = BossState::Windup;
      hp = arena.boss.hp;
      arena.damageBoss(1);
      check(openDamage > (hp - arena.boss.hp) * 2, "exposed core is more vulnerable");
    }
  }
  check(between(100, 120, .5f) == 110, "presentation interpolates between fixed steps");
  Boss poseTest;
  poseTest.state = BossState::Windup;
  poseTest.stateAge = poseTest.duration = .92f;
  auto held = bossPose(poseTest, 3);
  poseTest.state = BossState::Attack;
  poseTest.stateAge = 0;
  poseTest.duration = .82f;
  auto release = bossPose(poseTest, 3);
  check(std::fabs(held.weapon - release.weapon) < .01f &&
            std::fabs(held.lift - release.lift) < .01f,
        "hammer transitions from windup to swing without a pose pop");
  check(segmentRect(0, 5, 100, 5, {40, 0, 2, 10}), "swept bullet hits thin target");
  check(!segmentRect(0, 20, 100, 20, {40, 0, 2, 10}), "swept bullet misses outside target");
  Game g;
  g.load(0);
  g.enemies.clear();
  g.props.clear();
  g.items.clear();
  ticks(g, {}, 2);
  Input jump;
  jump.jump = true;
  g.update(jump);
  check(g.player.vy < 0 && !g.player.grounded, "jump launches immediately");
  float miny = g.player.y;
  for (int i = 0; i < 50; i++) {
    g.update({});
    miny = std::min(miny, g.player.y);
  }
  check(miny > 178 && miny < 185, "jump height in designed range");
  check(g.player.grounded, "jump lands");
  Input down;
  down.down = true;
  down.shoot = true;
  g.player.shot = 0;
  g.update(down);
  check(g.player.crouch, "down crouches on ground");
  bool horizontal = false;
  for (auto &b : g.bullets)
    if (b.alive && !b.hostile && b.vx > 0)
      horizontal = true;
  check(horizontal, "crouch shoots horizontally");
  g.bullets = {};
  g.player.y = 150;
  g.player.grounded = false;
  g.player.shot = 0;
  g.update(down);
  bool downward = false;
  for (auto &b : g.bullets)
    if (b.alive && b.vy > 400)
      downward = true;
  check(downward, "air down shoots down");
  g.load(0);
  g.items.clear();
  g.player.weapon = 1;
  g.player.ammo = 1;
  Input shot;
  shot.shoot = true;
  g.update(shot);
  check(g.player.weapon == 0 && g.player.ammo == 0, "empty special weapon returns to pistol");
  g.bullets = {};
  g.player.weapon = 4;
  g.player.ammo = 80;
  g.player.shot = 0;
  g.update(shot);
  bool flameShot = false;
  for (auto &b : g.bullets)
    if (b.alive && b.kind == 7)
      flameShot = true;
  check(flameShot && g.player.ammo == 79, "flame shot fires its close-range cone");
  g.bullets = {};
  g.player.weapon = 5;
  g.player.ammo = 36;
  g.player.shot = 0;
  g.update(shot);
  bool laserShot = false;
  for (auto &b : g.bullets)
    if (b.alive && b.kind == 8)
      laserShot = true;
  check(laserShot && g.player.ammo == 35, "laser fires a precision projectile");
  g.load(0, false, 1500);
  g.update({});
  check(g.player.weapon == 4 && g.player.ammo == 80, "flame pickup equips the weapon");
  g.load(0, false, 2860);
  g.update({});
  check(g.player.weapon == 5 && g.player.ammo == 36, "laser pickup equips the weapon");
  g.load(0);
  g.player.inv = 0;
  g.hitPlayer();
  check(g.player.health == 2 && g.player.lives == 3 && g.status == Status::Play,
        "first hit consumes health before a life");
  g.hitPlayer();
  check(g.player.health == 2 && g.player.lives == 3, "damage cooldown prevents repeated hits");
  g.player.inv = 0;
  g.hitPlayer();
  g.player.inv = 0;
  g.hitPlayer();
  check(g.player.lives == 2 && g.status == Status::Dying, "three hits lose one life");
  ticks(g, {}, 70);
  check(g.player.inv > 0 && g.player.health == g.player.maxHealth && g.status == Status::Play,
        "respawn protection and full health");
  g.player.inv = 0;
  g.player.vehicleHP = 3;
  g.hitPlayer();
  check(g.player.vehicleHP == 2 && g.player.lives == 2, "vehicle absorbs hit");
  g.load(0);
  g.player.x = g.vehicleX;
  Input enter;
  enter.interact = true;
  g.update(enter);
  check(g.player.vehicleHP == 3 && !g.vehicleAvailable, "enter vehicle");
  g.update(enter);
  check(g.player.vehicleHP == 0 && g.vehicleAvailable, "exit vehicle");
  g.load(0);
  g.items.clear();
  g.enemies.clear();
  Enemy e;
  e.x = 60;
  e.y = 232;
  e.kind = 2;
  e.active = true;
  e.hp = e.maxhp = 7;
  e.dir = -1;
  g.enemies.push_back(e);
  g.damageEnemy(g.enemies[0], 3, false, 1);
  check(g.enemies[0].hp == 7, "shield blocks frontal shots");
  g.damageEnemy(g.enemies[0], 3, true, 1);
  check(g.enemies[0].hp == 4, "explosives bypass shield");
  g.load(0);
  g.enemies.clear();
  g.items = {{g.player.x, g.player.y - 17, 0, false, 0}};
  g.update({});
  g.update({});
  check(g.rescued == 1, "rescue rewarded once");
  for (int i = 0; i < 800; i++)
    g.fire(100, 100, 1, 1, 1, 0);
  int active = 0;
  for (auto &b : g.bullets)
    active += b.alive;
  check(active == 256, "bullet pool stays bounded");
  g.load(0);
  g.debugInvincible = true;
  for (int i = 0; i < 36000; i++) {
    Input in;
    in.shoot = true;
    in.up = (i / 240) % 2;
    in.grenade = i % 120 == 0;
    g.update(in);
    check(std::isfinite(g.player.x) && std::isfinite(g.player.y),
          "ten-minute state remains finite");
  }
  std::cout << "PASS " << checks << " assertions incl. 10-minute simulation\n";
}
