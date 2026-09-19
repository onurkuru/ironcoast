#include "render.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Renderer::themedPlatforms(const Game &g,float camera) {
  if(g.cinematicReview() || g.levelIndex<=2)return; // Integrated scenes use their painted architecture.
  const auto &rig=tuning::productionStructures;
  const auto &deck=productionStructures.cells.at(g.levelIndex);
  for(const auto &p:g.level().platforms) {
    if(!p.oneWay)continue;
    const float x=p.box.x-camera,y=p.box.y,w=p.box.w;
    if(x+w<0 || x>W)continue;
    // Load-bearing posts stand behind actors, ending on the physical ground.
    int posts=std::max(2,int(std::ceil(w/rig.postSpacing))+1);
    for(int i=0;i<posts;++i) {
      float px=x+rig.postInset+i*(w-2*rig.postInset)/(posts-1);
      bool doorway=false;
      for(const auto &door:g.level().entrances)
        if(std::fabs(px-(door.x-camera))<tuning::productionProps.doorWidth*.55f)doorway=true;
      if(doorway || px<-rig.postWidth || px>W+rig.postWidth)continue;
      float ground=g.floorAt(px+camera,y+p.box.h+1);
      if(ground>H || ground<=y)continue;
      sprite(productionStructures,g.levelIndex==1?7:6,px-rig.postWidth/2,y+4,rig.postWidth,ground-y-4);
    }
    // Crop the last bay rather than squeezing its texture. Each top matches
    // the collider; all fascia and braces extend below the actors' feet.
    int first=std::max(0,int((camera-p.box.x)/rig.deckWidth));
    for(float start=first*rig.deckWidth;start<w;start+=rig.deckWidth) {
      float span=std::min(rig.deckWidth,w-start);
      if(x+start>W)break;
      float height=rig.deckWidth*deck.h/deck.w;
      SDL_Rect source=deck;source.w=std::max(1,int(deck.w*span/rig.deckWidth));
      SDL_FRect dest{x+start+offsetX,y+offsetY,span,height};
      SDL_RenderCopyF(r,productionStructures.texture,&source,&dest);
    }
  }
  for (int i = 0; i < g.routeControlCount(); ++i) {
    const auto box = g.routeControlBox(i);
    const float x = box.x - camera, y = box.y;
    if (x + box.w < 0 || x > W) continue;
    const bool off = g.routeDisabled[i];
    const uint32_t signal = off ? 0x7DD2A5FF : 0xF5B26CFF;
    const float feet=y+box.h, center=x+box.w*.5f;
    const bool valve=g.levelIndex==3;
    const float size=valve?36.f:34.f;
    contactShadow(center,feet,feet,15);
    // Reuse the authored valve/cabinet materials rather than placing flat
    // rectangular stand-ins over the painted industrial architecture. Their
    // baseline stays on the same deck as the unchanged interaction box.
    actorLight(productionProps,center,feet-16);
    if(off)SDL_SetTextureColorMod(productionProps.texture,166,203,181);
    groundedSprite(productionProps,valve?6:7,center-size*.5f,feet-size,size,size);
    SDL_SetTextureColorMod(productionProps.texture,255,255,255);
    // The marker belongs to the hardware and preserves completed-state and
    // route numbering readability against both snow and furnace backgrounds.
    rect(center-5,feet-size-10,11,10,0x09131DE8);
    text(std::to_string(i+1),center-2,feet-size-8,1,signal);
    rect(center-3,feet-3,6,2,signal);
    if(off) {
      line(center+5,feet-size-6,center+7,feet-size-4,signal);
      line(center+7,feet-size-4,center+10,feet-size-8,signal);
    }
  }
}
} // namespace kh
