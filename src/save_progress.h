#pragma once
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <istream>
#include <string>

namespace kh {
struct SaveProgress {
  int unlocked = 0, best = 0;
  bool muted = false, shake = true, assist = false, fullscreen = false;
  // -1 means unfinished; 0 means cleared without a rescue record.
  std::array<int, 6> missionRescues{{-1, -1, -1, -1, -1, -1}};
  int completedMissions() const {
    return int(std::count_if(missionRescues.begin(), missionRescues.end(),
                            [](int n) { return n >= 0; }));
  }
  int rescueRecord() const {
    int total = 0;
    for (int n : missionRescues) total += std::max(0, n);
    return total;
  }
  void recordMission(int chapter, int rescued, int score) {
    if (assist || chapter < 0 || chapter >= 6) return;
    unlocked = std::max(unlocked, std::min(5, chapter + 1));
    best = std::max(best, std::clamp(score, 0, 99999999));
    missionRescues[chapter] = std::max(missionRescues[chapter], std::clamp(rescued, 0, 3));
  }
};
inline SaveProgress parseSave(std::istream &f) {
  SaveProgress s;
  std::string magic;
  int u, b, m, sh, a, fs;
  if (!(f >> magic >> u >> b >> m >> sh >> a >> fs) ||
      (magic != "KH_SAVE_1" && magic != "KH_SAVE_2")) return s;
  s.unlocked = std::clamp(u, 0, 5);
  s.best = std::clamp(b, 0, 99999999);
  s.muted = m == 1; s.shake = sh == 1;
  s.assist = a == 1; s.fullscreen = fs == 1;
  if (magic == "KH_SAVE_1") {
    // Earlier unlocks prove clears, but legacy files never stored rescue counts.
    for (int i = 0; i < s.unlocked; ++i) s.missionRescues[i] = 0;
  } else {
    auto records = s.missionRescues;
    for (int &n : records)
      if (!(f >> n) || n < -1 || n > 3) return s;
    s.missionRescues = records;
  }
  return s;
}
inline SaveProgress readSave(const std::string &path) {
  std::ifstream f(path);
  return parseSave(f);
}
inline bool writeSave(const std::string &path, const SaveProgress &s) {
  const std::string tmp = path + ".tmp";
  std::ofstream f(tmp);
  if (!f) return false;
  f << "KH_SAVE_2 " << s.unlocked << ' ' << s.best << ' ' << s.muted << ' '
    << s.shake << ' ' << s.assist << ' ' << s.fullscreen;
  for (int n : s.missionRescues) f << ' ' << n;
  f << '\n'; f.close();
  if (!f) return false;
  return std::rename(tmp.c_str(), path.c_str()) == 0;
}
} // namespace kh
