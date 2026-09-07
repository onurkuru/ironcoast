# Iron Coast: Scrap Tide

An original 2D run-and-gun game for PS Vita and desktop. It includes running, jumping, crouching, directional fire, close-range attacks, grenades, special weapons, rescues, vehicles and two-phase boss encounters. Player combat uses three health points, hit invulnerability and checkpoint respawns so a single mistake does not immediately end a run.

## Teslim edilenler

- `Kiyi Hurdasi.app` — Apple Silicon macOS application bundle.
- `kiyi-hurdasi.vpk` — VitaSDK ARM Vita homebrew package.
- `data/campaign.json` — editable story, map, enemy, item and hazard data for all six missions.
- `src/` — C++ game, renderer and audio code.
- `assets/` — generated sprite and environment atlases.
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

`--stage 1..6`, `--demo`, `--showcase 1..3`, `--frames N`, `--capture path.png` and `--screen map|brief|controls` are development preview options.

## Teknik notlar

The game runs at a 480×272 logical resolution with nearest-neighbor pixel sampling and a 60 Hz simulation. Desktop uses SDL2; Vita uses the VitaSDK SDL2 port. Normal player poses share a 48×48 ground box, atlas cells retain a consistent authored scale, and the player is interpolated between fixed ticks for smoother motion. Backgrounds add a parallax silhouette layer, practical lights, haze and a light CRT pass. Music is generated in real time with per-stage motifs and a higher-intensity boss arrangement; 32 kHz sound effects use per-weapon noise balance and click-free envelopes. The visual designs are original and do not use existing commercial characters or vehicles.
