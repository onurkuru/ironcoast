# Architecture and lighting update — v0.3.0

All six missions now use authored buildings, connected roof routes and a continuous lower road. The renderer consumes the same roof coordinates as collision and ladder navigation. This is an implemented first architectural pass, not a claim of visual parity with INSIDE or Metal Slug.

## Playable changes

- Rusted Harbor: warehouse interiors, roof routes, container storage and a supported canal bridge.
- Toxic Marsh: pump buildings, stilt structures and an elevated pipe bridge over the service route.
- Ironline: freight-car wall panels, roof routes, wheels and a continuous service catwalk.
- Ember Foundry: brick/iron halls with maintenance mezzanines and furnace accents.
- Storm Relay: tall relay buildings with negative world elevations and a vertical camera.
- Final Wave: layered command hangars and maintenance roofs.
- Each boss has an architectural arena backdrop; the lower escape deck remains clear.
- 60 ladders connect upper and lower surfaces. Hold Up/Down without firing to attach; Jump releases the ladder. Vehicles stay on the lower road.
- 30 doors each introduce two reinforcements after a visible 0.6-second warning. The second actor is staggered by 0.85 seconds; actors move out before attacking. No new enemies are allocated during a wave.
- Three workers per mission are supported on reachable roofs. Checkpoints restore the lower road; rescue persistence remains intact.

## Art and lighting

Six original material panels supply warehouse brick, pump-house metal, freight steel, foundry brick, relay concrete and hangar panels. The scene adds foundations, side faces, recessed windows, open lower corridors, columns, roof thickness, railings and door shutters.

Lights are clipped to roof slabs and building interiors. Roofs and side columns block actor illumination; window projections use two lit panes separated by dark mullions. These are inexpensive authored 2D approximations, not global illumination or a 3D lighting engine.

Wet surfaces are explicit map regions. Light ripples and shallow, strip-sampled character/enemy reflections stay inside their bounds. Dry concrete has no reflection strip; metal uses restrained highlights. Footstep sounds follow simulation stride, with separate metal and water sounds and small water splashes.

The climbing atlas has 12 measured, complete poses and transparent gutters. Alternating phases mirror the back-view climbing poses; four transition poses cover attachment and release. The normal actor atlases are preserved.

## Verification

- Four CTest suites: gameplay/bosses, full-campaign stability, architecture movement/encounters and rendering.
- 60 ladder round trips, roof traversal to the opposite ladder, jump release, aim without accidental attachment and death/retry.
- 30 finite door encounters, warning delays and consistent retry without duplicate allocation.
- Six missions completed on foot and with vehicle boarding enabled using real movement and weapon damage. The route bot uses invulnerability to isolate progression from dodging skill.
- 724 whole-world samples, moving climbs in every mission, 248 actor cells drawn in both directions, light/pause/state restoration checks.
- Eight atlas tests, including reproducible climbing imports and transparent gutters.
- 13 full SDL/audio/save startup cases.

The accompanying 20-second silent preview is captured from real simulated ladder ascent/descent in the software SDL renderer at 15 captured frames per second. It is a visual review, not a physical-device performance benchmark.

Physical PS Vita installation, controls, frame pacing and memory measurements remain pending. Elevators, sloped collision and a full 3D renderer are not implemented. The six missions share a building system; further bespoke props, encounter choreography and animation refinement remain possible.

## Reproduce

```sh
cmake -S . -B build-qa -DKH_VITA=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-qa -j4
ctest --test-dir build-qa --timeout 180 --output-on-failure
python3 -m unittest discover -s tests -p test_atlases.py
python3 tools/test_runtime.py --binary build-qa/kiyi_hurdasi
```

Story/hazards: `tools/create_campaign.py`. Building routes: `tools/architecture.py`. Run the campaign generator and `tools/compile_campaign.py` after authoring changes. Sprite poses are defined in `tools/sourceboards/climb-frames.json`; the existing atlas importer reproduces the packed texture.
