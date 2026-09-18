#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace kh;
void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(){try {
  // Three seconds of sustained fire must cross magazine boundaries without
  // stalling, while still spending exactly one reserve round per powered shot.
  for(int weapon=0;weapon<6;++weapon) {
    Game arcade;arcade.load(0);arcade.enemies.clear();arcade.props.clear();arcade.items.clear();
    arcade.player.weapon=arcade.player.magazineWeapon=weapon;
    arcade.player.magazine=1;arcade.player.ammo=200;
    Input trigger;trigger.shoot=true;int shots=0,gap=0,longest=0;
    for(int tick=0;tick<180;++tick) {
      arcade.update(trigger);++gap;
      check(arcade.player.reloadTime==0,"Arcade fire stalled for a magazine");
      if(arcade.player.fireAge==0) {++shots;longest=std::max(longest,gap);gap=0;}
    }
    check(shots>=8 && longest<=24,"Arcade cadence has long dead time");
    if(weapon)check(arcade.player.ammo==200-shots,"Arcade refill duplicated reserve ammunition");
    arcade.player.magazine=0;trigger.shoot=false;trigger.reload=true;arcade.update(trigger);
    check(arcade.player.reloadTime>0,"Arcade manual reload was removed");
  }
  {
    Game arcade;arcade.load(0);arcade.enemies.clear();arcade.items.clear();arcade.props.clear();
    Enemy behind;behind.active=true;behind.x=arcade.player.x-20;behind.y=232;behind.hp=20;behind.entryAge=1;
    arcade.enemies.push_back(behind);Input trigger;trigger.shoot=true;arcade.update(trigger);
    check(arcade.enemies[0].hp==20 && arcade.player.actionKind!=1,"Melee hit an enemy behind the player");
    arcade.damageEnemy(arcade.enemies[0],1,false);float before=arcade.time;arcade.update({});
    check(arcade.time>before,"Ordinary bullets froze arcade movement");
    for(int kind=0;kind<3;++kind) {
      behind.kind=kind;Rect body=arcade.enemyBox(behind);
      check(std::fabs(body.h-arcade.playerBox().h)<3,"Human enemy anatomy mismatches hero height");
    }
    auto &shield=arcade.enemies[0];shield.kind=2;shield.dir=-1;shield.state=1;shield.timer=.2f;
    shield.flinch=0;shield.hurt=0;shield.x=arcade.player.x-70;shield.hp=20;
    arcade.update({});check(shield.dir==-1,"Shield instantly turned during committed windup");
    arcade.damageEnemy(shield,2,false,1);check(shield.hp==20,"Frontal shield block missing");
    arcade.damageEnemy(shield,2,false,-1);check(shield.hp==18,"Shield back cannot be flanked");
  }
  for(int w=0;w<6;++w) {
    Game g;g.load(0,false,40,true);g.enemies.clear();g.props.clear();g.items.clear();g.debugInvincible=true;
    g.player.weapon=w;g.player.ammo=200;g.player.magazineWeapon=w;g.player.magazine=1;
    Input fire;fire.shoot=true;g.update(fire);
    check(g.player.magazine==0,"Last round must empty the magazine");
    check(std::any_of(g.audioEvents.begin(),g.audioEvents.end(),[&](const auto&e){return int(e.sound)==w;}),"Weapon fire event missing");
    int ammo=g.player.ammo;g.update(fire);
    check(g.player.reloadTime>0,"Empty magazine must start reload");
    check(g.player.ammo==ammo,"Reload must not create or consume reserve ammo");
    for(int i=0;i<120;++i)g.update({});
    check(g.player.magazine==int(tuning::weapons[w].magazine),"Reload failed to refill magazine");
    check(g.player.ammo==ammo,"Waiting reload changed ammo");
    fire.reload=true;fire.shoot=false;g.update(fire);
    check(g.player.reloadTime==0,"Full magazine should not reload repeatedly");
    g.player.magazine=0;g.player.shot=0;g.update(fire);
    check(g.player.reloadTime>0,"Manual reload missing");
  }
  Game g;g.load(0,false,40,true);g.enemies.clear();g.debugInvincible=true;
  Enemy e;e.active=true;e.x=250;e.y=232;e.hp=100;
  for(int zone=0;zone<3;++zone) {
    Rect body=g.enemyBox(e);
    Bullet b;b.px=200;b.x=300;b.py=b.y=body.y+body.h*(zone==0?.08f:zone==1?.5f:.9f);
    check(int(g.hitZone(e,b))==zone,"Swept hit location classified incorrectly");
    g.damageEnemy(e,1,false,zone%2?1:-1,HitZone(zone));
    check(e.flinch>0 && e.hitZone==HitZone(zone),"Missing localized reaction");
  }
  Bullet vertical;vertical.px=vertical.x=250;vertical.py=180;vertical.y=250;
  check(g.hitZone(e,vertical)==HitZone::Head,"Downward shot should enter head");
  vertical.py=250;vertical.y=180;check(g.hitZone(e,vertical)==HitZone::Legs,"Upward shot should enter legs");
  float time=g.time;g.update({});check(g.time==time,"Hit-stop did not freeze simulation");
  for(int i=0;i<12;++i)g.update({});check(g.time>time,"Hit-stop failed to expire");
  e.hp=1;g.damageEnemy(e,4,true,-1,HitZone::Head);
  check(e.dead && e.death==e.deathDuration && e.deathDuration>0 && e.hitDir==-1,"Directional death missing");
  for(size_t i=0;i<tuning::secrets.size();++i) {
    const auto &s=tuning::secrets[i];g.load(int(s.map));
    check(std::fabs(g.floorAt(s.x,s.y-1)-s.y)<.01f,"Secret is not on a walkable roof");
    g.player.x=s.x;g.player.y=s.y;g.updateDiscoveries(DT);
    check(g.discoveries==(1u<<i) && g.discoveryTime>0,"Discovery missing");
    int score=g.score;g.updateDiscoveries(DT);check(g.score==score,"Discovery awarded twice");
    g.retry();check(g.discoveries==(1u<<i),"Continue lost discovery state");
    g.load((int(s.map)+1)%6,true);check(g.discoveries==(1u<<i),"Stage transition lost discovery");
  }
  for(int stage=0;stage<6;++stage) {
    g.load(stage,false,campaign()[stage].width-490);g.debugInvincible=true;
    for(int tick=0;tick<300;++tick) {
      g.update({});
      check(std::isfinite(g.cameraZoom) && g.cameraZoom>=1 && g.cameraZoom<=tuning::camera.bossZoom+.001f,"Camera zoom escaped bounds");
      check(g.camera>=0 && g.camera<=g.level().width-W,"Camera escaped map");
      check(g.cameraY<=0 && g.cameraY>=g.level().minY,"Camera escaped vertical bounds");
    }
    check(g.cameraZoom<1.002f,"Boss introduction did not return to combat framing");
  }
  std::cout<<"Six magazines, manual/automatic reload, swept hit zones, finite hit-stop, deaths and discoveries passed\n";
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;} }
