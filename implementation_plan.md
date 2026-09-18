# Late campaign sabotage routes

1. [MODIFY] src/game.h/src/game.cpp: three optional upper-route controls in chapters 4–6, each disabling its associated floor hazard and awarding one grenade/250 points once. Preserve disabled controls through Continue; reset on a new mission. Derive anchors from authored gallery colliders.
2. [MODIFY] src/themed_environment.cpp/src/hud_render.cpp: grounded control cabinets, armed/off feedback, interaction prompts and brief success feedback.
3. [MODIFY] tools/architecture.py, data/campaign.json and generated campaign: explain each chapter objective in briefing.
4. [MODIFY] tests/test_arcade_mechanics.cpp: normal-input control activation, vertical reach, once-only reward, retry persistence, new-mission reset.
5. Validate campaign and actual visual/gameplay captures, refresh local build and GitHub.

6. [MODIFY] src/render.cpp and docs/DEVELOPMENT_STATUS.md/system_architecture.md: show disabled outlets and document the nine-objective progression.
