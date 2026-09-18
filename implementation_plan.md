# Victory handoff continuation

1. [MODIFY] src/game.cpp: lock victory against further player damage, accept boss damage only during active play, and award boss/rescue completion rewards at Clear.
2. [MODIFY] tests/test_arcade_mechanics.cpp: cover both fatal-hit orderings, one-time completion rewards and carried rescue totals.
3. [MODIFY] task.md and system_architecture.md: record verified behavior.
4. Build, run campaign/application checks, refresh the local executable and push GitHub.

Game quality remains the priority; no Vita optimization in this pass.
