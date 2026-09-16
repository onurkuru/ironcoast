# Guard impact and collapse review

This phase updates the adult guard used by the workshop and Ironline art slices.
It keeps the recovered environment and locomotion/attack atlas. The six-map
campaign retains its existing enemy rigs and combat durations.

## Behavior

Head, torso and leg damage now select timed sequences instead of a single held
frame. The source contains twelve hit/recovery cells; playback starts on the
impact pose, skips the generated neutral lead-in, and reuses a kneeling pose in
the leg recovery. This is not twelve unique biomechanical actions. The short
damage tint ends before the recovery sequence so the armor remains visible.

Eight collapse cells progress from standing impact through knees/hip contact to
a prone body. The body stays opaque through collapse and a short settled hold,
then fades. A monotonic displacement replaces the old sine offset that pulled
the corpse back toward its origin. The opposite hit direction mirrors the same
strip; there is no separately authored forward-fall or physics ragdoll.

The imported art uses one scale for standing, kneeling and prone poses. Per-cell
root and contact anchors are measured in the source; collapsed sprites are not
enlarged to fill a standing box. The renderer does not squash the new leg poses
or rotate their full cells. Floor registration, local lighting and wet-ground
reflections use the same drawing baseline.

## Files and assets

- `assets/guard-reactions-v3.png`: new 640 x 800 atlas with twenty 160px cells.
- `tools/sourceboards/guard-reactions-v3-source.png`: preserved generated source.
- `tools/sourceboards/guard-reactions-v3.json`: source registration metadata.
- `tools/sourceboards/guard-reactions-v3-prompt.md`: exact built-in imagegen prompt,
  reference and selected source provenance.
- `tools/import_guard_reactions.py`: fixed-scale key/despill and packing; checks
  bounds before compositing to prevent silent clipping.
- `data/presentation.json`, `tools/compile_presentation.py`,
  `src/presentation_config.h`: configuration, timeline validation and compiled values.
- `src/game.h`, `src/game.cpp`: review-specific reaction and corpse lifetime.
- `src/animation.h`: shared pose timeline, direction and corpse fade helpers.
- `src/render.h`, `src/render.cpp`, `src/cinematic_render.cpp`: texture lifetime,
  anchored reaction drawing, reflections, shadows and fade handling.
- `src/main.cpp`: isolated `--guard-reaction-review` diagnostic sequence.
- `tests/test_guard_reactions.py`, `tests/test_workshop.cpp`,
  `tests/test_render.cpp`, `CMakeLists.txt`: geometry, timing and renderer regression checks.

## Tuning

Edit `data/presentation.json`. Recompile the generated header and rebuild after
changes. Reimport the atlas after editing cell size, source scale or baseline.

| Section/key | Default | Meaning |
| --- | --- | --- |
| guardReactions.enabled | 1 | New guard reactions in both review slices |
| cellSize / baseline | 160 / 156 | Atlas dimensions and registered contact line |
| sourceScale / renderSize | 0.54 / 95 | Fixed import scale and world-space quad size |
| headDuration | 0.34 s | Head impact and recovery |
| torsoDuration | 0.30 s | Torso impact and recovery |
| legDuration | 0.46 s | Knee buckle and recovery |
| flashDuration | 0.065 s | Brief damage color tint |
| collapseDuration | 0.72 s | Eight-cell collapse |
| corpseHold | 0.45 s | Opaque settled body |
| fadeDuration | 0.35 s | Final disappearance |
| hitTravel / deathTravel | 1.5 / 8 | Temporary hit shift / one-way collapse shift |
| guardHitTimeline | 3 x 4 entries | Frame indices and normalized end times |
| guardDeathTimeline | 8 entries | Collapse frame indices and normalized end times |

Existing `combat` values still control hit-stop, shake and particles. No new
sounds are added in this phase; existing spatial impact/fire audio is reused.
Setting `guardReactions.enabled` to zero restores the recovered guard strips and
their original reaction/death durations on a newly started review encounter.

## Reproduce

From the recovery worktree:

```sh
python3 tools/import_guard_reactions.py
python3 tools/compile_presentation.py
cmake --build ../../work/build-cinematic-review -j4
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
mkdir -p ../../work/guard-reaction-v3-frames
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ../../work/build-cinematic-review/kiyi_hurdasi --assets assets \
  --guard-reaction-review --frames 540 --fast \
  --record ../../work/guard-reaction-v3-frames --record-every 1
```

The nine-second diagnostic directly applies nonlethal head/torso/leg damage at
0.8, 1.8 and 2.8 seconds to isolate those sequences. At 3.8 seconds it lowers the
sentry's remaining health, then real player bullets supply the lethal hit. At
5.8 seconds it spawns a second sentry and moves the hero to the opposite side;
real leftward shots test the mirrored collapse. This is a controlled animation
test, not an uninterrupted ordinary combat recording. The exported 60fps MP4 is
silent and recorded offline, not a Vita performance measurement.

For ordinary play use `../Target-Recovery-Review/Play Workshop.command` or
`Play Ironline.command`. Check that a hit starts immediately, boots/knees/body
stay on the floor, prone bodies retain scale, and fading does not leave ghost
pixels on the next opponent. Existing guard walking and attack frames and hero
death/melee animations are outside this pass.

## Validation result

All eleven CTest suites passed on the development Mac, including the campaign
stability run, both review routes, atlas imports and rendering. The renderer
suite passed again after the final alpha clamp and faded-corpse replay check.
The source importer also passes two geometry/reproducibility tests. Visual
inspection covered head recoil, knee contact, collapse and the mirrored prone
pose in executable captures. Both lethal test encounters completed (200 score).
These checks do not establish artistic approval or measured Vita performance.

Review video: `../Target-Recovery-Review/guard-reactions-v3-60fps.mp4`.
Changes remain local to the recovery worktree. The original main checkout is
unchanged; no commit, push, VPK release or publication was made in this phase.
