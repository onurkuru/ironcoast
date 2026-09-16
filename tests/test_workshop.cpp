#include "game.h"
#include "animation.h"
#include "presentation_config.h"
#include <cmath>
#include <stdexcept>
#include <iostream>
using namespace kh;
static void check(bool c,const char*m){if(!c)throw std::runtime_error(m);}
int main(){try{
 Game g;g.load(0,false,tuning::workshop.heroStart,true);g.debugInvincible=true;
 {
  Player p;p.prevX=10;p.x=30;p.prevY=20;p.y=40;
  p.prevStride=1;p.stride=2;p.prevAnim=2;p.anim=3;
  p.prevClimbCycle=3;p.climbCycle=4;
  auto pose=interpolatedPlayer(p,.25f);
  check(pose.x==15 && pose.y==25 && pose.stride==1.25f && pose.anim==2.25f && pose.climbCycle==3.25f,
        "pose and movement interpolate on different clocks");
  p.fireAge=0;p.prevFireAge=1;
  check(interpolatedPlayer(p,.5f).fireAge==0,"new shot blended with old shot");
  Game motion;motion.load(0,false,tuning::workshop.heroStart,true);motion.enemies.clear();
  Input run;run.move=1;
  motion.update(run);
  check(motion.player.vx>0 && motion.player.vx<145,"cinematic run starts at full speed");
  for(int i=0;i<12;++i)motion.update(run);
  run.move=-1;motion.update(run);
  check(motion.player.vx>0 && motion.player.dir==1,"reversal flips before momentum changes");
  for(int i=0;i<12;++i)motion.update(run);
  check(motion.player.vx<0 && motion.player.dir==-1,"reversal never changes direction");
  motion.player.dir=1;motion.update({});
  check(motion.player.dir==1,"braking overrides the selected aim direction");
  for(int i=0;i<12;++i)motion.update({});
  check(motion.player.vx==0,"braking never settles");
  Player rig=motion.player;rig.grounded=true;rig.vx=100;
  for(int phase=0;phase<8;++phase){
    rig.stride=phase/8.f;rig.dir=1;auto right=muzzlePoint(rig,{});
    rig.dir=-1;auto left=muzzlePoint(rig,{});
    check(std::fabs(right.x+left.x-2*rig.x)<.001f && right.y==left.y,"running muzzle flip loses its root");
    check(right.y<rig.y-35 && right.y>rig.y-65,"running muzzle outside reviewed shoulder band");
  }
  rig.grounded=false;rig.vy=-100;Input airborne;airborne.shoot=true;
  check(useHeroAirFire(rig,airborne) && heroFirePhase(rig)==3,"rising fire pose missing");
  rig.vy=100;check(heroFirePhase(rig)==7,"falling fire pose missing");
  airborne.up=true;check(!useHeroAirFire(rig,airborne),"horizontal fire overrides upward aim");
  Game camera;camera.load(0,false,tuning::workshop.heroStart,true);camera.enemies.clear();
  float initial=camera.camera;
  for(int i=0;i<60;++i)camera.update({});
  check(std::fabs(camera.camera-initial)<.001f,"idle scene drifts immediately after loading");
  for(int i=0;i<180;++i) {
   Input move;move.move=i<90?1:-1;
   float before=camera.camera,lead=camera.cameraLead;camera.update(move);
   check(std::fabs(camera.camera-before)<=tuning::workshopView.maxCameraSpeed*DT+.001f,"camera exceeds speed cap");
   check(std::fabs(camera.cameraLead-lead)<4,"camera look-ahead snaps on reversal");
  }
 }
 {
  Game shot;shot.load(0,false,tuning::workshop.heroStart,true);shot.enemies.clear();
  shot.player.grounded=true;
  shot.updateWorkshopCamera({},DT);
  check(shot.workshopShot==WorkshopShot::Establishing,"opening shot missing");
  shot.time=4;
  for(int i=0;i<240;++i) {
    const float zoom=shot.cameraZoom,y=shot.cameraY;
    shot.updateWorkshopCamera({},DT);
    check(std::fabs(shot.cameraZoom-zoom)<=tuning::workshopCamera.maxZoomSpeed*DT+.00001f,"lens transition snaps");
    check(std::fabs(shot.cameraY-y)<=tuning::workshopCamera.maxVerticalSpeed*DT+.0001f,"vertical shot snaps");
  }
  check(shot.workshopShot==WorkshopShot::Detail && shot.cameraZoom>1.05f,"quiet detail shot missing");
  Enemy e;e.active=true;e.x=shot.player.x+150;e.y=shot.player.y;shot.enemies.push_back(e);
  shot.updateWorkshopCamera({},DT);
  check(shot.workshopShot==WorkshopShot::Combat,"two-subject framing missing");
  shot.enemies.front().dead=true;shot.updateWorkshopCamera({},DT);
  check(shot.workshopShot==WorkshopShot::Combat,"opponent death immediately cuts shot");
  shot.player.ladder=0;shot.player.y=95;Input climb;climb.up=true;
  for(int i=0;i<180;++i)shot.updateWorkshopCamera(climb,DT);
  check(shot.workshopShot==WorkshopShot::Gallery && shot.cameraY<-35,"gallery reveal missing");
  const auto &room=tuning::workshop;
  check(shot.cameraY>=232-room.width/room.sourceAspect*room.sourceFloor-room.cameraLift-.001f,
        "gallery camera reveals space above the source artwork");
  shot.player.ladder=-1;shot.player.y=232;shot.player.x=610;shot.workshopSecured=true;
  for(int i=0;i<180;++i)shot.updateWorkshopCamera({},DT);
  check(shot.workshopShot==WorkshopShot::Exit,"secured exit shot missing");
  check(shot.camera>=0 && shot.camera<=shot.level().width-W && shot.cameraZoom>=1,"shot exposes room edge");
 }
 for(HitZone zone:{HitZone::Head,HitZone::Torso,HitZone::Legs}) {
  Game hit;hit.load(0,false,tuning::workshop.heroStart,true);
  auto &enemy=hit.enemies.front();enemy.active=true;enemy.hp=100;
  auto body=hit.enemyBox(enemy);hit.damageEnemy(enemy,1,false,1,zone);
  check(!hit.audioEvents.empty(),"hit has no spatial audio event");
  float fraction=(hit.audioEvents.back().y-body.y)/body.h;
  check(zone==HitZone::Head?fraction<.26f:zone==HitZone::Legs?fraction>.73f:fraction>.26f && fraction<.73f,
        "impact effect detached from scaled hit region");
  check(enemy.flinch>0 && enemy.hitZone==zone,"hit reaction not selected");
  check(enemy.hurt<enemy.flinch,"damage tint masks the complete reaction");
  const int first=guardReactionFrame(enemy);
  check(first==(zone==HitZone::Head?1:zone==HitZone::Torso?5:9),"wrong localized impact pose");
  enemy.flinch=enemy.flinchDuration*.6f;
  check(guardReactionFrame(enemy)!=first,"localized reaction is a static pose");
 }
 {
  Game death;death.load(0,false,tuning::workshop.heroStart,true);death.debugInvincible=true;
  auto &enemy=death.enemies.front();enemy.active=true;enemy.hp=1;enemy.dir=-1;
  death.damageEnemy(enemy,1,false,1,HitZone::Torso);
  const auto &rig=tuning::guardReactions;
  check(std::fabs(enemy.deathDuration-rig.collapseDuration-rig.corpseHold-rig.fadeDuration)<.0001f,"review corpse lifetime mismatch");
  check(guardReactionFrame(enemy)==12 && guardCorpseOpacity(enemy)==1,"death skips its impact or fades immediately");
  for(int i=0;i<8;++i){
    float start=i? tuning::guardDeathTimeline[i-1].end:0;
    float mid=(start+tuning::guardDeathTimeline[i].end)*.5f;
    enemy.death=enemy.deathDuration-mid*rig.collapseDuration;
    check(guardReactionFrame(enemy)==int(tuning::guardDeathTimeline[i].frame),"collapse skips an authored frame");
    check(guardCorpseOpacity(enemy)==1,"body fades before settling");
  }
  enemy.death=rig.fadeDuration+rig.corpseHold*.5f;
  check(guardReactionFrame(enemy)==19 && guardCorpseOpacity(enemy)==1,"settled corpse hold missing");
  enemy.death=rig.fadeDuration*.5f;
  check(std::fabs(guardCorpseOpacity(enemy)-.5f)<.001f,"late fade is not continuous");
  check(!guardReactionFlip(enemy),"rightward death is mirrored");
  enemy.hitDir=-1;check(guardReactionFlip(enemy),"leftward death is not mirrored");
  for(int i=0;i<60;++i)death.update({});
  check(enemy.death==0,"corpse animation never expires");
 }
 check(g.workshopReview && g.level().width==tuning::workshop.width,"review room not selected");
 {
  Game patrol;patrol.load(0,false,80,true);patrol.debugInvincible=true;
  auto &e=patrol.enemies.front();e.x=e.prevX=e.origin=350;e.active=true;e.timer=999;
  float x=e.x,phase=e.gait;
  for(int i=0;i<60;++i)patrol.update({});
  check(std::fabs((phase+(x-e.x)/tuning::guardAction.strideLength)-e.gait)<.001f,"guard stride is not distance based");
  phase=e.gait;e.state=1;e.timer=999;
  for(int i=0;i<20;++i)patrol.update({});
  check(e.gait==phase,"stationary guard advances its stride");
  e.timer=DT*.5f;patrol.update({});
  check(e.fireAge==0 && e.state==2,"guard shot clock did not start on emission");
  auto muzzle=guardMuzzle(e);bool shot=false;
  for(const auto &b:patrol.bullets)if(b.alive && b.hostile){
    check(std::fabs(b.px-muzzle.x)<.001f && std::fabs(b.py-muzzle.y)<.001f,"guard projectile detached from atlas barrel");shot=true;
  }
  check(shot && guardFlashVisible(e),"actual guard shot has no visible flash");
  auto pose=interpolatedEnemy(e,.5f);check(pose.fireAge==0,"fresh enemy shot interpolates from stale age");
  e.dead=true;check(!guardFlashVisible(e),"dead guard still emits muzzle light");
  e.dead=false;e.flinch=.1f;check(!guardFlashVisible(e),"flinching guard emits muzzle light");
  e.flinch=0;
 }
 check(g.playerBox().h>50 && g.playerBox().h<65,"adult collision body mismatch");
 check(!g.vehicleAvailable && g.enemies.size()==1 && g.props.empty(),"unreviewed props or encounters leaked in");
 for(int frame=0;frame<720;++frame){
  Input in;in.move=frame<180?.4f:frame<360?-.4f:0;
  if(frame==360)g.player.dir=1;
  in.shoot=frame>=360;in.jump=(frame==80);g.update(in);
  check(std::isfinite(g.player.y) && std::isfinite(g.camera),"non-finite review motion");
  check(g.camera>=0 && g.camera<=g.level().width-W,"review camera bounds");
  check(!g.boss.active,"campaign boss triggered in isolated art review");
 }
 check(g.kills==1,"adult-height weapon did not hit the sentry");
 auto m=muzzlePoint(g.player,{});auto box=g.playerBox();
 check(m.y>box.y && m.y<box.y+box.h,"muzzle outside adult body height");
 g.player.x=tuning::workshop.exitX+1;Input exit;exit.interact=true;g.update(exit);
 check(g.workshopSecured && g.status==Status::Play,"review exit changed campaign progression");
 g.retry();check(g.workshopReview && g.player.presentationScale==tuning::workshop.bodyScale && !g.workshopSecured,"review retry lost rig or reset state");
 for(const auto &ladder:g.level().ladders){
  g.load(0,false,ladder.x,true);g.debugInvincible=true;g.enemies.clear();Input up;up.up=true;
  for(int i=0;i<230;++i)g.update(up);
  check(std::fabs(g.player.y-ladder.top)<.1f,"adult rig cannot exit ladder onto gallery");
 }
 g.load(0);check(!g.workshopReview && g.cinematicHero() && g.player.presentationScale==tuning::workshop.bodyScale && g.level().width>tuning::workshop.width,"campaign lost production rig or retained review geometry");
 std::cout<<"PASS workshop movement, scaled collision/muzzle, live combat, gallery climb, exit and retry isolation\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
