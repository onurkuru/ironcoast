#include "game.h"
#include "harbor_review.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace kh;
static void check(bool value, const char *message) {
  if (!value) throw std::runtime_error(message);
}

int main() {
  try {
    Game game;
    game.load(0, false, campaign()[0].ladders.front().x);
    game.debugInvincible = true;
    HarborRouteReview review;
    unsigned ladders = 0;
    float lastCamera = game.camera, lastCameraY = game.cameraY;
    int frames = 0;
    for (; frames < 3600 && !review.finished(); ++frames) {
      game.update(review.input(game));
      if (game.player.ladder >= 0) ladders |= 1u << game.player.ladder;
      check(game.status == Status::Play, "Harbor review left play state");
      check(std::isfinite(game.camera) && std::isfinite(game.cameraY), "Non-finite Harbor camera");
      check(std::fabs(game.camera-lastCamera) < 8 && std::fabs(game.cameraY-lastCameraY) < 8,
            "Harbor camera snapped during the continuous route");
      lastCamera = game.camera;
      lastCameraY = game.cameraY;
    }
    check(review.finished(), "Continuous Harbor route did not finish within 60 seconds");
    check(ladders == 63, "Harbor route did not use all six painted ladders");
    check(game.rescued == 3, "Harbor gallery route missed a worker");
    check(game.player.grounded && std::fabs(game.player.y-232) < .1f,
          "Harbor route did not return to the quay");
    for (float age : game.entranceAges) check(age >= 0, "Painted reinforcement door was not triggered");
    std::cout << "PASS continuous Harbor route: 3 galleries, 6 ladders, 3 rescues, 2 doors in "
              << frames << " frames\n";
  } catch (const std::exception &e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
