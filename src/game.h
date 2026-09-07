#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace kh {
constexpr float W = 480, H = 272, DT = 1.0f / 60.0f;
struct Rect {
  float x = 0, y = 0, w = 0, h = 0;
};
bool overlap(Rect a, Rect b);
bool segmentRect(float ax, float ay, float bx, float by, Rect r);
struct Input {
  float move = 0;
  bool up = false, down = false, jump = false, shoot = false, grenade = false, interact = false;
};
struct Platform {
  Rect box;
  bool oneWay = false;
};
struct Spawn {
  float x, y;
  int kind;
};
struct ItemSpec {
  float x, y;
  int kind;
};
struct Hazard {
  float x, y, w, h, period, on, offset;
  int kind;
};
struct Level {
  std::string name, subtitle, brief1, brief2, radio, ending, bossName;
  int theme = 0;
  float width = 3200, vehicleX = 0;
  int bossKind = 0;
  std::vector<Platform> platforms;
  std::vector<Spawn> spawns;
  std::vector<ItemSpec> items;
  std::vector<Hazard> hazards;
  std::vector<float> checkpoints;
};
const std::vector<Level> &campaign();
enum class Status { Play, Dying, GameOver, Clear };
enum class Sound {
  Shot,
  Heavy,
  Shotgun,
  Rocket,
  Flame,
  Laser,
  Grenade,
  Blast,
  Hit,
  Jump,
  Pickup,
  Rescue,
  Hurt,
  Boss,
  Step,
  Stomp
};
struct Player {
  float x = 40, y = 232, vx = 0, vy = 0, shot = 0, inv = 0, action = 0, anim = 0;
  float prevX = 40, prevY = 232;
  float stride = 0, fireAge = 1;
  float land = 0, recoil = 0, hitFlash = 0;
  int actionKind = 0;
  int dir = 1, weapon = 0, ammo = 0, grenades = 10, lives = 3, health = 3, maxHealth = 3,
      vehicleHP = 0;
  bool grounded = false, crouch = false;
};
struct Enemy {
  float x = 0, y = 0, baseY = 0, origin = 0, vy = 0, timer = 1, hurt = 0, death = 0;
  int kind = 0, hp = 1, maxhp = 1, dir = -1, state = 0;
  bool active = false, dead = false;
  float prevX = 0, prevY = 0;
};
struct Bullet {
  float x = 0, y = 0, px = 0, py = 0, vx = 0, vy = 0, life = 0, r = 2, damage = 1;
  int kind = 0;
  bool hostile = false, alive = false;
};
struct Particle {
  float x = 0, y = 0, vx = 0, vy = 0, life = 0, maxlife = 0, size = 0;
  int kind = 0;
  uint32_t color = 0;
  float prevX = 0, prevY = 0;
};
struct Item {
  float x, y;
  int kind;
  bool used = false;
  float anim = 0;
};
struct Prop {
  float x, y;
  int kind, hp;
  bool dead = false;
};
enum class BossState { Move, Windup, Recover, Attack, Enter, Overload };
struct Boss {
  float x = 0, y = 232, hp = 0, maxhp = 0, timer = 1, age = 0, hurt = 0, death = 0, targetX = 0;
  float prevX = 0, prevY = 232, vx = 0, vy = 0, moveX = 0, moveY = 232;
  float stateAge = 0, duration = 1.25f, gait = 0, recoil = 0, impact = 0, shotTimer = 0;
  int phase = 1, pattern = 0, volleys = 0;
  BossState state = BossState::Enter;
  bool active = false, dead = false;
};
struct Game {
  Player player;
  Boss boss;
  Status status = Status::Play;
  int levelIndex = 0, score = 0, rescued = 0, totalRescued = 0, continues = 0, kills = 0;
  float time = 0, camera = 0, shake = 0, flash = 0, deathTimer = 0, clearTimer = 0, checkpoint = 40,
        vehicleX = 0;
  float prevCamera = 0, prevTime = 0;
  bool vehicleAvailable = true, debugInvincible = false;
  uint32_t randomState = 1024;
  std::vector<Enemy> enemies;
  std::vector<Item> items;
  std::vector<Prop> props;
  std::array<Bullet, 256> bullets{};
  std::array<Particle, 384> particles{};
  std::vector<Sound> sounds;
  const Level &level() const;
  void load(int index, bool keepScore = false, float startX = 40);
  void retry();
  void update(Input input, float dt = DT);
  Rect playerBox() const;
  Rect enemyBox(const Enemy &) const;
  Rect bossBox() const;
  float floorAt(float x, float fromY = 0) const;
  void fire(float x, float y, float vx, float vy, float damage, int kind, bool hostile = false,
            float life = 2.0f);
  void burst(float x, float y, int kind, int count = 12, float power = 1);
  void explosion(float x, float y, float radius, float damage, bool hostile = false);
  void hitPlayer();
  void damageEnemy(Enemy &, float amount, bool explosive = false, int approach = 0);
  void damageBoss(float amount);
  void updateBoss(float dt);
  void fireBossVolley();
  void syncPresentation();
  bool hazardOn(const Hazard &) const;
  float random();
};
} // namespace kh
