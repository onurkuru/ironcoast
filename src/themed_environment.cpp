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
    contactShadow(x + 9, y + 28, y + 28, 13);
    rect(x-2,y+25,22,3,0x0B151CFF);
    rect(x,y,18,26,0x101E27FF);
    rect(x+1,y+1,16,2,0x70828AFF);
    rect(x+1,y+3,2,21,0x354955FF);
    rect(x+4,y+5,11,9,0x071117FF);
    if (g.levelIndex == 3) {
      ring(x+9,y+10,4,4,signal);
      line(x+9,y+6,x+9+(off?3:0),y+10,signal);
      line(x+5,y+10,x+13,y+10,signal);
    } else {
      for(int row=0;row<3;++row)
        rect(x+5,y+6+row*2,off?8:4+(row+i)%3,1,signal);
    }
    rect(x+5,y+17,3,3,signal);
    line(x+11,y+20,x+(off?14:11),y+16,0xD8D8BFFF);
    text(std::to_string(i+1),x+6,y-10,1,signal);
  }
}
} // namespace kh
