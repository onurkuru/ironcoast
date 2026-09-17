#pragma once
#include "game.h"
#include <algorithm>
#include <cmath>

namespace kh {
// Host review input only: the normal movement, combat and camera code runs
// unchanged. After the initial spawn, the actor is never teleported.
class PaintedRouteReview {
public:
  int completedGalleries = 0;
  bool finished() const { return completedGalleries == 3; }

  Input input(const Game &game) {
    Input in;
    if (finished() || game.time < .75f) return in;
    const auto &entry = game.level().ladders.at(completedGalleries * 2);
    const auto &exit = game.level().ladders.at(completedGalleries * 2 + 1);
    const auto &player = game.player;
    if (phase == Approach || phase == Traverse) {
      float target = phase == Approach ? entry.x : exit.x;
      float distance = target - player.x;
      if (std::fabs(distance) < .5f && std::fabs(player.vx) < 1) {
        phase = phase == Approach ? Climb : Descend;
      } else {
        in.move = std::clamp(distance * .12f, -1.f, 1.f);
        in.shoot = distance > 28;
        return in;
      }
    }
    if (phase == Climb) {
      if (player.ladder < 0 && player.grounded && std::fabs(player.y-entry.top) < .1f)
        phase = Traverse;
      else in.up = true;
    } else if (phase == Descend) {
      if (player.ladder < 0 && player.grounded && std::fabs(player.y-exit.bottom) < .1f) {
        ++completedGalleries;
        phase = Approach;
      } else in.down = true;
    }
    return in;
  }

private:
  enum Phase { Approach, Climb, Traverse, Descend };
  Phase phase = Approach;
};
} // namespace kh
