# Current development architecture

## Integrated campaign scenes

Harbor, Marsh and Ironline are campaign indices 0, 1 and 2. Each uses normalized
`<scene>BuiltScene`, `<scene>Galleries`, `<scene>Ladders` and `<scene>Doors` anchors
in `data/presentation.json`. `sourceFloor` registers the painted lower service
surface to world y=232; `sourceAspect` determines the painting's world height.
Use the actual asset aspect, not an assumed 3:1 ratio.

The respective `tools/<scene>_architecture.py` derives collision surfaces,
ladders, reinforcement entrances, rooftop encounters and rescue positions.
`tools/architecture.py` must call all three registered builders at the end of
full campaign generation. `tests/test_painted_geometry.py` checks this path.
Roof traversal tests use the actual attached ladder pair rather than a fixed
inset from the building bounds.

After editing anchors, run the relevant scene builder, then
`tools/compile_campaign.py` and `tools/compile_presentation.py`. Their generated
C++ outputs are committed. CTest validates that painting anchors, JSON and C++
remain in agreement; do not hand-edit generated coordinates.

The renderer does not overlay the old platform/ladder kits on these three maps.
Only measured reinforcement door panels slide over the authored openings.
`PaintedRouteReview` supplies normal controls for the three-gallery routes;
`--harbor-review`, `--marsh-review` and `--ironline-campaign-review` enable it.
The initial spawn is fixed and review invulnerability is enabled. These routes
are not combat-balance or hardware-performance certification.

## Ironline layers and scene ownership

The campaign train uses `assets/ironline-integrated-v2.png`, keyed once at import
with the existing green-matte/despill rule. The source PNG remains unchanged.
The first generated checkerboard attempt was rejected and is not a game asset.

Train art, collision, actors, doors and shadows use the same full world camera.
Mountains and forest have separate horizontal and vertical parallax depths in
`ironlineTravel`; the train advances through scenery time without sliding its
surface under actor feet. The same scene time reproduces the same pixels.
Alternating mirrored scenery panels maintain edge continuity, but scenery does
repeat. Weak cool roof fill complements the registered warm window/door lamps.

The short `--ironline-review` / `--ironline-demo` remains separate: two carriages,
a coupler jump, two guards and an isolated exit. It uses `ironlineReview` tuning.
Do not silently replace its geometry or campaign isolation while editing the
three-carriage campaign.

`releaseSceneLayers()` retains mountains and forest only for Ironline and drops
the campaign plate on entering a review scene. Temporary tint/alpha changes must
be restored because the short review shares the scenery textures. Tests cover
scenery motion, stable train contact, key contamination and leaving/reloading
the campaign scene.

## Quality and release boundaries

`pixel-art/QUALITY_TARGET.md` records the user's REPLACED benchmark. Integrated
architecture is a development milestone, not equivalent production quality.
The source and local Vita package are v0.4.0 development; the public release is
still v0.3.0. Real Vita installation, performance and controller feel remain
unverified. Keep claims and screenshots tied to the actual source/package tested.
