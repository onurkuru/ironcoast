#pragma once
#include "game.h"
#include <SDL.h>
#include <string>
#include <vector>
namespace kh {
struct Atlas {
  SDL_Texture *texture = nullptr;
  int width = 0, height = 0, cols = 1, rows = 1;
  std::vector<SDL_Rect> cells;
};
enum class Screen { Title, Map, Brief, Play, Pause, Debrief, Ending, Options, Controls };
struct ViewState {
  Screen screen = Screen::Title;
  int selected = 0, unlocked = 0, best = 0, menu = 0;
  float clock = 0, interpolation = 0;
  bool muted = false, shake = true, assist = false, fullscreen = false;
  Input input;
};
class Renderer {
  SDL_Renderer *r;
  std::string assets;
  Atlas hero, enemies, worlds, machines, props, aim, melee, vehicle;
  std::array<Atlas, 6> bosses;
  float offsetX = 0, offsetY = 0;
  Atlas load(const std::string &, int, int, bool trim = false, bool paperKey = false);

public:
  explicit Renderer(SDL_Renderer *, std::string);
  ~Renderer();
  void rect(float, float, float, float, uint32_t);
  void line(float, float, float, float, uint32_t);
  void text(std::string, float, float, int, uint32_t, bool shadow = true);
  void wrapped(std::string, float, float, int, int, uint32_t);
  void sprite(const Atlas &, int, float, float, float, float, bool flip = false, double angle = 0,
              uint8_t alpha = 255);
  void spritePart(const Atlas &, int, Rect, float, float, float, float,
                  float angle = 0, float pivotX = .5f, float pivotY = .5f, uint8_t alpha = 255);
  void ring(float, float, float, float, uint32_t);
  void drawBoss(const Game &, float, float);
  void background(int, float, float);
  void drawGame(const Game &, const ViewState &);
  void render(const Game &, const ViewState &);
  void screenshot(const std::string &);
};
} // namespace kh
