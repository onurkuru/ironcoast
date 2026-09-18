# Player menu continuity

1. [MODIFY] src/render.h: shared pause navigation, safe cancel/resume and controls return context.
2. [MODIFY] src/main.cpp: route pause entry/navigation and controls return through the shared flow.
3. [MODIFY] src/render.cpp: readable selectable pause menu and accurate keyboard/controller hints.
4. [MODIFY] tests/test_render.cpp: validate pause navigation and controls return, then inspect actual menu captures.
5. Refresh playable build, documentation and GitHub after integrated checks.

6. [MODIFY] tools/test_runtime.py: include the actual pause screen in application startup coverage.
