#include "game.h"
#include "painted_route_review.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace kh;
static void check(bool value, const char *message) {
  if (!value) throw std::runtime_error(message);
}

int main(int argc,char **argv) {
  try {
    Game game;
    const int stage=argc>1?std::stoi(argv[1])-1:0;
    check(stage==0 || stage==1, "Expected Harbor or Marsh review stage");
    game.load(stage, false, campaign()[stage].ladders.front().x);
    game.debugInvincible = true;
    PaintedRouteReview review;
    unsigned ladders = 0;
    float lastCamera = game.camera, lastCameraY = game.cameraY;
    int frames = 0;
    for (; frames < 3600 && !review.finished(); ++frames) {
      game.update(review.input(game));
      if (game.player.ladder >= 0) ladders |= 1u << game.player.ladder;
      check(game.status == Status::Play, "Painted route review left play state");
      check(std::isfinite(game.camera) && std::isfinite(game.cameraY), "Non-finite route camera");
      check(std::fabs(game.camera-lastCamera) < 8 && std::fabs(game.cameraY-lastCameraY) < 8,
            "Camera snapped during the continuous route");
      lastCamera = game.camera;
      lastCameraY = game.cameraY;
    }
    check(review.finished(), "Continuous painted route did not finish within 60 seconds");
    check(ladders == 63, "Route did not use all six painted ladders");
    check(game.rescued == 3, "Gallery route missed a worker");
    check(game.player.grounded && std::fabs(game.player.y-232) < .1f,
          "Gallery route did not return to the lower walkway");
    check(!game.boss.active, "Gallery traversal started the boss encounter too early");
    for (float age : game.entranceAges) check(age >= 0, "Painted reinforcement door was not triggered");
    std::cout << "PASS continuous " << game.level().name << " route: 3 galleries, 6 ladders, 3 rescues, 2 doors in "
              << frames << " frames\n";
  } catch (const std::exception &e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
