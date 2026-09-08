# Sprite and lighting audit — 0.2.2

The active renderer uses 236 atlas cells. Each was drawn through SDL in both
facing directions (472 checks), with its visible bounds checked against the
destination rectangle and floor line. This covers mapping and placement; it
does not measure performance on physical Vita hardware.

| Atlas | Cells | Sequences |
|---|---:|---|
| Hero | 64 | Run, idle/fire, jump, crouch, grenade, melee, death, run/fire |
| Enemy and worker | 48 | Guard, grenadier, shield, drone, turret, captive/rescue/run |
| Directional aim | 12 | Standing up, running up, airborne down |
| Scrap Walker | 16 | Movement, fire, hatch/boarding, damage/destruction |
| Six bosses | 96 | Movement, preparation/contact, recovery, destruction |

## Findings and corrections

- Adding gutters around uniform crops did not recover clipped anatomy. Boss
  and vehicle sources also cross the old grid boundaries. All ten active
  atlases now use measured full-pose rectangles in `tools/sourceboards/*-frames.json`.
  Each atlas keeps one source scale and a stable bottom anchor. Disconnected
  border fragments are removed after extracting the complete pose, including
  the thin neighbouring fragment in the hero run/fire strip.
- The aim board's columns drift horizontally. Its 12 complete poses are now
  centered in new cells. A larger padded aim canvas accommodates the raised gun
  while matching the hero's body scale. Long downward effects are rendered from
  the shared muzzle point so their length cannot shift the body between frames.
- Actor destination rectangles now preserve the square source-cell proportions.
  Foot measurement includes semi-transparent sole pixels. Grounded bob and
  whole-character rotation no longer move feet into the platform.
- Rescue playback now reaches kneel, stand, wave and run frames. Workers begin
  moving only during the run and stop at their platform edge while fading.
- The final jump pose is reachable. Fatal player hits now settle on solid
  platforms instead of falling through them. Vehicle damage/destruction rows
  play, and airborne wrecks fall without changing facing when the pilot turns.
- Boss preparation holds the raised pose. Attack contact occurs at the gameplay
  impact time (0.18 seconds for crane/hammer, 0.06 for other bosses), instead of
  restarting the preparation strip. Grounded locomotion follows distance. The
  flying boss's wreck falls to the arena floor during its destruction sequence.
- Each boss pose has a measured core coordinate. The generated `.anchors`
  files use the same crop/scale transform as the atlas. Light, recovery marker
  and charge glow share that coordinate. White-hot centers are protected from
  background keying, which previously punched transparent holes in them.
- Gameplay projectiles, player flashes and player light use one directional
  muzzle helper. Grenade and melee recoil no longer produces an unrelated gun
  light. Pause freezes light animation with simulation time.
- Boss projectile and muzzle-light offsets are generated from the painted
  attack pose into `src/boss_muzzles.h`. The locomotive and final boss no longer
  emit projectiles below the barrel. Vehicle hatch poses play on entry/exit;
  nonfatal damage uses the damaged hull pose and hit tint.
- Unreachable legacy melee/machine fallback drawing was removed. Out-of-range
  frame indices now fail explicitly instead of silently drawing a different cell.

## Reproduction

From the repository:

```sh
python3 tools/prepare_sprite_atlases.py
python3 tests/test_atlases.py
cmake --build ../../work/build-desktop -j4
ctest --test-dir ../../work/build-desktop --output-on-failure
# Existing directory: exports runtime atlas sheets and scene/actor samples.
../../work/build-desktop/kh_render_tests assets ../../work/sprite-audit/rendered
```

The Python checks cover all atlas gutters, source-map reproducibility, preserved
full boss bounds, stable aim roots and core anchors landing on opaque painted
pixels. C++ checks exercise all cells, scene return/frame clearing, enemy states,
rescue states, vehicle damage/destruction, all six boss encounters, directional
muzzle lights at five interpolation fractions, pause, fatal landing and the
existing campaign/long-simulation checks.

The comparison video is a silent 30 fps export of a 60 Hz simulation using the
actual SDL renderer. Native desktop packages and Vita cross-compilation are
checked separately; no physical Vita frame-rate or audio measurement is claimed.
