#include "game.h"
#include "arcade_review.h"
#include "painted_route_review.h"
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
    // Authored paintings do not share the old kit's fixed 26-unit inset.
    // Traverse the real pair attached to this roof.
    std::vector<float> attached;
    for(const auto &ladder:level.ladders)
      if(std::fabs(ladder.top-b.box.y)<.1f && ladder.x>=b.box.x && ladder.x<=b.box.x+b.box.w)
        attached.push_back(ladder.x);
    check(attached.size()==2,"roof route needs its two authored ladders");
    Game g;g.load(stage,false,*std::min_element(attached.begin(),attached.end()));g.debugInvincible=true;
    Input up;up.up=true;ticks(g,up,20+int((232-b.box.y)/72*60));
    check(std::fabs(g.player.y-b.box.y)<.1f,"roof route ascent");
    float target=*std::max_element(attached.begin(),attached.end());
    for (int n=0;n<600 && std::fabs(g.player.x-target)>.01f;++n) {
      // The production rig accelerates and brakes. A proportional controller
      // approaches either side of the marker without assuming instant velocity.
      Input move;move.move=std::clamp((target-g.player.x)*.12f,-1.f,1.f);move.shoot=true;
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
    ticks(g,{},230);
    int count=0;for (const auto &e:g.enemies) if(e.entrance==int(id)){check(e.active,"door reinforcement missing");++count;}
    check(count==5,"door wave is not a finite five-person squad");
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
  {
   // A continuous rescue route through the real chapter sequence. Assist
   // isolates reachability and save-state accounting; this is not a combat
   // balance or art-quality approval. Every movement is ordinary input.
   Game game;int openings=0,controls=0;
   for(int stage=0;stage<6;++stage) {
    game.beginChapter(stage,stage>0);game.debugInvincible=true;
    ArcadeReview combat;PaintedRouteReview route;
    unsigned visited=0;int frames=0;bool returned=false;
    for(;frames<18000 && game.status!=Status::Clear;++frames) {
     Input input;
     if(game.cinematicReview() || route.finished())input=combat.input(game);
     else {
      input=route.input(game);
      // Gallery cabinets can be activated without boarding the road vehicle.
      input.interact=game.nearbyRouteControl()>=0;
     }
     game.update(input);
     if(!game.cinematicReview() && game.player.ladder>=0)visited|=1u<<game.player.ladder;
     if(game.advanceSection())++openings;
     check(std::isfinite(game.player.y) && game.player.y<300,"continuous rescue route lost support");
     if(route.finished() && !returned) {
      check(game.player.grounded && std::fabs(game.player.y-232)<.1f,"full upper route did not return to the road");
      check(!game.boss.active,"upper route prematurely crossed the boss arena lock");
      check(game.rescued==3,"upper route missed a worker before the boss");
      returned=true;
     }
    }
    check(game.status==Status::Clear,"full rescue chapter timed out");
    check(returned && route.finished() && visited==63,"full rescue route skipped a gallery or return ladder");
    check(game.rescued==3,"continuous chapter route missed a worker");
    check(game.totalRescued==(stage+1)*3,"campaign rescue total lost or double-counted a worker");
    for(int control=0;control<game.routeControlCount();++control) {
     check(game.routeDisabled[control],"continuous upper route missed a sabotage cabinet");
     ++controls;
    }
    std::cout<<"RESCUE chapter="<<stage+1<<" workers="<<game.rescued<<" total="<<game.totalRescued
             <<" ladders=6 frames="<<frames<<'\n';
   }
   check(openings==2 && controls==9 && game.totalRescued==18,"full rescue campaign coverage incomplete");
   std::cout<<"PASS assisted continuous rescue campaign: 18 workers, 36 ladders, 9 controls, 2 opening rooms\n";
  }
 }catch(const std::exception &e){std::cerr << "FAIL " << e.what() << '\n';return 1;}
}
