# Ironline campaign continuation

Continue the recovered campaign and the user's REPLACED visual quality target.
The existing two-carriage review remains a separate, working art slice.

1. [NEW] `assets/ironline-integrated-v2.png` and `pixel-art/ironline-integrated/`:
   generate a foreground train with three roof routes, six attached ladders,
   two reinforcement doors and a connected lower service deck. Import the corrected
   chroma-matte source without rewriting the generated PNG.
2. [MODIFY] `data/presentation.json`, `tools/compile_presentation.py`:
   measured train anchors, independent scenery travel and restrained local lights.
3. [NEW] `tools/ironline_architecture.py`; [MODIFY] `tools/architecture.py`,
   `data/campaign.json`, generated `src/campaign.cpp` and `src/presentation_config.h`:
   derive playable surfaces, ladders, workers and encounters from the art.
4. [MODIFY] `src/cinematic_render.cpp`, `src/render.h`, `src/render.cpp`,
   `src/architecture_render.cpp`, `src/themed_environment.cpp`:
   integrate foreground geometry, independent moving scenery and texture lifetime.
5. [MODIFY] `src/main.cpp`, `tests/test_painted_routes.cpp`, `tests/test_render.cpp`,
   `CMakeLists.txt`: continuous campaign review, contact/route checks, deterministic
   scenery movement and transition validation. Keep the short train review intact.
6. [NEW] `docs/IRONLINE_INTEGRATION.md`, `system_architecture.md`;
   [MODIFY] `README.md`: actual executable captures, limitations, reproduction.
7. Build host and Vita, run relevant tests, inspect real motion and light/readability,
   export a review with game audio and a local package, commit and update GitHub.

This continuation is already authorized by the user's repeated request to continue
development and update GitHub. The plan does not require another approval round.
