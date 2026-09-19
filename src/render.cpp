#include "render.h"
#include "presentation_config.h"
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
static constexpr const char *controlHint(const char *vita, const char *host) {
#ifdef KH_VITA
  (void)host;
  return vita;
#else
  (void)vita;
  return host;
#endif
}
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
  hero = load("hero-cinematic-v2.png", 8, 8, false);
  enemies = load("enemies-cinematic-v2.png", 8, 6, false);
  guardShield = load("guard-shield-v1.png",1,1,false);
  vehicle = load("vehicle-v2.png", 4, 4, false);
  for (int i = 0; i < 6; i++)
    bosses[i] = load("boss" + std::to_string(i) + "-v2.png", 4, 4, false);
  effects = load("effects-cinematic-v2.png",8,4,false);
  props = load("props.png", 4, 4, false);
  productionProps=load("production-props-v1.png",4,2,false);
  productionStructures=load("production-structures-v1.png",4,2,true);
  aim = load("aim-cinematic-v2.png", 4, 3, false);
  climb = load("climb-cinematic-v2.png", 4, 3, false);
  harborScene = load("harbor-cinematic-v1.png",1,1,false);
  harborExterior = load("harbor-exterior-cinematic-v1.png",1,1,false);
  alcoves = load("environment-alcoves-v1.png",3,2,false);
  signature = load("ataturk-signature.png",1,1,false);
  hiddenFlag = load("hidden-flag.png",1,1,false);
  architectureTiles = load("architecture-v1.png", 3, 2, false);
  // One small radial alpha mask is reused for lamps, fog and contact shadows.
  // Reuse one native-resolution canvas: atmospheric overdraw should not scale
  // with the output window. Target-less backends retain the direct path.
  if(SDL_RenderTargetSupported(r)) {
    canvas=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,W,H);
    if(canvas) {
      SDL_SetTextureBlendMode(canvas,SDL_BLENDMODE_NONE);
      SDL_SetTextureScaleMode(canvas,SDL_ScaleModeNearest);
    }
  }
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
  SDL_DestroyTexture(canvas);
  SDL_DestroyTexture(guardShield.texture);
  SDL_DestroyTexture(productionProps.texture);
  SDL_DestroyTexture(productionStructures.texture);
  SDL_DestroyTexture(forgeTitan.texture);SDL_DestroyTexture(foundryHall.texture);
  SDL_DestroyTexture(ironlineTrain.texture);SDL_DestroyTexture(ironlineDistance.texture);
  SDL_DestroyTexture(ironlineForest.texture);
  SDL_DestroyTexture(heroLocomotion.texture);
  SDL_DestroyTexture(guardReactions.texture);
  for (auto *a : {&hero, &enemies, &scenery, &props, &aim, &vehicle, &architectureTiles, &climb, &signature, &hiddenFlag, &alcoves, &harborScene, &scenePlate, &harborExterior, &effects, &workshopHero, &workshopGuard, &workshopAim, &workshopClimb, &workshopPlate, &workshopShell, &workshopDistance})
    SDL_DestroyTexture(a->texture);
  for (auto &a : bosses)
    SDL_DestroyTexture(a.texture);
  SDL_DestroyTexture(lightMask);
}
Atlas Renderer::load(const std::string &name, int cols, int rows, bool trim, bool paperKey, bool windowMatte) {
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
    if(name=="ironline-carriages-review-v1.png" || name=="ironline-forest-review-v1.png" || name=="ironline-integrated-v2.png" || name=="guard-shield-v1.png") {
      // Soft coverage and despill remove antialiased key edges after resizing.
      // This import rule is restricted to authored opaque green-key assets.
      int neutral=std::max(p[0],p[2]),spill=int(p[1])-neutral;
      if(spill>12) {
        float coverage=1-std::clamp((spill-12)/64.f,0.f,1.f);
        p[3]=Uint8(p[3]*coverage);p[1]=Uint8(neutral);
      }
    }
    if(windowMatte) {
      // Build a color-derived glass matte once at load time. The source asset
      // stays intact. Dark mullions, the hoist and warm interior props remain
      // opaque; only blue glass inside authored apertures transmits distance.
      const auto &v=tuning::workshopParallax;
      float u=float(i%a.width)/a.width,w=float(i/a.width)/a.height;
      float aperture=0;
      for(const auto &pane:tuning::workshopApertures) {
        float edge=std::min({u-pane.x,pane.x+pane.w-u,w-pane.y,pane.y+pane.h-w});
        aperture=std::max(aperture,std::clamp(edge/v.edgeFeather,0.f,1.f));
      }
      float blue=std::clamp((std::min(p[2]-p[0],p[1]-p[0])-v.chromaThreshold)/v.chromaFeather,0.f,1.f);
      float value=std::clamp((p[1]-v.darkThreshold)/v.darkFeather,0.f,1.f);
      p[3]=Uint8(p[3]*(1-aperture*blue*value*v.transmission));
    }
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
          if (pixels[(y * a.width + x) * 4 + 3] > 0) {
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
  if(kind==3) {
    // Core points are authored in the fixed 320px cells, tied to each pose.
    const auto &rig=tuning::forgeTitan;
    const auto point=tuning::forgeTitanCores[forgeTitanFrame(g.boss,alpha)];
    return {between(g.boss.prevX,g.boss.x,alpha)-camera+(point.x-rig.cellSize/2)*rig.renderSize/rig.cellSize,
            between(g.boss.prevY,g.boss.y,alpha)+(point.y-rig.baseline)*rig.renderSize/rig.cellSize};
  }
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
  const auto &atlas = kind==3?forgeTitan:bosses[kind];
  if(kind==3)frame=forgeTitanFrame(b,alpha);
  float cx = between(b.prevX, b.x, alpha) - camera;
  float cy = between(b.prevY, b.y, alpha) + (kind == 4 && !b.dead ? 7 : 0);
  float size = kind==3?tuning::forgeTitan.renderSize:kind == 4 ? 142.0f : 152.0f;
  auto core = bossCore(g, camera, alpha);
  uint8_t opacity = b.dead ? uint8_t(std::clamp(b.death * 180, 0.0f, 255.0f)) : 255;
  contactShadow(cx, cy, g.floorAt(b.x, cy - 2), kind == 4 ? 42 : 64);
  actorLight(atlas, core.x, core.y, b.hurt > 0);
  groundedSprite(atlas, frame, cx - size / 2, cy - size, size, size, false, 0, opacity);
  SDL_SetTextureColorMod(atlas.texture, 255, 255, 255);
  if (b.dead) return;
  if (b.state == BossState::Windup) {
    const bool odd=b.pattern%2;
    const bool fallingVolley=(kind==0 && odd) || (kind==3 && !odd) || (kind==5 && odd);
    auto warningLabel=[&](const char *label,float x,float y,int letters) {
      const float width=letters*6.f+8;
      x=std::clamp(x-width*.5f,4.f,W-width-4);
      rect(x,y-2,width,11,0x091018DD);
      text(label,x+4,y,1,0xFFDFA1FF);
    };
    if (fallingVolley) {
      // These positions mirror fireBossVolley, including its asymmetric
      // phase-dependent spread. The old single centre ring hid outer lanes.
      const int count=kind==0?b.phase+1:kind==3?b.phase+2:3;
      const float first=kind==0?-20.f:kind==3?-45.f:-40.f;
      const float startY=kind==0?42.f:kind==3?40.f:24.f;
      for(int i=0;i<count;++i) {
        const float worldX=b.targetX+first+i*40;
        const float x=worldX-camera;
        float floor=232;
        for(const auto &platform:g.level().platforms)
          if(!platform.oneWay && worldX>=platform.box.x && worldX<=platform.box.x+platform.box.w && platform.box.y>=startY)
            floor=std::min(floor,platform.box.y);
        if(x<-10 || x>W+10)continue;
        // Sparse prediction marks show direction without resembling a live beam.
        for(float y=startY+8;y<floor-12;y+=18) {
          rect(x-1,y-1,3,6,0x091018A0);
          line(x,y,x,y+3,0xFFC093CC);
        }
        line(x-4,startY+3,x,startY+7,0xFFB68BFF);
        line(x,startY+7,x+4,startY+3,0xFFB68BFF);
        ring(x,floor-2,10+pose.charge*3,3,0x091018FF);
        ring(x,floor-2,8+pose.charge*3,2,0xFF795DFF);
      }
      warningLabel("MOVE!",b.targetX-camera+first+(count-1)*20,182,5);
    } else if(kind==3 && odd) {
      // Foundry's alternate attack sweeps horizontally from the muzzle;
      // displaying a falling-strike marker here instructed the wrong dodge.
      const auto muzzle=bossMuzzlePoint(b,kind,alpha);
      const float right=std::clamp(muzzle.x-camera,0.f,float(W));
      for(int volley=0;volley<2+b.phase;++volley) {
        for(float x=12;x<right-8;x+=32) {
          const float y=muzzle.y+(muzzle.x-camera-x)/120.f*foundrySweepVY(volley);
          rect(x-1,y-3,6,7,0x091018A0);
          line(x+3,y-2,x,y,0xFFC093DD);
          line(x,y,x+3,y+2,0xFFC093DD);
        }
      }
      warningLabel("SWEEP",right*.5f,159,5);
    } else if (kind == 4) {
      // The first radial volley uses these same angles in fireBossVolley.
      // Short spokes communicate a spreading attack without drawing live beams.
      const int rays = 8 + b.phase * 2;
      const float originY = between(b.prevY,b.y,alpha) - 50;
      for (int i = 0; i < rays; ++i) {
        const float angle = 6.283185f * i / rays + b.pattern * .18f;
        const float dx=std::cos(angle), dy=std::sin(angle);
        const float radius=27+pose.charge*8;
        line(cx+dx*radius,originY+dy*radius,cx+dx*(radius+9),originY+dy*(radius+9),0x08131EFF);
        line(cx+dx*(radius+1),originY+dy*(radius+1),cx+dx*(radius+7),originY+dy*(radius+7),TEAL);
      }
      warningLabel("RADIAL",cx,originY-52,6);
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
  world = &g; worldCamera = camera;
  lights.clear();
  if(g.ironlineReview) {
    const auto &rail=tuning::ironlineReview;
    lights.push_back({W*.55f,25,rail.roofY,260,.4f,0xA4C8DA00u,false});
    for(float x:{75.f,245.f,540.f,705.f})
      lights.push_back({x-camera,rail.roofY+55,rail.roofY,90,.24f,0xFFB26700u,false});
  }
  // Authored practical emitters are registered to painted windows and lamps.
  const bool production=tuning::productionPlates[g.levelIndex].enabled>0 || g.levelIndex==3;
  if(!production && !g.cinematicReview() && g.levelIndex==0)for(const auto &l:tuning::harborLights)
    lights.push_back({l.x-camera,l.y,232,l.radius,l.strength,l.color&0xFFFFFF00u,false});
  for(const auto &l:tuning::sceneLights)if(!production && !g.cinematicReview() && int(l.map)==g.levelIndex && l.x>camera-l.radius && l.x<camera+W+l.radius) {
    float pulse=1+l.flicker*std::sin(time*l.rate+l.x);
    lights.push_back({l.x-camera,l.y,232,l.radius,l.strength*pulse,l.color&0xFFFFFF00u,false,l.window>0});
  }
  if(production && !g.cinematicReview())for(const auto &l:tuning::productionLights)if(int(l.map)==g.levelIndex) {
    const auto &plate=tuning::productionPlates[g.levelIndex];
    float aspect=g.levelIndex==3?tuning::foundryHall.sourceAspect:plate.sourceAspect;
    float floor=g.levelIndex==3?tuning::foundryHall.sourceFloor:plate.sourceFloor;
    float width=g.level().width,height=width/aspect,x=l.u*width,y=232+(l.v-floor)*height;
    if(x>camera-l.radius && x<camera+W+l.radius)
      lights.push_back({x-camera,y,232,l.radius,l.strength,l.color&0xFFFFFF00u,false,true});
  }
  if(g.levelIndex==2 && !g.cinematicReview()) {
    const float roof=g.level().platforms.at(1).box.y;
    const auto &travel=tuning::ironlineTravel;
    lights.push_back({W*.5f,roof-45,roof,300,travel.skyStrength,travel.skyColor,false});
  }
  if(g.workshopReview)for(const auto &l:tuning::workshopLights)
    lights.push_back({l.x-camera,l.y,232,l.radius,l.strength*(1+.025f*std::sin(time*1.7f+l.x)),l.color&0xFFFFFF00u,false,l.window>0});
  if(g.workshopReview)for(const auto &screen:tuning::workshopScreens) {
    const auto &room=tuning::workshop;const auto &motion=tuning::workshopMotion;
    float height=room.width/room.sourceAspect;
    float pulse=.7f+.3f*std::sin(time*motion.terminalSpeed+screen.x*13);
    lights.push_back({(screen.x+screen.w*.5f)*room.width-camera,
      232-height*room.sourceFloor+(screen.y+screen.h*.5f)*height,232,
      motion.terminalRadius,motion.terminalGain*pulse,motion.terminalColor&0xFFFFFF00u,false});
  }
  if (g.player.recoil > 0 && g.player.action <= 0 && g.player.ladder < 0 && g.status == Status::Play) {
    float x = between(g.player.prevX, g.player.x, alpha) - camera;
    float y = between(g.player.prevY, g.player.y, alpha);
    Player presentation = g.player;
    presentation.x = x + camera;
    presentation.y = y;
    auto muzzle = muzzlePoint(presentation, input);
    lights.push_back({muzzle.x - camera, muzzle.y, y, 60,
                      std::min(1.0f, g.player.recoil * 14),
                      tuning::weapons[std::clamp(g.player.firedWeapon,0,5)].flashColor & 0xFFFFFF00u, false});
  }
  for(const auto &source:g.enemies) {
    if(!g.cinematicGuard(source))continue;
    auto enemy=interpolatedEnemy(source,alpha);
    if(!guardFlashVisible(enemy))continue;
    auto muzzle=guardMuzzle(enemy);const auto &rig=tuning::guardAction;
    if(muzzle.x<camera-rig.lightRadius || muzzle.x>camera+W+rig.lightRadius)continue;
    lights.push_back({muzzle.x-camera,muzzle.y,enemy.y,rig.lightRadius,
      rig.lightStrength*(1-enemy.fireAge/rig.flashDuration),rig.lightColor&0xFFFFFF00u,false});
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
  const auto &p = tuning::lighting;
  const auto &m = tuning::maps[sceneTheme];
  float red = 255*p.fill, green = 255*p.fill, blue = 255*p.fill;
  for (const auto &light : lights) {
    if (world && world->lightBlocked(light.x + worldCamera, light.y, x + worldCamera, y)) continue;
    float dx = (x - light.x) / light.radius;
    float dy = (y - light.y) / (light.radius * 1.2f);
    float amount = std::max(0.0f, 1 - dx * dx - dy * dy) * light.strength * p.key;
    amount += p.rim * std::max(0.f, 1-std::fabs(dx)) * (light.y < y ? .5f : .1f);
    red += amount * ((light.color >> 24) / 255.0f) * 88;
    green += amount * (((light.color >> 16) & 255) / 255.0f) * 66;
    blue += amount * (((light.color >> 8) & 255) / 255.0f) * 43;
  }
  if(world && world->cinematicHero()){red*=tuning::workshop.ambientGain;green*=tuning::workshop.ambientGain;blue*=tuning::workshop.ambientGain;}
  red *= .85f + .15f*((m.keyColor>>24)/255.f);
  green *= .85f + .15f*(((m.keyColor>>16)&255)/255.f);
  blue *= .85f + .15f*(((m.keyColor>>8)&255)/255.f);
  SDL_SetTextureColorMod(atlas.texture, hurt ? Uint8(tuning::combat.hitColor>>24) : Uint8(std::min(red, 255.0f)),
                         hurt ? Uint8((tuning::combat.hitColor>>16)&255) : Uint8(std::min(green, 255.0f)),
                         hurt ? Uint8((tuning::combat.hitColor>>8)&255) : Uint8(std::min(blue, 255.0f)));
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
  const float blur = tuning::lighting.dof;
  if (blur > 0) {
    // Defocus only distant scenery. Never sample an actor atlas neighbour.
    for (int tap : {-1, 1})
      sprite(scenery, 0, -(width-W)*progress+tap*blur, 0, width, H, false, 0, Uint8(tuning::environment.skyDofAlpha));
  }
  if (tuning::lighting.motionBlur > 0) {
    float travel = backgroundMotion;
    sprite(scenery,0,-(width-W)*progress-travel,0,width,H,false,0,Uint8(tuning::environment.motionAlpha));
  }
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
void Renderer::lightingPass(const Game &g, float, float, float) {
  SDL_Rect previousClip{};
  const bool hadClip = SDL_RenderIsClipEnabled(r);
  SDL_RenderGetClipRect(r, &previousClip);
  for (const auto &light : lights) {
    float left = -100, right = W + 100, top = g.level().minY - 100, bottom = light.floor;
    const float wx = light.x + worldCamera;
    for (const auto &b : g.level().buildings)
      if (b.style != 7 && wx > b.box.x && wx < b.box.x + b.box.w &&
          light.y > b.box.y + 8 && light.y < 232) {
        left = std::max(left, b.box.x - worldCamera + 6);
        right = std::min(right, b.box.x + b.box.w - worldCamera - 6);
        top = std::max(top, b.box.y + 8);
      }
    for (const auto &p : g.level().platforms)
      if (p.oneWay && wx >= p.box.x && wx <= p.box.x + p.box.w) {
        if (p.box.y > light.y) bottom = std::min(bottom, p.box.y);
        else if (p.box.y + p.box.h < light.y) top = std::max(top, p.box.y + p.box.h);
      }
    SDL_Rect clip{int(std::floor(left + offsetX)), int(std::floor(top + offsetY)),
                  std::max(0,int(std::ceil(right-left))), std::max(0,int(std::ceil(bottom-top)))};
    if (hadClip) SDL_IntersectRect(&clip, &previousClip, &clip);
    SDL_RenderSetClipRect(r, &clip);
    softLight(light.x, light.y + 16, light.radius, light.radius * .85f,
              light.color, Uint8(34 * light.strength));
    if (!light.fixture) {
      SDL_RenderSetClipRect(r, hadClip ? &previousClip : nullptr);
      continue;
    }
    if (light.window) {
      // Hand-authored window projection: two warm panes separated by mullions.
      for (float y = light.y + 10; y < bottom; y += 3) {
        float reach = (y - light.y) * .18f;
        rect(light.x - 10 - reach, y, 8 + reach, 3, light.color | 13);
        rect(light.x + 3, y, 8 + reach, 3, light.color | 13);
      }
      SDL_RenderSetClipRect(r, hadClip ? &previousClip : nullptr);
      continue;
    }
    // Visible grounded lamp: the cone, glow and reflected pool share its root.
    rect(light.x + 6, light.y - 8, 2, light.floor - light.y + 8, 0x080F17FF);
    line(light.x + 7, light.y - 8, light.x + 7, light.floor, 0x36515F9A);
    rect(light.x - 8, light.y - 6, 17, 5, 0x08121AFF);
    rect(light.x - 5, light.y - 1, 10, 1, light.color | 220);
    softLight(light.x, light.y, 17, 8, light.color, 110);
    // Analytic cone with smooth edges, low alpha, and a real source.
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    for (int y = int(light.y + 2); y < int(light.floor); y += 2) {
      if (g.lightBlocked(light.x + worldCamera, light.y, light.x + worldCamera, float(y))) break;
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
    SDL_RenderSetClipRect(r, hadClip ? &previousClip : nullptr);
  }
}

void Renderer::drawGame(const Game &g, const ViewState &v) {
  releaseSceneLayers(g);
  if(g.cinematicHero())loadActors();
  if(g.workshopReview)loadWorkshop();
  if(g.levelIndex==3)loadFoundry();
  Atlas &hero=g.cinematicHero()?workshopHero:this->hero;
  Atlas &aim=g.cinematicHero()?workshopAim:this->aim;
  Atlas &climb=g.cinematicHero()?workshopClimb:this->climb;
  backgroundMotion=std::clamp((g.camera-g.prevCamera)*tuning::lighting.motionBlur,-3.f,3.f);
  const float alpha = std::clamp(v.interpolation, 0.0f, 1.0f);
  const float camera = between(g.prevCamera, g.camera, alpha);
  const float time = between(g.prevTime, g.time, alpha);
  float baseScaleX,baseScaleY;SDL_RenderGetScale(r,&baseScaleX,&baseScaleY);
  float zoom=between(g.prevCameraZoom,g.cameraZoom,alpha);
  SDL_RenderSetScale(r,baseScaleX*zoom,baseScaleY*zoom);
  float sx = v.shake ? std::sin(time * 83) * g.shake : 0,
        sy = v.shake ? std::cos(time * 73) * g.shake * .5f : 0;
  sx+=W*(1/zoom-1)*.5f;sy+=H*(1/zoom-1)*.5f;
  offsetX = sx;
  offsetY = sy;
  collectLights(g, v.input, camera, time, alpha);
  // Far scenery moves vertically at a smaller depth, while all playable
  // architecture, feet, shadows and lights use the exact same world camera.
  float cameraY = between(g.prevCameraY, g.cameraY, alpha);
  offsetY = sy - cameraY * tuning::lighting.focusParallax;
  background(g.levelIndex, camera, time);
  atmosphere(g, camera, time, false);
  offsetY = sy - cameraY - (g.workshopReview?tuning::workshop.cameraLift:0.f);
  cinematicHarbor(g,camera,time,cameraY);
  if(g.workshopReview)workshopMotion(camera,time);
  architecture(g, camera, time);
  secretProps(g,camera);
  wallLights(g,camera);
  lightingPass(g, camera, time, alpha);
  cinematicLights(g, camera, time);
  const auto &l = g.level();
  themedPlatforms(g, camera);
  surfaceLights(g, camera);
  wetSurfaces(g, camera, time);
  for (auto &h : l.hazards) {
    float x = h.x - camera;
    if (x + h.w < 0 || x > W)
      continue;
    bool active = g.hazardOn(h);
    if (g.hazardDisabled(h)) {
      rect(x-2,h.y+h.h-4,h.w+4,4,0x101D25FF);
      for (int j = 2; j < h.w; j += 5)
        rect(x+j,h.y+h.h-3,2,2,0x7DD2A5FF);
      continue;
    }
    const float warning = g.hazardWarning(h);
    if (warning > 0) {
      const float floor=h.y+h.h;
      rect(x-2,floor-7,h.w+4,3,0x101820FF);
      rect(x-1,floor-6,(h.w+2)*warning,1,0xFFD38AFF);
      for(float edge:{x-2,x+h.w+1})
        for(float y=h.y;y<floor-9;y+=9)
          rect(edge,y,1,3,0xF6BE7200u|Uint8(80+warning*140));
      rect(x+h.w*.5f-4,h.y-15,9,11,0x101820E0);
      text("!",x+h.w*.5f-2,h.y-13,1,0xFFD38AFF);
    }
    if(g.levelIndex==1 && h.kind==0) {
      // A shallow steel vent sits on the painted lane. Steam stays inside the
      // damaging height instead of suggesting an invisible tall hitbox.
      const float floor=h.y+h.h-1;
      rect(x-1,floor-2,h.w+2,3,0x131B1EFF);
      line(x,floor-2,x+h.w,floor-2,0x566161CC);
      for(int j=3;j<int(h.w)-2;j+=4)
        line(x+j,floor-1,x+j+1,floor,0x090F12FF);
      for(float edge:{x,x+h.w-2})
        rect(edge,floor-2,2,2,active?0xDFA262FF:0x927354FF);
      if(active)for(int j=3;j<int(h.w)-2;j+=3) {
        float phase=std::fmod(time*2+j*.137f,1.f);
        float y=floor-1-phase*(h.h-2);
        float drift=std::sin(time*5+j)*1.2f;
        line(x+j+drift,y,x+j+drift+.5f,y+2,0xC9D5C8A0);
      }
      continue;
    }
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
    if (!p.dead && p.x > camera - 40 && p.x < camera + W + 40) {
      float size=p.kind==7?tuning::productionProps.crateSize:tuning::productionProps.barrelSize;
      actorLight(productionProps,p.x-camera,p.y-size*.5f);
      groundedSprite(productionProps,p.kind==7?0:1,p.x-camera-size/2,p.y-size,size,size);
      SDL_SetTextureColorMod(productionProps.texture,255,255,255);
    }
  for (float cp : l.checkpoints)
    if (!g.cinematicReview() && g.levelIndex>2 && cp > camera - 25 && cp < camera + W + 25) {
      float size=tuning::productionProps.beaconSize;
      groundedSprite(productionProps,3,cp-camera-size/2,232-size,size,size);
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
      float size=tuning::productionProps.workerSize;
      groundedSprite(enemies, workerFrame(i.used, i.used ? i.anim : time),
                     x - size/2, feet - size, size, size, false, 0,
                     i.used ? uint8_t(255 * std::clamp((2 - i.anim) / .65f, 0.0f, 1.0f)) : 255);
      SDL_SetTextureColorMod(enemies.texture, 255, 255, 255);
    } else if (!i.used) {
      float y = i.y + std::sin(time * 4 + i.x) * 2;
      const float feet=g.floorAt(i.x,i.y),size=tuning::productionProps.supplySize;
      if(i.kind==4)sprite(props,9,x-9,feet-21,18,20);
      else groundedSprite(productionProps,2,x-size/2,feet-size,size,size);
      const char *label = i.kind == 1   ? "H"
                          : i.kind == 2 ? "S"
                          : i.kind == 3 ? "R"
                          : i.kind == 4 ? "G"
                          : i.kind == 6 ? "F"
                          : i.kind == 9 ? "L"
                                        : "$";
      if (g.workshopReview) text(label, x - 2, y - 5, 1, CREAM);
    }
  }
  if (g.vehicleAvailable && g.vehicleX > camera - 80 && g.vehicleX < camera + W + 80) {
    float feet = g.floorAt(g.vehicleX, 200);
    actorLight(vehicle, g.vehicleX - camera, feet - 30);
    contactShadow(g.vehicleX - camera, feet, feet, 27);
    int frame = g.vehicleHatch > 0 ? 8 + std::clamp(int((.36f - g.vehicleHatch) / .36f * 4), 0, 3) : 0;
    groundedSprite(vehicle, frame, g.vehicleX - camera - 38, feet - 76, 76, 76);
    SDL_SetTextureColorMod(vehicle.texture, 255, 255, 255);
    if (g.canBoardVehicle())
      text(controlHint("TRIANGLE", "E / PAD Y"), g.vehicleX - camera - 32, 155, 1, GOLD);
  }
  for (const auto &source : g.enemies) {
    auto e = interpolatedEnemy(source,alpha);
    const bool authoredGuard=g.cinematicGuard(e);
    Atlas &enemies=authoredGuard?workshopGuard:this->enemies;
    if ((e.dead && e.death <= 0) || !e.active || e.x < camera - 60 || e.x > camera + W + 60)
      continue;
    float life=e.deathDuration>0?e.deathDuration:tuning::combat.deathDuration;
    float deathT = e.dead ? 1.0f - std::clamp(e.death / life, 0.0f, 1.0f) : 0.0f;
    bool authoredReaction=authoredGuard && tuning::guardReactions.enabled>0 && (e.dead || e.flinch>0);
    auto classTint=[&](const Atlas &atlas) {
      if(!authoredGuard || e.hurt>0 || e.kind==0)return;
      Uint8 red,green,blue;SDL_GetTextureColorMod(atlas.texture,&red,&green,&blue);
      // Preserve mids in the dark uniforms: multiplying every channel down
      // made the class tint disappear into the similarly coloured scenery.
      if(e.kind==1){red=255;green=Uint8(std::max(190,int(green)));blue=Uint8(blue*.70f);}
      else {red=Uint8(red*.82f);green=Uint8(std::max(215,int(green)));blue=255;}
      SDL_SetTextureColorMod(atlas.texture,red,green,blue);
    };
    auto guardPass=[&](const Atlas &atlas,uint8_t opacity,const auto &draw) {
      draw(opacity);
      if(!authoredGuard || !g.arcadeCombat() || e.hurt>0)return;
      // A restrained material fill lifts the authored uniform's dark mids.
      // Reuse the exact pose and alpha, so no outline/halo, geometry change or
      // extra light is painted onto the wall behind the soldier.
      SDL_BlendMode previous;SDL_GetTextureBlendMode(atlas.texture,&previous);
      SDL_SetTextureBlendMode(atlas.texture,SDL_BLENDMODE_ADD);
      draw(uint8_t(opacity*.34f));
      SDL_SetTextureBlendMode(atlas.texture,previous);
    };
    // Full source poses: walk 0..3, attack 4, hurt 5, collapse 6/7.
    int frame = e.dead
                    ? (e.kind == 5 ? std::min(1, int(deathT * 2))
                                   : 6 + std::min(1, int(deathT * 2)))
                    : e.hurt > 0 ? 5
                    : e.state == 1 ? 3
                    : e.state == 2 ? 4
                                    : int(e.gait * tuning::effects.enemyWalkFps) % 4;
    int idx = e.kind * 8 + frame;
    if(authoredGuard) {
      if(e.dead)idx=20+std::min(3,int(deathT*4));
      else if(e.flinch>0)idx=16+int(e.hitZone);
      else if(e.state==2 && e.fireAge<tuning::guardAction.attackDuration)idx=12+guardFirePhase(e);
      else if(std::fabs(source.x-source.prevX)>.01f)idx=int(e.gait*8)%8;
      else idx=8+int(time*3)%4;
    }
    float h = authoredGuard?tuning::workshop.guardSize:(e.kind == 3 ? 40 : 43)*g.enemyBodyScale(e), w = h;
    float drawW = w, drawH = h;
    const auto &reaction=tuning::combat;
    float wave=e.dead?std::sin(deathT*3.14159f):std::sin((1-e.flinch/std::max(.001f,e.flinchDuration))*3.14159f);
    if(!e.dead && e.flinch<=0)wave=0;
    float visualX=e.x-camera+e.hitDir*wave*(e.dead?reaction.deathTravel:reaction.knockback);
    if(!authoredReaction && !e.dead && e.hitZone==HitZone::Legs) drawH*=1-wave*reaction.legSquash;
    float yy=e.y-drawH-(e.dead?wave*reaction.deathLift:0);
    double angle=e.hitDir*wave*(e.dead?reaction.deathAngle:e.hitZone==HitZone::Head?reaction.headAngle:e.hitZone==HitZone::Torso?reaction.torsoAngle:0);
    if(authoredGuard) {
      // The authored hit/death frames already bend the body. Rotating their
      // full atlas cells lifts the boots off the floor during a standing hit.
      angle=0;yy=e.y-drawH;
      visualX=e.x-camera+e.hitDir*wave*(e.dead?8:reaction.knockback);
    }
    // Lift the rotating full cell by its extra vertical extent, so its lowest
    // painted foot cannot sink below the unchanged collision surface.
    yy-=std::sin(std::fabs(angle)*.0174533f)*drawW*.5f;
    uint8_t alpha = e.dead ? uint8_t(std::clamp(e.death/life,0.f,1.f)*255) : uint8_t(std::min(1.0f, e.entryAge / .45f) * 255);
    if (!e.dead)
      contactShadow(e.x - camera, e.y, g.floorAt(e.x, e.y - 2), drawW * .34f);
    else {
      uint8_t shadowAlpha = uint8_t(std::clamp((1.0f - deathT) * 80.0f, 0.0f, 80.0f));
      float shadowX=e.x-camera;
      if(authoredReaction)shadowX+=e.hitDir*ease(enemyDeathElapsed(e)/tuning::guardReactions.collapseDuration)*tuning::guardReactions.deathTravel;
      rect(shadowX - drawW * .34f, e.y - 2, drawW * .68f, 2,
           0x08131A00u | shadowAlpha);
    }
    if(authoredReaction) {
      const auto &rig=tuning::guardReactions;
      int pose=guardReactionFrame(e);bool flip=guardReactionFlip(e);
      // Monotonic collapse displacement: the corpse must not slide back as
      // sin(deathT*pi) returns to zero. A hit uses only a small temporary shift.
      float travel=e.dead?ease(enemyDeathElapsed(e)/rig.collapseDuration)*rig.deathTravel:wave*rig.hitTravel;
      visualX=e.x-camera+e.hitDir*travel;
      uint8_t opacity=e.dead?uint8_t(255*guardCorpseOpacity(e)):alpha;
      actorLight(guardReactions,visualX,e.y-rig.renderSize*.5f,e.hurt>0);
      classTint(guardReactions);
      guardPass(guardReactions,opacity,[&](uint8_t a){
        sprite(guardReactions,pose,visualX-rig.renderSize/2,
               e.y-rig.renderSize*rig.baseline/rig.cellSize,rig.renderSize,rig.renderSize,flip,0,a);
      });
      if(!e.dead)reflection(g,guardReactions,pose,visualX,e.y,rig.renderSize,flip,camera,time);
      SDL_SetTextureColorMod(guardReactions.texture,255,255,255);
    } else {
      if(authoredGuard && !e.dead && e.flinch<=0) {
        visualX=guardPoseX(e,idx)-camera;
        drawW=drawH=tuning::guardAction.renderSize;yy=e.y-drawH;
      }
      actorLight(enemies, e.x - camera, e.y - drawH * .5f, e.hurt > 0);
      classTint(enemies);
      guardPass(enemies,alpha,[&](uint8_t a){
        groundedSprite(enemies,idx,visualX-drawW/2,yy,drawW,drawH,e.dir>0,angle,a);
      });
      if (!e.dead && e.flinch<=0) reflection(g, enemies, idx, visualX, e.y, drawW, e.dir > 0, camera, time);
      SDL_SetTextureColorMod(enemies.texture, 255, 255, 255);
    }
    if(authoredGuard && e.kind==2 && !e.dead) {
      const bool open=e.state==2;
      const float sx=e.x-camera+e.dir*17;
      actorLight(guardShield,sx,e.y-28,e.hurt>0);
      sprite(guardShield,0,sx-29,e.y-58+(open?9:0),58,58,e.dir>0,open?-e.dir*22:0,alpha);
      SDL_SetTextureColorMod(guardShield.texture,255,255,255);
    }
    if (e.state == 1 && !e.dead) {
      const float warningY=e.y-g.enemyBox(e).h-12;
      const uint32_t warning=e.kind==1?0xFF9764FF:0xFFDFA1FF;
      // Keep a short, stable warning above the body rather than flashing the
      // whole sprite, which resembles a hit reaction during busy firefights.
      rect(e.x-camera-4,warningY-2,9,11,0x091018D8);
      text("!",e.x-camera-2,warningY,1,warning);
    }
  }
  drawBoss(g, camera, alpha);
  auto p = interpolatedPlayer(g.player,alpha);
  float playerX = p.x;
  float playerY = p.y;
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
      const float playerW = g.cinematicHero()?tuning::workshop.heroSize:48.f, playerH = playerW;
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
        if(g.cinematicHero())frame=48+std::min(5,int(deathT*7));
        y = playerY - h;
        angle = 0;
      } else if (p.ladder >= 0 || p.climbTransition > 0) {
        int step = int(p.climbCycle * 8) % 8;
        static constexpr int cycle[] = {0, 1, 2, 4, 0, 1, 2, 4};
        int cell = p.climbTransition > 0 ? p.climbPose : cycle[step];
        actorLight(climb, px, playerY - 24, p.hitFlash > 0);
        float cs=g.cinematicHero()?tuning::workshop.climbSize:48.f;
        groundedSprite(climb, cell, px - cs/2, playerY - cs, cs, cs,
                       p.climbTransition <= 0 && step >= 4);
        SDL_SetTextureColorMod(climb.texture, 255, 255, 255);
        frame = -1;
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
        float as=g.cinematicHero()?tuning::workshop.aimSize:56.f;
        groundedSprite(aim, idx, px-as/2, playerY-as, as, as, p.dir < 0);
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
        static constexpr int runFireFrames[] = {56, 57, 58, 59, 60, 61, 62, 63};
        frame = runFireFrames[int(p.stride * 8) % 8];
      } else if (std::fabs(p.vx) > 1) {
        static constexpr int runFrames[] = {0, 1, 2, 3, 4, 5, 6, 7};
        frame = runFrames[int(p.stride * 8) % 8];
      } else if (v.input.shoot || p.shot > 0) {
        frame = 12 + std::min(3, int(p.fireAge * 28));
      }
      if (frame >= 0) {
        const auto &wp=tuning::weapons[std::clamp(p.firedWeapon,0,5)];
        float kick=std::max(0.f,1-p.fireAge/wp.flashDuration)*wp.recoil;
        if(!g.cinematicHero())px -= p.dir*kick;
        if(g.cinematicHero() && (useHeroLocomotion(p) || useHeroAirFire(p,v.input)) && g.status==Status::Play) {
          const auto &rig=tuning::heroLocomotion;
          int pose=heroFirePhase(p)+(v.input.shoot || p.shot>0?8:0);
          actorLight(heroLocomotion,px,playerY-30,p.hitFlash>0);
          // Preserve the authored flight height. Per-cell bottom alignment
          // would pull both airborne boots back down to the roof every cycle.
          sprite(heroLocomotion,pose,px-rig.renderSize/2,
                 playerY-rig.renderSize*rig.baseline/rig.cellSize,
                 rig.renderSize,rig.renderSize,p.dir<0);
          if(p.grounded)reflection(g,heroLocomotion,pose,px,playerY,rig.renderSize,p.dir<0,camera,time);
          SDL_SetTextureColorMod(heroLocomotion.texture,255,255,255);
        } else {
          groundedSprite(hero, frame, px - w / 2, y, w, h, p.dir < 0, angle);
          if (p.grounded) reflection(g, hero, frame, px, playerY, w, p.dir < 0, camera, time);
        }
        SDL_SetTextureColorMod(hero.texture, 255, 255, 255);
      }
      const auto &profile=tuning::weapons[std::clamp(p.firedWeapon,0,5)];
      if (p.fireAge < profile.flashDuration && p.action <= 0 && p.ladder < 0) {
        auto muzzle = muzzlePoint(p, v.input);
        float fx=muzzle.x-camera, fy=muzzle.y;
        float fade=1-p.fireAge/profile.flashDuration;
        float length=profile.flashLength*(.65f+.35f*fade), width=profile.flashWidth*fade;
        float dx=muzzle.vertical?0.f:float(p.dir), dy=muzzle.vertical?(v.input.down?1.f:-1.f):0.f;
        for(int ray=-1;ray<=1;++ray) {
          float spread=ray*width;
          line(fx,fy,fx+dx*length-dy*spread,fy+dy*length+dx*spread,profile.flashColor);
        }
        softLight(fx,fy,profile.flashLength,profile.flashWidth*2,profile.flashColor,Uint8(fade*100));
      }
      if (p.reloadTime>0 && !g.cinematicHero()) {
        float f=1-p.reloadTime/profile.reload;
        rect(px-10,playerY-49,20,2,0x142531FF);
        rect(px-10,playerY-49,20*f,2,profile.tracerColor);
      }
    }
  }
  for (const auto *atlas : {&hero, &aim, &vehicle})
    SDL_SetTextureColorMod(atlas->texture, 255, 255, 255);
  atmosphere(g, camera, time, true);
  // Cosmetic smoke is rendered before hostile projectiles so threats stay visible.
  for (const auto &source : g.particles) {
    auto part = source;
    part.x = between(source.prevX, source.x, alpha);
    part.y = between(source.prevY, source.y, alpha);
    if (part.life > 0 && part.x > camera - 100 && part.x < camera + W + 100) {
      float x = part.x - camera, age = 1 - part.life / part.maxlife;
      if(part.kind==Particle::GuardSpark) {
        const auto &c=tuning::guardImpact;
        float speed=std::hypot(part.vx,part.vy);
        float tail=std::min(c.sparkTrailTime,c.sparkTrailMax/std::max(1.f,speed));
        uint32_t color=(part.color&0xFFFFFF00u)|uint32_t((1-age)*220);
        line(x,part.y,x-part.vx*tail,part.y-part.vy*tail,color);
      } else if(part.kind==Particle::BodyDust) {
        float w=part.size*(1+age),h=part.size*.45f;
        float y=std::min(part.y,part.floor-h);
        softLight(x,y,w,h,part.color,Uint8((1-age)*tuning::guardImpact.dustOpacity),false);
      } else if (part.kind >= 5) {
        uint32_t color=(part.color&0xFFFFFF00u)|uint32_t((1-age)*255);
        if(part.kind==5) {
          float a=age*12; line(x,part.y,x+std::cos(a)*part.size*2,part.y+std::sin(a)*part.size*2,color);
        } else if(part.kind==7) softLight(x,part.y,part.size*(1+age),part.size,part.color,Uint8((1-age)*45),false);
        else {
          line(x,part.y,x-part.vx*.02f,part.y-part.vy*.02f,color);
          if(part.variant==5)line(x,part.y-part.size,x,part.y+part.size,color);
          else if(part.variant==4)softLight(x,part.y,part.size*(1+age),part.size,part.color,Uint8((1-age)*70));
          else if(part.variant>=0)rect(x,part.y,std::max(1.f,part.size*(1-age)),std::max(1.f,part.size*(1-age)),color);
        }
      } else if (part.kind == 3) {
        int idx = std::min(7, int(age * tuning::effects.explosionFrames));
        float s = part.size * tuning::effects.explosionSize * (1 + age * .25f);
        sprite(effects, idx, x - s / 2, part.y - s / 2, s, s, false, 0,
               uint8_t(std::min(255.0f, part.life * 600)));
      } else if (part.kind == 1) {
        rect(x - part.size / 2, part.y - part.size / 2, part.size, part.size,
             0xA3B6B000 | uint8_t((1 - age) * 50));
      } else if (part.kind == 4) {
        line(x, part.y, x + part.vx * .025f, part.y + 2, 0x90C9D000u | uint32_t((1-age)*180));
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
        rect(x-4,b.y-4,8,8,0x091018FF);
        rect(x - 3, b.y - 3, 6, 6, b.hostile ? 0xFF795DFF : 0x9DB65AFF);
        rect(x - 1, b.y - 4, 2, 2, CREAM);
        if(b.hostile) {
          const float pulse=.5f+.5f*std::sin(time*20);
          ring(x,b.y,6+pulse*2,6+pulse*2,0xFF795D00u|uint32_t(90+pulse*90));
        }
      } else if (b.hostile && g.cinematicHero()) {
        const float angle=std::atan2(b.vy,b.vx),dx=std::cos(angle),dy=std::sin(angle);
        const uint32_t danger=b.kind==5?0x69E8FFFF:0xFF795DFF;
        // Enemy rounds use a dark-edged warm core, clearly distinct from the
        // player's long gold tracers. Their bright core matches the hit radius.
        line(x-dx*2,b.y-dy*2,x-dx*9,b.y-dy*9,(danger&0xFFFFFF00u)|160);
        rect(x-3,b.y-3,6,6,0x091018EE);
        rect(x-2,b.y-2,4,4,danger);
        rect(x-1,b.y-1,2,2,0xFFF4E0FF);
      } else if (b.hostile) {
        rect(x - 4, b.y - 4, 8, 8, INK);
        rect(x - 3, b.y - 3, 6, 6, b.kind == 5 ? TEAL : RED);
        rect(x - 1, b.y - 1, 2, 2, CREAM);
      } else if (b.kind == 7) {
        float angle=std::atan2(b.vy,b.vx)*180/3.141593f;
        int frame=8+int(time*tuning::effects.flameFps+b.x*.07f)%8;
        sprite(effects,frame,x-tuning::effects.flameWidth*.5f,b.y-tuning::effects.flameHeight*.5f,
               tuning::effects.flameWidth,tuning::effects.flameHeight,false,angle);
      } else if (b.kind == 8) {
        bool vertical = std::fabs(b.vy) > std::fabs(b.vx);
        float len = 17;
        rect(x - (vertical ? 2 : len / 2), b.y - (vertical ? len / 2 : 2), vertical ? 4 : len,
             vertical ? len : 4, TEAL);
        rect(x - (vertical ? 1 : len / 2), b.y - (vertical ? len / 2 : 1), vertical ? 2 : len,
             vertical ? len : 2, CREAM);
      } else {
        const auto &wp=tuning::weapons[std::clamp(b.weapon,0,5)];
        float len = wp.tracerLength;
        bool vertical = std::fabs(b.vy) > std::fabs(b.vx);
        rect(x - (vertical ? 1 : len / 2), b.y - (vertical ? len / 2 : 1), vertical ? 2 : len,
             vertical ? len : 2, wp.tracerColor);
        rect(x - 1, b.y - 1, 2, 2, wp.flashColor);
      }
    }
  }
  // Foreground silhouettes are authored into the scene plate; no repeating fascia.

  cinematicGrade(g);
  SDL_RenderSetScale(r,baseScaleX,baseScaleY);
  offsetX = offsetY = 0;
  contextualHud(g,v);
  if (g.flash > 0 && v.shake) rect(0,0,W,H,0xF4D8A030);
}
void Renderer::render(const Game &g, const ViewState &v) {
  SDL_Texture *destination=SDL_GetRenderTarget(r);
  SDL_Rect viewport;SDL_RenderGetViewport(r,&viewport);
  float scaleX,scaleY;SDL_RenderGetScale(r,&scaleX,&scaleY);
  const bool nativeCanvas=canvas && SDL_SetRenderTarget(r,canvas)==0;
  if(nativeCanvas) {
    SDL_RenderSetViewport(r,nullptr);
    SDL_RenderSetScale(r,1,1);
  }
  backgroundMotion=0;
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
    loadWorkshop();
    sprite(workshopPlate,0,0,0,W,H);
    rect(0, 0, 480, 272, 0x06132162);
    rect(0, 0, 480, 7, INK);
    rect(0, 265, 480, 7, INK);
    text("OPERATION / IRON GRID", 27, 30, 1, GOLD);
    text("IRON", 24, 48, 5, 0xA55531FF);
    text("IRON", 22, 45, 5, CREAM);
    text("COAST", 25, 92, 5, 0xA55531FF);
    text("COAST", 22, 88, 5, GOLD);
    text("ONE COAST. SIX FRONTS. ONE LAST SIGNAL.", 26, 135, 1, CREAM);
    groundedSprite(workshopHero,8+int(v.clock*3)%4,330,96,100,100);
    const char *opts[] = {"START CAMPAIGN", "CAMPAIGN MAP", "SETTINGS", "CONTROLS", "EXIT"};
    for (int i = 0; i < 5; i++) {
      if (v.menu == i) {
        rect(21, 158 + i * 16, 202, 13, 0x0A1C27E5);
        rect(21, 158 + i * 16, 2, 13, GOLD);
      }
      text((v.menu == i ? "> " : "  ") + std::string(opts[i]), 29, 161 + i * 16, 1,
           v.menu == i ? GOLD : CREAM);
    }
    text(controlHint("CROSS  SELECT", "ENTER / Z  SELECT"), 26, 248, 1, TEAL);
    text("PS VITA HOMEBREW", 337, 249, 1, CREAM);
  }
  if (v.screen == Screen::Map) {
    rect(0, 0, 480, 272, 0x061525E8);
    text("COASTAL FRONT", 18, 14, 3, CREAM);
    text("CLEARED " + std::to_string(v.completedMissions()) + "/6   RESCUE RECORD " +
         std::to_string(v.rescueRecord()) + "/18", 20, 42, 1, TEAL);
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
    text(controlHint("< > SELECT   CROSS START   CIRCLE BACK", "< > SELECT   ENTER / Z START   ESC BACK"), 20, 245, 1, CREAM);
    if (v.selected > v.unlocked && !v.assist)
      text("COMPLETE THE PREVIOUS MISSION", 20, 234, 1, RED);
    else if (v.missionRescues[v.selected] >= 0)
      text("CLEARED / BEST RESCUE " + std::to_string(v.missionRescues[v.selected]) + "/3",
           20, 234, 1, TEAL);
    else
      text("MISSION NOT CLEARED", 20, 234, 1, GOLD);
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
    static constexpr const char *routes[]={"UPPER GALLERIES / WORKER EXTRACTION",
      "PUMP WALKWAYS / TOXIC VENT CYCLES","ROOF BOARDING / SHIELD FLANKS",
      "COOLING GANTRIES / PRESS TIMING","VERTICAL ASCENT / DRONE AMBUSHES",
      "ARMORED DECKS / FINAL GRID SHUTDOWN"};
    text(routes[v.selected], 33, 202, 1, TEAL);
    text(controlHint("CROSS  DEPLOY", "ENTER / Z  DEPLOY"), 33, 231, 1, GOLD);
  }
  if (v.screen == Screen::Pause) {
    rect(0, 0, 480, 272, 0x071320C8);
    text("PAUSED", 167, 45, 4, CREAM);
    const char *choices[] = {"RESUME", "CONTROLS", "MAIN MENU"};
    for (int i = 0; i < 3; ++i) {
      if (v.menu == i) rect(137, 92 + i * 22, 208, 18, 0x071320E8);
      text((v.menu == i ? "> " : "  ") + std::string(choices[i]),
           149, 98 + i * 22, 1, v.menu == i ? GOLD : CREAM);
    }
    text(controlHint("CROSS SELECT / CIRCLE OR START RESUME",
                     "ENTER SELECT / ESC OR P RESUME"), 90, 176, 1, TEAL);
    text(v.menu == 2 ? "LEAVES THIS MISSION. UNLOCKS ARE KEPT." :
         controlHint("UP / DOWN SELECT", "UP / DOWN SELECT    M SOUND"), 90, 191, 1, CREAM);
    text("LIVES "+std::to_string(g.player.lives)+"  HEALTH "+std::to_string(g.player.health)+"  SCORE "+std::to_string(g.score),90,210,1,CREAM);
    text("AMMO "+(g.player.weapon?std::to_string(g.player.ammo):std::string("UNLIMITED"))+"  GRENADES "+std::to_string(g.player.grenades)+"  RESCUED "+std::to_string(g.rescued)+"/3",90,224,1,CREAM);
    text(g.level().name,90,238,1,GOLD);
  }
  if (v.screen == Screen::Play && g.status == Status::GameOver) {
    rect(0, 0, 480, 272, 0x101523CB);
    text("SIGNAL LOST", 61, 89, 3, RED);
    text(controlHint("CROSS  CONTINUE FROM CHECKPOINT", "ENTER / Z  CONTINUE FROM CHECKPOINT"), 92, 140, 1, CREAM);
    text(controlHint("CIRCLE  MAIN MENU", "ESC / B  MAIN MENU"), 182, 160, 1, GOLD);
  }
  if (v.screen == Screen::Debrief) {
    rect(0, 0, 480, 272, 0x071A27EB);
    text("MISSION COMPLETE", 35, 31, 3, GOLD);
    wrapped(g.level().ending, 35, 78, 1, 405, CREAM);
    text("SCORE  " + std::to_string(g.score), 35, 128, 2, CREAM);
    text("RESCUED  " + std::to_string(g.rescued) + " / 3", 35, 153, 1, TEAL);
    text("ENEMIES  " + std::to_string(g.kills), 35, 172, 1, CREAM);
    if (g.rescued >= 3) text("ALL WORKERS EXTRACTED", 35, 190, 1, TEAL);
    if (g.routeControlCount())
      text("CIRCUITS DISABLED  " + std::to_string(std::count(g.routeDisabled.begin(),
           g.routeDisabled.end(), true)) + "/3", 35, 207, 1, TEAL);
    text(g.levelIndex==5?controlHint("CROSS  EPILOGUE","ENTER / Z  EPILOGUE"):
         controlHint("CROSS  NEXT MISSION", "ENTER / Z  NEXT MISSION"), 35, 228, 1, GOLD);
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
    text(v.assist ? "TRAINING RUN / RECORDS UNCHANGED" :
         v.completedMissions() == 6 ? "ALL SIX MISSIONS CLEARED" : "FINAL MISSION CLEARED",
         30, 231, 1, TEAL);
    text(controlHint("CROSS  MAIN MENU", "ENTER / Z   MAIN MENU"), 30, 245, 1, CREAM);
  }
  if (v.screen == Screen::Options) {
    rect(0, 0, 480, 272, 0x071A27E8);
    text("SETTINGS", 27, 30, 3, GOLD);
    std::vector<std::string> opts = {
        std::string("SOUND: ") + (v.muted ? "OFF" : "ON"),
        std::string("SCREEN SHAKE: ") + (v.shake ? "ON" : "OFF"),
        std::string("MODE: ") + (v.assist ? "TRAINING / NO DAMAGE" : "ARCADE / "+std::to_string(int(tuning::campaignPresentation.health))+" HP"),
        std::string("FULLSCREEN: ") + (v.fullscreen ? "ON" : "OFF"), "BACK"};
    for (int i = 0; i < 5; i++)
      text((v.menu == i ? "> " : "  ") + opts[i], 30, 91 + i * 25, 1, i == v.menu ? GOLD : CREAM);
    text("TRAINING MODE DOES NOT UNLOCK THE CAMPAIGN.", 30, 241, 1, TEAL);
  }
  if (v.screen == Screen::Controls) {
    rect(0, 0, 480, 272, 0x071A27ED);
    text("CONTROLS", 25, 24, 3, GOLD);
#ifdef KH_VITA
    const char *rows[] = {"MOVE         D-PAD / LEFT STICK",
                         "JUMP         CROSS",
                         "FIRE         SQUARE",
                         "GRENADE      CIRCLE / R",
                         "INTERACT     TRIANGLE / RELOAD L",
                         "AIM UP       UP + SQUARE",
                         "AIM DOWN     AIR DOWN + SQUARE",
                         "PAUSE        START"};
#else
    const char *rows[] = {"MOVE         ARROWS / WASD       D-PAD / LEFT STICK",
                          "JUMP         Z / SPACE          PAD A",
                          "FIRE         X / J              PAD X",
                          "GRENADE      C / K              PAD B / RB",
                          "INTERACT     E / PAD Y   RELOAD R / LB",
                          "AIM UP       UP + FIRE",
                          "AIM DOWN     AIR DOWN + FIRE",
                          "PAUSE        ESC / P            START"};
#endif
    for (int i = 0; i < 8; i++)
      text(rows[i], 23, 69 + i * 18, 1, CREAM);
    text("CLIMB        UP / DOWN   JUMP TO RELEASE", 23, 218, 1, CREAM);
    text(controlHint("CROSS / CIRCLE  BACK", "ENTER / ESC  BACK"), 25, 244, 1, TEAL);
  }
  if (v.screen == Screen::Play || v.screen == Screen::Pause) {
    // Keep small character details clear; only the frame edges are shaded.
    for (int i = 0; i < 8; i++) {
      uint32_t shade = 0x020A1014 | uint32_t((18 - i * 2) & 255);
      rect(0, i, 480, 1, shade);
      rect(0, 264 - i, 480, 1, shade);
      rect(i, 0, 1, 272, shade);
      rect(479 - i, 0, 1, 272, shade);
    }
  }
  if (v.saveFailed) {
    rect(0,H-14,W,14,0x251512F0);
    text("PROGRESS NOT SAVED - CHECK STORAGE",12,H-10,1,0xFFB283FF);
  }
  if(nativeCanvas) {
    SDL_SetRenderTarget(r,destination);
    SDL_RenderSetViewport(r,&viewport);
    SDL_RenderSetScale(r,scaleX,scaleY);
    const SDL_FRect screen{0,0,float(W),float(H)};
    SDL_RenderCopyF(r,canvas,nullptr,&screen);
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
