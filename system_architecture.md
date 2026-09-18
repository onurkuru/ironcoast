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

## Arcade gameplay pass — 17 September

Campaign pacing uses Game::arcadeCombat(): ordinary campaign and chapter openings use arcade controls, while isolated art reviews retain their cinematic timing. campaignPresentation owns run/climb/vehicle/jump speed. Powered weapons consume finite reserve rounds without forced magazine stalls; optional manual reload remains. Ordinary hits no longer stop the whole simulation; explosive impacts pause at most one tick. Infantry commit facing through windup, and melee only reaches forward.

All human enemy kinds 0–2 share the adult guard rig; drones and turrets keep mechanical silhouettes. Shield equipment has an authored chroma-key PNG, lowers during the real vulnerable attack state and uses the same facing as its collision behavior. Grenade troops fire from the authored muzzle. Gameplay lighting lifts uniform detail and hostile projectiles use a distinct warning palette. HUD presents health, lives, reserve ammo, grenades, score and boss health; contextual cues explain low targets and real ladder access.

arcade_encounters.py appends bounded wingmen and five-person door squads after each architecture generator rebuilds its base lists. Early and post-door weapon caches sustain the new cadence. Do not call this append pass twice on existing generated lists; rerun the full architecture generator instead. Entrances retain warning/exit time and do not spawn indefinitely.

Swept projectile contacts choose the nearest entry time across terrain, enemies, bosses and props. Solid grenade impacts reflect the hit axis; one-way decks accept downward top crossings only. Blast radii intersect actual body rectangles. tests/test_arcade_mechanics.cpp covers order independence, cover, grenade normals, body-scaled blasts and crouched fire against low turrets.

ArcadeReview drives ordinary inputs and actually engages ground squads; --arcade-review uses normal damage, while --arcade-review-assist explicitly enables invulnerability for presentation captures. Neither controller teleports actors or injects boss damage. Assisted captures establish movement and integration; normal-damage results are recorded separately.

Renderer owns a reusable native 480x272 target when SDL supports it, then upscales with nearest filtering and restores the caller target, viewport and scale. Target-less backends keep the direct rendering path. This already-implemented optimization is retained; additional Vita work is deferred behind gameplay review at the user’s request.

## Fair encounter continuation

Props participate in nearest-contact selection for hostile as well as friendly bullets. The same cover must not stop only the player’s weapon. Foundry warnings now distinguish even-pattern falling columns from odd-pattern horizontal sweeps, and drop columns use the real phase-dependent counts/offsets for Harbor, Foundry and Final Wave. Keep warning formulas synchronized with `fireBossVolley()` when changing those patterns.

`ArcadeReview` aligns beneath elevated drone targets before upward firing and resets its cooldown bookkeeping when mission time resets. The acceptance test now requires all six ordinary-damage fresh mission clears and a full `beginChapter`/`advanceSection` campaign, using normal `retry` after Game Over with a bounded Continue count. Normal and assisted outcomes remain distinct.

## Secured boss victory

Boss damage is accepted only during Status::Play. Once boss.dead is set, hitPlayer ignores further arena damage during destruction. If the player dies first, later shots cannot bank a posthumous boss victory. The 2500 boss points and campaign rescue total are committed exactly once on transition to Clear, together with rescue points. Six-boss regression coverage checks both hit orders and carried totals.

## Pause and controls navigation

ViewState::enterPause resets selection to Resume for keyboard, controller disconnect and lost-focus entry. pauseInput prioritizes pause/cancel over confirm because Start maps to both on host controllers. Main Menu requires explicit selection. Controls opened from Pause return to Pause and retain paused audio; controls opened from Title return to Title. Host hints use actual keyboard bindings and SDL controller A/B/X/Y names; Vita retains its native labels.
