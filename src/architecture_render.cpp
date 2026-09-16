#include "render.h"
#include "animation.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>

namespace kh {
void Renderer::architecture(const Game &g, float camera, float time) {
  if(g.levelIndex==0 && !g.cinematicReview()) {
    // The frame, lintel, ladders, gallery and wall are one authored painting.
    // Animate only its measured door panel; no foreign kit is overlaid here.
    const auto &rig=tuning::harborBuiltScene;
    const float width=g.level().width,height=width/rig.sourceAspect;
    const float top=232-height*rig.sourceFloor;
    for(size_t i=0;i<tuning::harborDoors.size();++i) {
      float age=i<g.entranceAges.size()?g.entranceAges[i]:-1;
      if(age<0)continue;
      const auto &door=tuning::harborDoors[i];
      float x=door.u*width-camera,y=top+door.v*height,w=door.w*width,h=door.h*height;
      if(x+w<0 || x>W)continue;
      float opening=ease(age/rig.doorOpenDuration);
      rect(x,y,w,h,rig.doorInterior);
      rect(x+3,y+3,w-6,h-3,rig.doorShade);
      if(opening<1 && scenePlate.texture) {
        SDL_Rect source{int(door.u*scenePlate.width),int(door.v*scenePlate.height+door.h*scenePlate.height*opening),
          std::max(1,int(door.w*scenePlate.width)),std::max(1,int(door.h*scenePlate.height*(1-opening)))};
        SDL_FRect dest{x+offsetX,y+offsetY,w,h*(1-opening)};
        SDL_RenderCopyF(r,scenePlate.texture,&source,&dest);
      }
    }
    return;
  }
  // The scene paintings provide architecture. Only interactive doors and
  // climbable rails are composited here, using the exact collision geometry.
  for (size_t i = 0; i < g.level().entrances.size(); ++i) {
    const auto &door = g.level().entrances[i];
    float x = door.x - camera, y = door.y;
    if (x < -40 || x > W + 40) continue;
    float age = i < g.entranceAges.size() ? g.entranceAges[i] : -1;
    const auto &rig=tuning::productionProps;
    float opening=age<0?0:ease(age/rig.doorOpenDuration);
    const float width=rig.doorWidth,height=rig.doorHeight;
    groundedSprite(productionProps,5,x-width/2,y-height,width,height);
    SDL_Rect prior{};bool clipped=SDL_RenderIsClipEnabled(r);SDL_RenderGetClipRect(r,&prior);
    SDL_Rect shutter{int(std::floor(x-width/2+offsetX)),int(std::floor(y-height+offsetY)),
                    int(std::ceil(width)),int(std::ceil(height*(1-opening)))};
    if(clipped)SDL_IntersectRect(&shutter,&prior,&shutter);
    SDL_RenderSetClipRect(r,&shutter);
    if(opening<1)groundedSprite(productionProps,4,x-width/2,y-height,width,height);
    SDL_RenderSetClipRect(r,clipped?&prior:nullptr);
    uint32_t warning = age >= 0 && age < 2.2f && int(time * 8) % 2 ? 0xFFC378FF : 0x796443FF;
    if(age>=0 && age<2.2f)softLight(x,y-height+8,7,3,warning,50);
  }
  for (const auto &ladder : g.level().ladders) {
    float x = ladder.x - camera;
    if (x < -16 || x > W + 16) continue;
    // A traversable structure must stay visible; proximity must not make
    // collision geometry appear out of an otherwise empty wall.
    // Keep traversal readable without the bright debug-like ladder lines that
    // previously dominated the cinematic plates.
    rect(x - 5, ladder.top - 9, 2, ladder.bottom - ladder.top + 9, 0x02070BC0);
    for (float rail : {-8.0f, 8.0f}) {
      line(x + rail + 2, ladder.top - 10, x + rail + 2, ladder.bottom, 0x071019EE);
      line(x + rail, ladder.top - 10, x + rail, ladder.bottom, 0x36535DCC);
    }
    for (float y = ladder.top - 2; y < ladder.bottom; y += 8)
      line(x - 8, y, x + 8, y, 0x44656ACC);
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
    rect(x, wet.y + 1, wet.w, wet.h, 0x0A263118);
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
    // Keep filtered source pixels inside the authored puddle. Without this
    // logical clip, SDL's scaled sampler can leave a one-pixel fringe outside
    // the wet surface and it reads as a detached duplicate sprite.
    SDL_Rect previousClip{};
    const bool hadClip = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &previousClip);
    const int clipLeft = int(std::ceil(wet.x - camera + offsetX));
    const int clipRight = int(std::floor(wet.x + wet.w - camera + offsetX));
    SDL_Rect reflectionClip{clipLeft,
                            int(std::floor(wet.y + offsetY + 1)),
                            std::max(1, clipRight - clipLeft),
                            std::max(1, int(std::ceil(wet.h - 1)))};
    if (hadClip) SDL_IntersectRect(&reflectionClip, &previousClip, &reflectionClip);
    SDL_RenderSetClipRect(r, &reflectionClip);
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
    SDL_RenderSetClipRect(r, hadClip ? &previousClip : nullptr);
  }
}
} // namespace kh
