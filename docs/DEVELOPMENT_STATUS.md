# Development status — 18 September 2026

The current deliverable is an arcade gameplay development preview. A single completion percentage would hide the difference between playable features and the much higher cinematic art target.

| Area | Evidence | Remaining validation |
|---|---|---|
| Campaign | Six missions, two opening sections, six bosses, checkpoints, rescues, vehicles and end flow run | Human full-campaign difficulty/playfeel pass |
| Arcade combat | Faster movement, sustained finite-ammo weapons, bounded squads, adult infantry, shield openings and readable incoming fire | Broader player balance feedback |
| Normal-damage automation | Six separate clears plus a full sequential campaign with two opening rooms and three legitimate Continues | Automated clear is not human difficulty approval |
| Logic | Nearest swept hits, cover ordering, proper grenade normals, body-sized blasts and nonrepeatable rescue health | Continue targeted regressions for new bugs |
| Visual quality | Adult roles, shield equipment, hostile-shot readability, three painted/registered environments and remaining scene kits | REPLACED-level animation and scene consistency not achieved |
| Distribution | GitHub source and independent local Mac playtest | Public Vita download is still v0.3.0; no new release claimed |

The user explicitly prioritized game completion over further Vita optimization. Existing native-resolution rendering remains, but hardware work is deferred until gameplay review. No physical Vita frame-rate claim is made. See [arcade verification](ARCADE_GAMEPLAY.md) for reproducible results.

Late-campaign content pass: Foundry coolant valves, Relay breakers and Final Wave defense overrides add nine optional upper-route objectives. Each disables one floor trap and gives a once-only score/grenade reward. All nine routes are covered by ladder/walk interaction tests. This is gameplay content progression, not completion of the cinematic art target.

19 September completion pass: all 18 workers, 36 ladder traversals and nine sabotage controls verified in an assisted continuous six-chapter campaign. Separate normal-damage six-mission and sequential tests remain passing. Per-mission completion/best-rescue records now persist with legacy-save migration and visible write failures. Late-stage cabinets use the existing authored valve/cabinet atlas and ladder connections were clarified. 27 automated tests and 14 full-application checks pass. Final cinematic artwork and human playfeel validation remain open.

Late-boss readability pass: timed hazards now telegraph their existing cycle for 0.7 seconds, Relay previews radial firing directions, and Foundry sweep rounds originate at the real muzzle. Phase two now fires all four advertised sweep rounds. Normal-damage Foundry completes in 2697 frames (44.95 seconds), ending with one life/three health; no boss HP or damage reduction. The review controller now leaves the full falling-column spread before impact.

Continue rescue fix: missed workers behind a checkpoint remain available after Continue; already rescued workers and disabled circuits remain completed. Reproduced before fixing, then verified in all three late chapters using actual last-life death and movement back to an upper-route worker. All 27 tests pass.

Mac delivery: a self-contained Apple Silicon app includes SDL, assets and dependency licenses. Current host binary requires macOS 26.2 or newer; app is locally ad-hoc signed, not notarized. Bundled resource discovery is checked from /tmp without --assets. The convenience launcher retains its existing local save path.
