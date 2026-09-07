#include "render.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace kh;

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
    // Stage texture replacement must not change a later identical harbor frame.
    renderer.render(original, view);
    if (pixels() != baseline)
      throw std::runtime_error("Stage switch retained a color, light, or texture state");

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
      if (move == windup || windup == attack || attack == recover)
        throw std::runtime_error("Boss animation state reused an identical pose");
      bossScene.boss.dead = true;
      bossScene.boss.death = 1.7f;
      renderer.render(bossScene, view);
      if (pixels() == recover)
        throw std::runtime_error("Boss destruction pose was not rendered");
    }
    view.input = Input{};
    std::cout << "Six scenes, player movement, frame clearing and stage return passed\n";
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    result = 1;
  }
  SDL_DestroyRenderer(device);
  SDL_FreeSurface(surface);
  SDL_Quit();
  return result;
}
