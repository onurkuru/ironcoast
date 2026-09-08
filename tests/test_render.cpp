#include "render.h"
#include "animation.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace kh;

namespace kh {
struct RendererAudit {
  static std::vector<std::pair<std::string, const Atlas *>> atlases(const Renderer &r) {
    std::vector<std::pair<std::string, const Atlas *>> result = {
        {"hero", &r.hero}, {"enemies", &r.enemies}, {"aim", &r.aim}, {"vehicle", &r.vehicle}};
    for (int i = 0; i < 6; ++i) result.push_back({"boss" + std::to_string(i), &r.bosses[i]});
    return result;
  }
  static void lights(Renderer &r, const Game &g, const Input &input, float camera, float alpha) {
    r.collectLights(g, input, camera, g.time, alpha);
    auto p = g.player;
    p.x = between(p.prevX, p.x, alpha);
    p.y = between(p.prevY, p.y, alpha);
    const auto muzzle = muzzlePoint(p, input);
    if (p.recoil > 0 && p.action <= 0) {
      auto it = std::find_if(r.lights.begin(), r.lights.end(), [](const auto &l) { return !l.fixture; });
      if (it == r.lights.end() || std::fabs(it->x - (muzzle.x - camera)) > .01f ||
          std::fabs(it->y - muzzle.y) > .01f)
        throw std::runtime_error("Muzzle light detached from the interpolated shot origin");
    }
    for (const auto &l : r.lights)
      if (!std::isfinite(l.strength) || l.strength < 0 || l.strength > 1.1f)
        throw std::runtime_error("Light animation has an invalid strength");
    if (g.boss.active && !g.boss.dead) {
      auto core = r.bossCore(g, camera, alpha);
      const auto &l = r.lights.back();
      if (std::hypot(core.x - l.x, core.y - l.y) > .01f)
        throw std::runtime_error("Boss light detached from its authored core");
    }
  }
};
}

// Use the actual SDL renderer on a software surface. The visual checks do not
// depend on a logged-in desktop, a window, or a GPU screenshot permission.
int main(int argc, char **argv) {
  if (argc < 2)
    return 2;
  SDL_Init(0);
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 960, 544, 32, SDL_PIXELFORMAT_RGBA32);
  SDL_Renderer *device = surface ? SDL_CreateSoftwareRenderer(surface) : nullptr;
  if (!device) {
    std::cerr << SDL_GetError() << '\n';
    return 1;
  }
  SDL_RenderSetLogicalSize(device, 480, 272);
  int result = 0;
  try {
    Renderer renderer(device, argv[1]);
    auto pixels = [&]() {
      std::vector<Uint32> data(960 * 544);
      if (SDL_RenderReadPixels(device, nullptr, SDL_PIXELFORMAT_RGBA32, data.data(), 960 * 4) != 0)
        throw std::runtime_error(SDL_GetError());
      return data;
    };
    ViewState view;
    view.screen = Screen::Play;
    view.interpolation = 1;
    Game original;
    original.load(0);
    original.player.inv = 0;
    original.player.grounded = true;
    original.syncPresentation();
    renderer.render(original, view);
    const auto baseline = pixels();
    Game moved = original;
    moved.player.x += 95;
    moved.player.vx = 145;
    moved.player.stride = .5f;
    moved.syncPresentation();
    renderer.render(moved, view);
    if (pixels() == baseline)
      throw std::runtime_error("Moving player did not change the rendered frame");
    renderer.render(original, view);
    if (pixels() != baseline)
      throw std::runtime_error("Previous moving sprite or lighting state leaked into the next frame");
    std::vector<Uint32> lastScene;
    for (int stage = 0; stage < 6; ++stage) {
      Game scene;
      scene.load(stage, false, 120);
      scene.player.inv = 0;
      scene.player.grounded = true;
      scene.time = 6; // unobstructed gameplay, after the controls hint
      scene.syncPresentation();
      renderer.render(scene, view);
      auto current = pixels();
      if (current == lastScene)
        throw std::runtime_error("Two missions rendered the same scene");
      lastScene = std::move(current);
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/stage" + std::to_string(stage + 1) + ".png");
    }
    // Sweep the complete world, including platform edges and every checkpoint.
    // Exercise scenery, lighting, actor indexing and camera clamping beyond the
    // opening screenshots. Alternate grounded and airborne presentation.
    int worldSamples = 0;
    for (int stage = 0; stage < 6; ++stage) {
      for (float x = 12; x < campaign()[stage].width; x += 32) {
        Game scene;
        scene.load(stage, false, x);
        scene.time = 9;
        scene.player.inv = 0;
        scene.player.weapon = worldSamples % 6;
        scene.player.vx = worldSamples % 2 ? 145 : -145;
        scene.player.dir = worldSamples % 2 ? 1 : -1;
        scene.player.y = worldSamples % 2 ? 160 : scene.player.y;
        scene.player.grounded = worldSamples % 2 == 0;
        scene.player.stride = worldSamples * .19f;
        scene.syncPresentation();
        view.input = {};
        view.input.shoot = true;
        view.input.up = worldSamples % 3 == 0;
        renderer.render(scene, view);
        ++worldSamples;
      }
    }
    std::cout << worldSamples << " full-world render samples passed\n";
    view.input = {};
    // Stage texture replacement must not change a later identical harbor frame.
    renderer.render(original, view);
    if (pixels() != baseline)
      throw std::runtime_error("Stage switch retained a color, light, or texture state");

    // Draw every active actor cell through the runtime path in both facing
    // directions, checking its real screen bounds and floor contact. This
    // catches destination scaling/baseline bugs that PNG border checks cannot.
    int auditedFrames = 0;
    for (const auto &[name, atlas] : RendererAudit::atlases(renderer)) {
      float size = name == "hero" ? 48 : name == "enemies" ? 43 : name == "aim" ? 56
                       : name == "vehicle" ? 76 : name == "boss4" ? 142 : 152;
      SDL_SetTextureColorMod(atlas->texture, 255, 255, 255);
      for (int frame = 0; frame < int(atlas->cells.size()); ++frame) {
        for (bool flip : {false, true}) {
          SDL_SetRenderDrawColor(device, 0, 0, 0, 255);
          SDL_RenderClear(device);
          renderer.groundedSprite(*atlas, frame, 240 - size / 2, 230 - size, size, size, flip);
          auto image = pixels();
          int bottom = -1, left = 960, right = -1;
          for (int y = 0; y < 544; ++y)
            for (int x = 0; x < 960; ++x)
              if (image[y * 960 + x] != image[0]) {
                bottom = std::max(bottom, y);
                left = std::min(left, x);
                right = std::max(right, x);
              }
          if (bottom < 456 || bottom > 460 || left < (240 - size / 2) * 2 - 1 ||
              right > (240 + size / 2) * 2 + 1)
            throw std::runtime_error(name + ":" + std::to_string(frame) + " escaped its draw box/floor: " +
                                     std::to_string(left) + "," + std::to_string(right) + "," + std::to_string(bottom));
        }
        auditedFrames++;
      }
      // Contact sheets use the real SDL sampler, with the foot plane visible.
      if (argc > 2) {
        SDL_SetRenderDrawColor(device, 23, 35, 44, 255);
        SDL_RenderClear(device);
        float cw = 480.0f / atlas->cols, ch = 248.0f / atlas->rows;
        float reviewSize = std::min(cw - 8, ch - 9);
        for (int frame = 0; frame < int(atlas->cells.size()); ++frame) {
          float x = (frame % atlas->cols + .5f) * cw;
          float feet = 20 + (frame / atlas->cols + 1) * ch - 2;
          renderer.line(x - cw / 2 + 3, feet, x + cw / 2 - 3, feet, 0x426879FF);
          renderer.groundedSprite(*atlas, frame, x - reviewSize / 2, feet - reviewSize,
                                  reviewSize, reviewSize);
        }
        renderer.text(name + " / ALL FRAMES", 8, 5, 1, 0xF5D798FF);
        renderer.screenshot(std::string(argv[2]) + "/atlas-" + name + ".png");
      }
    }

    // Exercise every presentation family that can expose an atlas mapping
    // regression: directional aim, airborne/down aim, vehicle damage, worker
    // rescue, enemy hurt/death and every boss state. The assertions compare
    // complete frames so a state that silently falls back to the previous
    // pose cannot pass unnoticed.
    Game actors = original;
    actors.time = 9;
    actors.player.grounded = true;
    actors.player.anim = .23f;
    actors.player.stride = .41f;
    actors.player.weapon = 2;
    view.input = Input{};
    renderer.render(actors, view);
    auto idle = pixels();
    view.input.up = true;
    view.input.shoot = true;
    actors.player.recoil = .12f;
    renderer.render(actors, view);
    auto aim = pixels();
    if (aim == idle)
      throw std::runtime_error("Upward firing pose did not change the frame");
    view.input = Input{};
    view.input.down = true;
    actors.player.grounded = false;
    actors.player.vy = 180;
    renderer.render(actors, view);
    if (pixels() == aim)
      throw std::runtime_error("Downward airborne pose did not change the frame");
    actors.player.grounded = true;
    actors.player.vehicleHP = 2;
    actors.player.hitFlash = .1f;
    view.input = Input{};
    renderer.render(actors, view);
    if (pixels() == idle)
      throw std::runtime_error("Vehicle damage pose did not change the frame");
    actors.player.vehicleHP = 0;
    actors.player.vehicleDeath = .3f;
    actors.player.vehicleDeathX = actors.player.x;
    actors.player.vehicleDeathY = actors.player.y;
    renderer.render(actors, view);
    if (pixels() == idle)
      throw std::runtime_error("Vehicle destruction strip was not rendered");

    for (int kind = 0; kind < 5; ++kind) {
      Game scene = original;
      scene.time = 9;
      scene.enemies.clear();
      Enemy e;
      e.kind = kind; e.x = e.prevX = 180; e.y = e.prevY = 232;
      e.active = true;
      scene.enemies.push_back(e);
      for (int state = 0; state < 5; ++state) {
        auto &enemy = scene.enemies.front();
        enemy.state = std::min(state, 2); enemy.hurt = state == 3 ? .1f : 0;
        enemy.dead = state == 4; enemy.death = .2f;
        renderer.render(scene, view);
        if (argc > 2 && state == 4)
          renderer.screenshot(std::string(argv[2]) + "/enemy-death-" + std::to_string(kind) + ".png");
      }
    }
    for (float age : {.1f, .3f, .6f, .9f, 1.4f}) {
      Game scene = original;
      scene.items = {{180, 215, 0, true, age}};
      renderer.render(scene, view);
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/worker-" + std::to_string(int(age * 10)) + ".png");
    }

    for (int dir : {-1, 1})
      for (int aimDirection = 0; aimDirection < 3; ++aimDirection)
        for (float alpha : {0.0f, .25f, .5f, .75f, 1.0f}) {
          Game scene = original;
          scene.player.recoil = .1f; scene.player.dir = dir;
          scene.player.x = 120; scene.player.prevX = 118;
          scene.player.y = 150; scene.player.prevY = 148; scene.player.grounded = false;
          Input input; input.up = aimDirection == 1; input.down = aimDirection == 2;
          RendererAudit::lights(renderer, scene, input, 30, alpha);
        }

    for (int stage = 0; stage < 6; ++stage) {
      Game bossScene;
      bossScene.load(stage, false, 120);
      bossScene.time = 8;
      bossScene.player.inv = 0;
      bossScene.boss.active = true;
      bossScene.camera = bossScene.level().width - W;
      bossScene.boss.x = bossScene.level().width - 150;
      bossScene.boss.y = stage == 4 ? 174 : 232;
      bossScene.boss.state = BossState::Move;
      bossScene.boss.stateAge = .18f;
      bossScene.boss.duration = 1.2f;
      bossScene.boss.gait = .35f;
      bossScene.syncPresentation();
      renderer.render(bossScene, view);
      auto move = pixels();
      bossScene.boss.state = BossState::Windup;
      bossScene.boss.stateAge = .38f;
      bossScene.boss.duration = .92f;
      renderer.render(bossScene, view);
      auto windup = pixels();
      bossScene.boss.state = BossState::Attack;
      bossScene.boss.stateAge = .31f;
      bossScene.boss.duration = .82f;
      bossScene.boss.recoil = .12f;
      renderer.render(bossScene, view);
      auto attack = pixels();
      bossScene.boss.state = BossState::Recover;
      bossScene.boss.stateAge = .4f;
      bossScene.boss.duration = 1.45f;
      renderer.render(bossScene, view);
      auto recover = pixels();
      for (float alpha : {0.0f, .5f, 1.0f})
        RendererAudit::lights(renderer, bossScene, {}, bossScene.camera, alpha);
      if (move == windup || windup == attack || attack == recover)
        throw std::runtime_error("Boss animation state reused an identical pose");
      bossScene.boss.dead = true;
      bossScene.boss.death = 1.7f;
      renderer.render(bossScene, view);
      if (pixels() == recover)
        throw std::runtime_error("Boss destruction pose was not rendered");
      if (argc > 2)
        renderer.screenshot(std::string(argv[2]) + "/boss-death-" + std::to_string(stage) + ".png");
    }
    view.input = Input{};
    view.screen = Screen::Pause;
    renderer.render(original, view);
    auto paused = pixels();
    view.clock += 100;
    renderer.render(original, view);
    if (pixels() != paused) throw std::runtime_error("Lighting advanced while paused");
    std::cout << auditedFrames << " atlas cells in both directions, six scenes, actors, lights and pause passed\n";
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    result = 1;
  }
  SDL_DestroyRenderer(device);
  SDL_FreeSurface(surface);
  SDL_Quit();
  return result;
}
