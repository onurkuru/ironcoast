#include "arcade_review.h"
#include <iostream>
#include <stdexcept>
using namespace kh;
static void check(bool ok,const char *message) { if(!ok)throw std::runtime_error(message); }
int main(int argc,char **argv) { try {
  const bool traceRelay=argc>1 && std::string(argv[1])=="--trace-relay";
  int normalClears=0;
  for(bool assisted:{true,false})for(int chapter=0;chapter<6;++chapter) {
    Game game;game.load(chapter);game.debugInvincible=assisted;
    ArcadeReview pilot;
    int jumps=0,grenades=0,hostile=0,frames=0;
    for(;frames<18000 && game.status!=Status::Clear && game.status!=Status::GameOver;++frames) {
      const auto controls=pilot.input(game);
      jumps+=controls.jump;grenades+=controls.grenade;
      const auto previous=game.player;
      Bullet nearest;float distance=10000;
      if(traceRelay && chapter==4 && !assisted)for(const auto &bullet:game.bullets) {
        if(!bullet.alive || !bullet.hostile)continue;
        const auto box=game.playerBox();
        const float dx=bullet.x-std::clamp(bullet.x,box.x,box.x+box.w);
        const float dy=bullet.y-std::clamp(bullet.y,box.y,box.y+box.h);
        if(dx*dx+dy*dy<distance){distance=dx*dx+dy*dy;nearest=bullet;}
      }
      game.update(controls);
      if(traceRelay && chapter==4 && !assisted &&
          (game.player.health<previous.health || game.player.vehicleHP<previous.vehicleHP || frames%120==0)) {
        std::cout<<"RELAY t="<<game.time<<" player="<<game.player.x<<','<<game.player.y
                 <<" health="<<game.player.health<<" lives="<<game.player.lives<<" tank="<<game.player.vehicleHP
                 <<" weapon="<<game.player.weapon<<" ammo="<<game.player.ammo<<" boss="<<game.boss.hp
                 <<" boss_xy="<<game.boss.x<<','<<game.boss.y<<" state="<<int(game.boss.state)
                 <<" aim="<<controls.up<<','<<controls.down<<" move="<<controls.move
                 <<" near_bullet="<<nearest.kind<<','<<std::sqrt(distance)<<','<<nearest.vx<<','<<nearest.vy<<'\n';
      }
      for(const auto &bullet:game.bullets)hostile+=bullet.alive&&bullet.hostile;
    }
    std::cout<<"ARCADE assist="<<assisted<<" stage="<<chapter+1<<" frames="<<frames<<" kills="<<game.kills
             <<" jumps="<<jumps<<" grenades="<<grenades<<" hostile_frames="<<hostile
             <<" result="<<(game.status==Status::Clear?"CLEAR":game.status==Status::GameOver?"GAME_OVER":"TIMEOUT")
             <<" x="<<game.player.x<<" boss="<<game.boss.hp<<" lives="<<game.player.lives<<" health="<<game.player.health<<'\n';
    if(assisted && game.status!=Status::Clear)for(const auto &enemy:game.enemies)
      if(!enemy.dead && enemy.active)std::cout<<"LIVE kind="<<enemy.kind<<" x="<<enemy.x<<" y="<<enemy.y<<" hp="<<enemy.hp<<" state="<<enemy.state<<'\n';
    if(assisted)check(game.status==Status::Clear,"arcade pilot cannot complete a stage with ordinary controls");
    else {
      normalClears+=game.status==Status::Clear;
      check(game.boss.active,"ordinary-damage pilot could not reach a stage boss");
      check(game.status==Status::Clear || game.status==Status::GameOver,"ordinary-damage pilot stalled");
    }
    check(game.kills>=10,"arcade review bypassed its ground encounters");
    check(grenades>0 && hostile>0,"arcade review did not exercise combat resources and incoming fire");
    for(float age:game.entranceAges)check(age>=4,"arcade review skipped a reinforcement entrance");
  }
  check(normalClears==6,"ordinary-damage pilot no longer completes all six stages");
  std::cout<<"PASS six assisted combat routes; ordinary-damage clears="<<normalClears<<"/6\n";
  {
    // Play the actual chapter sequence with carried lives; room transitions
    // also preserve equipment through the ordinary advanceSection() path.
    // A continue is the same checkpoint retry exposed by the Game Over menu;
    // record it explicitly instead of silently granting fresh mission lives.
    Game game;int transitions=0,totalRetries=0,totalDeaths=0;
    for(int chapter=0;chapter<6;++chapter) {
      game.beginChapter(chapter,chapter>0);
      ArcadeReview pilot;
      int frames=0,retries=0,deaths=0;
      for(;frames<36000 && game.status!=Status::Clear;++frames) {
        if(game.status==Status::GameOver) {
          check(totalRetries<12,"normal campaign exceeded twelve legitimate continues");
          game.retry();++retries;++totalRetries;
        }
        const int lives=game.player.lives;
        game.update(pilot.input(game));
        if(game.player.lives<lives){++deaths;++totalDeaths;}
        if(game.advanceSection())++transitions;
        check(!game.debugInvincible,"normal campaign accidentally enabled assist");
      }
      std::cout<<"CAMPAIGN stage="<<chapter+1<<" result="<<(game.status==Status::Clear?"CLEAR":"TIMEOUT")
               <<" frames="<<frames<<" deaths="<<deaths<<" continues="<<retries
               <<" lives="<<game.player.lives<<" health="<<game.player.health
               <<" boss="<<game.boss.hp<<'\n';
      check(game.status==Status::Clear,"sequential normal-damage chapter timed out");
    }
    check(transitions==2,"normal campaign skipped an opening room");
    check(game.continues==totalRetries,"recorded campaign continues diverge from game state");
    std::cout<<"PASS normal sequential campaign chapters=6 openings="<<transitions
             <<" deaths="<<totalDeaths<<" continues="<<totalRetries<<'\n';
  }
} catch(const std::exception &error) { std::cerr<<error.what()<<'\n';return 1; } }
