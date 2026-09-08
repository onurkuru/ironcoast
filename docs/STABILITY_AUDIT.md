# Campaign stability audit — 9 September 2026

Target: PS Vita v0.2.5 (`00.25`). This audit describes bounded tests, not a guarantee that every possible play sequence or physical device is fault-free.

## Reproduced and fixed

**Rescue progress was lost after death.** A rescued worker remained marked as used, but `hitPlayer()` cleared the mission's rescue counter. The player could not collect that same worker again, so the HUD and completion reward no longer matched the world.

A regression test reproduced the failure before the fix. Life-loss respawns now preserve the counter. Continuing after Game Over also restores both the rescued count and the corresponding workers' used flags, preventing lost or duplicate rescue credit. All six missions are covered.

## Test coverage

| Check | Scope |
|---|---|
| Full campaign runs | Six missions on foot, then six with vehicle boarding enabled. Actual movement, pickups, enemies and boss projectiles; no direct boss-damage call or stage skip. |
| Main-route progression | Each run must reach its boss, defeat it and enter Clear within a fixed limit. An unexpected backwards teleport/fall fails the route test. |
| Checkpoints | All 25 authored checkpoints, six opening spawns and six boss checkpoints: 37 starting locations. Three life losses and Game Over/Continue at each. |
| Rescue regression | Rescue, life loss, respawn, Game Over, Continue and revisiting the rescued worker in all six missions. |
| Random input | 48 deterministic five-minute scenarios, spread over ground segments and boss arenas. Shooting, movement, jumping, aiming, grenades, vehicle interaction and retry. |
| State invariants | Finite positions, velocities and projectile/particle values; camera bounds; health bounds; bounded item/enemy collections. |
| Entire-map rendering | 724 sample positions at 32 px intervals over the six maps, alternating facing, weapons, grounded/airborne poses and aim. |
| Existing renderer checks | 236 atlas cells in both directions, six scenes, boss animation states, anchored lights, pause stability and stale-frame detection. |
| Application startup | 13 full-executable cases: six mission openings, three menu screens and four malformed/truncated/out-of-range save files. Actual SDL render loop and dummy audio device. |

The extended simulation performs **912,007 update frames**, about 253 minutes of simulated game time across independent scenarios. This is not 253 minutes of uninterrupted physical-Vita play.

The full campaign bot uses invulnerability to isolate route and boss progression from dodging skill. It does not establish that a human can finish every mission without damage. Death/Continue scenarios and random-input scenarios run separately with damage enabled. Rendering samples inspect the whole horizontal extent; they are not a playthrough of every optional platform or every input combination.

## Local results

- Release-like host build: all three CTest groups passed.
- UBSan plus float-cast-overflow checks: all three groups passed with no report.
- Full application/SDL/audio/malformed-save smoke: all 13 cases passed.
- VitaSDK cross-compilation passed; VPK validation confirmed the SELF, Title ID, version, indexed launcher images, XML and all 37 resource files.

The local Apple AddressSanitizer runtime deadlocked in its own initialization **before `main()`**. A sampled stack identified recursive ASan initialization through the macOS dyld/malloc path; a minimal standalone program with the installed Homebrew runtime also timed out. This is not counted as a passing memory test or an observed game-loop freeze. The upstream [LLVM report](https://github.com/llvm/llvm-project/issues/200447) and [Darwin sanitizer fix](https://github.com/llvm/llvm-project/pull/182943) document the corresponding runtime problem.

**Linux result: passed.** [Run 34287370100](https://github.com/onurkuru/ironcoast/actions/runs/34287370100) tested commit `a470441` with ASan, UBSan, float-cast-overflow and leak detection enabled. All three CTest groups, atlas validation and all 13 application cases passed without sanitizer reports. The repository repeats these checks via [Campaign stability](https://github.com/onurkuru/ironcoast/actions/workflows/stability.yml). The local ASan limitation remains recorded rather than hidden.

## Reproduce

Host QA requires SDL2/pkg-config, CMake and a C++17 compiler. This produces development test executables, not a desktop game distribution.

```sh
cmake -S . -B build-qa -DKH_VITA=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-qa -j4
ctest --test-dir build-qa --timeout 180 --output-on-failure
python3 tools/test_runtime.py --binary build-qa/kiyi_hurdasi
```

For sanitizer flags and the Ubuntu environment, use `.github/workflows/stability.yml`. All long-running tests have time limits so a future stall becomes a failed check.

## What is still unverified

- Physical Vita installation, suspend/resume, device audio/input, memory pressure and sustained performance.
- Human difficulty balance and every optional-route/input combination.
- Full UI navigation through every menu transition; startup checks cover map, briefing and controls rendering.
- Durability of saves under device power loss or storage removal. Malformed-save startup tests do not simulate those failures.

The Metal Slug map research has produced a six-mission redesign plan, but the proposed playable buildings, depot interiors/roofs and bridge layout are **not implemented in this release**. Launcher artwork is promotional art. See [level research](../LEVEL_DESIGN_RESEARCH.md).
