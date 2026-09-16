# Cinematic presentation update — v0.4.0

This update strengthens Iron Coast's own industrial art direction using the supplied references for light, depth and material contrast. It keeps the SDL2/GXM Vita renderer and the existing collision maps. No Metal Slug or REPLACED artwork is bundled.

## One tuning source

Edit **`data/presentation.json`**, then run:

```sh
python3 tools/compile_presentation.py
cmake --build build-vita -j4
```

`src/presentation_config.h` is generated, committed C++ data, not a second editable configuration. Vita needs no JSON parser. Rebuild/reinstall after tuning; this is not a live settings file. `--check` detects a stale generated header. Colors are `RRGGBBAA`, durations are seconds and distances are logical world pixels (480×272 view).

| Section | Main controls |
|---|---|
| `lighting` | Key/fill/rim contribution, exposure, source bloom, shaft intensity/spread/noise/sample spacing, contact occlusion, fog depth/drift, background defocus, motion blur and grade |
| `maps` | Six key/fill/fog/edge/sky colors, fog density/drift, material texture gain |
| `camera` | Horizontal anchor/look-ahead, follow speed, climb anticipation, boss reveal duration/lift, bounded zoom and settling speed |
| `surface`, `surfaceMaterials` | Wall/floor intensity, specular intensity, sample spacing, light reach; concrete, metal and timber diffuse/specular/roughness responses |
| `weapons` | Six flash colors/shapes/durations, tracers, recoil, casings, magazine capacities, reload times, fire synthesis and impact sizes |
| `audio` | Distance falloff/filtering, music level/ducking, envelope/transient/tail behavior, reload latch timing, empty/impact/discovery cues |
| `combat` | Head/leg classification thresholds, three flinch durations, lean/squash, impact travel, hit-stop/cooldown, shake, tint, particles and directional death timing |
| `environment`, `materials` | Prop size/spacing, atlas brightness, fan/terminal lighting, additional material swatches |
| `secrets`, `discovery` | Map positions, trigger radius, artwork sizes, feedback time/color and reward |

Geometry such as window outlines, ladder rungs and the original gameplay balance remains authored in its existing source files. The config controls the new presentation systems; it is not a replacement for the entire campaign format.

## 1. Lighting, environment and camera

**Added:** `src/cinematic_render.cpp`, `data/presentation.json`, `src/presentation_config.h`, `tools/compile_presentation.py`.

**Changed:** `src/render.cpp`, `src/render.h`, `src/game.cpp`, `src/game.h`, `CMakeLists.txt`.

- Lamps/windows drive sampled light shafts with animated density. Samples test the actual roof/column occluders.
- Actor lighting combines ambient fill, local key light and a directional brightness contribution for separation.
- Source bloom, exposure-compressed shaft radiance, a restrained scene grade and authored contact darkening improve depth.
- Concrete and timber receive broad diffuse floor light. Metal adds a narrower specular response. Light reaches wall textures and recessed interiors, including transient muzzle sources; roofs block transmission to the floor below.
- Wet patches retain their clipped light and actor reflections. Reflections stay inside puddles and sample only the selected atlas cell.
- Far scenery receives a small multi-tap defocus and camera-velocity smear. Actors, projectiles and HUD are never sampled from an earlier rendered frame.
- Camera movement anticipates the direction of travel and upward climbing. Boss entrances briefly lift the composition and ease to a maximum 1.025× zoom, then return to the combat view. The whole world uses the same transform; HUD scale is restored. Disabling camera shake also disables cinematic zoom.

**Backend limits:** This is portable SDL compositing. Light-shaft sampling runs on the CPU; there is no installed GPU volumetric shader. Bloom is source-based, tone mapping applies to the computed shaft radiance, and contact occlusion is authored geometry, not full-frame HDR bloom or depth-buffer SSAO. Defocus and motion blur affect selected scenery layers. The side-view camera reframes and changes scale; it does not reveal true 3D oblique angles. Physical Vita performance remains unverified.

**Test:** In Harbor, move below a warm warehouse lamp, then climb onto its roof. Compare the lit floor with the shadow beneath a roof. In Ironline, compare metal highlights with Harbor stone and timber. Fire near a wall to see its transient light. Climb a tall Storm Relay ladder and approach each boss entrance; HUD and collision alignment must remain steady.

## 2. Weapon variety

**Added:** `src/weapon_effects.cpp`, `tests/test_audio.cpp`.

**Changed:** `src/game.cpp`, `src/game.h`, `src/render.cpp`, `src/audio.cpp`, `src/audio.h`, `src/main.cpp`.

| Weapon | Visual and sound character | Magazine / reload |
|---|---|---|
| Pistol | Short pale flash, short tracer, brass case, sharp crack | 12 / 0.72 s |
| Heavy MG | Amber flash, longer tracer, brass stream, lower mechanical report | 30 / 1.15 s |
| Shotgun | Wide flash, pellet traces, larger red shell, bass/noise burst | 6 / 1.25 s |
| Rocket | Broad orange launch flash, heavy recoil, launch gas, low tail | 1 / 1.30 s |
| Flame Shot | Orange plume, close-range spray, gas particles, filtered hiss | 24 / 0.95 s |
| Laser | Narrow cyan flash/trace, charge discharge, energy particles | 8 / 0.85 s |

Reload automatically when an empty magazine is fired, or manually with **Vita L / controller left shoulder / keyboard R**. Reload does not create ammunition. Exhausted special-weapon reserves produce an empty cue and return to the pistol, preserving the original fallback. The pistol has unlimited reserve ammunition. Gas/energy weapons intentionally eject gas/energy particles rather than brass shells.

Fire synthesis separates a transient, body and decaying tail. Reload/empty cues use weapon-specific pitches and mechanical/charge envelopes. Distant gunfire loses level and high-frequency detail. Enemy reports use the same positional path. The prior synthesis bug that ignored `noiseMix` in the triangle/noise mode is fixed. Sound remains synthesized mono; these are not recorded firearm libraries or a full acoustic simulation.

**Test:** Fire each pickup standing, crouching and aiming vertically; drain a magazine, reload manually, then exhaust a special weapon's reserve. Listen to nearby and distant enemies. `kh_audio_tests review.wav` exports the actual mixer: pistol, MG, shotgun, rocket, flame, laser; each group contains fire, reload and empty, 1.5 seconds per segment.

## 3. Enemy impact animation

**Changed:** `src/game.cpp`, `src/game.h`, `src/render.cpp`, `src/weapon_effects.cpp`.

**Added:** `tests/test_presentation.cpp`.

Swept projectile entry points classify head, torso and leg impacts, including upward/downward shots. Head hits lean, torso hits recoil and leg hits compress the pose; AI pauses briefly during the flinch. Blood-toned particles distinguish infantry from machine dust/sparks. Short hit-stop has a separate expiry timer and cooldown, so repeated pellets cannot lock the simulation. Directional death presentation combines existing collapse frames with a direction-dependent arc and rotation. This is authored sprite motion, not ragdoll physics or a newly redrawn set of character sheets.

**Test:** Shoot above/below an infantry enemy from different platform heights, then kill enemies while facing each direction. Machine impacts should use dust/sparks. Existing shield-facing protection still applies. Check that the camera resumes immediately after a hit and that ordinary movement retains visible feet without trailing sprite fragments.

## 4. Map identity and original art

**Added:** `assets/environment-alcoves-v1.png`, `tools/sourceboards/environment-alcoves-prompt.txt`, `src/themed_environment.cpp`.

**Changed:** `src/architecture_render.cpp`, `src/render.cpp`, `src/render.h`.

The original 1536×1024 alcove atlas was generated with the built-in imagegen tool. The complete prompt is stored with the sourceboards. Its six exact 512×512 source panels are rendered as square 56×56 props; no transparent actor mapping is reused. Existing six architecture textures and six panoramic backgrounds remain in use.

- Harbor: blue brick, cable drums, crates, dock gantries and stone quay courses.
- Marsh: mossy pump works, pressure gauges, pipe bends, reeds and timber deck boards.
- Ironline: carriage vents/windows, couplers, overhead supply frames and rail sleepers.
- Foundry: soot brick, furnace fan housings, animated warm light and perforated grating.
- Relay: cold concrete, server banks, dishes and snow-capped platform lips.
- Command: armored terminal bays, green CRT light, structural ribs and cooling slots.

**Test:** Visit all six missions and compare the lower interiors and roof profiles. Climb both sides of each building: new props are scenery, not invisible blockers. The established 60 ladders, platform colliders and finite door encounters are unchanged.

## 5. Hidden discoveries

**Added:** `src/discoveries.cpp`, `src/secret_render.cpp`, `assets/ataturk-signature.png`, `assets/hidden-flag.png`, `tools/sourceboards/ataturk-signature.svg`, `tools/sourceboards/flag.svg`, `tools/build_secret_assets.py`, `licenses/ataturk-signature.txt`.

**Changed:** `src/game.cpp`, `src/game.h`, `src/render.cpp`, `src/render.h`.

A small rooftop keepsake contains Mustafa Kemal Atatürk's signature; another contains the Turkish flag. Discovery awards 250 points, a brief English notification, particles and a soft chime, once per campaign session. Discovery flags survive Continue and mission changes but are not written into the existing progress-save format. All menus, dialogue and installation documentation remain English.

The signature is the attributed vectorization by Ichwan Palongengi, marked public domain on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Signature_of_Mustafa_Kemal_Atat%C3%BCrk.svg). The flag is original SVG geometry. `tools/build_secret_assets.py` reproducibly rasterizes both vectors using CairoSVG; no runtime dependency is added.

<details><summary>Discovery testing locations — spoilers</summary>

Harbor: second warehouse roof, world x=1020, feet y=112. Relay: high communications roof, x=1800, feet y=-48. Both are reachable through existing ladders. Walk within 27 pixels; leave and return to confirm the reward does not repeat. Continue after death and verify the discovery stays recorded for that campaign session.

</details>

## Reproducible verification

```sh
cmake -S . -B build-qa -DKH_VITA=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-qa -j4
ctest --test-dir build-qa --timeout 180 --output-on-failure
python3 -m unittest discover -s tests -p test_atlases.py
python3 tools/test_runtime.py --binary build-qa/kiyi_hurdasi
mkdir -p review
build-qa/kh_render_tests assets review
build-qa/kh_audio_tests review/weapons.wav
```

The suite covers gameplay, campaign stability, climbing routes, magazine/reload semantics, swept hit zones, finite hit-stop, discovery state, camera bounds, material light response, roof occlusion, render-scale restoration, positional audio, mute/pause and generated configuration freshness. The renderer audit samples 724 world positions and 248 atlas cells in both directions. GitHub CI runs host tests with AddressSanitizer and UndefinedBehaviorSanitizer. Vita release packaging is checked separately with `tools/validate_vpk.py`.

Passing these tests does not establish physical Vita frame rate, audio-device behavior or hardware installation. Those remain the next device checks.
