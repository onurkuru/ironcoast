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
- Added grounded actor shadows so jumps, drones and deaths read clearly against the painted backgrounds.
- Expanded the procedural soundtrack with chord pads, bass movement, kick, snare, hi-hat and ghost rhythm layers. Boss encounters raise the tempo, transpose the motif and add tom/alarm accents.
- Audio now runs at 32 kHz with per-effect noise balance and short attack/release envelopes: metallic shots stay punchy, laser stays tonal, and flame/blast effects retain controlled texture. Added dedicated Flame Shot and Laser sound signatures while preserving the existing weapon, rescue, hit, vehicle and boss cues.

## Verification

- `ctest --test-dir work/build-desktop --output-on-failure` passes all gameplay, geometry, boss, weapon-pool and long-simulation assertions.
- Desktop and Vita targets compile successfully.
- The packaged macOS app launches with the updated HUD and gameplay code.
- The VPK contains the updated executable and all nine runtime assets.

Physical Vita hardware was not used in this pass; testing remains on the desktop target as requested.

Reference: [SNK Metal Slug overview](https://www.snk-corp.co.jp/us/games/acaneogeo/metalslug/), [SNK Metal Slug series page](https://game.snk-corp.co.jp/official/metalslug_sp/english/index.html), and [RetroGameZone Metal Slug Sprite Database](https://retrogamezone.co.uk/metalslug/).
