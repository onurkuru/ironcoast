# Debug and Combat Pass

## Baseline findings

- The simulation already used a fixed 60 Hz step, but the player animation clock was reset to zero whenever movement stopped. That made a run-to-idle transition snap back to the first frame.
- Normal poses, upward aim and crouch used different destination heights. The sprite artwork therefore appeared to change size while the character stayed on the same ground line.
- A non-vehicle hit immediately entered the death state. Three lives existed, but there was no health buffer or readable recovery window.
- The combat loop had pistol, heavy machine gun, shotgun and rocket launcher. The pickup system was ready for more item kinds, so two additional weapons could be added without changing the campaign format.

## Reference principles applied

SNK describes the original Metal Slug as an action shooting game where players use varied weapons and vehicles. The implementation uses those high-level principles as a genre reference while keeping the characters, art, names and story original:

- weapon pickups have distinct jobs: sustained fire, close spread, explosive area damage, short-range flame control and precision laser damage;
- attacks are readable through muzzle flash, projectile color, impact particles and boss telegraphs;
- close-range attacks, grenades and a vehicle remain useful choices instead of replacing the base pistol;
- the camera stays locked to a 480×272 logical canvas with integer scaling.

The RetroGameZone Metal Slug Sprite Database is useful as a frame-by-frame reference: its archive separates player, vehicle, enemy, boss, HUD and effect sheets, and its notes call out multiple contact-distance punch frames. I used that structure as a timing target—anticipation, contact, follow-through and recovery—without importing ripped commercial sprites.

## Changes applied

### Animation and scale

- Added a continuous animation clock, landing timer, recoil timer and hit flash to the player state.
- Normal player poses now share a stable 48×48 ground box. Aim, grenade, melee, jump and fire transitions keep the same baseline.
- Authored atlas cells are now kept at their original grid bounds instead of being trimmed independently; the renderer interpolates the player between fixed 60 Hz simulation ticks for smoother camera motion.
- Added walk bob, idle breathing, vehicle bob, muzzle flash and landing dust.
- Extended melee and grenade pose timing so their frames are readable instead of being consumed in a few ticks.
- Fire poses and muzzle flashes persist for the weapon cooldown, so releasing the button cannot cut the contact frame in half.
- Replaced the player, infantry and Scrap Walker presentation with original v2 pose boards. The player now has eight rows and distance-driven run/run-fire frames; enemies use six eight-frame rows and retain a readable procedural death arc.
- Added six separate 4×4 boss atlases with locomotion, attack, recovery and destruction rows. The renderer selects a pose from the explicit boss state and uses one texture per boss to stay within Vita texture limits.
- Enemy deaths now use a short upward arc, rotation, scale change, fade and ground shadow. Player deaths use the four death frames with a squash, arc and rotation. Boss deaths keep their multi-burst explosion sequence.

### Weapons

- Added **Flame Shot (F)**: 80 rounds, short range, three-pellet cone, high close damage.
- Added **Laser (L)**: 36 rounds, fast precision projectile, high single-target damage.
- Added F and L pickups to every mission, plus orange flame and cyan laser projectile rendering and separate audio cues.
- Existing HMG, shotgun and rocket roles remain unchanged and auto-fallback to the pistol when ammo is empty.

### Survivability

- Added 3 HP per life, an 0.82 second damage cooldown, hit flash and knockback.
- A life is consumed only after HP reaches zero. Respawn restores full HP and two seconds of protection at the latest checkpoint.
- Added an HP meter to the HUD beside the lives counter.

### Visual and audio polish

- Added midground parallax silhouettes, practical lights, warm coastal haze and a restrained CRT scanline/vignette pass.
- Added a four-rate 2.5D depth stack: the painted world scrolls slowly, authored prop panels drift at 34%, gameplay remains at 100%, and a sparse rail/cable foreground sweeps at 112–128%. The foreground is drawn after actors with low alpha so it gives camera movement depth without hiding targets or changing collision.
- Added grounded actor shadows so jumps, drones and deaths read clearly against the painted backgrounds.
- Expanded the procedural soundtrack with chord pads, bass movement, kick, snare, hi-hat and ghost rhythm layers. Boss encounters raise the tempo, transpose the motif and add tom/alarm accents.
- Audio now runs at 32 kHz with per-effect noise balance and short attack/release envelopes: metallic shots stay punchy, laser stays tonal, and flame/blast effects retain controlled texture. Added dedicated Flame Shot and Laser sound signatures while preserving the existing weapon, rescue, hit, vehicle and boss cues.

## Verification

- `ctest --test-dir work/build-desktop --output-on-failure` passes all gameplay, geometry, boss, weapon-pool and long-simulation assertions.
- Desktop and Vita targets compile successfully.
- The packaged macOS app launches with the updated HUD and gameplay code.
- The VPK contains the updated executable plus the v2 hero, enemy, vehicle and six boss atlases alongside the environment, aim, melee and effects sheets.

Physical Vita hardware was not used in this pass; testing remains on the desktop target as requested.

## Boss motion regression — September 7, 2026

### Root causes

- Most bosses never changed their world X position. The dredger subtracted 35 units on one attack and the flying relay reassigned X after a volley; neither had continuous movement.
- The renderer drew each boss as a single image with a two-pixel wobble. There were no independently moving feet, weapons or suspension.
- Player-only interpolation left the camera and other moving objects on different presentation times. Integer rounding discarded part of the remaining smooth motion.
- Firing and running poses were selected from a global clock, independent of the shot event or distance travelled.

### Implemented behavior

| Boss | Movement and articulated parts |
| --- | --- |
| Claw Crane | Accelerating walk, alternating feet, pivoted boom and swinging hook |
| Ash Dredger | Tracks, drill vibration, telegraphed forward charge and braking |
| Black Locomotive | Repositioning on tracks, lifted barrel, timed recoil on repeated salvos |
| Forge Titan | Walking feet, hammer preparation, contact after 0.18 seconds, recovery |
| Four Poles | Continuous horizontal/vertical flight, moving coil assemblies and thruster |
| Iron Grid / Sarp | Alternating legs, independent gun recoil and warning markers |

All bosses preserve a left escape corridor and expose the core during recovery. The second phase increases movement speed and reduces cycle downtime while retaining at least 0.72 seconds of attack warning. Entry, move, windup, attack, recovery and overload are explicit states. Death breaks the rig apart while the existing explosion sequence completes the mission.

The renderer samples player, boss, camera, enemy, bullet and particle positions from the same fixed-step interval, including subpixel sprite placement. Pause draws the current state, and load/respawn synchronize snapshots to prevent interpolation across teleports. Running frames are distance-driven; firing frames are event-driven.

### Validation and limits

- 101,674 assertions pass, including the existing ten-minute simulation and six levels' completion checks.
- Each of the six bosses was exercised for 30 seconds in each phase. Regression checks cover visible travel, arena bounds, no teleport frames, actual projectiles, preparation time, recovery vulnerability and moving weapon/leg poses.
- A separate transition check prevents the hammer from snapping between its held windup and released swing.
- Desktop and Vita builds succeed. The six-boss montage was captured from the real game renderer at 30 fps and inspected at preparation and repositioning points.
- The new player, enemy, vehicle and boss frames are original artwork generated for this project; the linked Metal Slug sheets were used only as high-level timing and silhouette references. Broad playtesting of difficulty and physical Vita frame-time/audio testing remain separate work.

Reference: [SNK Metal Slug overview](https://www.snk-corp.co.jp/us/games/acaneogeo/metalslug/), [SNK Metal Slug series page](https://game.snk-corp.co.jp/official/metalslug_sp/english/index.html), and [RetroGameZone Metal Slug Sprite Database](https://retrogamezone.co.uk/metalslug/).
