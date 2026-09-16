#include "game.h"
#include "presentation_config.h"
#include <algorithm>
#include <cmath>
namespace kh {
void Game::updateDiscoveries(float dt) {
  discoveryTime=std::max(0.f,discoveryTime-dt);
  if(status!=Status::Play || cinematicReview())return;
  for(size_t i=0;i<tuning::secrets.size();++i) {
    const auto &secret=tuning::secrets[i];
    if(int(secret.map)!=levelIndex || (discoveries&(1u<<i)))continue;
    if(std::hypot(player.x-secret.x,player.y-secret.y)>secret.radius)continue;
    discoveries|=1u<<i;discoveryTime=tuning::discovery.duration;score+=int(tuning::discovery.score);
    audioEvents.push_back({Sound::Discovery,secret.x,secret.y,-1});
    burst(secret.x,secret.y-15,2,6);
  }
}
} // namespace kh
