#include "game.h"
#include "presentation_config.h"
#include <algorithm>

namespace kh {
bool Game::cinematicHero() const {
  return cinematicReview() || tuning::campaignPresentation.enabled > 0;
}
bool Game::cinematicGuard(const Enemy &e) const {
  // The recovered sentry is a rifle guard. Grenadiers, shields, drones and
  // turrets retain their own rigs until their dedicated replacements exist.
  return cinematicReview() || (tuning::campaignPresentation.enabled > 0 && e.kind == 0);
}
float Game::enemyBodyScale(const Enemy &e) const {
  return cinematicGuard(e) || (cinematicHero() && e.kind < 3) ? tuning::workshop.guardScale : 1.f;
}
void Game::beginChapter(int index, bool keepScore) {
  if (index == 0)
    load(index, keepScore, tuning::workshop.heroStart, true);
  else if (index == 2)
    load(index, keepScore, tuning::ironlineReview.heroStart, false, true);
  else load(index, keepScore);
  chapterSequence = true;
}
bool Game::advanceSection() {
  if (!chapterSequence || !cinematicReview() || !workshopSecured ||
      status != Status::Play || sectionExitAge < tuning::campaignPresentation.exitHold)
    return false;
  // A section is part of its chapter, not a new mission or a free heal.
  const Player equipment = player;
  const bool assist = debugInvincible;
  const int sectionRescued = rescued;
  const int sectionKills = kills;
  load(levelIndex, true);
  chapterSequence = true;
  debugInvincible = assist;
  rescued = sectionRescued;
  kills = sectionKills;
  player.health = std::min(equipment.health, player.maxHealth);
  player.weapon = equipment.weapon;
  player.ammo = equipment.ammo;
  player.magazine = equipment.magazine;
  player.magazineWeapon = equipment.magazineWeapon;
  player.firedWeapon = equipment.firedWeapon;
  player.reloadTime = equipment.reloadTime;
  player.grenades = equipment.grenades;
  syncPresentation();
  return true;
}
} // namespace kh
