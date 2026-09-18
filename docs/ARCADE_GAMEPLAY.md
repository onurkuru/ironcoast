# Arcade gameplay pass

This pass prioritizes the playable six-mission campaign. The cinematic art target remains ongoing; Vita-specific optimization is deferred until gameplay review.

## Player-visible changes

- Run speed 145 → 205 and climb speed 72 → 130 world units/second. Acceleration and camera response support the faster pace without changing the established jump-height geometry.
- Campaign weapons cycle magazines without a forced pause. Powered ammo remains finite, pistol ammo is unlimited, and optional manual reload still exists.
- Infantry share adult proportions. Grenadiers launch from their authored muzzle, shields lower during firing, and turning behind a committed shield exposes its back.
- Five-person door squads, extra supported roof guards and early/post-door weapon caches create sustained, finite encounters.
- Ordinary bullet hits do not freeze movement. Forward-only melee, nearest swept impact and physically oriented grenade bounce remove concrete combat errors.
- Health, lives, reserve ammo, grenades, score and boss health are visible. Hostile rounds and grenade fuses have separate readable cues.
- Rescues restore one missing health point plus the existing two grenades. Used workers remain used after checkpoint retry.

## Reproduce gameplay

Build the host target, then run `kiyi_hurdasi --stage 1 --arcade-review --frames 4200` from the repository root. This controller uses ordinary movement, firing, grenades, crouch and vehicle interaction; it waits for actual door squads and fights them. It uses normal damage. `--arcade-review-assist` is a separately named invulnerable capture mode. The older traversal demo now continues walking after a boss dies.

The controller never teleports actors, restores health or injects damage. Its perfect access to current game state still makes it an automated regression instrument, not a human enjoyment or difficulty verdict.

## Verification

Final integrated test and normal-damage results are appended after the team handoff. Physical Vita performance and REPLACED-equivalent art quality are not certified by host tests.

### Normal-damage controller results

| Mission | Time | Outcome |
|---|---:|---|
| Harbor | 27.7 s | Clear, 3 lives / 1 HP |
| Marsh | 34.8 s | Clear, 2 lives / 3 HP |
| Ironline | 47.7 s | Clear, 1 life / 3 HP |
| Foundry | 44.7 s | Clear, 1 life / 1 HP |
| Relay | 45.6 s | Game over, boss 36.38 HP remaining |
| Final Wave | 57.1 s | Clear, 1 life / 2 HP |

Each route engages 13–16 enemies. All six assisted routes clear; this is separately reported and does not overwrite the Relay normal-damage failure. The tests require normal routes to reach their bosses and terminate, plus at least the five currently confirmed clears. Human pacing, difficulty and enjoyment still require playtesting.

### Integrated checks

All 26 host CTest cases pass, including six-stage progression, continuous routes, new combat/controller regressions and texture-state restoration. All 13 full-application SDL/audio/menu/malformed-save startup checks pass.

![Actual normal-damage Harbor gameplay](arcade-gameplay.png)

The frame above is captured from the executable during the ordinary-damage Harbor review. The local review includes its complete 30-second recording with game audio.
