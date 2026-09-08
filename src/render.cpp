#include "render.h"
#include "animation.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace kh {
static constexpr uint32_t INK = 0x0C1926FF, CREAM = 0xF7E6BAFF, GOLD = 0xF4B64AFF,
                          TEAL = 0x65D8D0FF, RED = 0xF47859FF;
static const std::map<char, std::array<unsigned char, 7>> FONT = {
    {'A', {14, 17, 17, 31, 17, 17, 17}}, {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}}, {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}}, {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 15}}, {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},      {'J', {7, 2, 2, 2, 18, 18, 12}},
    {'K', {17, 18, 20, 24, 20, 18, 17}}, {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 21, 17, 17, 17}}, {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}}, {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}}, {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},   {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}}, {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 21, 21, 21, 10}}, {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},     {'Z', {31, 1, 2, 4, 8, 16, 31}},
    {'0', {14, 17, 19, 21, 25, 17, 14}}, {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},     {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},     {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}}, {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}}, {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'.', {0, 0, 0, 0, 0, 6, 6}},        {',', {0, 0, 0, 0, 0, 6, 4}},
    {':', {0, 6, 6, 0, 6, 6, 0}},        {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'/', {1, 2, 2, 4, 8, 8, 16}},       {'!', {4, 4, 4, 4, 4, 0, 4}},
    {'?', {14, 17, 1, 2, 4, 0, 4}},      {'+', {0, 4, 4, 31, 4, 4, 0}},
    {'>', {16, 8, 4, 2, 4, 8, 16}},      {'<', {1, 2, 4, 8, 4, 2, 1}},
    {'=', {0, 0, 31, 0, 31, 0, 0}},      {'[', {14, 8, 8, 8, 8, 8, 14}},
    {']', {14, 2, 2, 2, 2, 2, 14}},      {'(', {2, 4, 8, 8, 8, 4, 2}},
    {')', {8, 4, 2, 2, 2, 4, 8}}};
Renderer::Renderer(SDL_Renderer *rr, std::string path) : r(rr), assets(std::move(path)) {
  // Keep the authored grid cells intact. Per-cell trimming makes a pose with
  // an outstretched arm occupy a different visual scale from its idle pose.
  hero = load("hero-v2.png", 8, 8, false);
  enemies = load("enemies-v2.png", 8, 6, false);
  vehicle = load("vehicle-v2.png", 4, 4, false);
  for (int i = 0; i < 6; i++)
    bosses[i] = load("boss" + std::to_string(i) + "-v2.png", 4, 4, false);
  props = load("props.png", 4, 4, false);
  aim = load("aim-v2.png", 4, 3, false);
  // One small radial alpha mask is reused for lamps, fog and contact shadows.
  // No full-screen render targets or per-frame texture uploads are required.
  SDL_Surface *mask = SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, SDL_PIXELFORMAT_RGBA32);
  if (!mask)
    throw std::runtime_error(SDL_GetError());
  for (int y = 0; y < 64; ++y)
    for (int x = 0; x < 64; ++x) {
      float dx = (x - 31.5f) / 31.5f, dy = (y - 31.5f) / 31.5f;
      float falloff = std::max(0.0f, 1.0f - dx * dx - dy * dy);
      auto *row = reinterpret_cast<Uint32 *>(static_cast<Uint8 *>(mask->pixels) + y * mask->pitch);
      row[x] = SDL_MapRGBA(mask->format, 255, 255, 255, Uint8(falloff * falloff * 255));
    }
  lightMask = SDL_CreateTextureFromSurface(r, mask);
  SDL_FreeSurface(mask);
  if (!lightMask)
    throw std::runtime_error(SDL_GetError());
}
Renderer::~Renderer() {
  for (auto *a : {&hero, &enemies, &scenery, &props, &aim, &vehicle})
    SDL_DestroyTexture(a->texture);
  for (auto &a : bosses)
    SDL_DestroyTexture(a.texture);
  SDL_DestroyTexture(lightMask);
}
Atlas Renderer::load(const std::string &name, int cols, int rows, bool trim, bool paperKey) {
  Atlas a;
  a.cols = cols;
  a.rows = rows;
  a.trimmed = trim;
  int n;
  unsigned char *pixels = stbi_load((assets + "/" + name).c_str(), &a.width, &a.height, &n, 4);
  if (!pixels)
    throw std::runtime_error("Asset missing: " + assets + "/" + name);
  // Import-time alpha test: no source artwork is rewritten. This also keys the
  // neutral paper background of the directional pose reference at render time.
  for (int i = 0; i < a.width * a.height; i++) {
    unsigned char *p = pixels + i * 4;
      bool whitePaper = std::min({p[0], p[1], p[2]}) > 210 &&
                        std::max({p[0], p[1], p[2]}) - std::min({p[0], p[1], p[2]}) < 22;
      bool greenPaper = p[1] > 210 && p[0] < 80 && p[2] < 80;
      if (paperKey && (whitePaper || greenPaper))
      p[3] = 0;
    if (trim && p[3] < 112)
      p[3] = 0;
  }
  for (int row = 0; row < rows; row++)
    for (int col = 0; col < cols; col++) {
      int x0 = col * a.width / cols, x1 = (col + 1) * a.width / cols, y0 = row * a.height / rows,
          y1 = (row + 1) * a.height / rows;
      int margin = name == "props.png" ? 6 : name.find("-v2.png") != std::string::npos ? 0 : 1;
      x0 += margin;
      y0 += margin;
      x1 -= margin;
      y1 -= margin;
      int left = x1, right = x0, top = y1, bottom = y0;
      if (trim) {
        for (int y = y0; y < y1; y++)
          for (int x = x0; x < x1; x++)
            if (pixels[(y * a.width + x) * 4 + 3] > 150) {
              left = std::min(left, x);
              right = std::max(right, x);
              top = std::min(top, y);
              bottom = std::max(bottom, y);
            }
      }
      if (!trim || right <= left) {
        left = x0;
        right = x1 - 1;
        top = y0;
        bottom = y1 - 1;
      }
      int visibleBottom = y0;
      bool visible = false;
      for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
          if (pixels[(y * a.width + x) * 4 + 3] > 16) {
            visible = true;
            visibleBottom = std::max(visibleBottom, y + 1);
          }
      a.baselines.push_back(visible ? std::clamp(float(visibleBottom - y0) / float(y1 - y0), 0.0f, 1.0f)
                                    : 1.0f);
      a.cells.push_back({left, top, right - left + 1, bottom - top + 1});
    }
  if (name.rfind("boss", 0) == 0) {
    std::ifstream input(assets + "/" + name.substr(0, name.size() - 4) + ".anchors");
    SDL_FPoint point;
    while (input >> point.x >> point.y) {
      if (!(point.x >= 0 && point.x <= 1 && point.y >= 0 && point.y <= 1)) {
        stbi_image_free(pixels);
        throw std::runtime_error("Invalid light anchor: " + name);
      }
      a.anchors.push_back(point);
    }
    if (a.anchors.size() != a.cells.size()) {
      stbi_image_free(pixels);
      throw std::runtime_error("Missing light anchors: " + name);
    }
  }
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(pixels, a.width, a.height, 32, a.width * 4,
                                                      SDL_PIXELFORMAT_RGBA32);
  a.texture = SDL_CreateTextureFromSurface(r, s);
  SDL_FreeSurface(s);
  stbi_image_free(pixels);
  if (!a.texture)
    throw std::runtime_error(SDL_GetError());
  SDL_SetTextureBlendMode(a.texture, SDL_BLENDMODE_BLEND);
  return a;
}
void Renderer::rect(float x, float y, float w, float h, uint32_t c) {
  SDL_SetRenderDrawColor(r, c >> 24, (c >> 16) & 255, (c >> 8) & 255, c & 255);
  SDL_FRect b{x + offsetX, y + offsetY, w, h};
  SDL_RenderFillRectF(r, &b);
}
void Renderer::line(float x, float y, float xx, float yy, uint32_t c) {
  SDL_SetRenderDrawColor(r, c >> 24, (c >> 16) & 255, (c >> 8) & 255, c & 255);
  SDL_RenderDrawLineF(r, x + offsetX, y + offsetY, xx + offsetX, yy + offsetY);
}
void Renderer::text(std::string s, float x, float y, int size, uint32_t color, bool shadow) {
  if (shadow)
    text(s, x + size, y + size, size, 0x051018DD, false);
  float start = x;
  for (unsigned char ch : s) {
    if (ch == '\n') {
      x = start;
      y += size * 9;
      continue;
    }
    if (ch >= 'a' && ch <= 'z')
      ch -= 32;
    auto it = FONT.find(ch);
    if (it != FONT.end())
      for (int yy = 0; yy < 7; yy++)
        for (int xx = 0; xx < 5; xx++)
          if (it->second[yy] & (1 << (4 - xx)))
            rect(x + xx * size, y + yy * size, size, size, color);
    x += 6 * size;
  }
}
void Renderer::wrapped(std::string s, float x, float y, int size, int width, uint32_t color) {
  std::istringstream stream(s);
  std::string word, row;
  int chars = width / (6 * size);
  while (stream >> word) {
    if (int(row.size() + word.size() + 1) > chars) {
      text(row, x, y, size, color);
      y += size * 11;
      row.clear();
    }
    if (!row.empty())
      row += ' ';
    row += word;
  }
  if (!row.empty())
    text(row, x, y, size, color);
}
void Renderer::sprite(const Atlas &a, int idx, float x, float y, float w, float h, bool flip,
                      double angle, uint8_t alpha) {
  SDL_Rect src = a.cells.at(idx);
  SDL_FRect dest{x + offsetX, y + offsetY, w, h};
  SDL_SetTextureAlphaMod(a.texture, alpha);
  SDL_RenderCopyExF(r, a.texture, &src, &dest, angle, nullptr,
                    flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  SDL_SetTextureAlphaMod(a.texture, 255);
}
void Renderer::groundedSprite(const Atlas &a, int idx, float x, float y, float w, float h,
                              bool flip, double angle, uint8_t alpha) {
  // Trimmed cells are already mapped from their visible bounding box to the
  // complete destination box, so their visible bottom is exactly y + h.
  // Applying the full-cell baseline a second time pushed run-fire and worker
  // poses below the collision line. Only untrimmed atlases need the correction.
  float shift = !a.trimmed
                    ? (1.0f - a.baselines.at(idx)) * h
                    : 0.0f;
  sprite(a, idx, x, y + shift, w, h, flip, angle, alpha);
}
void Renderer::spritePart(const Atlas &a, int idx, Rect part, float x, float y, float w, float h,
                          float angle, float pivotX, float pivotY, uint8_t alpha) {
  const auto &cell = a.cells.at(idx);
  int left = int(std::round(part.x * cell.w)), top = int(std::round(part.y * cell.h));
  int right = int(std::round((part.x + part.w) * cell.w));
  int bottom = int(std::round((part.y + part.h) * cell.h));
  SDL_Rect src{cell.x + left, cell.y + top, right - left, bottom - top};
  SDL_FRect dst{x + offsetX, y + offsetY, w, h};
  SDL_FPoint pivot{w * pivotX, h * pivotY};
  SDL_SetTextureAlphaMod(a.texture, alpha);
  SDL_RenderCopyExF(r, a.texture, &src, &dst, angle, &pivot, SDL_FLIP_NONE);
  SDL_SetTextureAlphaMod(a.texture, 255);
}
void Renderer::ring(float x, float y, float rx, float ry, uint32_t color) {
  for (int i = 0; i < 24; i++) {
    float a = i * 6.283185f / 24, b = (i + 1) * 6.283185f / 24;
    line(x + std::cos(a) * rx, y + std::sin(a) * ry,
         x + std::cos(b) * rx, y + std::sin(b) * ry, color);
  }
}
SDL_FPoint Renderer::bossCore(const Game &g, float camera, float alpha) const {
  int kind = g.level().bossKind, frame = bossFrame(g.boss, kind, alpha);
  const auto &atlas = bosses[kind];
  float size = kind == 4 ? 142.0f : 152.0f;
  float x = between(g.boss.prevX, g.boss.x, alpha) - camera;
  float y = between(g.boss.prevY, g.boss.y, alpha) + (kind == 4 && !g.boss.dead ? 7 : 0);
  auto anchor = atlas.anchors.at(frame);
  return {x - size / 2 + anchor.x * size, y + (anchor.y - atlas.baselines[frame]) * size};
}
void Renderer::drawBoss(const Game &g, float camera, float alpha) {
  const auto &b = g.boss;
  if (!b.active) return;
  int kind = g.level().bossKind, frame = bossFrame(b, kind, alpha);
  const auto pose = bossPose(b, kind, alpha);
  const auto &atlas = bosses[kind];
  float cx = between(b.prevX, b.x, alpha) - camera;
  float cy = between(b.prevY, b.y, alpha) + (kind == 4 && !b.dead ? 7 : 0);
  float size = kind == 4 ? 142.0f : 152.0f;
  auto core = bossCore(g, camera, alpha);
  uint8_t opacity = b.dead ? uint8_t(std::clamp(b.death * 180, 0.0f, 255.0f)) : 255;
  contactShadow(cx, cy, g.floorAt(b.x, cy - 2), kind == 4 ? 42 : 64);
  actorLight(atlas, core.x, core.y, b.hurt > 0);
  groundedSprite(atlas, frame, cx - size / 2, cy - size, size, size, false, 0, opacity);
  SDL_SetTextureColorMod(atlas.texture, 255, 255, 255);
  if (b.dead) return;
  if (b.state == BossState::Windup) {
    float target = b.targetX - camera, floor = g.floorAt(b.targetX, 200);
    bool groundStrike = (kind == 0 && b.pattern % 2) || kind == 3 || (kind == 5 && b.pattern % 2);
    if (groundStrike) {
      ring(target, floor - 2, 24 + pose.charge * 18, 5, RED);
      text("MOVE!", target - 14, floor - 50, 1, GOLD);
    } else if (kind == 1 && b.pattern % 2)
      for (int j = 0; j < 7; ++j) text("<", cx - 68 - j * 16, cy - 8, 1, RED);
  }
  if (pose.charge > 0) {
    softLight(core.x, core.y, 12 + pose.charge * 12, 12 + pose.charge * 12,
              kind == 4 ? TEAL : GOLD, Uint8(65 * pose.charge));
    text(b.state == BossState::Overload ? "OVERDRIVE" : "!", cx - 24, cy - size, 1, GOLD);
  }
  if (pose.core > 0) {
    // Recovery exposes the core. Keep the cue on the painted mechanism,
    // fading with its animation instead of displaying a floating status label.
    float radius = 6 + pose.core * 4;
    softLight(core.x, core.y, radius, radius, TEAL, Uint8(110 * pose.core));
  }
  if (pose.kick > 0) {
    auto muzzle = bossMuzzlePoint(b, kind, alpha);
    softLight(muzzle.x - camera, muzzle.y, 22, 15, kind == 4 ? TEAL : GOLD, Uint8(pose.kick * 95));
  }
  if (b.impact > .3f)
    ring(cx - 35, g.floorAt(b.x, 200) - 1, (1 - b.impact) * 50 + 12, 3, 0xEAB66CAA);
}
void Renderer::softLight(float x, float y, float rx, float ry, uint32_t color,
                         uint8_t opacity, bool additive) {
  SDL_SetTextureBlendMode(lightMask, additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
  SDL_SetTextureColorMod(lightMask, color >> 24, (color >> 16) & 255, (color >> 8) & 255);
  SDL_SetTextureAlphaMod(lightMask, opacity);
  SDL_FRect dest{x - rx + offsetX, y - ry + offsetY, rx * 2, ry * 2};
  SDL_RenderCopyF(r, lightMask, nullptr, &dest);
}
void Renderer::collectLights(const Game &g, const Input &input, float camera, float time,
                             float alpha) {
  sceneTheme = g.levelIndex;
  lights.clear();
  // Fixtures are anchored to world coordinates, including their light/shadow.
  // Only a few can be visible at once; there is no screen-space drifting light.
  int first = std::max(0, int((camera - 160) / 360));
  for (int i = first; i < first + 4; ++i) {
    float wx = 180 + i * 360.0f;
    if (wx > g.level().width - 75)
      continue;
    float ground = g.floorAt(wx, 200);
    if (ground > H || wx - camera > W + 120 || wx - camera < -120)
      continue;
    bool cool = sceneTheme == 4 || (i + sceneTheme) % 3 == 1;
    uint32_t color = cool ? 0x72CFDF00 : sceneTheme == 1 ? 0xC6D99900 : 0xFFC07C00;
    float strength = .94f + .025f * std::sin(time * 1.7f + i);
    lights.push_back({wx - camera, ground - 76, ground, 102, strength, color, true});
  }
  if (g.player.recoil > 0 && g.player.action <= 0 && g.status == Status::Play) {
    float x = between(g.player.prevX, g.player.x, alpha) - camera;
    float y = between(g.player.prevY, g.player.y, alpha);
    Player presentation = g.player;
    presentation.x = x + camera;
    presentation.y = y;
    auto muzzle = muzzlePoint(presentation, input);
    lights.push_back({muzzle.x - camera, muzzle.y, y, 60,
                      std::min(1.0f, g.player.recoil * 14),
                      g.player.weapon == 5 ? 0x75E7EE00u : 0xFFD59700u, false});
  }
  if (g.player.vehicleDeath > 0) {
    float fade = std::clamp(g.player.vehicleDeath / .48f, 0.0f, 1.0f);
    float feet = between(g.player.prevVehicleDeathY, g.player.vehicleDeathY, alpha);
    lights.push_back({g.player.vehicleDeathX - camera, feet - 28,
                      feet, 78, fade,
                      0xF08A4600u, false});
  }
  if (g.boss.active && !g.boss.dead) {
    const int kind = g.level().bossKind;
    const auto pose = bossPose(g.boss, kind, alpha);
    // Core and muzzle lighting are driven by the same pose values used by
    // drawBoss; a closed core no longer emits a constant detached glow.
    float pulse = std::max(pose.core, pose.charge * .8f);
    float strength = .18f + pulse * .62f + std::min(1.0f, g.boss.recoil * 3.5f) * .25f;
    uint32_t color = pose.core > .05f || sceneTheme == 4 ? 0x6EDAE900u : 0xEB845600u;
    if (pose.kick > 0) {
      auto muzzle = bossMuzzlePoint(g.boss, kind, alpha);
      lights.push_back({muzzle.x - camera, muzzle.y, g.boss.y, 55, pose.kick,
                        kind == 4 ? 0x6EDAE900u : 0xFFD59700u, false});
    }
    auto core = bossCore(g, camera, alpha);
    lights.push_back({core.x, core.y, between(g.boss.prevY, g.boss.y, alpha), 94, strength, color, false});
  }
}
void Renderer::actorLight(const Atlas &atlas, float x, float y, bool hurt) {
  float red = 183, green = 202, blue = 215;
  for (const auto &light : lights) {
    float dx = (x - light.x) / light.radius;
    float dy = (y - light.y) / (light.radius * 1.2f);
    float amount = std::max(0.0f, 1 - dx * dx - dy * dy) * light.strength;
    red += amount * ((light.color >> 24) / 255.0f) * 88;
    green += amount * (((light.color >> 16) & 255) / 255.0f) * 66;
    blue += amount * (((light.color >> 8) & 255) / 255.0f) * 43;
  }
  SDL_SetTextureColorMod(atlas.texture, hurt ? 255 : Uint8(std::min(red, 255.0f)),
                         hurt ? 172 : Uint8(std::min(green, 255.0f)),
                         hurt ? 142 : Uint8(std::min(blue, 255.0f)));
}
void Renderer::contactShadow(float x, float feet, float ground, float width) {
  if (ground > H || ground < feet - 3)
    return;
  float height = std::max(0.0f, ground - feet);
  float strength = std::max(.12f, 1 - height / 140);
  softLight(x, ground + 1, width * (.65f + .35f * strength), 2.2f,
            0x02060A00, Uint8(190 * strength), false);
  // A restrained cast shadow extends away from the closest practical lamp.
  for (const auto &light : lights)
    if (light.fixture && std::fabs(light.x - x) < 95 && height < 5) {
      float extension = std::clamp((x - light.x) * .16f, -13.0f, 13.0f);
      softLight(x + extension, ground + 2, width + std::fabs(extension), 2.5f,
                0x02060A00, 55, false);
      break;
    }
}
void Renderer::background(int theme, float camera, float time) {
  static constexpr const char *plates[] = {"harbor-night.png", "marsh-night.png",
      "ironline-night.png", "foundry-night.png", "relay-night.png", "command-night.png"};
  theme = std::clamp(theme, 0, 5);
  if (sceneryTheme != theme) {
    // Keep only the active scene texture resident, especially on Vita.
    SDL_DestroyTexture(scenery.texture);
    scenery = Atlas{};
    scenery = load(plates[theme], 1, 1);
    sceneryTheme = theme;
  }
  // Each continuous panorama spans its mission without mirroring landmarks.
  float width = H * float(scenery.width) / scenery.height;
  float progress = std::clamp(camera / (campaign()[theme].width - W), 0.0f, 1.0f);
  sprite(scenery, 0, -(width - W) * progress, 0, width, H);
  // Far harbor haze separates distant architecture from the playable lane.
  for (int i = 0; i < 3; ++i) {
    float x = i * 270.0f - std::fmod(camera * .24f + time * 2, 270.0f);
    softLight(x, 180 + i * 7, 205, 24, theme == 3 ? 0x926F6500 : 0x5E9DAD00, 18);
  }
  // Dock-side architecture at a distinct depth. Discrete walls have no
  // autonomous travel: the only motion here is camera parallax.
  int first = int(camera * .48f / 164) - 1;
  for (int i = first; i < first + 5; ++i) {
    float x = i * 164.0f - camera * .48f;
    float height = 18 + (std::abs(i) % 3) * 7;
    rect(x, 232 - height, 74, height, 0x07131CB8);
    line(x, 232 - height, x + 74, 232 - height, 0x33505A6A);
    for (int j = 0; j < 5; ++j)
      line(x + 7 + j * 14, 235 - height, x + 7 + j * 14, 229, 0x33434E48);
    rect(x + 96, 224, 44, 8, 0x07131C9A);
  }
  if (theme == 4 || theme == 5)
    for (int i = 0; i < 46; ++i) {
      float x = std::fmod(i * 71.37f - time * 58 + 40000, 500.0f);
      float y = std::fmod(i * 43.13f + time * 140, 280.0f);
      line(x, y, x - 2, y + 6, 0x87CCDC35);
    }
  if (theme == 3)
    for (int i = 0; i < 15; ++i) {
      float x = std::fmod(i * 53.1f + time * 9, 480.0f);
      float y = 272 - std::fmod(i * 19.7f + time * 22, 260.0f);
      rect(x, y, 1, 1, 0xFCAA5980);
    }
}
void Renderer::foregroundDepth(int, float camera, float) {
  // Near-field dock fascia is safely below the actor feet at every camera x.
  int first = int(camera * 1.12f / 118) - 1;
  for (int i = first; i < first + 6; ++i) {
    float x = i * 118.0f - camera * 1.12f;
    rect(x, 259, 86, 10, 0x040B12B0);
    line(x + 2, 259, x + 83, 259, 0x4D70703A);
    rect(x + 6, 262, 2, 2, 0x47585965);
  }
}
void Renderer::lightingPass(const Game &, float, float, float) {
  for (const auto &light : lights) {
    softLight(light.x, light.y + 16, light.radius, light.radius * .85f,
              light.color, Uint8(34 * light.strength));
    if (!light.fixture)
      continue;
    // Visible grounded lamp: the cone, glow and reflected pool share its root.
    rect(light.x + 6, light.y - 8, 2, light.floor - light.y + 8, 0x080F17FF);
    line(light.x + 7, light.y - 8, light.x + 7, light.floor, 0x36515F9A);
    rect(light.x - 8, light.y - 6, 17, 5, 0x08121AFF);
    rect(light.x - 5, light.y - 1, 10, 1, light.color | 220);
    softLight(light.x, light.y, 17, 8, light.color, 110);
    // Analytic cone with smooth edges, low alpha, and a real source.
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    for (int y = int(light.y + 2); y < int(light.floor); y += 2) {
      float t = (y - light.y) / (light.floor - light.y);
      float width = 4 + t * 35;
      for (int band = 0; band < 3; ++band) {
        float half = width * (1 - band * .22f);
        rect(light.x - half, float(y), half * 2, 2,
             light.color | Uint8((1 - t * .55f) * (band == 0 ? 3 : 2) * light.strength));
      }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    rect(light.x + 3, light.floor - 3, 8, 3, 0x09151EFF);
  }
}
void Renderer::surfaceLights(const Game &g, float camera) {
  for (const auto &platform : g.level().platforms) {
    const auto &box = platform.box;
    if (box.x + box.w < camera || box.x > camera + W)
      continue;
    for (const auto &light : lights) {
      if (light.y > box.y || box.y - light.y > 140)
        continue;
      float start = std::max(box.x - camera, light.x - 62);
      float end = std::min(box.x + box.w - camera, light.x + 62);
      if (start >= end)
        continue;
      // Horizontal broken highlights stay clipped to solid ground, never gaps.
      SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
      for (int row = 0; row < std::min(9, int(box.h)); ++row) {
        float reach = (end - start) * (.32f + .045f * ((row * 7) % 9));
        float center = (start + end) * .5f + std::sin(row * 4.1f) * 5;
        float left = std::max(start, center - reach * .5f);
        float right = std::min(end, center + reach * .5f);
        rect(left, box.y + row, right - left, 1,
             light.color | Uint8((row == 0 ? 68 : 27 - row * 2) * light.strength));
      }
      SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    }
  }
}

void Renderer::drawGame(const Game &g, const ViewState &v) {
  const float alpha = std::clamp(v.interpolation, 0.0f, 1.0f);
  const float camera = between(g.prevCamera, g.camera, alpha);
  const float time = between(g.prevTime, g.time, alpha);
  float sx = v.shake ? std::sin(time * 83) * g.shake : 0,
        sy = v.shake ? std::cos(time * 73) * g.shake * .5f : 0;
  offsetX = sx;
  offsetY = sy;
  collectLights(g, v.input, camera, time, alpha);
  background(g.levelIndex, camera, time);
  lightingPass(g, camera, time, alpha);
  const auto &l = g.level();
  for (auto &p : l.platforms) {
    float x = p.box.x - camera;
    if (x + p.box.w < 0 || x > W)
      continue;
    int idx = l.theme;
    SDL_SetTextureColorMod(props.texture, 107, 140, 160);
    for (float t = 0; t < p.box.w; t += 32) {
      float size = std::min(32.0f, p.box.w - t);
      sprite(props, idx, x + t, p.box.y, size, p.box.h);
    }
    SDL_SetTextureColorMod(props.texture, 255, 255, 255);
    rect(x, p.box.y, p.box.w, p.box.h, 0x0A17263E);
    rect(x, p.box.y, p.box.w, std::min(p.box.h, 10.0f), 0x142531EC);
    line(x, p.box.y, x + p.box.w, p.box.y, 0x799BA4FF);
    line(x, p.box.y + 3, x + p.box.w, p.box.y + 3, 0x3A555FC0);
    for (float joint = 32; joint < p.box.w; joint += 64)
      line(x + joint, p.box.y + 1, x + joint - 2, p.box.y + 8, 0x0A141D98);
    if (p.oneWay)
      line(x, p.box.y + p.box.h, x + p.box.w, p.box.y + p.box.h, INK);
  }
  surfaceLights(g, camera);
  for (auto &h : l.hazards) {
    float x = h.x - camera;
    if (x + h.w < 0 || x > W)
      continue;
    bool active = g.hazardOn(h);
    rect(x, h.y + h.h - 3, h.w, 3, h.kind == 0 ? 0x7EC65DFF : GOLD);
    if (active) {
      if (h.kind == 0) {
        rect(x, h.y, h.w, h.h, 0x93DB5088);
        for (int j = 0; j < int(h.w); j += 9)
          rect(x + j, h.y - 2 + std::sin(time * 8 + j) * 2, 3, 3, 0xC8EF75FF);
      } else if (h.kind == 1) {
        for (int j = 0; j < int(h.w); j += 4) {
          float hh = h.h * (.6f + .4f * std::sin(time * 17 + j));
          rect(x + j, h.y + h.h - hh, 3, hh, 0xFF8A39BB);
          rect(x + j + 1, h.y + h.h - hh * .7f, 1, hh * .7f, 0xFFEBA6FF);
        }
      } else {
        for (int j = 0; j < 5; j++) {
          float y = h.y + j * h.h / 5;
          line(x + h.w / 2 + std::sin(time * 25 + j) * 6, y,
               x + h.w / 2 + std::sin(time * 25 + j + 1) * 6, y + h.h / 5, TEAL);
        }
      }
    } else {
      for (int j = 0; j < h.w; j += 8)
        rect(x + j, h.y + h.h - 2, 4, 2, 0xA97F42FF);
    }
  }
  for (auto &p : g.props)
    if (!p.dead && p.x > camera - 40 && p.x < camera + W + 40)
      sprite(props, p.kind == 7 ? 6 : 7, p.x - camera - 14, p.y - 28, 28, 28);
  for (float cp : l.checkpoints)
    if (cp > camera - 25 && cp < camera + W + 25) {
      sprite(props, 10, cp - camera - 9, 202, 18, 30);
      if (cp <= g.checkpoint)
        rect(cp - camera - 2, 204, 4, 3, TEAL);
    }
  for (auto &i : g.items) {
    if (i.x < camera - 40 || i.x > camera + W + 40)
      continue;
    float x = i.x - camera;
    if (i.kind == 0) {
      if (i.used && i.anim >= 2) continue;
      float feet = g.floorAt(i.x, i.y);
      float run = i.used ? std::max(0.0f, i.anim - .85f) * 40 : 0;
      for (const auto &platform : g.level().platforms)
        if (platform.box.y == feet && i.x >= platform.box.x && i.x <= platform.box.x + platform.box.w)
          run = std::min(run, std::max(0.0f, platform.box.x + platform.box.w - i.x - 12));
      x += run;
      actorLight(enemies, x, feet - 21, false);
      contactShadow(x, feet, feet, 13);
      groundedSprite(enemies, workerFrame(i.used, i.used ? i.anim : time),
                     x - 21.5f, feet - 43, 43, 43, false, 0,
                     i.used ? uint8_t(255 * std::clamp((2 - i.anim) / .65f, 0.0f, 1.0f)) : 255);
      SDL_SetTextureColorMod(enemies.texture, 255, 255, 255);
    } else if (!i.used) {
      float y = i.y + std::sin(time * 4 + i.x) * 2;
      sprite(props, i.kind == 4 ? 9 : 8, x - 11, y - 10, 22, 20);
      const char *label = i.kind == 1   ? "H"
                          : i.kind == 2 ? "S"
                          : i.kind == 3 ? "R"
                          : i.kind == 4 ? "G"
                          : i.kind == 6 ? "F"
                          : i.kind == 9 ? "L"
                                        : "$";
      text(label, x - 2, y - 5, 1, CREAM);
    }
  }
  if (g.vehicleAvailable && g.vehicleX > camera - 80 && g.vehicleX < camera + W + 80) {
    float feet = g.floorAt(g.vehicleX);
    actorLight(vehicle, g.vehicleX - camera, feet - 30);
    contactShadow(g.vehicleX - camera, feet, feet, 27);
    int frame = g.vehicleHatch > 0 ? 8 + std::clamp(int((.36f - g.vehicleHatch) / .36f * 4), 0, 3) : 0;
    groundedSprite(vehicle, frame, g.vehicleX - camera - 38, feet - 76, 76, 76);
    SDL_SetTextureColorMod(vehicle.texture, 255, 255, 255);
    if (std::fabs(g.player.x - g.vehicleX) < 48)
      text("E / TRIANGLE", g.vehicleX - camera - 32, 155, 1, GOLD);
  }
  for (const auto &source : g.enemies) {
    auto e = source;
    e.x = between(source.prevX, source.x, alpha);
    e.y = between(source.prevY, source.y, alpha);
    if ((e.dead && e.death <= 0) || !e.active || e.x < camera - 60 || e.x > camera + W + 60)
      continue;
    float deathT = e.dead ? 1.0f - std::clamp(e.death / .45f, 0.0f, 1.0f) : 0.0f;
    // Full source poses: walk 0..3, attack 4, hurt 5, collapse 6/7.
    int frame = e.dead
                    ? (e.kind == 5 ? std::min(1, int(deathT * 2))
                                   : 6 + std::min(1, int(deathT * 2)))
                    : e.hurt > 0 ? 5
                    : e.state == 1 ? 3
                    : e.state == 2 ? 4
                                    : int(time * 8 + e.origin) % 4;
    int idx = e.kind * 8 + frame;
    float h = e.kind == 3 ? 40 : 43, w = h;
    float drawW = w, drawH = h;
    float yy = e.y - drawH - (e.dead ? std::sin(deathT * 3.14159f) * 12.0f : 0.0f);
    double angle = 0;
    uint8_t alpha = e.dead ? uint8_t(e.death / .45f * 255) : 255;
    if (!e.dead)
      contactShadow(e.x - camera, e.y, g.floorAt(e.x, e.y - 2), drawW * .34f);
    else {
      uint8_t shadowAlpha = uint8_t(std::clamp((1.0f - deathT) * 80.0f, 0.0f, 80.0f));
      rect(e.x - camera - drawW * .34f, e.y - 2, drawW * .68f, 2,
           0x08131A00u | shadowAlpha);
    }
    actorLight(enemies, e.x - camera, e.y - drawH * .5f, e.hurt > 0);
    groundedSprite(enemies, idx, e.x - camera - drawW / 2, yy, drawW, drawH, e.dir > 0, angle, alpha);
    SDL_SetTextureColorMod(enemies.texture, 255, 255, 255);
    if (e.state == 1 && !e.dead) {
      text("!", e.x - camera - 2, yy - 10, 1, GOLD);
    }
  }
  drawBoss(g, camera, alpha);
  auto p = g.player;
  p.anim = std::max(0.0f, p.anim - DT * (1 - alpha));
  float playerX = p.prevX + (p.x - p.prevX) * alpha;
  float playerY = p.prevY + (p.y - p.prevY) * alpha;
  // Use interpolated coordinates for every presentation anchor, including
  // flashes. This keeps a 120 Hz desktop render from showing the effect one
  // fixed-step behind the sprite.
  p.x = playerX;
  p.y = playerY;
  float px = playerX - camera;
  if (p.vehicleDeath > 0) {
    float fade = std::clamp(p.vehicleDeath / .48f, 0.0f, 1.0f);
    int frame = 12 + std::min(3, int((.48f - p.vehicleDeath) * 9.0f));
    float feet = between(p.prevVehicleDeathY, p.vehicleDeathY, alpha);
    actorLight(vehicle, p.vehicleDeathX - camera, feet - 28, false);
    contactShadow(p.vehicleDeathX - camera, feet, g.floorAt(p.vehicleDeathX, feet - 2), 27);
    groundedSprite(vehicle, frame, p.vehicleDeathX - camera - 38,
                   feet - 76, 76, 76, p.vehicleDeathDir < 0, 0,
                   uint8_t(fade * 255.0f));
  }
  float playerShadowW = p.vehicleHP ? 38.0f : (p.grounded ? 22.0f : 14.0f);
  contactShadow(px, playerY, g.floorAt(playerX, playerY - 2), playerShadowW * .65f);
  for (const auto *atlas : {&hero, &aim, &vehicle})
    actorLight(*atlas, px, playerY - 24, p.hitFlash > 0);
  if (g.debugInvincible || p.inv <= 0 || int(p.inv * 15) % 2 == 0) {
    if (p.vehicleHP) {
      // Keep the firing pose for the complete weapon cooldown, even if the
      // button is released on the same render frame as the shot.
      int frame = p.hitFlash > 0 ? 12
                  : g.vehicleHatch > 0 ? 8 + std::clamp(int((.36f - g.vehicleHatch) / .36f * 4), 0, 3)
                  : p.shot > 0 ? 4 + std::clamp(int(p.fireAge / .082f * 4), 0, 3)
                  : std::fabs(p.vx) > 1 ? int(p.stride * 4) % 4 : 0;
      // Vehicle frames already include suspension motion. Moving the whole
      // destination quad made the wheels dip through the platform.
      groundedSprite(vehicle, frame, px - 38, playerY - 76, 76, 76, p.dir < 0);
    } else {
      // Every normal player pose shares one 48x48 ground box. The stable
      // baseline prevents visual size pops when switching between aim, fire,
      // grenade, melee and jump poses.
      constexpr float playerW = 48.0f, playerH = 48.0f;
      float w = playerW, h = playerH;
      // Authored poses provide the body motion. Moving the entire grounded
      // quad down for breathing/landing buries the feet below the floor.
      float y = playerY - h;
      static constexpr int idleFrames[] = {0, 1, 2, 3, 2, 1};
      int frame = 8 + idleFrames[int(p.anim * 5) % 6];
      // Rotating the complete 48px cell also rotates the feet and exposes the
      // transparent corner; impact motion is represented by the authored hurt
      // pose and the physics arc instead.
      double angle = 0;
      if (g.status == Status::Dying) {
        float deathT = std::clamp(1.0f - g.deathTimer, 0.0f, 1.0f);
        static constexpr int deathFrames[] = {48, 49, 50, 51, 52, 53, 54, 54};
        frame = deathFrames[std::min(7, int(deathT * 8))];
        y = playerY - h;
        angle = 0;
      } else if (p.action > 0 && p.actionKind == 1) {
        frame = 40 + std::clamp(int((.42f - p.action) / .42f * 8.0f), 0, 7);
      } else if (p.action > 0) {
        frame = 32 + std::clamp(int((.45f - p.action) * 18), 0, 7);
      } else if (v.input.up || (v.input.down && !p.grounded)) {
        int idx = v.input.up
                      ? (std::fabs(p.vx) > 1 ? 4 + int(p.anim * 12) % 4 : int(p.anim * 7) % 4)
                      : 8 + int(p.anim * 9) % 4;
        // The directional atlas includes the raised gun above a body of the
        // same height as hero. Its larger padded canvas preserves that scale.
        groundedSprite(aim, idx, px - 28, playerY - 56, 56, 56, p.dir < 0);
        frame = -1;
      } else if (!p.grounded) {
        // Row 2 is a mixed transition strip: cell 16 is a crouch settle and
        // cells 17..21 are the airborne arc. Do not use the crouch cell as a
        // jump frame; it makes the character appear to snap into the floor.
        static constexpr int jumpFrames[] = {17, 18, 19, 20, 21};
        int jumpPhase = p.vy < -180 ? 0 : p.vy < -55 ? 1 : p.vy < 30 ? 2 : p.vy < 150 ? 3 : 4;
        frame = jumpFrames[std::clamp(jumpPhase, 0, 4)];
      } else if (p.crouch) {
        // Row 2 cell 16 and row 3 cells 24..30 are the authored crouch set;
        // row 2 cells 17..21 remain reserved for the jump arc.
        if (v.input.shoot || p.shot > 0)
          frame = 26 + int(p.anim * 12) % 5;
        else {
          static constexpr int crouchIdleFrames[] = {16, 24, 25};
          frame = crouchIdleFrames[int(p.anim * 5) % 3];
        }
      } else if (std::fabs(p.vx) > 1 && (v.input.shoot || p.shot > 0)) {
        static constexpr int runFireFrames[] = {56, 57, 58, 59, 60, 61, 62, 62};
        frame = runFireFrames[int(p.stride * 8) % 8];
      } else if (std::fabs(p.vx) > 1) {
        static constexpr int runFrames[] = {0, 1, 2, 3, 4, 5, 6, 6};
        frame = runFrames[int(p.stride * 8) % 8];
      } else if (v.input.shoot || p.shot > 0) {
        frame = 12 + std::min(3, int(p.fireAge * 28));
      }
      if (frame >= 0) {
        groundedSprite(hero, frame, px - w / 2, y, w, h, p.dir < 0, angle);
        SDL_SetTextureColorMod(hero.texture, 255, 255, 255);
      }
      if (p.recoil > 0 && (v.input.shoot || p.shot > 0) && p.action <= 0) {
        auto muzzle = muzzlePoint(p, v.input);
        float fx = muzzle.x - camera, fy = muzzle.y;
        if (muzzle.vertical) {
          rect(fx - 2, fy - 4, 4, 8, CREAM);
          rect(fx - 1, fy + (v.input.down ? 3 : -7), 2, 4, GOLD);
        } else {
          rect(fx - 3, fy - 2, 7, 4, CREAM);
          rect(fx + p.dir * 3 - 2, fy - 1, 4, 2, GOLD);
        }
      }
    }
  }
  for (const auto *atlas : {&hero, &aim, &vehicle})
    SDL_SetTextureColorMod(atlas->texture, 255, 255, 255);
  // Cosmetic smoke is rendered before hostile projectiles so threats stay visible.
  for (const auto &source : g.particles) {
    auto part = source;
    part.x = between(source.prevX, source.x, alpha);
    part.y = between(source.prevY, source.y, alpha);
    if (part.life > 0 && part.x > camera - 100 && part.x < camera + W + 100) {
      float x = part.x - camera, age = 1 - part.life / part.maxlife;
      if (part.kind == 3) {
        int idx = 12 + std::min(3, int(age * 4));
        float s = part.size * (.55f + age * .6f);
        sprite(props, idx, x - s / 2, part.y - s / 2, s, s, false, 0,
               uint8_t(std::min(255.0f, part.life * 600)));
      } else if (part.kind == 1) {
        rect(x - part.size / 2, part.y - part.size / 2, part.size, part.size,
             0xA3B6B000 | uint8_t((1 - age) * 50));
      } else if (part.kind == 2) {
        rect(x - 3, part.y - 1, 6, 2, 0xFFF1BBFF);
        rect(x - 1, part.y - 3, 2, 6, GOLD);
      } else
        rect(x, part.y, part.size * (1 - age) + 1, part.size * (1 - age) + 1,
             age < .3f    ? CREAM
             : age < .65f ? GOLD
                          : 0xB97944CC);
    }
  }
  for (const auto &source : g.bullets) {
    auto b = source;
    b.x = between(source.px, source.x, alpha);
    b.y = between(source.py, source.y, alpha);
    if (b.alive) {
      float x = b.x - camera;
      if (b.kind == 3) {
        rect(x - 3, b.y - 3, 6, 6, b.hostile ? RED : 0x9DB65AFF);
        rect(x - 1, b.y - 4, 2, 2, CREAM);
      } else if (b.hostile) {
        rect(x - 4, b.y - 4, 8, 8, INK);
        rect(x - 3, b.y - 3, 6, 6, b.kind == 5 ? TEAL : RED);
        rect(x - 1, b.y - 1, 2, 2, CREAM);
      } else if (b.kind == 7) {
        bool vertical = std::fabs(b.vy) > std::fabs(b.vx);
        float len = 13 + std::sin(time * 45 + b.x * .03f) * 3;
        rect(x - (vertical ? 2 : len / 2), b.y - (vertical ? len / 2 : 2), vertical ? 4 : len,
             vertical ? len : 4, 0xFF8B36FF);
        rect(x - (vertical ? 1 : len / 2), b.y - (vertical ? len / 2 : 1), vertical ? 2 : len,
             vertical ? len : 2, 0xFFE6A0FF);
      } else if (b.kind == 8) {
        bool vertical = std::fabs(b.vy) > std::fabs(b.vx);
        float len = 17;
        rect(x - (vertical ? 2 : len / 2), b.y - (vertical ? len / 2 : 2), vertical ? 4 : len,
             vertical ? len : 4, TEAL);
        rect(x - (vertical ? 1 : len / 2), b.y - (vertical ? len / 2 : 1), vertical ? 2 : len,
             vertical ? len : 2, CREAM);
      } else {
        float len = b.kind == 2 ? 9 : 5;
        bool vertical = std::fabs(b.vy) > std::fabs(b.vx);
        rect(x - (vertical ? 1 : len / 2), b.y - (vertical ? len / 2 : 1), vertical ? 2 : len,
             vertical ? len : 2, CREAM);
        rect(x - 1, b.y - 1, 2, 2, GOLD);
      }
    }
  }
  foregroundDepth(g.levelIndex, camera, time);
  offsetX = offsetY = 0;
  rect(0, 0, 480, 27, 0x0B1828EC);
  rect(0, 26, 480, 1, 0xC39449FF);
  text("DENIZ", 10, 5, 1, TEAL);
  text("LIVES " + std::to_string(p.lives), 10, 16, 1, CREAM);
  text("HP", 58, 16, 1, CREAM);
  for (int i = 0; i < p.maxHealth; i++)
    rect(72 + i * 5, 16, 4, 5, i < p.health ? TEAL : 0x2C4856FF);
  const char *weapon = p.vehicleHP     ? "SCRAP WALKER"
                       : p.weapon == 0 ? "PISTOL"
                       : p.weapon == 1 ? "HEAVY MG"
                       : p.weapon == 2 ? "SHOTGUN"
                       : p.weapon == 3 ? "ROCKET"
                       : p.weapon == 4 ? "FLAME SHOT"
                                       : "LASER";
  text(weapon, 94, 5, 1, GOLD);
  text(p.weapon == 0 || p.vehicleHP ? "AMMO --" : "AMMO " + std::to_string(p.ammo), 94, 16, 1,
       CREAM);
  text("GRENADES " + std::to_string(p.grenades), 205, 5, 1, CREAM);
  text("RESCUE " + std::to_string(g.rescued) + "/3", 205, 16, 1, TEAL);
  char score[30];
  std::snprintf(score, sizeof(score), "%07d", g.score);
  text(score, 310, 5, 2, CREAM);
  text(std::to_string(g.levelIndex + 1) + "/6", 449, 5, 1, GOLD);
  text("II", 456, 16, 1, CREAM);
  rect(0, 269, 480, 3, 0x0A1525BB);
  rect(0, 269, 480 * p.x / l.width, 2, TEAL);
  if (p.vehicleHP) {
    for (int i = 0; i < 3; i++)
      rect(9 + i * 15, 32, 12, 4, i < p.vehicleHP ? TEAL : 0x2C4856FF);
  }
  if (g.boss.active && !g.boss.dead) {
    rect(95, 35, 290, 16, 0x0A182CE6);
    text(l.bossName, 103, 38, 1, CREAM);
    rect(103, 48, 274, 3, 0x633F3AFF);
    rect(103, 48, 274 * g.boss.hp / g.boss.maxhp, 3, g.boss.phase == 2 ? RED : GOLD);
  } else if (time < 5) {
    text(l.subtitle, 12, 35, 1, GOLD);
    text("MOVE: ARROWS  JUMP: Z  FIRE: X  GRENADE: C", 12, 249, 1, CREAM);
  } else if (g.player.x > l.width * .45f && g.player.x < l.width * .55f) {
    rect(7, 34, 466, 25, 0x071C29DC);
    wrapped(l.radio, 14, 40, 1, 450, TEAL);
  }
  if (v.assist)
    text("TRAINING", 10, 238, 1, TEAL);
  if (g.flash > 0 && v.shake)
    rect(0, 27, 480, 245, 0xF4D8A030);
}
void Renderer::render(const Game &g, const ViewState &v) {
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, 8, 18, 28, 255);
  SDL_RenderClear(r);
  offsetX = offsetY = 0;
  if (v.screen == Screen::Play || v.screen == Screen::Pause || v.screen == Screen::Debrief ||
      v.screen == Screen::Ending) {
    drawGame(g, v);
  } else
    background(v.selected, v.clock * 12, v.clock);
  if (v.screen == Screen::Title) {
    rect(0, 0, 480, 272, 0x06132162);
    rect(0, 0, 480, 7, INK);
    rect(0, 265, 480, 7, INK);
    text("OPERATION / IRON GRID", 27, 30, 1, GOLD);
    text("IRON", 24, 48, 5, 0xA55531FF);
    text("IRON", 22, 45, 5, CREAM);
    text("COAST", 25, 92, 5, 0xA55531FF);
    text("COAST", 22, 88, 5, GOLD);
    text("ONE COAST. SIX FRONTS. ONE LAST SIGNAL.", 26, 135, 1, CREAM);
    text("HAZIRIZ", 383, 30, 1, TEAL);
    sprite(hero, 12 + int(v.clock * 5) % 4, 365, 123, 88, 91);
    const char *opts[] = {"START CAMPAIGN", "CAMPAIGN MAP", "SETTINGS", "CONTROLS", "EXIT"};
    for (int i = 0; i < 5; i++) {
      if (v.menu == i) {
        rect(21, 158 + i * 16, 202, 13, 0x0A1C27E5);
        rect(21, 158 + i * 16, 2, 13, GOLD);
      }
      text((v.menu == i ? "> " : "  ") + std::string(opts[i]), 29, 161 + i * 16, 1,
           v.menu == i ? GOLD : CREAM);
    }
    text("ENTER / X  SELECT", 26, 248, 1, TEAL);
    text("PS VITA + DESKTOP", 337, 249, 1, CREAM);
  }
  if (v.screen == Screen::Map) {
    rect(0, 0, 480, 272, 0x061525E8);
    text("COASTAL FRONT", 18, 14, 3, CREAM);
    text("CAMPAIGN MAP / 6 MISSIONS", 20, 42, 1, TEAL);
    const float xs[] = {45, 119, 193, 267, 341, 423}, ys[] = {96, 121, 89, 123, 91, 112};
    for (int i = 0; i < 5; i++) {
      line(xs[i], ys[i], xs[i + 1], ys[i + 1], 0x7A9576FF);
      line(xs[i], ys[i] + 1, xs[i + 1], ys[i + 1] + 1, 0x314A4DFF);
    }
    for (int i = 0; i < 6; i++) {
      rect(xs[i] - 12, ys[i] - 12, 25, 25, i == v.selected ? GOLD : 0x284451FF);
      rect(xs[i] - 10, ys[i] - 10, 21, 21, INK);
      text(std::to_string(i + 1), xs[i] - 3, ys[i] - 4, 1, i <= v.unlocked ? TEAL : 0x66808CFF);
      if (i == v.selected)
        text("V", xs[i] - 2, ys[i] - 24, 1, GOLD);
    }
    const auto &l = campaign()[v.selected];
    text(l.subtitle, 20, 151, 1, GOLD);
    text(l.name, 20, 166, 2, CREAM);
    for (auto &p : l.platforms) {
      float x = 20 + p.box.x / l.width * 435, y = 197 + p.box.y / 232 * 26;
      rect(x, y, std::max(1.0f, p.box.w / l.width * 435), p.oneWay ? 1 : 4, p.oneWay ? GOLD : TEAL);
    }
    for (auto &e : l.spawns)
      rect(20 + e.x / l.width * 435, 196 + e.y / 232 * 26, 2, 2, RED);
    text("< > SELECT   ENTER / X START   ESC BACK", 20, 245, 1, CREAM);
    if (v.selected > v.unlocked && !v.assist)
      text("COMPLETE THE PREVIOUS MISSION", 20, 234, 1, RED);
  }
  if (v.screen == Screen::Brief) {
    rect(0, 0, 480, 272, 0x071522BA);
    rect(18, 22, 444, 230, 0x091A26E9);
    line(18, 22, 462, 22, GOLD);
    auto &l = campaign()[v.selected];
    text(l.subtitle, 33, 36, 1, TEAL);
    text(l.name, 33, 55, 2, GOLD);
    wrapped(l.brief1, 33, 91, 1, 405, CREAM);
    wrapped(l.brief2, 33, 132, 1, 405, CREAM);
    text("OBJECTIVE: " + l.bossName, 33, 185, 1, RED);
    text("RESCUE 3 WORKERS / DESTROY THE COMMAND NODE", 33, 202, 1, TEAL);
    text("ENTER / X  DEPLOY", 33, 231, 1, GOLD);
  }
  if (v.screen == Screen::Pause) {
    rect(0, 0, 480, 272, 0x071320C8);
    text("PAUSED", 167, 79, 4, CREAM);
    text("ENTER / START   RESUME", 149, 131, 1, GOLD);
    text("M   TOGGLE SOUND", 149, 152, 1, CREAM);
    text("ESC / O   MAIN MENU", 149, 173, 1, CREAM);
  }
  if (v.screen == Screen::Play && g.status == Status::GameOver) {
    rect(0, 0, 480, 272, 0x101523CB);
    text("SIGNAL LOST", 61, 89, 3, RED);
    text("ENTER / X  CONTINUE FROM CHECKPOINT", 92, 140, 1, CREAM);
    text("ESC / O  MAIN MENU", 182, 160, 1, GOLD);
  }
  if (v.screen == Screen::Debrief) {
    rect(0, 0, 480, 272, 0x071A27EB);
    text("MISSION COMPLETE", 35, 31, 3, GOLD);
    wrapped(g.level().ending, 35, 78, 1, 405, CREAM);
    text("SCORE  " + std::to_string(g.score), 35, 128, 2, CREAM);
    text("RESCUED  " + std::to_string(g.rescued) + " / 3", 35, 153, 1, TEAL);
    text("ENEMIES  " + std::to_string(g.kills), 35, 172, 1, CREAM);
    text("ENTER / X  NEXT MISSION", 35, 228, 1, GOLD);
  }
  if (v.screen == Screen::Ending) {
    rect(0, 0, 480, 272, 0x071A27E8);
    text("THE COAST IS OURS", 25, 39, 3, GOLD);
    wrapped("The Iron Grid is silent. Deniz and Efe board the last evacuation ship. The coast lights "
            "flicker on one by one.",
            30, 91, 1, 420, CREAM);
    wrapped(g.totalRescued >= 15
                ? "The rescued workers will rebuild the shipyard. No machine will silence human voices "
                  "again."
                : "People are still waiting behind the storm. Return for everyone on the next campaign.",
            30, 141, 1, 420, TEAL);
    text("CAMPAIGN SCORE " + std::to_string(g.score), 30, 199, 1, GOLD);
    text("RESCUE " + std::to_string(g.totalRescued) + " / 18", 30, 217, 1, CREAM);
    text("ENTER / X   MAIN MENU", 30, 245, 1, CREAM);
  }
  if (v.screen == Screen::Options) {
    rect(0, 0, 480, 272, 0x071A27E8);
    text("SETTINGS", 27, 30, 3, GOLD);
    std::vector<std::string> opts = {
        std::string("SOUND: ") + (v.muted ? "OFF" : "ON"),
        std::string("SCREEN SHAKE: ") + (v.shake ? "ON" : "OFF"),
        std::string("MODE: ") + (v.assist ? "TRAINING / NO DAMAGE" : "ARCADE / 3 HP"),
        std::string("FULLSCREEN: ") + (v.fullscreen ? "ON" : "OFF"), "BACK"};
    for (int i = 0; i < 5; i++)
      text((v.menu == i ? "> " : "  ") + opts[i], 30, 91 + i * 25, 1, i == v.menu ? GOLD : CREAM);
    text("TRAINING MODE DOES NOT UNLOCK THE CAMPAIGN.", 30, 241, 1, TEAL);
  }
  if (v.screen == Screen::Controls) {
    rect(0, 0, 480, 272, 0x071A27ED);
    text("CONTROLS", 25, 24, 3, GOLD);
    const char *rows[] = {"MOVE         ARROWS / WASD       D-PAD / LEFT STICK",
                          "JUMP         Z / SPACE          X (CROSS)",
                          "FIRE         X / J              SQUARE",
                          "GRENADE      C / K              CIRCLE / R",
                          "VEHICLE      E                  TRIANGLE",
                          "AIM UP       UP + FIRE          UP + SQUARE",
                          "AIM DOWN     AIR DOWN + FIRE     AIR DOWN + SQUARE",
                          "PAUSE        ESC                START"};
    for (int i = 0; i < 8; i++)
      text(rows[i], 23, 75 + i * 18, 1, CREAM);
    text("ENTER / O / ESC  BACK", 25, 244, 1, TEAL);
  }
  if (v.screen == Screen::Play || v.screen == Screen::Pause) {
    // Keep small character details clear; only the frame edges are shaded.
    for (int i = 0; i < 8; i++) {
      uint32_t shade = 0x020A1014 | uint32_t((18 - i * 2) & 255);
      rect(0, 27 + i, 480, 1, shade);
      rect(0, 264 - i, 480, 1, shade);
      rect(i, 27, 1, 237, shade);
      rect(479 - i, 27, 1, 237, shade);
    }
  }
}
void Renderer::screenshot(const std::string &path) {
  int w, h;
  SDL_GetRendererOutputSize(r, &w, &h);
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
  if (SDL_RenderReadPixels(r, nullptr, s->format->format, s->pixels, s->pitch) == 0)
    stbi_write_png(path.c_str(), w, h, 4, s->pixels, s->pitch);
  SDL_FreeSurface(s);
}
} // namespace kh
