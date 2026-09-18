#pragma once
#include "game.h"
#include <algorithm>
#include <cmath>

namespace kh {
// A repeatable host review played entirely through ordinary controls. Unlike
// the traversal demo, this pilot finishes nearby fights before advancing.
// It never edits health, ammunition, actor positions or encounter state.
class ArcadeReview {
public:
  Input input(const Game &game) {
    Input in;
    if(game.time<lastTime) { lastGrenade=-10;lastJump=-10; }
    lastTime=game.time;
    if (game.status != Status::Play || game.time < .3f) return in;
    const auto &player = game.player;
    in.move = 1;
    in.shoot = true;
    const Enemy *target = nullptr;
    float best = 10000;
    int nearby = 0;
    for (const auto &enemy : game.enemies) {
      if (enemy.dead || !enemy.active || enemy.entryAge < .45f) continue;
      const float dx = enemy.x-player.x, dy = enemy.y-player.y;
      if (std::fabs(dy)>75 || std::fabs(dx)>300 || enemy.x<game.camera-90) continue;
      ++nearby;
      const float cost = std::fabs(dx)+std::fabs(dy)*1.5f;
      if (cost<best) { target=&enemy;best=cost; }
    }
    if (target) {
      const float dx=target->x-player.x;
      const int direction=dx<0?-1:1;
      // Walk into effective range, then keep the muzzle on the target without
      // the old demo running through it. A small tap changes facing normally.
      in.move=std::fabs(dx)>165?float(direction):player.dir!=direction?.12f*direction:0;
      in.up=target->y<player.y-55 && std::fabs(dx)<65;
      in.down=target->kind==4 && target->y>player.y-10 && player.grounded;
      in.grenade=player.grenades>0 && std::fabs(dx)<190 &&
                 (target->kind==2 || target->kind==4 || nearby>=3) && game.time-lastGrenade>1.4f;
    } else if (!game.boss.active) {
      for (size_t i=0;i<game.entranceAges.size();++i) {
        const float age=game.entranceAges[i];
        const auto &door=game.level().entrances[i];
        if (age>0 && age<4.5f && std::fabs(player.x-door.x)<195) {
          // The final squad member must have time to leave the visible door.
          in.move=player.x>door.x-85?0:1;
          break;
        }
      }
    }
    if (game.boss.active && !game.boss.dead) {
      const float dx=game.boss.x-player.x;
      const int direction=dx<0?-1:1;
      float safeX=game.level().width-405;
      // Final Wave marks the player's old column before its alternating drop.
      // Step off that column during the visible windup, then hold the new lane.
      if(game.level().bossKind==5 && game.boss.pattern%2==1 &&
         (game.boss.state==BossState::Windup || game.boss.state==BossState::Attack ||
          game.boss.state==BossState::Recover))safeX+=75;
      in.move=std::fabs(player.x-safeX)>8?std::clamp((safeX-player.x)*.05f,-1.f,1.f):
              player.dir!=direction?.12f*direction:0;
      in.up=false;
      in.grenade=player.grenades>0 && std::fabs(dx)<240 && game.time-lastGrenade>1.4f;
      // Relay's elevated core can be reached by the ordinary jump shot.
      if (game.levelIndex==4 && player.grounded) in.jump=true;
    }
    const float look=game.floorAt(player.x+in.move*35,player.y-2);
    if (player.grounded && look>player.y+18) in.jump=true;
    if (player.grounded && game.time-lastJump>.65f) {
      const Rect body=game.playerBox();
      for (const auto &bullet:game.bullets) {
        if (!bullet.alive || !bullet.hostile) continue;
        const float dx=bullet.x-player.x;
        if (bullet.kind==3 && bullet.life<.45f && std::fabs(dx)<65) {
          in.move=dx>0?-1.f:1.f;in.jump=true;
        } else if (bullet.kind==6 && std::fabs(dx)<35 && bullet.y>body.y-75) {
          in.move=dx>0?-1.f:1.f;
        } else if (dx*bullet.vx<0 && std::fabs(dx)<90 &&
                   bullet.y>body.y && bullet.y<body.y+body.h) in.jump=true;
      }
      for (const auto &hazard:game.level().hazards)
        if (game.hazardOn(hazard) && player.x+35>hazard.x && player.x<hazard.x+hazard.w)
          in.jump=true;
    }
    if (game.cinematicReview()) in.interact=true;
    else if (target && target->kind==4 && player.vehicleHP>0) in.interact=true;
    else if ((!target || target->kind!=4) && game.canBoardVehicle()) in.interact=true;
    if (in.jump) lastJump=game.time;
    if (in.grenade) lastGrenade=game.time;
    return in;
  }
private:
  float lastGrenade=-10, lastJump=-10,lastTime=0;
};
} // namespace kh
