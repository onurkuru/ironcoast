# Campaign completion pass

1. Root: [NEW] src/save_progress.h, tests/test_save_progress.cpp; [MODIFY] src/main.cpp, src/render.h, src/render.cpp, CMakeLists.txt. Persist mission completion and best rescues; migrate legacy saves; show progress on map and ending.
2. Combat agent: verify all 18 rescue routes including late chapter galleries using real movement inputs; report/fix blockers within tests/review helpers.
3. Design agent: improve late chapter platform/support/entrance integration in src/themed_environment.cpp and src/architecture_render.cpp; retain all nine sabotage controls.
4. Integrated build/tests, real visual capture, refresh playable delivery and GitHub.

Completion gates: campaign and full-rescue traversal, completion persistence, coherent late-stage scene contact, regression-free delivery. REPLACED benchmark and human playfeel remain explicit quality gates, not inferred from automated tests.
