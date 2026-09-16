# Hero locomotion review

This pass replaces the grounded horizontal run and run-and-fire strips in the
workshop and Ironline review slices. It keeps the recovered environment art and
the main campaign rig. It is a phase preview, not a complete character animation
replacement or a claim of parity with the visual references.

The new atlas has eight poses per cycle, registered around the pelvis and a
common floor line. Unlike bounding-box centering, extending the gun does not
move the actor root. Flight poses retain their foot clearance instead of being
snapped down to the floor. Footstep timing stays linked to distance traveled.
Moving fire uses per-pose barrel anchors for projectiles, flash and local light.
Horizontal airborne fire reuses the two tucked-leg firing poses, with the same
barrel anchors, instead of shooting from a lowered gun in the old jump strip.
The old whole-sprite recoil translation is removed in both cinematic slices.
Movement accelerates and brakes briefly, with facing following actual motion
during an input reversal. Releasing movement preserves the selected facing.

## Changed files

- `assets/hero-locomotion-v3.png`: new 8 x 2 runtime atlas, padded 160px cells.
- `tools/sourceboards/hero-locomotion-v3-source.png`: preserved generated source.
- `tools/sourceboards/hero-locomotion-v3.json`: measured source root/floor anchors.
- `tools/sourceboards/hero-locomotion-v3-prompt.md`: exact imagegen prompt and provenance.
- `tools/import_hero_locomotion.py`: deterministic keying and anchor-based packing.
- `data/presentation.json`: locomotion and barrel-anchor tuning.
- `tools/compile_presentation.py`, `src/presentation_config.h`: validation and compiled tuning.
- `src/animation.h`: shared pose selection and moving-fire muzzle location.
- `src/game.cpp`: acceleration, braking and configured stride length.
- `src/render.h`, `src/render.cpp`, `src/cinematic_render.cpp`: atlas lifetime,
  fixed-root drawing, reflection baseline and recoil correction.
- `src/main.cpp`: isolated reproducible `--hero-motion-review` sequence.
- `tests/test_hero_locomotion.py`, `tests/test_workshop.cpp`, `CMakeLists.txt`:
  import, clipping, chroma, motion, muzzle and existing combat regression checks.

## Tuning

All new runtime and import values live in `data/presentation.json`:

| Key | Default | Meaning |
| --- | --- | --- |
| heroLocomotion.enabled | 1 | Use new horizontal grounded strips in review slices |
| heroLocomotion.cellSize | 160 | Atlas cell size in pixels |
| heroLocomotion.baseline | 156 | Common floor line in each cell |
| heroLocomotion.sourceScale | 0.405 | Source-to-atlas scale |
| heroLocomotion.renderSize | 100 | Runtime quad size in world units |
| heroLocomotion.strideLength | 88 | World distance per eight-pose cycle |
| heroLocomotion.acceleration | 1600 | Speed increase/reversal per second |
| heroLocomotion.braking | 2000 | Speed reduction per second after release |
| heroRunMuzzles[0..7] | per pose | Barrel x and height relative to source root/floor |

Reimport after changing cell size, baseline or source scale. Recompile the
presentation header and rebuild after any tuning changes. Setting `enabled`
to zero restores old run strips and muzzle positioning; acceleration/braking
and the recoil correction remain in effect.

## Reproduce and inspect

From the recovery worktree:

```sh
python3 tools/import_hero_locomotion.py
python3 tools/compile_presentation.py
cmake --build ../../work/build-cinematic-review -j4
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
mkdir -p ../../work/hero-motion-v3-frames
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ../../work/build-cinematic-review/kiyi_hurdasi --assets assets \
  --hero-motion-review --frames 360 --fast \
  --record ../../work/hero-motion-v3-frames --record-every 1
```

The six-second sequence shows idle, acceleration, run, moving fire, a jump,
reversal and braking. It removes opponents to isolate the hero silhouette.
Use `../Target-Recovery-Review/Play Ironline.command` for interactive combat,
or the workshop launcher for indoor lighting and reflections.

Inspect feet at ground contact, pose boundaries, direction changes, and the
barrel flash. The PNG frames are actual executable output. The 60fps exported
clip is silent and recorded offline; it does not measure real-time Vita speed.

Idle, upward aim, non-firing jump, melee, death, climb and guard animations still use the
recovered v2 artwork. Their transitions need further visual review. The new
source's opposing leg silhouettes remain similar; extra pose count alone does
not establish animation quality. No new sounds are part of this pass.

## Validation result

All ten CTest suites passed after the final air-fire correction, including full
campaign stability, both playable review routes and deterministic rendering.
After measuring the final barrel offsets, the five affected suites were rerun
and passed again (both review routes, import, config and rendering).
The atlas import also passes three Python tests for geometry/chroma, exact
reproduction and per-pose barrel anchor alignment against opaque atlas pixels.
Those automated checks establish specific technical properties,
not artistic approval. The first runtime capture exposed a lowered jump gun
with a detached muzzle; the airborne firing pose and shared anchor were then
corrected. Review output is `../Target-Recovery-Review/hero-motion-v3-60fps.mp4`.
The original main checkout was left unchanged; this work is local to the
`cinematic-target-recovery` worktree and has not been published.
