#include "game.h"
#include "animation.h"
#include "presentation_config.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace kh;
void check(bool value,const char *message){if(!value)throw std::runtime_error(message);}
int main(){try{
  const auto &rail=tuning::ironlineReview;
  Game g;g.load(2,false,rail.heroStart,false,true);g.debugInvincible=true;
  check(g.ironlineReview && !g.workshopReview && g.cinematicReview(),"review isolation missing");
  check(std::fabs(g.player.y-rail.roofY)<.01f,"player starts inside carriage");
  check(g.playerBox().h>50 && g.enemies.size()==2 && !g.vehicleAvailable,"wrong rig or encounter");
  for(int i=0;i<1400;++i) {
    Input in;in.shoot=true;in.move=g.kills>0?.6f:0;
    in.jump=g.player.grounded && g.player.x>rail.bridgeEnd-45 && g.player.x<rail.bridgeEnd+12;
    in.interact=g.player.x>rail.exitX;
    g.update(in);
    check(std::isfinite(g.player.y) && g.player.y<rail.bridgeY+5,"route drops through a carriage");
    check(g.camera>=0 && g.camera<=g.level().width-W && !g.boss.active,"campaign camera or boss leaked in");
  }
  check(g.player.x>rail.exitX && g.kills==2 && g.workshopSecured,"carriages cannot be completed");
  g.retry();check(g.ironlineReview && !g.workshopSecured && g.player.y==rail.roofY,"retry loses roof/scene");
  auto muzzle=muzzlePoint(g.player,{});auto body=g.playerBox();
  check(muzzle.y>body.y && muzzle.y<body.y+body.h,"muzzle is outside adult train rig");
  g.load(0,false,205,true);check(g.workshopReview && !g.ironlineReview && g.level().width==640,"train leaks into workshop");
  g.load(2);check(!g.cinematicReview() && g.cinematicHero() && g.player.presentationScale==tuning::workshop.bodyScale,"campaign lost production rig or retained review scene");
  std::cout<<"PASS train roofs, coupler jump, two sentries, exit, retry, adult rig and scene isolation\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
