#include "arcade_review.h"
#include <iostream>
#include <stdexcept>
using namespace kh;
static void check(bool ok,const char *message) { if(!ok)throw std::runtime_error(message); }
int main() { try {
  int normalClears=0;
  for(bool assisted:{true,false})for(int chapter=0;chapter<6;++chapter) {
    Game game;game.load(chapter);game.debugInvincible=assisted;
    ArcadeReview pilot;
    int jumps=0,grenades=0,hostile=0,frames=0;
    for(;frames<18000 && game.status!=Status::Clear && game.status!=Status::GameOver;++frames) {
      const auto controls=pilot.input(game);
      jumps+=controls.jump;grenades+=controls.grenade;
      game.update(controls);
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
  check(normalClears>=5,"ordinary-damage combat regressed below five completed stages");
  std::cout<<"PASS six assisted combat routes; ordinary-damage clears="<<normalClears<<"/6\n";
} catch(const std::exception &error) { std::cerr<<error.what()<<'\n';return 1; } }
