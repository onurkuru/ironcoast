# Cinematic target recovery

This development worktree resumes the approved workshop target. The main
checkout remains at 378ec47. That commit is not a verified release called 6.0.
The recovery stash remains intact. The six rejected campaign screenshots are
not approved art and are not evidence that the campaign meets the targets.

## Visual contract

The references are the three images in `../Phase-1-Visual-Targets`:
workshop, carriage-roof action, and foundry boss. They are concept art, not
executable captures. Retain adult proportions, grounded feet, charcoal/teal
clothing, amber armored sentries, localized warm/cold lighting, wet-floor
response, restrained foreground framing and physically supported routes.

Do not multiply sprite draw size without updating collision, muzzle, aim,
climb and reflection anchors together. Do not hide required ladders by proximity.
Do not declare visual success from passing unit tests. Capture actual motion.

## Current review slice

The isolated workshop recovers the approved adult rigs and scene painting.
Rain is bounded to authored window panes, terminal scans and their local light
use the same clock, and subtle puddle ripples run below the walking surface.
Impact particle/audio origins now follow the scaled head/torso/leg region.
Standing sentry hit poses use their authored bend, without a second full-cell
rotation that would lift the boots above the floor.
Run, aim, crouch, jump, grenade, melee, flinch and death use the recovered atlases.
There is no new hand-drawn reload strip or new boss rig in this revision.
The painting still contains baked lighting and foreground framing; it is not
a full 3D environment or a replacement for independently animated depth layers.

All new environment tuning is in `data/presentation.json` under
`workshopMotion`, `workshopPanes` and `workshopScreens`. Pane/screen coordinates
are fractions of the approved painting. Run `python3 tools/compile_presentation.py`
after editing, then rebuild.

## Build and review

```
cmake -S . -B ../../work/build-cinematic-review -DKH_VITA=OFF
cmake --build ../../work/build-cinematic-review -j4
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
../../work/build-cinematic-review/kiyi_hurdasi --assets assets --workshop-review
```

Move: arrows. Jump: Z/Space. Fire: X/J. Grenade: C/K. Reload: R.
Up/down aim or climb. E interacts. Escape pauses. `--review-weapon 0..5`
selects pistol, heavy MG, shotgun, rocket, flame or laser for a separate review.
`--review-demo --frames 780 --fast --record DIR --record-every 2` records
deterministic 30 fps frames, including actual movement and combat. Exported
motion previews have no audio unless captured separately; do not use them to
approve weapon sound quality. Desktop builds are QA tools, not public releases.

## Per-map production sequence

| Map | Route and encounter identity | Required bespoke animation/art | Review status |
| --- | --- | --- | --- |
| Harbor | Workshop tension, service stair/gallery, dock exit | CRTs, rain, door mechanics, crane rig | Workshop in review; remainder pending |
| Marsh | Pump-house walkways, sluice crossings, sheltered lower route | Moving reeds, water gates, pump pistons, hazmat foes | Pending |
| Ironline | Carriage roofs, couplers, interior/exterior transitions | Separate train and distant mountain layers, wheel motion, wind, shield reactions | Two-carriage roof review implemented; interior, wheels and shield rig pending |
| Foundry | Furnace service floor, cooling gantries, broad titan arena | Articulated titan, press cycle, fans, molten flow, steam | Approved concept; implementation pending |
| Relay | Tower switchbacks, maintenance rooms, antenna approach | Rain exposure, antenna motion, electrical warning cycle | Pending |
| Command | Flooded hangar, control-room ascent, final deck | Bulkheads, sea reflections, command boss transformations | Pending |

Each map needs one actual wide shot and a movement/combat clip reviewed before
expanding its route. Match playable surfaces to painted architecture before
placing enemies. First validate feet, muzzle anchors, climb transitions and
occlusion; then validate encounters, checkpoints and exits. A successful
workshop review does not imply approval of all six maps. Vita performance and
memory still require a separate device validation phase.

## Files used in this pass

### Perspective and motion follow-up

`workshopView` in the central presentation config controls camera lead response,
camera speed limit, near-floor depth, depth start and sampling strip height.
Near-floor perspective starts below feet/contact reflections. Walls and the
walk plane retain the original projection. This is a restrained 2.5D floor
effect. The follow-up below adds a separate distant window panorama; the camera
still uses 2D composition, without freely rotating 3D geometry.
Character size and collision dimensions remain unchanged.

Workshop camera lead follows smoothed velocity, initializes at its resting
anchor and caps travel speed. Camera framing no longer depends on the shake
toggle. Actor position, stride, idle clock and climb cycle interpolate together;
enemy gait uses its previous/current fixed-step values. New shots bypass age
blending so flashes remain immediate. Workshop footsteps use two contacts per
full stride instead of four.

`--review-motion --frames 360 --fast --record DIR --record-every 1` records a
six-second right-run, jump, reversal and left-run clip. Encode at 60 fps; every
frame comes from the executable. This offline export is not a real-time Vita
performance measurement. Test with the playable launcher at the display's
refresh rate to assess input and camera feel.

| Files | Purpose |
| --- | --- |
| `assets/workshop-plate-v1.png` | Recovered approved environment painting |
| `assets/workshop-{hero,guard,aim,climb}-v2.png` | Recovered adult character atlases; no raster edits in this pass |
| `src/cinematic_render.cpp`, `src/render.h` | Window rain, terminal scans and floor ripples |
| `src/render.cpp` | Terminal lighting, standing hit grounding, remove rejected campaign-only size multipliers |
| `src/game.cpp` | Scale-aware hit effect/audio origin |
| `src/main.cpp` | Configured start position and weapon review option |
| `src/architecture_render.cpp` | Remove proximity-only ladder visibility |
| `data/presentation.json`, `src/presentation_config.h` | Central authored regions and motion tuning; generated C++ header |
| `tests/test_workshop.cpp`, `tests/test_render.cpp` | Hit-region regression, animation determinism and frame-history checks |

Validation: host build passed; all eight CTest suites and both workshop atlas
tests passed. After the final standing-hit correction, workshop and rendering
suites passed again. These checks cover behavior and state leakage, not visual
equivalence to the concept art or Vita device performance.
