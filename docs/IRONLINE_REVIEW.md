# Ironline: carriage roof review

This second playable art slice follows the approved `02-ironline-final-target`
concept. It is separate from the workshop and the six-map campaign. Run the
local `Play Ironline.command` launcher in `../Target-Recovery-Review`.

Two carriages have independently registered roof and coupler surfaces. The
coupler sits 16.7 world units below the roof, requiring a jump back up to the
next carriage. Defeat both sentries, then interact near the right edge to secure
the carriages. Retry preserves the adult rig and starts on a roof, not inside
the carriage. There is no campaign boss or campaign progression in this slice.

The train is the player's reference frame. Mountains move at 5.5 percent of
train travel, trees at 48 percent, fog drifts independently, and wind streaks
cross the exposed roofs. Camera look-ahead is smoothed and travel is bounded.
Actors and roof shadows share a fixed surface transform. Mirrored alternate
background panels preserve edge continuity during long travel; the landscape
therefore repeats over time and is not a unique infinite map.

Three new imagegen assets were produced using the approved train concept:

- `assets/ironline-carriages-review-v1.png`: two detailed carriages, warm windows,
  steel roofs and coupler; runtime chroma matte with edge despill.
- `assets/ironline-mountains-review-v1.png`: continuous mountain/viaduct vista.
- `assets/ironline-forest-review-v1.png`: independently moving conifers.

All textures are imported at a width of 2046 pixels. The source PNGs stay
unchanged after import. Green in the two keyed files is a source matte and must
not appear in an executable screenshot. Generation prompts and source filenames
are preserved in `tools/sourceboards/ironline-review-prompts.md`.

This phase reuses the recovered adult hero and guard animations. It does not
add a shield rig, new enemy animation strips, moving wheels, a carriage interior,
train-specific sound, or the full Ironline mission. Those remain later review
phases. This implementation is not a claim of exact visual equivalence to the
approved concept or a measured Vita performance result.

## Files and tuning

- `src/ironline_review.cpp`: authored route and train camera.
- `src/game.h`, `src/game.cpp`: separate review identity, shared adult rig,
  spawn height, retry and exit isolation.
- `src/cinematic_render.cpp`, `src/render.h`, `src/render.cpp`: train layers,
  texture ownership, key cleanup and reuse of grounded actor rendering.
- `src/themed_environment.cpp`, `src/hud_render.cpp`: painted roof presentation
  and contextual English exit feedback.
- `src/main.cpp`: `--ironline-review` and reproducible `--ironline-demo`.
- `tests/test_ironline.cpp`: full traversal, combat, completion, retry and isolation.
- `tests/test_render.cpp`: chroma contamination, landscape movement, static train
  body and deterministic frame replay.
- `CMakeLists.txt`: review code and test target.

Scene dimensions, roof registration, coupler, spawns, exit, travel speed, layer
depths and camera tuning are in `data/presentation.json` under `ironlineReview`.
The hero/guard scale remains shared with the approved workshop rig. Run
`python3 tools/compile_presentation.py` after config changes, then rebuild.

## Review controls

Arrows move, Z/Space jumps, X/J fires, R reloads, C/K throws a grenade, and E
interacts at the exit. Keep an eye on the feet while crossing the coupler and
on the mountain/tree speed difference while standing still. The current
preview uses the pistol; `--review-weapon 0..5 --ironline-review` selects another
existing weapon.

```sh
cmake --build ../../work/build-cinematic-review -j4
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
mkdir -p ../../work/ironline-review-frames
../../work/build-cinematic-review/kiyi_hurdasi --assets assets --ironline-demo \
  --frames 660 --fast --record ../../work/ironline-review-frames --record-every 1
```

Encode at 60 fps. The exported video is silent and is an offline executable
capture, not a measurement of real-time Vita rendering or a sound-quality demo.

Validation on the development Mac: all nine CTest suites passed. The first
composite revealed green key fringes, which were corrected with soft coverage
and edge despill. The corrected wide screenshot was visually inspected for
roof contact, carriage framing and visible key contamination. The renderer
test also checks that the train remains still while distant layers move and
that replaying the same scene time reproduces the same pixels.
