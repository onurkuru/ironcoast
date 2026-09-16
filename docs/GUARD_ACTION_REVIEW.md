# Guard movement and firing synchronization

This phase corrects the retained guard atlas in the workshop and Ironline
review slices. No new character art was generated. The built-in image generator
returned `429 usage_limit_reached`; the requested replacement walk/aim/fire
sheet remains pending in `tools/sourceboards/guard-motion-v4-request.md`.
No API fallback, credit reset, image repaint or existing PNG replacement occurred.

## Changes

The walking cycle now advances once per configured world distance, including
doorway entry. Stopping does not advance that cycle. The campaign keeps its
previous gait units and enemy behavior. The existing eight walk cells remain
visually similar; these timing fixes do not supply missing anatomical poses.

Sixteen measured belt-center anchors align walking, ready and firing cells to
the same actor root. In the old atlas, a long rifle or baked muzzle flash pulled
the bounding-box center away from the hips. The renderer now compensates that
offset for either facing, and uses the same drawing center for reflections.
The feet retain the original registered floor line.

A per-enemy shot clock starts only when a projectile is emitted. Attack frames
12..15 use that clock instead of the generic AI state timer. The retained first
two firing frames contain their painted flash/casings; they are shown for only
70ms, followed by recovery. Melee or another non-projectile attack does not
incorrectly display those firing frames.

Each firing frame has a measured barrel endpoint. Projectile spawn, spatial
fire sound position and the short dynamic light use that root. Enemy light
passes through the existing wall/floor lighting system. Interpolated rendering
shares the shot age and position with the sprite, and fresh shots are not
blended backward into an old shot. Dead or flinching guards emit no muzzle light.

## Files

- `data/presentation.json`: action timing, dimensions, pelvis and barrel anchors.
- `tools/compile_presentation.py`, `src/presentation_config.h`: validated compiled configuration.
- `src/game.h`, `src/game.cpp`: enemy shot clock, distance gait and projectile/audio origin.
- `src/animation.h`: shared enemy interpolation, pose registration and muzzle helpers.
- `src/render.cpp`: registered old atlas cells, firing timeline, light and reflection alignment.
- `src/main.cpp`: `--guard-action-review` reproducible combat sequence.
- `tests/test_workshop.cpp`: distance gait, stopped phase, projectile origin and shot discontinuity.
- `tests/test_render.cpp`: enemy flash light alignment, expiration and frame replay.
- `tests/test_guard_action_anchors.py`, `CMakeLists.txt`: checks against the actual retained raster.
- `tools/sourceboards/guard-motion-v4-request.md`: pending artwork prompt and generation failure record.

The reused PNG is `assets/workshop-guard-v2.png`; this phase creates no new PNG.
The previous `guard-reactions-v3.png` continues to handle hurt and death.

## Configuration

All new tunables are in `data/presentation.json`:

| Key | Default | Meaning |
| --- | --- | --- |
| guardAction.cellSize / baseline | 160 / 156 | Atlas cell and ground registration |
| guardAction.renderSize | 95 | World-space drawing size |
| guardAction.strideLength | 48 | Distance per eight-cell walk cycle |
| guardAction.attackDuration | 0.30 s | Full firing/recovery sequence |
| guardAction.flashDuration | 0.07 s | Flash and dynamic light lifetime |
| guardAction.lightRadius | 58 | World-space local light radius |
| guardAction.lightStrength | 0.72 | Initial light strength |
| guardAction.lightColor | FFD09AFF | Warm muzzle color |
| guardPoseRoots[0..15].x | per frame | Measured pelvis center in atlas pixels |
| guardFireAnchors[0..3].x/y | per frame | Barrel endpoint in atlas pixels |
| guardFireAnchors[0..3].end | .035 / .07 / .16 / .30 s | Firing frame boundaries |

After editing config, run `python3 tools/compile_presentation.py` and rebuild.
The existing enemy profiles still control patrol speed, range and windup.

## Test and review

```sh
cmake --build ../../work/build-cinematic-review -j4
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
mkdir -p ../../work/guard-action-v3-frames
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ../../work/build-cinematic-review/kiyi_hurdasi --assets assets \
  --guard-action-review --frames 540 --fast \
  --record ../../work/guard-action-v3-frames --record-every 1
```

The isolated scenario sets a three-health sentry to approach the stationary
hero, aim and fire through its normal AI. At 5.4 seconds the automated player
returns fire. Invincibility is enabled for reproducibility. Unlike the previous
localized hit diagnostic, this sequence does not directly call damageEnemy or
teleport characters during the recording.

`../Target-Recovery-Review/guard-action-v3-60fps.mp4` is a nine-second offline
executable capture at 960 x 544 / 60fps, with no audio track. It is not a Vita
frame-rate benchmark. Interactive testing uses `Play Workshop.command` or
`Play Ironline.command` in the same review output directory.

The eleven existing CTest suites passed, including full campaign stability and
rendering. The newly added raster-anchor suite and updated workshop test passed
as well (twelve suites total). The pixel checks validate pivots and endpoints
against the retained atlas; they do not establish artistic quality. New
locomotion artwork remains outstanding because of the generation limit.

Changes are local to the recovery worktree; the original main checkout and
published releases are unchanged.
