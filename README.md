# Iron Coast: Scrap Tide

An original 2D run-and-gun game for PS Vita and desktop. It includes running, jumping, crouching, directional fire, close-range attacks, grenades, special weapons, rescues, vehicles and two-phase boss encounters. Player combat uses three health points, hit invulnerability and checkpoint respawns so a single mistake does not immediately end a run.

## Teslim edilenler

- `Kiyi Hurdasi.app` — Apple Silicon macOS application bundle.
- `kiyi-hurdasi.vpk` — VitaSDK ARM Vita homebrew package.
- `data/campaign.json` — editable story, map, enemy, item and hazard data for all six missions.
- `src/` — C++ game, renderer and audio code.
- `assets/` — original sprite, boss, vehicle and environment atlases.
- `tests/` — level reachability and gameplay rule tests.

## macOS'ta çalıştırma

Open `Kiyi Hurdasi.app` from Finder. For a quick mission test from Terminal:

```sh
./Kiyi\ Hurdasi.app/Contents/MacOS/kiyi_hurdasi --assets "Kiyi Hurdasi.app/Contents/Resources/assets"
```

Building from source requires SDL2 and CMake:

```sh
cmake -S . -B ../../work/build-desktop -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build ../../work/build-desktop -j4
ctest --test-dir ../../work/build-desktop --output-on-failure
```

## Vita paketini yeniden üretme

The VitaSDK `bin` directory must be on PATH and `VITASDK` must be set:

```sh
cmake -S . -B ../../work/build-vita -DKH_VITA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build ../../work/build-vita -j4
```

The output is `../../work/build-vita/kiyi-hurdasi.vpk`. Install it with a homebrew installer such as VitaShell. The device must be configured to run homebrew.

## Kontroller

Keyboard: arrows or WASD move; Z/Space jump; X/J fire; C/K grenade; E vehicle; Esc/P pause.

Vita/gamepad: D-pad or left stick move; Cross jump/confirm; Square fire; Circle grenade/back; Triangle vehicle; Start pause.

Up + fire aims upward. Down + fire in the air aims downward. A fire command aimed at a nearby infantry target becomes a close-range attack.

## Hızlı görsel denemeler

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./build/kiyi_hurdasi --assets ../assets --stage 4 --showcase 2 --frames 150 --fast --capture boss.png
```

`--stage 1..6`, `--demo`, `--showcase 1..5`, `--frames N`, `--capture path.png` and `--screen map|brief|controls` are development preview options.

## Original sprite atlas pass

The player uses `hero-v2.png` with eight authored rows: run, idle/fire, jump/crouch,
grenade, melee, death and run-fire. `enemies-v2.png` contains eight frames for
guards, grenadiers, shield troops, drones, turrets and rescued workers. `vehicle-v2.png`
adds movement, firing, damage and collapse frames for the Scrap Walker. Each of the
six missions has its own `boss0-v2.png` through `boss5-v2.png` 4×4 atlas so Vita never
needs to upload an oversized texture. `tools/prepare_sprite_atlases.py` documents the
normalization and backdrop-keying step; the `tools/sourceboards/` files are only source
boards and are not loaded by the game.

## Boss motion and animation pass

All six bosses now move through the arena with acceleration and braking. Encounters follow entry, repositioning, preparation, attack and recovery states, with an overload transition below half health. The crane and walkers use authored locomotion frames, the hammer has a timed backswing/contact/recovery, tracked machines roll and recoil, and the flying relay follows a continuous aerial path. The new boss atlases provide complete movement, attack, recovery and destruction poses; the state machine selects them without whole-sprite scale pops.

Player, camera, enemies, bullets and particles now share interpolation between simulation ticks. Player running frames follow distance travelled, and firing frames restart on the actual shot. Mechanical footsteps and impacts play at animation events.

To enter an interactive boss practice encounter without playing the whole mission:

```sh
../../work/build-desktop/kiyi_hurdasi --assets assets --boss-preview --stage 4
```

Use `--stage 1..6` to select a boss and `--preview-phase 2` for its second phase. Practice enables no-damage training mode and does not unlock campaign progress.

The reproducible six-boss video uses the actual game renderer and requires FFmpeg:

```sh
python3 tools/capture_bosses.py --binary ../../work/build-desktop/kiyi_hurdasi \
  --work-dir ../../work --output ../iron-coast-boss-motion.mp4
```

The comparison video is a silent 12-second, 30 fps recording of the 60 Hz simulation; it is not a hardware performance benchmark. Physical Vita performance remains unverified.

## Teknik notlar

The game runs at a 480×272 logical resolution with nearest-neighbor pixel sampling and a 60 Hz simulation. Desktop uses SDL2; Vita uses the VitaSDK SDL2 port. Normal player poses share a 48×48 ground box, atlas cells retain a consistent authored scale, and the player is interpolated between fixed ticks for smoother motion. Backgrounds use four depth rates: painted scenery at 16%, industrial middle silhouettes at 34%, the gameplay plane at 100%, and a sparse authored-prop foreground rail/cable pass at 112–128%. This creates a restrained 2.5D effect without changing collision geometry or adding a large texture. Practical lights, haze and a light CRT pass complete the scene. Music is generated in real time with per-stage motifs and a higher-intensity boss arrangement; 32 kHz sound effects use per-weapon noise balance and click-free envelopes. The visual designs are original and do not use existing commercial characters or vehicles.
