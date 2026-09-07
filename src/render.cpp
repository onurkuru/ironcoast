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
  hero = load("hero.png", 8, 4, false);
  enemies = load("enemies.png", 4, 6, false);
  worlds = load("worlds.png", 2, 3);
  machines = load("machines.png", 3, 3, false);
  props = load("props.png", 4, 4, false);
  aim = load("aim.png", 4, 3, false, true);
  melee = load("melee.png", 2, 2, false, true);
}
Renderer::~Renderer() {
  for (auto *a : {&hero, &enemies, &worlds, &machines, &props, &aim, &melee})
    SDL_DestroyTexture(a->texture);
}
Atlas Renderer::load(const std::string &name, int cols, int rows, bool trim, bool paperKey) {
  Atlas a;
  a.cols = cols;
  a.rows = rows;
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
      int margin = name == "props.png" ? 6 : 1;
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
      a.cells.push_back({left, top, right - left + 1, bottom - top + 1});
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
  idx = std::max(0, std::min(idx, int(a.cells.size() - 1)));
  SDL_Rect src = a.cells[idx];
  SDL_FRect dest{x + offsetX, y + offsetY, w, h};
  SDL_SetTextureAlphaMod(a.texture, alpha);
  SDL_RenderCopyExF(r, a.texture, &src, &dest, angle, nullptr,
                    flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  SDL_SetTextureAlphaMod(a.texture, 255);
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
void Renderer::drawBoss(const Game &g, float camera, float alpha) {
  const auto &b = g.boss;
  if (!b.active)
    return;
  const int k = g.level().bossKind;
  const auto pose = bossPose(b, k, alpha);
  float cx = between(b.prevX, b.x, alpha) - camera;
  float cy = between(b.prevY, b.y, alpha);
  // Stable foot anchors compensate for padding in the six authored cells.
  static constexpr float basePadding[] = {6, 11, 11, 1.5f, 0, 1.5f};
  float x = cx - 76, y = cy - 128 + basePadding[k];
  float gait = pose.stride;
  uint8_t opacity = b.dead ? uint8_t(std::clamp(b.death * 180, 0.0f, 255.0f)) : 255;
  ring(cx, 231, k == 4 ? 42 : 64, 5, 0x06131A88);
  if (b.state == BossState::Windup && !b.dead) {
    float target = b.targetX - camera;
    bool groundStrike = (k == 0 && b.pattern % 2) || k == 3 || (k == 5 && b.pattern % 2);
    if (groundStrike) {
      ring(target, 230, 24 + pose.charge * 18, 5, RED);
      line(target, 221, target, 236, GOLD);
      text("MOVE!", target - 14, 182, 1, GOLD);
    } else if (k == 1 && b.pattern % 2) {
      for (int j = 0; j < 7; j++)
        text("<", cx - 68 - j * 16, 224, 1, RED);
    }
  }
  if (b.hurt > 0)
    SDL_SetTextureColorMod(machines.texture, 255, 180, 150);
  // Render joints from the existing authored atlas. Parts share their source
  // proportions and move about local pivots; no whole-sprite scale animation.
  auto piece = [&](Rect source, float dx = 0, float dy = 0, float angle = 0,
                   float pivotX = .5f, float pivotY = .5f) {
    spritePart(machines, k, source, x + source.x * 152 + dx, y + source.y * 128 + dy,
               source.w * 152, source.h * 128, angle, pivotX, pivotY, opacity);
  };
  float bodyY = pose.lift;
  float debris = pose.collapse * 17;
  if (k == 0 || k == 3 || k == 5) {
    // Feet alternate their swing and planted phases; the upper hull rides the suspension.
    piece({0, .68f, .5f, .32f}, gait * 4 - debris, -std::max(0.0f, gait) * 5 + debris,
          gait * 8 - pose.collapse * 30, .65f, 0);
    piece({.5f, .68f, .5f, .32f}, -gait * 4 + debris, -std::max(0.0f, -gait) * 5 + debris,
          -gait * 8 + pose.collapse * 30, .35f, 0);
    if (k == 0) {
      piece({0, 0, .55f, .40f}, 0, bodyY, pose.weapon * .3f + pose.lean, .95f, .9f);
      piece({.55f, 0, .45f, .40f}, pose.kick * 2, bodyY, pose.lean);
      piece({0, .40f, .21f, .28f}, pose.weapon * .13f, bodyY - pose.charge * 12,
            std::sin(b.age * 2.4f) * 6 + pose.weapon * .4f, .5f, 0);
      piece({.21f, .40f, .79f, .29f}, pose.kick * 2, bodyY, pose.lean);
    } else if (k == 3) {
      piece({0, 0, 1, .38f}, 0, bodyY, pose.lean);
      piece({.32f, .38f, .68f, .31f}, 0, bodyY, pose.lean);
      piece({0, .38f, .32f, .30f}, -debris, bodyY - pose.charge * 6,
            pose.weapon, .9f, .1f);
    } else {
      piece({0, 0, 1, .34f}, 0, bodyY, pose.lean);
      piece({.4f, .34f, .6f, .35f}, 0, bodyY, pose.lean);
      piece({0, .34f, .4f, .34f}, pose.kick * 8 - debris, bodyY,
            pose.weapon * .35f, .95f, .5f);
    }
  } else if (k == 1 || k == 2) {
    piece({0, .72f, 1, .20f}, 0, 0);
    if (k == 2) {
      piece({0, 0, 1, .46f}, pose.kick * 9, bodyY, pose.weapon * .45f, .72f, .85f);
      piece({0, .46f, 1, .27f}, 0, bodyY * .4f);
    } else {
      piece({.37f, 0, .63f, .73f}, pose.kick * 2, bodyY);
      piece({0, 0, .37f, .73f}, -pose.charge * 3 - debris, bodyY,
            std::sin(b.age * (b.state == BossState::Attack ? 90 : 24)) * 2.5f, .9f, .7f);
    }
    if (!b.dead) {
      float scroll = std::fmod(b.gait * 40, 12.0f);
      for (int j = 0; j < 8; j++)
        rect(x + 36 + j * 12 - scroll, cy - 5, 5, 2, 0xBF995AFF);
    }
  } else {
    piece({.31f, 0, .38f, 1}, 0, bodyY, pose.lean);
    piece({0, 0, .32f, 1}, -pose.charge * 4 - debris, bodyY + gait * 4,
          -pose.charge * 8 - gait * 3, 1, .5f);
    piece({.68f, 0, .32f, 1}, pose.charge * 4 + debris, bodyY - gait * 4,
          pose.charge * 8 + gait * 3, 0, .5f);
    if (!b.dead) {
      float jet = 9 + std::sin(b.age * 43) * 4;
      rect(cx - 4, cy - 7, 8, jet, 0x37CCE6AA);
      rect(cx - 2, cy - 7, 4, jet * .7f, CREAM);
    }
  }
  SDL_SetTextureColorMod(machines.texture, 255, 255, 255);
  if (b.dead)
    return;
  if (pose.charge > 0) {
    ring(cx, cy - 60 + bodyY, 15 + (1 - pose.charge) * 16, 12 + (1 - pose.charge) * 12,
         k == 4 ? TEAL : GOLD);
    text(b.state == BossState::Overload ? "OVERDRIVE" : "!", cx - 24, y - 9, 1, GOLD);
  }
  if (pose.core > 0) {
    ring(cx, cy - 60 + bodyY, 10 + pose.core * 6, 10 + pose.core * 6, TEAL);
    text("CORE OPEN", cx - 26, y - 9, 1, TEAL);
  }
  if (pose.kick > 0) {
    float muzzleX = cx - 49, muzzleY = cy - 51 + bodyY;
    rect(muzzleX - 13 * pose.kick, muzzleY - 2, 13 * pose.kick, 4, GOLD);
    rect(muzzleX - 8 * pose.kick, muzzleY - 1, 8 * pose.kick, 2, CREAM);
  }
  if (b.impact > .3f)
    ring(cx - 35, 231, (1 - b.impact) * 50 + 12, 3, 0xEAB66CAA);
}
void Renderer::background(int theme, float camera, float time) {
  float travel = camera * .16f + (theme == 2 ? time * 28 : 0);
  float drift = std::fmod(travel, W);
  bool mirrored = int(travel / W) % 2;
  sprite(worlds, theme, -drift, 0, W, H, mirrored);
  sprite(worlds, theme, W - drift, 0, W, H, !mirrored);
  rect(0, 0, W, H, 0x0A182A28);
  // Slow second-depth silhouettes keep scrolling distinct from the far scenery.
  for (int i = 0; i < 6; i++) {
    float x = i * 121 - std::fmod(camera * .38f, 121.0f);
    line(x, 0, x, 35 + std::sin(float(i)) * 12, 0x14263080);
  }
  // Midground silhouettes give the painted panels a second depth layer. They
  // scroll slower than the gameplay plane, making camera motion feel richer
  // without changing collision geometry.
  for (int i = 0; i < 11; i++) {
    float x = std::fmod(i * 92.0f - camera * .62f + 960.0f, 560.0f) - 40.0f;
    float h = 10.0f + std::fmod(i * 17.0f, 25.0f);
    rect(x, 211 - h, 24 + (i % 3) * 9, h, theme == 1 ? 0x132D36A0 : 0x101E2FA0);
    if (i % 2 == 0)
      rect(x + 5, 211 - h - 6, 2, 6, 0x172A35A0);
  }
  // Small practical lights make the coast feel occupied instead of frozen.
  for (int i = 0; i < 9; i++) {
    float x = std::fmod(i * 67.0f - camera * .78f + 700.0f, 540.0f) - 20.0f;
    bool lit = (int(time * 2.0f) + i + theme) % 4 != 0;
    if (lit)
      rect(x, 202 + (i % 3) * 3, 2, 2, theme == 1 ? 0xB6E56A99 : 0xF4B64A99);
  }
  if (theme == 1) {
    for (int i = 0; i < 5; i++)
      rect(std::fmod(i * 117 + time * 7, 600.0f) - 60, 175 + i * 9, 150, 3, 0x6CBAA516);
  }
  if (theme == 4 || theme == 5) {
    for (int i = 0; i < 75; i++) {
      float x = std::fmod(i * 71.37f - time * 90 + 40000, 500.0f),
            y = std::fmod(i * 43.13f + time * 175, 280.0f);
      line(x, y, x - 3, y + 9, 0x87CCDC5A);
    }
  }
  if (theme == 3) {
    for (int i = 0; i < 20; i++) {
      float x = std::fmod(i * 53.1f + time * 9, 480.0f),
            y = 272 - std::fmod(i * 19.7f + time * 22, 260.0f);
      rect(x, y, 1, 2, 0xFCAA5977);
    }
  }
  if (theme == 0 || theme == 5) {
    // Warm coastal haze catches the sunset palette while preserving the
    // high-contrast silhouettes used for gameplay readability.
    for (int i = 0; i < 4; i++)
      rect(0, 142 + i * 17, W, 2, 0xD07A3510);
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
  background(g.levelIndex, camera, time);
  const auto &l = g.level();
  for (auto &p : l.platforms) {
    float x = p.box.x - camera;
    if (x + p.box.w < 0 || x > W)
      continue;
    int idx = l.theme;
    for (float t = 0; t < p.box.w; t += 32) {
      float size = std::min(32.0f, p.box.w - t);
      sprite(props, idx, x + t, p.box.y, size, p.box.h);
    }
    line(x, p.box.y, x + p.box.w, p.box.y, 0xDEC494FF);
    if (p.oneWay)
      line(x, p.box.y + p.box.h, x + p.box.w, p.box.y + p.box.h, INK);
  }
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
      if (!i.used)
        sprite(enemies, 20 + int(time * 2) % 2, x - 18, i.y - 27, 36, 43);
      else if (i.anim < 2)
        sprite(enemies, i.anim < .5f ? 22 : 23, x - 18 + i.anim * 40, i.y - 27, 36, 43, false, 0,
               uint8_t(255 * (1 - i.anim / 2)));
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
    sprite(machines, 6, g.vehicleX - camera - 35, g.floorAt(g.vehicleX) - 62, 70, 62);
    if (std::fabs(g.player.x - g.vehicleX) < 48)
      text("E / TRIANGLE", g.vehicleX - camera - 32, 155, 1, GOLD);
  }
  for (const auto &source : g.enemies) {
    auto e = source;
    e.x = between(source.prevX, source.x, alpha);
    e.y = between(source.prevY, source.y, alpha);
    if ((e.dead && e.death <= 0) || !e.active || e.x < camera - 60 || e.x > camera + W + 60)
      continue;
    int frame = e.state == 1 ? 1 : e.state == 2 ? 2 : int(time * 7 + e.origin) % 4;
    int idx = e.kind * 4 + frame;
    float h = e.kind == 3   ? 31
              : e.kind == 4 ? 31
                            : 43,
          w = e.kind == 3   ? 40
              : e.kind == 4 ? 43
                            : 40;
    float deathT = e.dead ? 1.0f - std::clamp(e.death / .45f, 0.0f, 1.0f) : 0.0f;
    float drawW = e.dead ? w * (1.0f + deathT * .16f) : w;
    float drawH = e.dead ? h * (1.0f - deathT * .14f) : h;
    float yy = e.y - drawH - (e.dead ? std::sin(deathT * 3.14159f) * 12.0f : 0.0f);
    double angle = e.dead ? deathT * 100 * e.dir : 0;
    uint8_t alpha = e.dead ? uint8_t(e.death / .45f * 255) : 255;
    if (!e.dead)
      rect(e.x - camera - drawW * .34f, e.y - 2, drawW * .68f, 2, 0x08131A66);
    else
      rect(e.x - camera - drawW * .34f, e.y - 2, drawW * .68f, 2,
           0x08131A66 | uint8_t((1.0f - deathT) * 80));
    if (e.hurt > 0)
      SDL_SetTextureColorMod(enemies.texture, 255, 170, 120);
    sprite(enemies, idx, e.x - camera - drawW / 2, yy, drawW, drawH, e.dir > 0, angle, alpha);
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
  float px = playerX - camera;
  float playerShadowW = p.vehicleHP ? 38.0f : (p.grounded ? 22.0f : 14.0f);
  rect(px - playerShadowW / 2, playerY - 1, playerShadowW, 2, 0x06131A88);
  if (p.inv <= 0 || int(p.inv * 15) % 2 == 0) {
    if (p.vehicleHP) {
      // Keep the firing pose for the complete weapon cooldown, even if the
      // button is released on the same render frame as the shot.
      int frame = (v.input.shoot || p.shot > 0) ? 8 : 6 + int(p.anim * 9) % 2;
      float bob = std::sin(p.anim * 13) * (std::fabs(p.vx) > 1 ? 1.2f : .35f);
      sprite(machines, frame, px - 38, playerY - 66 + bob, 76, 66, p.dir < 0);
    } else {
      // Every normal player pose shares one 48x48 ground box. The stable
      // baseline prevents visual size pops when switching between aim, fire,
      // grenade, melee and jump poses.
      constexpr float playerW = 48.0f, playerH = 48.0f;
      float w = playerW, h = playerH;
      float bob = p.land > 0 ? std::sin((p.land / .16f) * 3.14159f) * 1.5f
                             : (std::fabs(p.vx) > 1 && p.grounded ? std::sin(p.anim * 20) * 1.1f
                                                                   : std::sin(p.anim * 4) * .35f);
      float y = playerY - h + bob;
      static constexpr int idleFrames[] = {0, 1, 2, 3, 2, 1};
      int frame = 8 + idleFrames[int(p.anim * 5) % 6];
      double angle = p.hitFlash > 0 ? std::sin(time * 90) * 3
                                    : (p.recoil > 0 ? -p.dir * 2.0 : 0.0);
      if (g.status == Status::Dying) {
        float deathT = std::clamp(1.0f - g.deathTimer, 0.0f, 1.0f);
        frame = 28 + std::min(3, int((1 - g.deathTimer) * 5));
        w = playerW * (1.0f + deathT * .18f);
        h = playerH * (1.0f - deathT * .16f);
        y = playerY - h - std::sin(deathT * 3.14159f) * 9.0f;
        angle = std::sin(time * 26) * 8;
      } else if (p.action > 0 && p.actionKind == 1) {
        int meleeFrame = std::min(4, int((.42f - p.action) * 12));
        if (meleeFrame < 4) {
          // Anticipation, contact and follow-through use the four original
          // wrench frames; the fifth phase is a readable recovery pose.
          sprite(melee, meleeFrame, px - w / 2, y, w, h, p.dir < 0,
                 meleeFrame == 1 ? -p.dir * 5.0 : meleeFrame == 2 ? p.dir * 4.0 : 0.0);
          frame = -1;
        } else {
          frame = 8 + int(p.anim * 5) % 4;
        }
      } else if (p.action > 0) {
        frame = 24 + std::min(3, int((.45f - p.action) * 9));
      } else if (v.input.up || (v.input.down && !p.grounded)) {
        int idx = v.input.up
                      ? (std::fabs(p.vx) > 1 ? 4 + int(p.anim * 12) % 4 : int(p.anim * 7) % 4)
                      : 8 + int(p.anim * 9) % 4;
        sprite(aim, idx, px - w / 2, y, w, h, p.dir < 0);
        frame = -1;
      } else if (!p.grounded) {
        frame = 16 + (p.vy < -140 ? 1 : p.vy < 30 ? 2 : 3);
      } else if (p.crouch) {
        frame = 20 + (v.input.shoot ? 2 + int(p.anim * 10) % 2 : 1);
      } else if (std::fabs(p.vx) > 1) {
        frame = int(p.stride * 8) % 8;
      } else if (v.input.shoot || p.shot > 0) {
        frame = 12 + std::min(3, int(p.fireAge * 28));
      }
      if (frame >= 0) {
        if (p.hitFlash > 0)
          SDL_SetTextureColorMod(hero.texture, 255, 190, 155);
        sprite(hero, frame, px - w / 2, y, w, h, p.dir < 0, angle);
        SDL_SetTextureColorMod(hero.texture, 255, 255, 255);
      }
      if (p.recoil > 0 && (v.input.shoot || p.shot > 0) && p.action <= 0) {
        float fx = px + p.dir * 24, fy = playerY - 27;
        rect(fx - 3, fy - 2, 7, 4, CREAM);
        rect(fx + p.dir * 3 - 2, fy - 1, 4, 2, GOLD);
      }
    }
  }
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
    // A restrained CRT pass keeps the pixel art cohesive at the 2x desktop
    // scale and is cheap enough for the Vita renderer.
    for (int y = 29; y < 269; y += 4)
      rect(0, y, 480, 1, 0x07131E18);
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
