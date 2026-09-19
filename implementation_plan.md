# Parallel completion audit

1. Combat agent: inspect late-chapter death/Continue/checkpoint route state; owns src/game.cpp and tests/test_arcade_mechanics.cpp.
2. Design agent: inspect platform support/entrance/ladder/control visual overlaps; owns src/themed_environment.cpp and src/architecture_render.cpp.
3. Independent QA: read-only completion/save/training and ending-flow audit.
4. Root: prioritize confirmed findings, apply save/menu fixes if necessary, review and integrate agent changes, run relevant checks and refresh delivery.

Do not invent extra features when an audit finds no bug.
