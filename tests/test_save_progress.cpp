#include "save_progress.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

static void check(bool ok, const char *message) {
  if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
int main(int argc, char **argv) {
  check(argc == 2, "temporary save path required");
  kh::SaveProgress progress;
  check(progress.completedMissions() == 0, "new players have no clears");
  for (int i = 0; i < 6; ++i) progress.recordMission(i, 3, (i + 1) * 1000);
  check(progress.completedMissions() == 6 && progress.rescueRecord() == 18,
        "all mission and rescue records survive a campaign");
  progress.recordMission(0, 0, 50);
  check(progress.rescueRecord() == 18 && progress.best == 6000, "replay cannot erase a better result");
  progress.muted = true;
  check(kh::writeSave(argv[1], progress), "save write failed");
  auto loaded = kh::readSave(argv[1]);
  check(loaded.missionRescues == progress.missionRescues && loaded.best == 6000 && loaded.muted,
        "disk round trip lost progress or preferences");
  std::remove(argv[1]);
  kh::SaveProgress training; training.assist = true;
  training.recordMission(5, 3, 9000);
  check(training.unlocked == 0 && training.best == 0 && training.completedMissions() == 0,
        "training may not award campaign records");
  std::istringstream legacy("KH_SAVE_1 4 7000 1 0 0 1");
  auto migrated = kh::parseSave(legacy);
  check(migrated.unlocked == 4 && migrated.best == 7000 && migrated.completedMissions() == 4 &&
        migrated.rescueRecord() == 0 && !migrated.shake && migrated.fullscreen,
        "legacy migration must preserve unlocks without inventing rescue counts");
  for (const auto *bad : {"KH_SAVE_2 2 100 0 1 0 0 3", "KH_SAVE_2 2 100 0 1 0 0 3 3 8 0 0 0"}) {
    std::istringstream input(bad); auto damaged = kh::parseSave(input);
    check(damaged.unlocked == 2 && damaged.best == 100 && damaged.completedMissions() == 0,
          "partial or invalid records must not invent completed missions");
  }
  std::istringstream overflow("KH_SAVE_2 999999999999999999999 0 0 1 0 0");
  check(kh::parseSave(overflow).unlocked == 0, "overflow must fail safely");
  check(!kh::writeSave(std::string(argv[1]) + "/missing/save", progress), "write failure must be reported");
  std::cout << "PASS completion records, rescue totals, legacy saves and failed writes\n";
}
