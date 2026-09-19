# Late campaign combat readability

1. [MODIFY] src/game.h/src/game.cpp: expose disabled-hazard state and a 0.7-second pre-activation warning from the existing hazard cycle; do not alter damage or timing.
2. [MODIFY] src/render.cpp: show floor hazard charge-up and Relay radial volley directions during windup. Preserve disabled outlet indicators.
3. [MODIFY] tests/test_arcade_mechanics.cpp: verify warning boundaries, active/safe phases and disabled circuits.
4. Independent agent audits late boss logic; address verified blockers if found.
5. Build/test/capture; refresh playable package and GitHub.

6. Verified Foundry fixes: allow the fourth phase-two sweep volley before recovery; emit all sweep rounds from the real muzzle and share their vertical velocities with warning rays.
