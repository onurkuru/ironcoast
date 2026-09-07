# Iron Coast — cinematic art pass, 0.2.1

Six original environment panoramas establish a coherent industrial coastal world. Large readable structures, quiet areas behind characters and motivated practical lights carry the composition. The art direction takes REPLACED as a mood and lighting reference while retaining Iron Coast's own locations and characters.

| Mission | Palette and focal point | Runtime asset |
|---|---|---|
| Rusted Harbor | Petrol night, amber shipyard windows, monumental gantry | `assets/harbor-night.png` |
| Toxic Marsh | Green-grey haze, jaundiced service lamps, flooded pipe works | `assets/marsh-night.png` |
| Ironline | Blue-grey mountains, layered ravine fog, distant rail viaduct | `assets/ironline-night.png` |
| Ember Foundry | Charcoal steel, limited furnace orange, receding columns | `assets/foundry-night.png` |
| Storm Relay | Ink-blue sea, cyan equipment lamps, radar silhouettes | `assets/relay-night.png` |
| Final Wave | Violet-grey storm, crimson control lights, offshore citadel | `assets/command-night.png` |

## Implementation

- Panoramas preserve their 3:1 aspect ratio and traverse continuously over each mission. Landmarks are not mirrored. One active panorama is held in GPU memory.
- Low-contrast haze, architectural middle silhouettes, gameplay and a floor-level near field move at separate depths.
- Lamps have fixed world positions and visible fixtures. Their light cones, ground reflections and actor illumination use the same positions.
- A reusable 64×64 radial alpha texture provides soft light and contact shadows. These are economical 2D lighting approximations; physical Vita frame-rate/memory measurements remain outstanding.
- Ground reflection strips are clipped to actual platform bounds. Shadows are projected onto the floor below an actor rather than following airborne feet.
- Actor texture modulation adds cool ambient light and nearby warm/cool light. Hit feedback remains readable.
- Ground materials have subdued trim. Screen-wide CRT scanlines and disconnected decorative shafts were removed.

## Sprite correction

The previous source import incorrectly assumed equal rows and columns. Deleting disconnected edge components could remove fragments but could not restore the character parts that uniform crops had already cut off.

`hero-frames.json` maps all 61 complete hero poses into 64 runtime cells. `enemies-frames.json` maps 48 complete infantry, drone, turret and worker poses. Each atlas uses a consistent source scale and shared baseline; padding protects every cell. Grounded hero movement no longer offsets the entire sprite vertically for breathing or running. Enemy attack and hurt frames are separate, and collapsed poses retain their authored proportions.

The six boss and Scrap Walker boards now use the same guarded cell import as
the player and enemy boards. Each 192px source cell is resized into a 182px
inner area with a five-pixel transparent gutter. That protects claws, wheels,
weapon flashes and destruction debris from neighbouring-cell sampling while
preserving the authored pose. Runtime baselines are measured per cell, so
grounded bosses and the vehicle share the collision line without a whole-quad
vertical bob.

## Verification and preview

- Gameplay/campaign tests and software SDL render regressions pass.
- Render regressions cover all six scenes, player movement, a return to an identical earlier frame and a return after switching scenes. They detect residual sprite/texture/light state.
- Atlas checks cover transparent borders for all 224 actor cells (hero, enemy,
  workers, vehicle and six bosses), full run-pose bounds, and reproducibility
  from the checked-in source maps. Software render checks exercise directional
  aim, airborne/down aim, vehicle damage, worker rescue, enemy states and all
  four boss states for all six bosses.
- The six-scene overview is made from the game renderer. The motion preview is a silent six-second capture of the game's 60 Hz simulation, encoded at 30 fps. It is a visual review, not a performance benchmark.
- Desktop and Vita packages are built from this source. The packaged Mac title screen and an in-game marsh scene were visually checked in the native app. Motion/frame clearing was checked with the software renderer. Physical Vita testing remains outstanding.

## Art provenance

The six environment plates were generated with the built-in `image_gen` tool for this project. Exact prompts are stored in `tools/sourceboards/environment-prompts.json`. All selected images are included in `assets/`; runtime loading does not depend on a generated-images directory.

Reference study: [Sad Cat Studios' visual discussion](https://store.epicgames.com/es-MX/news/the-inspirations-and-innovations-of-replaced) and [the director's animation and cinematography notes](https://news.xbox.com/en-us/2026/04/14/replaced-combat/).
