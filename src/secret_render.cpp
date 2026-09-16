#include "render.h"
#include "presentation_config.h"
#include <cmath>
namespace kh {
void Renderer::secretProps(const Game &g,float camera) {
  if(g.cinematicReview())return;
  for(size_t i=0;i<tuning::secrets.size();++i) {
    const auto &s=tuning::secrets[i];if(int(s.map)!=g.levelIndex)continue;
    float x=s.x-camera,y=s.y;
    if(x<-50||x>W+50)continue;
    const auto &p=tuning::discovery;
    float w=s.kind==0?p.plaqueWidth:p.flagWidth, h=s.kind==0?p.plaqueHeight:p.flagHeight;
    // A framed keepsake propped against rooftop equipment. Bottom touches roof.
    rect(x-w/2-2,y-h-4,w+4,h+4,tuning::materials.color36434AFF);
    rect(x-w/2,y-h-2,w,h,s.kind==0?tuning::materials.colorADA793FF:tuning::materials.color722B33FF);
    sprite(s.kind==0?signature:hiddenFlag,0,x-w/2,y-h-2,w,h);
    line(x-w/2-2,y,x+w/2+2,y,tuning::materials.color091822FF);
    if((g.discoveries&(1u<<i)) && g.discoveryTime>0)
      softLight(x,y-h*.5f,w, h*2,p.color,Uint8(50*(g.discoveryTime/p.duration)));
  }
}
} // namespace kh
