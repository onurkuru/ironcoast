#include "render.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Renderer::contextualHud(const Game &g,const ViewState &v) {
  if(v.screen!=Screen::Play)return;
  const auto &h=tuning::hud;const auto &p=g.player;
  auto color=[&](uint32_t c,float life){return (c&0xFFFFFF00u)|uint32_t(255*h.opacity*std::clamp(life/h.fadeDuration,0.f,1.f));};
  if(g.arcadeCombat()) {
    rect(0,0,W,22,0x061018C0);
    text("1UP "+std::to_string(p.lives),10,5,1,0xF4D79AFF);
    for(int i=0;i<p.maxHealth;++i)
      rect(10+i*8,15,6,3,i<p.health?0xECA761FF:0x4B4D51FF);
    static constexpr const char *names[]={"P","H","S","R","F","L"};
    const std::string ammo=p.vehicleHP?"VEHICLE":std::string(names[std::clamp(p.weapon,0,5)])+" "+
      (p.weapon?std::to_string(p.ammo):"INF");
    text(ammo,95,7,1,0xF7E6BAFF);
    text("G "+std::to_string(p.grenades),196,7,1,0xF4D79AFF);
    text("SCORE "+std::to_string(g.score),292,7,1,0xF7E6BAFF);
    text(std::to_string(g.levelIndex+1)+"/6",449,7,1,0x91B8C7FF);
    if(p.reloadTime>0)text("RELOAD",95,16,1,0xF4D79AFF);
    if(!v.input.shoot && !p.vehicleHP && p.ladder<0) {
      for(const auto &ladder:g.level().ladders) {
        if(std::fabs(p.x-ladder.x)<12 && p.y>=ladder.top-1 && p.y<=ladder.bottom+1) {
          text(p.y<=ladder.top+1?"DOWN: CLIMB":"UP: CLIMB",95,29,1,0xF4D79AFF);
          break;
        }
      }
    }
    if(v.input.shoot && !p.crouch && !v.input.up && !p.vehicleHP) {
      for(const auto &enemy:g.enemies) {
        const float ahead=(enemy.x-p.x)*p.dir;
        if(enemy.kind==4 && enemy.active && !enemy.dead && ahead>60 && ahead<230 &&
           std::fabs(enemy.y-p.y)<16) {
          text("LOW TARGET: DOWN + FIRE",95,29,1,0xF4D79AFF);
          break;
        }
      }
    }
    if(g.boss.active && !g.boss.dead) {
      rect(142,H-14,196,5,0x071320DC);
      rect(144,H-13,192*std::clamp(g.boss.hp/std::max(1.f,g.boss.maxhp),0.f,1.f),3,0xF47859FF);
      text(g.level().bossName,144,H-24,1,0xF7E6BAFF);
    }
  }
  if(!g.arcadeCombat() && p.healthNotice>0)for(int i=0;i<p.maxHealth;++i)
    rect(h.margin+i*8,H-39,5,3,color(h.healthColor,p.healthNotice)*(i<p.health?1:0));
  if(!g.arcadeCombat() && (p.fireAge<h.weaponDuration || p.reloadTime>0)) {
    uint32_t c=color(h.weaponColor,p.reloadTime>0?1:h.weaponDuration-p.fireAge);
    // Small equipment silhouette; no frame, scoreboard or persistent header.
    rect(h.margin,H-24,14,3,c);rect(h.margin+2,H-21,3,5,c);
    line(h.margin+4,H-25,h.margin+10,H-25,c);
    text(p.reloadTime>0?"RELOAD":std::to_string(p.magazine),h.margin+22,H-25,1,c,false);
  }
  if(g.discoveryTime>0)text("HIDDEN RELIC FOUND",h.margin,H-55,1,color(tuning::discovery.color,g.discoveryTime),false);
  if (g.routeControlCount() && g.status == Status::Play && !g.boss.active) {
    const int near = g.nearbyRouteControl();
    if (g.routeNotice > 0 || near >= 0) {
      rect(24,H-64,432,19,0x071320E8);
      if (g.routeNotice > 0) {
        text("CIRCUIT OFFLINE  +250  SUPPLY SECURED",34,H-58,1,0x7DD2A5FF);
      } else {
        text(std::string(
#ifdef KH_VITA
          "TRIANGLE: "
#else
          "E / PAD Y: "
#endif
          ) + g.routeControlName(),34,H-58,1,0xF5D39AFF);
      }
    }
  }
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
