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
