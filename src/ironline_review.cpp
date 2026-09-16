#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
const Level &ironlineLevel() {
  static const Level level=[] {
    const auto &s=tuning::ironlineReview;
    Level l;l.name="IRONLINE / NIGHT EXPRESS";l.subtitle="BOARD THE NIGHT EXPRESS";
    l.width=s.width;l.minY=0;l.vehicleX=0;
    l.platforms={{{0,s.roofY,s.bridgeStart,12},true,1},
                 {{s.bridgeStart,s.bridgeY,s.bridgeEnd-s.bridgeStart,8},true,1},
                 {{s.bridgeEnd,s.roofY,s.width-s.bridgeEnd,12},true,1}};
    l.spawns={{s.firstGuard,s.roofY,0},{s.secondGuard,s.roofY,0}};
    l.checkpoints={s.heroStart};
    return l;
  }();return level;
}
void Game::updateIronlineCamera(float dt) {
  const auto &s=tuning::ironlineReview;
  cameraLead+=(tuning::camera.lookAhead*std::clamp(player.vx/145.f,-1.f,1.f)-cameraLead)*(1-std::exp(-dt*s.leadResponse));
  float target=std::clamp(player.x-s.cameraAnchor+cameraLead,0.f,level().width-W);
  camera+=std::clamp((target-camera)*(1-std::exp(-dt*s.cameraResponse)),-dt*s.cameraMaxSpeed,dt*s.cameraMaxSpeed);
  // Roof, actors and shadows share one frame. Train travel is expressed by
  // independent environmental layers, never by shaking feet off the roof.
  cameraY=0;cameraZoom+=(1-cameraZoom)*(1-std::exp(-dt*s.cameraResponse));
}
}
