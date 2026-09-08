# Iron Coast: Scrap Tide

An original 2D run-and-gun game for PS Vita and desktop. It includes running, jumping, crouching, directional fire, close-range attacks, grenades, special weapons, rescues, vehicles and two-phase boss encounters. Player combat uses three health points, hit invulnerability and checkpoint respawns so a single mistake does not immediately end a run.

## Teslim edilenler

Metal Slug 1 ve 3'ün bölüm düzeni araştırması, mevcut haritaların denetimi ve altı görev için yeni mimari/karşılaşma planı: [LEVEL_DESIGN_RESEARCH.md](LEVEL_DESIGN_RESEARCH.md). Bu tasarım henüz oynanabilir paketlere uygulanmadı.

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

Version 0.2.3 removes the floating `CORE OPEN` label and targeting ring from all six bosses. Recovery now uses a small teal glow on the authored core anchor, fading with the existing recovery animation. Damage rules are unchanged. Desktop gameplay/render tests and a native macOS boss preview passed; the Vita package was cross-compiled, not tested on a physical device.

Version 0.2.2 uses explicit full-pose rectangles for every active actor in `tools/sourceboards/*-frames.json`. The original boards are irregularly spaced; dividing them into equal cells cut off limbs and introduced neighboring poses. All actor atlases now use a stable authored scale, transparent gutters and a measured foot anchor. See [SPRITE_AUDIT.md](SPRITE_AUDIT.md) for the full audit of 236 cells and the lighting corrections.

The player uses `hero-v2.png` with eight authored rows: run, idle/fire, a mixed
jump transition strip, crouch-fire, grenade, melee, death and run-fire. The
renderer maps the jump arc to cells 17–21, crouch idle to 16/24/25, crouch-fire
to 26–30, and the full melee swing to 40–47 so mixed rows never select the wrong
pose. `enemies-v2.png` contains eight frames for
guards, grenadiers, shield troops, drones, turrets and rescued workers. `vehicle-v2.png`
adds movement, firing, hatch/boarding, damage and collapse frames for the Scrap Walker. Each of the
six missions has its own `boss0-v2.png` through `boss5-v2.png` 4×4 atlas so Vita never
needs to upload an oversized texture. Vehicle and boss cells have transparent
gutters around full source poses; worker rescue uses dedicated gesture/run
cells. Boss core coordinates are generated alongside the image into `.anchors`
files. `tools/prepare_sprite_atlases.py` documents the
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

The game runs at a 480×272 logical resolution with nearest-neighbor pixel sampling and a 60 Hz simulation. Desktop uses SDL2; Vita uses the VitaSDK SDL2 port. Normal player poses share a 48×48 ground box, atlas cells retain a consistent authored scale, visible sprite bottoms are measured at load time, and grounded frames receive a small baseline correction so feet stay on the same collision line. Trimmed actor atlases skip that second correction because their visible bounds already fill the destination box. The player is interpolated between fixed ticks for smoother motion. Each of the six missions now has a continuous cinematic panorama, spanning the level without mirrored landmarks. Haze travels at 24%, industrial middle silhouettes at 48%, the gameplay plane at 100%, and a floor-level near-field silhouette at 112%. This creates a restrained 2.5D effect without hanging geometry crossing the actors or adding a large texture. The sprite preparation tool packs complete hero and infantry poses from explicit source bounds; seven-pose rows hold their final valid pose. World-anchored practical lights illuminate actors and cast soft contact shadows. Reflected light stays clipped to platform surfaces. Background/foreground detail is subdued and CRT scanlines are removed. Only the active environment texture is resident, and the light pass reuses a 64×64 alpha texture. Music is generated in real time with per-stage motifs and a higher-intensity boss arrangement; 32 kHz sound effects use per-weapon noise balance and click-free envelopes. The visual designs are original and do not use existing commercial characters or vehicles.


## Cinematic art direction (0.2.2)

See [ART_DIRECTION.md](ART_DIRECTION.md) for the six palettes, asset provenance, renderer design and validation. The original environment PNGs are in `assets/*-night.png`; exact built-in image generation prompts are in `tools/sourceboards/environment-prompts.json`.

Run visual regressions without opening a desktop window:

```sh
ctest --test-dir ../../work/build-desktop --output-on-failure
python3 tests/test_atlases.py
# Optional: capture all six scenes into an existing output directory.
../../work/build-desktop/kh_render_tests assets ../../work/art-review
```
