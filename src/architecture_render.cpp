#include "render.h"
#include "animation.h"
#include <algorithm>
#include <cmath>

namespace kh {
void Renderer::architecture(const Game &g, float camera, float time) {
  const uint32_t warm = g.levelIndex == 3 ? 0xF4A566FF : 0xCDAA75FF;
  for (const auto &b : g.level().buildings) {
    float x = b.box.x - camera, y = b.box.y, w = b.box.w, h = b.box.h;
    if (x + w < -30 || x > W + 30) continue;
    if (b.style == 7) {
      // Elevated pipe bridges have supports down to the service road.
      if (y < 232) {
        for (float k = 10; k < w; k += 70) {
          rect(x + k, y + 8, 5, 232 - y - 8, 0x2A424AFF);
          line(x + k, y + 8, x + std::min(k + 60, w), y + 27, 0x627C7FFF);
        }
        line(x, y - 13, x + w, y - 13, 0x70858BCC);
      }
      continue;
    }
    // A shallow side face and foundation anchor the front elevation in depth.
    rect(x + w, y + 7, 13, h - 7, 0x08141DFF);
    line(x + w, y, x + w + 13, y + 7, 0x678089FF);
    rect(x - 3, 231, w + 18, 3, 0x020810EE);
    int tile = b.style < 6 ? b.style : b.style == 8 ? g.levelIndex : 2;
    SDL_SetTextureColorMod(architectureTiles.texture, 160, 181, 195);
    const auto cell = architectureTiles.cells.at(tile);
    for (float tx = 0; tx < w; tx += 96)
      for (float ty = 8; ty < h; ty += 96) {
        float dw = std::min(96.0f, w - tx), dh = std::min(96.0f, h - ty);
        SDL_Rect source{cell.x, cell.y, int(cell.w * dw / 96), int(cell.h * dh / 96)};
        SDL_FRect dest{x + tx + offsetX, y + ty + offsetY, dw, dh};
        SDL_RenderCopyF(r, architectureTiles.texture, &source, &dest);
      }
    SDL_SetTextureColorMod(architectureTiles.texture, 255, 255, 255);
    // Lower level is an open cutaway arcade: no wall lies across the route.
    const float ceiling = std::max(y + 22, 171.0f);
    rect(x + 7, ceiling, w - 14, 232 - ceiling, 0x040B12F4);
    for (int row = 0; row < 7; ++row)
      rect(x + 8, ceiling + row * 3, w - 16, 3, 0x00000000u | uint32_t(82 - row * 10));
    // Receding beams and wall seams provide interior scale without colliders.
    for (float k = 12; k < w - 15; k += 64) {
      line(x + k, ceiling + 5, x + k + 16, ceiling + 14, 0x364552FF);
      rect(x + k + 16, ceiling + 14, 3, 232 - ceiling - 14, 0x243541FF);
      line(x + k + 20, 226, x + k + 58, 226, 0x243843FF);
    }
    // Windows are recessed into the upper facade; mullions cast dark bars.
    for (float wx = x + 65; wx < x + w - 42; wx += 77) {
      for (float wy = y + 23; wy + 21 < ceiling - 4; wy += 49) {
        rect(wx - 3, wy - 3, 34, 24, 0x070F17FF);
        rect(wx, wy, 27, 16, 0x273A43FF);
        rect(wx + 1, wy + 1, 24, 13, (warm & 0xFFFFFF00u) | 85);
        line(wx + 13, wy, wx + 13, wy + 17, 0x061017FF);
        line(wx, wy + 8, wx + 27, wy + 8, 0x061017FF);
        line(wx - 3, wy + 21, wx + 31, wy + 21, 0x758387FF);
      }
    }
    // Structural columns frame the route; their dark base stays behind feet.
    for (float cx : {x, x + w - 6}) {
      rect(cx, y + 8, 6, h - 8, 0x263843FF);
      line(cx, y + 8, cx, 231, 0x6A7A7EFF);
      rect(cx - 2, 226, 10, 5, 0x36454AFF);
    }
    if (b.style == 2) {
      // Freight cars: recessed windows above a continuous service catwalk.
      for (float k = 34; k < w - 28; k += 87) {
        ring(x + k, 244, 11, 11, 0x15232BFF);
        ring(x + k, 244, 5, 5, 0x58646CFF);
      }
      rect(x + 7, 229, w - 14, 3, 0x60717CFF);
    }
    if (b.style == 3) {
      rect(x + w - 35, y + 19, 12, 32, 0xEE8A4333);
      for (int bar = 0; bar < 5; ++bar)
        line(x + w - 34, y + 21 + bar * 6, x + w - 24, y + 21 + bar * 6, 0xF0A46E88);
    }
    // Roof thickness and supports share the platform's exact top coordinate.
    rect(x - 2, y, w + 4, 8, 0x293F4AFF);
    line(x - 2, y, x + w + 2, y, 0xA4B3AFFF);
    line(x, y + 7, x + w, y + 7, 0x061019FF);
    for (float rx = x + 6; rx < x + w; rx += 40) {
      line(rx, y - 12, rx, y - 2, 0x41555DCC);
      line(rx, y - 12, std::min(rx + 40, x + w), y - 12, 0x60747D99);
    }
    if (w > 240) text(b.style == 2 ? "FREIGHT" : b.style == 4 ? "RELAY" : "SERVICE", x + 62, ceiling - 10, 1, 0xA7AEA9FF);
  }
  for (size_t i = 0; i < g.level().entrances.size(); ++i) {
    const auto &door = g.level().entrances[i];
    float x = door.x - camera, y = door.y;
    if (x < -40 || x > W + 40) continue;
    float age = i < g.entranceAges.size() ? g.entranceAges[i] : -1;
    float opening = age < 0 ? 0 : ease(age / .6f);
    rect(x - 22, y - 49, 44, 49, 0x314654FF);
    rect(x - 19, y - 46, 38, 46, 0x010509FF);
    rect(x - 18, y - 45, 36, 44 * (1 - opening), 0x48565DFF);
    for (int row = 0; row < int(44 * (1 - opening)); row += 5)
      line(x - 17, y - 45 + row, x + 17, y - 45 + row, 0x1D2B35FF);
    uint32_t warning = age >= 0 && age < 2.2f && int(time * 8) % 2 ? 0xFFC378FF : 0x796443FF;
    rect(x - 4, y - 53, 8, 3, warning);
    line(x - 18, y - 1, x + 18, y - 1, 0x899991FF);
  }
  for (const auto &ladder : g.level().ladders) {
    float x = ladder.x - camera;
    if (x < -16 || x > W + 16) continue;
    // Cast shadow behind both rails; rungs remain visible behind climbing hands.
    rect(x - 6, ladder.top - 9, 2, ladder.bottom - ladder.top + 9, 0x03090CE0);
    for (float rail : {-8.0f, 8.0f}) {
      line(x + rail + 2, ladder.top - 10, x + rail + 2, ladder.bottom, 0x071019FF);
      line(x + rail, ladder.top - 10, x + rail, ladder.bottom, 0x9AA79FFF);
    }
    for (float y = ladder.top - 2; y < ladder.bottom; y += 8)
      line(x - 8, y, x + 8, y, 0x7A8C89FF);
  }
}

void Renderer::wetSurfaces(const Game &g, float camera, float time) {
  for (const auto &b : g.level().buildings) if (b.style == 7) {
    float x = b.box.x - camera, w = b.box.w;
    if (x + w < 0 || x > W) continue;
    // Deck top stays at collision y=232; water is below its supporting piers.
    rect(x, 240, w, 32, 0x122F3BFF);
    for (float k = 12; k < w; k += 72) {
      rect(x + k, 240, 11, 32, 0x0A131AFF);
      line(x + k + 11, 244, x + std::min(k + 67,w), 265, 0x53636BFF);
      line(x + k + 11, 265, x + std::min(k + 67,w), 244, 0x354A55FF);
    }
    for (int row = 0; row < 5; ++row)
      line(x + 8, 244 + row * 5, x + w - 8, 244 + row * 5, 0x41677855);
  }
  for (const auto &wet : g.level().puddles) {
    float x = wet.x - camera;
    if (x + wet.w < 0 || x > W) continue;
    rect(x, wet.y + 1, wet.w, wet.h, 0x0A2631D0);
    for (const auto &light : lights) {
      if (g.lightBlocked(light.x + camera, light.y, wet.x + wet.w / 2, wet.y - 1)) continue;
      float center = std::clamp(light.x, x, x + wet.w);
      for (int row = 1; row < wet.h; ++row) {
        float half = 12 + row * 1.8f;
        float left = std::max(x, center - half + std::sin(time * 1.7f + row) * 2);
        float right = std::min(x + wet.w, center + half);
        if (right > left && std::fabs(center - light.x) < 65)
          rect(left, wet.y + row, right - left, 1, light.color | uint32_t(45 - row * 3));
      }
    }
    line(x + 4, wet.y + wet.h, x + wet.w - 4, wet.y + wet.h, 0x53768155);
  }
}

void Renderer::reflection(const Game &g, const Atlas &atlas, int frame, float x, float feet,
                           float size, bool flip, float camera, float time) {
  if (frame < 0) return;
  for (const auto &wet : g.level().puddles) {
    if (std::fabs(feet - wet.y) > 2 || x + size / 2 < wet.x - camera ||
        x - size / 2 > wet.x + wet.w - camera) continue;
    const auto &source = atlas.cells.at(frame);
    const float baseline = atlas.trimmed ? 1 : atlas.baselines.at(frame);
    // Only the painted pixels immediately above the feet reflect below them.
    // Strip sampling clips to the puddle and cannot expose another atlas cell.
    SDL_SetTextureAlphaMod(atlas.texture, 65);
    for (int row = 1; row < wet.h; ++row) {
      int sy = source.y + int(baseline * source.h) - 1 - int(row * source.h / size);
      if (sy < source.y || sy >= source.y + source.h) continue;
      float dx = x - size / 2 + std::sin(time * 2 + row * .8f) * .7f;
      float left = std::max(dx, wet.x - camera), right = std::min(dx + size, wet.x + wet.w - camera);
      if (right <= left) continue;
      int start = int((left - dx) / size * source.w), count = std::max(1, int((right - left) / size * source.w));
      SDL_Rect strip{source.x + (flip ? source.w - start - count : start), sy, count, 1};
      SDL_FRect dest{left + offsetX, feet + row + offsetY, right - left, 1};
      SDL_RenderCopyExF(r, atlas.texture, &strip, &dest, 0, nullptr, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
    SDL_SetTextureAlphaMod(atlas.texture, 255);
  }
}
} // namespace kh
