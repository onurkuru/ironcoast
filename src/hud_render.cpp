#include "render.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Renderer::contextualHud(const Game &g,const ViewState &v) {
  if(v.screen!=Screen::Play)return;
  const auto &h=tuning::hud;const auto &p=g.player;
  auto color=[&](uint32_t c,float life){return (c&0xFFFFFF00u)|uint32_t(255*h.opacity*std::clamp(life/h.fadeDuration,0.f,1.f));};
  if(p.healthNotice>0)for(int i=0;i<p.maxHealth;++i)
    rect(h.margin+i*8,H-39,5,3,color(h.healthColor,p.healthNotice)*(i<p.health?1:0));
  if(p.fireAge<h.weaponDuration || p.reloadTime>0) {
    uint32_t c=color(h.weaponColor,p.reloadTime>0?1:h.weaponDuration-p.fireAge);
    // Small equipment silhouette; no frame, scoreboard or persistent header.
    rect(h.margin,H-24,14,3,c);rect(h.margin+2,H-21,3,5,c);
    line(h.margin+4,H-25,h.margin+10,H-25,c);
    text(p.reloadTime>0?"RELOAD":std::to_string(p.magazine),h.margin+22,H-25,1,c,false);
  }
  if(g.discoveryTime>0)text("HIDDEN RELIC FOUND",h.margin,H-55,1,color(tuning::discovery.color,g.discoveryTime),false);
  if(g.workshopSecured)text(g.ironlineReview?"CARRIAGES SECURED":"WORKSHOP SECURED",W/2-64,H-30,1,h.weaponColor);
  else if((g.ironlineReview && g.player.x>tuning::ironlineReview.exitX-20) ||
          (g.workshopReview && g.player.x>tuning::workshop.exitX-20))
    text(
#ifdef KH_VITA
      "TRIANGLE - EXIT",
#else
      "E / TRIANGLE - EXIT",
#endif
      W/2-68,H-30,1,h.weaponColor);
  // Radio text is cinematic subtitle information, not a HUD panel.
  if(!g.cinematicReview() && g.player.x>g.level().width*.45f && g.player.x<g.level().width*.55f)
    wrapped(g.level().radio,30,H-36,1,420,h.weaponColor);
  if(g.chapterSequence && g.cinematicReview() && g.workshopSecured)
    rect(0,0,W,H,uint32_t(255*std::clamp(g.sectionExitAge/tuning::campaignPresentation.exitHold,0.f,1.f)));
}
} // namespace kh
