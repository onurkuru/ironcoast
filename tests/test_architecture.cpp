#include "game.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace kh;
static void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
static void ticks(Game &g, Input in, int n) {
  for (int i=0;i<n;++i) {
    g.update(in);
    check(std::isfinite(g.cameraY) && g.cameraY >= g.level().minY-.01f && g.cameraY <= .01f, "vertical camera bounds");
  }
}
int main() {
 try {
  int climbed=0, doors=0;
  for (int stage=0;stage<6;++stage) {
   const auto &level=campaign()[stage];
   check(!level.buildings.empty() && !level.ladders.empty(), "missing playable architecture");
   for (float x=12;x<level.width-12;x+=12) {
    Game g;g.load(stage);
    check(g.floorAt(x,200)==232, "lower vehicle route has a gap");
   }
   for (size_t id=0;id<level.ladders.size();++id) {
    const auto &ladder=level.ladders[id];
    Game g;g.load(stage,false,ladder.x);g.debugInvincible=true;
    g.enemies.clear();g.props.clear();
    Input up;up.up=true;
    ticks(g,up,8+int((ladder.bottom-ladder.top)/72*60)+25);
    check(std::fabs(g.player.y-ladder.top)<.1f && g.player.grounded && g.player.ladder<0,"ladder top exit failed");
    ticks(g,{},20);
    if (ladder.top<120) check(g.cameraY < -10,"camera failed to follow ascent");
    Input down;down.down=true;
    ticks(g,down,8+int((ladder.bottom-ladder.top)/72*60)+25);
    check(std::fabs(g.player.y-ladder.bottom)<.1f && g.player.ladder<0,"ladder descent failed");
    ticks(g,{},90);
    check(std::fabs(g.cameraY)<1,"camera failed to return to road");
    // Aim/shoot at the ladder must not unexpectedly latch the player.
    up.shoot=true;ticks(g,up,5);check(g.player.ladder<0,"firing grabbed ladder");
    up.shoot=false;ticks(g,up,30);check(g.player.ladder>=0,"ladder re-entry failed");
    Input jump;jump.jump=true;jump.move=1;ticks(g,jump,1);
    check(g.player.ladder<0 && g.player.vy<0,"jump release failed");
    g.player.inv=0;g.player.health=1;g.hitPlayer();ticks(g,{},70);
    check(g.status==Status::Play && g.player.ladder<0 && g.player.y==232,"ladder death did not restore safe road checkpoint");
    ++climbed;
   }
   for (const auto &b : level.buildings) {
    if (b.style == 8 || b.box.y >= 232) continue;
    Game g;g.load(stage,false,b.box.x+26);g.debugInvincible=true;
    Input up;up.up=true;ticks(g,up,20+int((232-b.box.y)/72*60));
    check(std::fabs(g.player.y-b.box.y)<.1f,"roof route ascent");
    float target=b.box.x+b.box.w-26;
    for (int n=0;n<600 && g.player.x<target-.01f;++n) {
      Input move;move.move=std::min(1.0f,(target-g.player.x)/(145*DT));move.shoot=true;
      ticks(g,move,1);check(std::fabs(g.player.y-b.box.y)<.1f,"roof route loses support");
    }
    check(std::fabs(g.player.x-target)<.1f,"roof route traversal stuck");
    Input down;down.down=true;ticks(g,down,25+int((232-b.box.y)/72*60));
    check(g.player.y==232 && g.player.ladder<0,"alternate ladder return stuck");
   }
   for (size_t id=0;id<level.entrances.size();++id) {
    const auto &door=level.entrances[id];
    Game g;g.load(stage,false,door.triggerX-20);g.debugInvincible=true;
    ticks(g,{},45);
    check(g.entranceAges[id]<0,"door triggered ahead of its marker");
    g.player.x=door.triggerX+1;ticks(g,{},20);
    for (const auto &e:g.enemies) if(e.entrance==int(id)) check(!e.active,"door spawned before warning finished");
    ticks(g,{},130);
    int count=0;for (const auto &e:g.enemies) if(e.entrance==int(id)){check(e.active,"door reinforcement missing");++count;}
    check(count==2,"door wave is not finite pair");
    auto size=g.enemies.size();ticks(g,{},300);check(g.enemies.size()==size,"door repeated unbounded reinforcements");
    g.retry();ticks(g,{},140);check(g.enemies.size()==size,"retry duplicated door wave");
    ++doors;
   }
   for (const auto &b:level.buildings) if(b.box.y<200 && b.style!=7 && b.style!=8) {
    Game g;g.load(stage);
    float x=b.box.x+b.box.w/2;
    check(g.lightBlocked(x,b.box.y-30,x,b.box.y+30),"roof leaks light");
    check(!g.lightBlocked(x,b.box.y-30,x,b.box.y-2),"light blocked in open air");
   }
  }
  std::cout << "PASS " << climbed << " ladder round trips, " << doors << " finite door encounters, roof occlusion and continuous roads\n";
 }catch(const std::exception &e){std::cerr << "FAIL " << e.what() << '\n';return 1;}
}
