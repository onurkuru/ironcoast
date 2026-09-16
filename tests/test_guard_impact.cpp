#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace kh;
static void check(bool ok,const char *msg){if(!ok)throw std::runtime_error(msg);}
int main(){try{
 Game g;g.load(0,false,130,true);g.debugInvincible=true;g.update({});
 auto &e=g.enemies.front();e.active=true;e.state=1;e.timer=999;e.hp=1;
 for(auto &p:g.particles)p={};
 auto body=g.enemyBox(e);
 g.fire(body.x-2,body.y+body.h*.5f,600,0,1,0,false);
 g.update({});check(e.dead,"test projectile did not kill the guard");
 int sparks=0;
 for(const auto &p:g.particles)if(p.life>0){
  check(p.kind==Particle::GuardSpark,"duplicate generic square/explosion particles remain");++sparks;
 }
 check(sparks==int(tuning::guardImpact.sparkCount),"guard impact exceeds its particle budget");
 int landings=0;
 for(int i=0;i<180;++i){
  g.update({});
  for(const auto &event:g.audioEvents)if(event.sound==Sound::BodyLand){
   ++landings;
   float elapsed=e.deathDuration-e.death;
   float expected=tuning::guardReactions.collapseDuration*tuning::guardImpact.landingPhase;
   check(elapsed>=expected && elapsed<expected+DT*1.1f,"body landing cue is not tied to ground-contact frame");
   check(event.y==e.y,"body landing cue is detached from the floor");
  }
  for(const auto &p:g.particles)if(p.life>0 && (p.kind==Particle::GuardSpark || p.kind==Particle::BodyDust))
   check(std::isfinite(p.x) && std::isfinite(p.y) && p.y<=p.floor,"impact particles cross below the floor");
 }
 check(landings==1 && e.landingEffectPlayed,"body landing repeats or never plays");
 check(std::none_of(g.particles.begin(),g.particles.end(),[](const auto&p){return p.life>0;}),"impact particles do not expire");
 for(size_t i=0;i<g.particles.size();++i){auto &p=g.particles[i];p.life=1;p.kind=Particle::BodyDust;p.x=float(i);}
 g.guardImpact(300,232,232,1,true);
 for(size_t i=0;i<g.particles.size();++i)check(g.particles[i].life==1 && g.particles[i].x==float(i),"saturated effect overwrites active particles");
 std::cout<<"Guard impact budget, real projectile path, one-shot landing and floor bounds passed\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
