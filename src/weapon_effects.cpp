#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Game::guardImpact(float x,float y,float floor,int direction,bool landing) {
  const auto &c=tuning::guardImpact;
  int count=int(landing?c.dustCount:c.sparkCount);
  for(int i=0;i<count;++i)for(auto &p:particles)if(p.life<=0) {
    p=Particle{};p.kind=landing?Particle::BodyDust:Particle::GuardSpark;
    p.x=p.prevX=x;p.y=p.prevY=y;p.floor=floor;
    p.life=p.maxlife=landing?c.dustLife:c.sparkLife;
    p.color=landing?c.dustColor:c.sparkColor;
    p.size=landing?c.dustSize:1;
    p.vx=landing?(random()*2-1)*c.dustSpeed:direction*c.sparkSpeed*(.4f+.6f*random());
    p.vy=landing?-c.dustLift*(.4f+.6f*random()):(random()-.6f)*c.sparkSpeed;
    break;
  }
  if(landing)audioEvents.push_back({Sound::BodyLand,x,y,-1});
}
HitZone Game::hitZone(const Enemy &enemy, const Bullet &b) const {
  Rect box=enemyBox(enemy);
  // Entry point of the swept segment, including vertical fire. A high-speed
  // projectile can end outside the enemy; classify its intersection, not end.
  float lo=0,hi=1;
  auto slab=[&](float a,float d,float mn,float mx) {
    if(std::fabs(d)<.00001f)return;
    float t0=(mn-a)/d,t1=(mx-a)/d;if(t0>t1)std::swap(t0,t1);
    lo=std::max(lo,t0);hi=std::min(hi,t1);
  };
  slab(b.px,b.x-b.px,box.x,box.x+box.w);slab(b.py,b.y-b.py,box.y,box.y+box.h);
  float y=b.py+(b.y-b.py)*std::clamp(lo,0.f,1.f);
  float f=(y-box.y)/box.h;
  return f<tuning::combat.headThreshold?HitZone::Head:f>tuning::combat.legThreshold?HitZone::Legs:HitZone::Torso;
}
void Game::weaponEffect(float x, float y, int weapon, bool impact, bool audible) {
  const auto &w=tuning::weapons[std::clamp(weapon,0,5)];
  int count=impact ? int(tuning::combat.impactParticles) : (w.casing>0 ? 1 : 3);
  for (int i=0;i<count;++i) for (auto &p:particles) if (p.life<=0) {
    p=Particle{}; p.x=p.prevX=x; p.y=p.prevY=y;
    float angle=random()*6.283185f;
    p.variant=weapon;
    p.kind=impact?6:(w.casing>0?5:7);
    p.life=p.maxlife=impact?tuning::combat.impactLife:w.casingLife;
    p.size=impact?w.impactSize:w.casing;
    p.color=impact?w.tracerColor:(w.casing==2?0xA6513BFFu:0xCCA56AFFu);
    float speed=impact?tuning::combat.impactSpeed:w.casingSpeed;
    p.vx=impact?std::cos(angle)*speed:-player.dir*speed*(.6f+random()*.4f);
    p.vy=impact?std::sin(angle)*speed:-speed;
    if (!impact && w.casing==0) {p.color=w.flashColor;p.size=w.impactSize;p.vx*=.2f;p.vy*=.25f;}
    break;
  }
  if (impact && audible) audioEvents.push_back({Sound::Hit,x,y,weapon});
}
} // namespace kh
