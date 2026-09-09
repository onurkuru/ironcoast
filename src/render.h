#pragma once
#include "game.h"
#include <SDL.h>
#include <string>
#include <vector>
namespace kh {
struct Atlas {
  SDL_Texture *texture = nullptr;
  int width = 0, height = 0, cols = 1, rows = 1;
  bool trimmed = false;
  std::vector<SDL_Rect> cells;
  std::vector<float> baselines;
  std::vector<SDL_FPoint> anchors;
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
  friend struct RendererAudit;
  SDL_Renderer *r;
  std::string assets;
  Atlas hero, enemies, scenery, props, aim, vehicle, architectureTiles, climb;
  const Game *world = nullptr;
  float worldCamera = 0;
  int sceneryTheme = -1;
  std::array<Atlas, 6> bosses;
  struct LocalLight {
    float x, y, floor, radius, strength;
    uint32_t color;
    bool fixture;
    bool window = false;
  };
  std::vector<LocalLight> lights;
  SDL_Texture *lightMask = nullptr;
  int sceneTheme = 0;
  float offsetX = 0, offsetY = 0;
  Atlas load(const std::string &, int, int, bool trim = false, bool paperKey = false);
  void softLight(float, float, float, float, uint32_t, uint8_t, bool additive = true);
  void collectLights(const Game &, const Input &, float, float, float);
  void architecture(const Game &, float, float);
  void wetSurfaces(const Game &, float, float);
  void reflection(const Game &, const Atlas &, int, float, float, float, bool, float, float);
  void surfaceLights(const Game &, float);
  void actorLight(const Atlas &, float, float, bool hurt = false);
  void contactShadow(float, float, float, float);
  SDL_FPoint bossCore(const Game &, float, float) const;

public:
  explicit Renderer(SDL_Renderer *, std::string);
  ~Renderer();
  void rect(float, float, float, float, uint32_t);
  void line(float, float, float, float, uint32_t);
  void text(std::string, float, float, int, uint32_t, bool shadow = true);
  void wrapped(std::string, float, float, int, int, uint32_t);
  void sprite(const Atlas &, int, float, float, float, float, bool flip = false, double angle = 0,
              uint8_t alpha = 255);
  void groundedSprite(const Atlas &, int, float, float, float, float, bool flip = false,
                      double angle = 0, uint8_t alpha = 255);
  void spritePart(const Atlas &, int, Rect, float, float, float, float,
                  float angle = 0, float pivotX = .5f, float pivotY = .5f, uint8_t alpha = 255);
  void ring(float, float, float, float, uint32_t);
  void drawBoss(const Game &, float, float);
  void foregroundDepth(int, float, float);
  void lightingPass(const Game &, float, float, float);
  void background(int, float, float);
  void drawGame(const Game &, const ViewState &);
  void render(const Game &, const ViewState &);
  void screenshot(const std::string &);
};
} // namespace kh
