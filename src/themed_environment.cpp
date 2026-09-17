#include "render.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Renderer::themedPlatforms(const Game &g,float camera) {
  if(g.cinematicReview() || g.levelIndex<=1)return; // Harbor and Marsh use their painted architecture.
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
}
} // namespace kh
