#include "game.h"
#include "animation.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace kh;
static void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main(){try{
  for(int chapter=0;chapter<6;++chapter) {
    Game g;g.beginChapter(chapter);g.debugInvincible=true;
    check(g.chapterSequence && g.cinematicHero(),"chapter lost production presentation");
    check(g.workshopReview==(chapter==0) && g.ironlineReview==(chapter==2),"wrong chapter opening");
    if(!g.cinematicReview())continue;
    check(!g.advanceSection(),"unsecured room skipped");
    // Run through the actual room and defeat sentries with real projectiles.
    for(int n=0;n<2400 && !g.workshopSecured;++n) {
      Input in;in.shoot=true;in.interact=true;in.move=1;
      in.jump=g.player.grounded && g.floorAt(g.player.x+18,g.player.y-2)>g.player.y+18;
      g.update(in);
    }
    check(g.workshopSecured && g.kills>0,"chapter opening cannot be completed");
    g.player.health=4;g.player.weapon=g.player.magazineWeapon=g.player.firedWeapon=2;
    g.player.ammo=13;g.player.magazine=2;g.player.grenades=3;
    int score=g.score,kills=g.kills;float scale=g.player.presentationScale;
    for(int n=0;n<90 && g.cinematicReview();++n){g.update({});g.advanceSection();}
    check(!g.cinematicReview() && g.chapterSequence && g.levelIndex==chapter,"opening did not enter its mission");
    check(g.score==score && g.kills==kills && g.player.health==4,"section resets chapter progress");
    check(g.player.weapon==2 && g.player.ammo==13 && g.player.magazine==2 && g.player.magazineWeapon==2 && g.player.grenades==3,"section resets equipment");
    check(g.player.presentationScale==scale && g.player.y==232,"actor scale or footing jumps at section boundary");
    check(!g.advanceSection(),"section advanced twice");
    g.beginChapter(chapter);g.retry();
    check(g.chapterSequence && g.cinematicReview() && !g.workshopSecured,"retry loses chapter opening");
    g.debugInvincible=false;g.player.inv=0;g.player.health=1;g.hitPlayer();
    for(int n=0;n<75;++n)g.update({});
    const float floor=chapter==2?tuning::ironlineReview.roofY:232.f;
    check(g.status==Status::Play && std::fabs(g.player.y-floor)<.1f,"real death respawns below the section floor");
  }
  for(int kind=0;kind<5;++kind) {
    Game g;g.load(0);g.enemies.clear();g.props.clear();g.player.x=100;
    Enemy e;e.active=true;e.kind=kind;e.x=e.origin=210;e.y=e.baseY=232;e.hp=e.maxhp=30;e.timer=999;e.state=2;
    g.enemies.push_back(e);
    if(kind<3){
      auto muzzle=muzzlePoint(g.player,{});auto body=g.enemyBox(e);
      check(muzzle.y>=body.y && muzzle.y<=body.y+body.h,"hero shoots over a grounded enemy class");
    }
    check(g.cinematicGuard(e)==(kind<3),"human and mechanical rigs mixed");
  }
  for(int chapter=0;chapter<6;++chapter) {
    Game g;g.load(chapter);g.debugInvincible=true;g.enemies.clear();
    const float boundary=g.level().width-W+tuning::campaignPresentation.bossArenaInset;
    g.player.x=boundary-4;g.player.vx=145;g.camera=g.player.x-210;g.syncPresentation();
    bool entered=false;
    for(int n=0;n<150;++n) {
      const float oldX=g.player.x,oldCamera=g.camera;
      Input in;in.move=g.boss.active?0:1;g.update(in);
      check(std::fabs(g.camera-oldCamera)<=tuning::campaignPresentation.cameraMaxSpeed/60.f+.01f,
            "boss entry cuts the camera instead of tracking");
      check(std::fabs(g.player.x-oldX)<4,"boss entry teleports the player");
      if(g.boss.active) {
        entered=true;
        check(g.player.x-g.camera>=tuning::campaignPresentation.bossArenaInset-.1f,
              "boss framing loses player at the left edge");
      }
    }
    check(entered,"boss entry no longer triggers");
    check(g.level().width-W-g.camera<1,"boss camera never settles on the arena");
    for(const auto &prop:g.props) {
      check(std::fabs(prop.y-g.floorAt(prop.x,prop.y-.1f))<.1f,"prop floats above its supporting floor");
      auto box=g.propBox(prop);
      check(std::fabs(box.y+box.h-prop.y)<.01f,"prop collision does not end at its feet");
      check(segmentRect(box.x-10,box.y+box.h/2,box.x+box.w+10,box.y+box.h/2,box),
            "grounded prop cannot receive a swept projectile");
    }
  }
  {
    // Exercise the real chapter sequence through the final boss. Invulnerability
    // isolates progression from the bot's lack of human dodging; no boss damage
    // injection, room teleport or gate bypass is used.
    Game g;int transitions=0;
    for(int chapter=0;chapter<6;++chapter) {
      g.beginChapter(chapter,chapter>0);g.debugInvincible=true;
      for(int n=0;n<36000 && g.status!=Status::Clear;++n) {
        Input in;in.shoot=true;in.move=g.boss.active?0:1;
        in.interact=g.cinematicReview();
        in.jump=g.player.grounded && (g.floorAt(g.player.x+18,g.player.y-2)>g.player.y+18 || (chapter==4 && g.boss.active));
        in.grenade=g.boss.active && n%90==1;
        g.update(in);if(g.advanceSection())++transitions;
        check(std::isfinite(g.player.y) && g.player.y<300,"campaign section loses its floor");
      }
      check(g.status==Status::Clear,"production chapter timed out before completion");
      std::cout<<"CLEAR production chapter="<<chapter+1<<" score="<<g.score<<'\n';
    }
    check(transitions==2,"opening sections were skipped or repeated");
  }
  std::cout<<"PASS six chapter entries, playable room exits, equipment continuity, retry and mixed-class scale\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
