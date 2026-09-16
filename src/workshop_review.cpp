#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Game::updateWorkshopCamera(const Input &in, float dt) {
  const auto &c=tuning::workshopCamera;
  const auto &view=tuning::workshopView;
  const auto &framing=tuning::camera;
  auto response=[dt](float speed){return 1-std::exp(-dt*speed);};
  const bool quiet=player.grounded && std::fabs(player.vx)<c.idleSpeed &&
                   !in.shoot && player.hitFlash<=0;
  cameraIdle=quiet?cameraIdle+dt:0;
  // Hold the last opponent briefly through death/flinch so the shot does not
  // jump back and forth between subjects during automatic fire.
  const Enemy *subject=nullptr;
  float nearest=c.combatDistance;
  for(const auto &e:enemies) {
    const float distance=std::fabs(e.x-player.x);
    if(e.active && !e.dead && std::fabs(e.y-player.y)<c.combatHeight && distance<nearest) {
      nearest=distance;subject=&e;
    }
  }
  if(subject) {cameraSubjectX=subject->x;cameraCombatHold=c.combatHold;}
  else cameraCombatHold=std::max(0.f,cameraCombatHold-dt);
  float targetZoom=c.trackingZoom, targetY=0, anchor=framing.anchorX;
  float wanted=framing.lookAhead*std::clamp(player.vx/145.f,-1.f,1.f);
  cameraLead+=(wanted-cameraLead)*response(view.leadResponse);
  float target=player.x-anchor+cameraLead;
  workshopShot=WorkshopShot::Tracking;
  if(player.ladder>=0 || (player.grounded && player.y<c.galleryHeight)) {
    workshopShot=WorkshopShot::Gallery;
    targetZoom=c.galleryZoom;
    const auto &room=tuning::workshop;
    const float artworkTop=232-room.width/room.sourceAspect*room.sourceFloor-room.cameraLift;
    targetY=std::clamp(player.y-c.galleryFootY-(in.up?framing.climbLook:0),std::max(level().minY,artworkTop),0.f);
  } else if(cameraCombatHold>0) {
    workshopShot=WorkshopShot::Combat;
    targetZoom=c.combatZoom;targetY=c.combatLift;
    target=player.x+(cameraSubjectX-player.x)*c.combatSubjectWeight-W*.5f;
  } else if(workshopSecured) {
    workshopShot=WorkshopShot::Exit;
    targetZoom=c.exitZoom;targetY=c.detailLift;
  } else if(time<c.establishDuration) {
    workshopShot=WorkshopShot::Establishing;targetZoom=c.establishZoom;
  } else if(cameraIdle>c.idleDelay && player.x>c.detailStart && player.x<c.detailEnd) {
    workshopShot=WorkshopShot::Detail;
    targetZoom=c.detailZoom;targetY=c.detailLift;
    target=player.x-c.detailAnchor;
  }
  // Favor a safe player region when framing an opponent; room bounds take
  // precedence at the exits. Zoom stays >=1 to avoid exposing horizontal edges.
  target=std::clamp(target,player.x-c.safeRight,player.x-c.safeLeft);
  target=std::clamp(target,0.f,level().width-W);
  camera+=std::clamp((target-camera)*response(c.panResponse),-dt*view.maxCameraSpeed,dt*view.maxCameraSpeed);
  cameraY+=std::clamp((targetY-cameraY)*response(c.verticalResponse),-dt*c.maxVerticalSpeed,dt*c.maxVerticalSpeed);
  cameraZoom+=std::clamp((targetZoom-cameraZoom)*response(c.zoomResponse),-dt*c.maxZoomSpeed,dt*c.maxZoomSpeed);
}
const Level &workshopLevel() {
  static const Level room=[] {
    Level l; l.name="DOCKSIDE WORKSHOP";l.subtitle="BREAK THE DOCKSIDE LOCKDOWN";
    l.width=tuning::workshop.width;l.minY=-95;l.vehicleX=0;
    l.platforms={{{0,232,l.width,40},false,1},{{12,52,93,8},true,1}};
    l.buildings={{{12,52,93,180},0},{{175,135,160,97},8},{{490,60,140,172},8}};
    l.ladders={{40,52,232},{84,52,232}};
    l.spawns={{tuning::workshop.guardStart,232,0}};
    l.puddles={{150,232,135,10},{335,232,170,10}};
    l.checkpoints={tuning::workshop.heroStart};
    return l;
  }();
  return room;
}
}
