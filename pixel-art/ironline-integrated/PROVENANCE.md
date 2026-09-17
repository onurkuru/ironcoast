# Integrated Ironline foreground

Produced on 17 September 2026 with the built-in Imagegen tool.

1. `prompt.txt` extended the existing `assets/ironline-carriages-review-v1.png`
   into three carriages and a flatbed. The user's REPLACED screenshot was a
   quality reference only. Output `exec-4a97ddcc-42c2-4fd3-b714-936eba0acc27.png`
   contained a painted checkerboard instead of alpha and was rejected.
2. `matte-correction-prompt.txt` removed that background in an Imagegen edit,
   preserved the train geometry and added lower coupling gangways. Output
   `exec-92d60ee8-fec5-4f65-8b8c-89fdccbcbeae.png` was copied unchanged into
   `assets/ironline-integrated-v2.png` (2164×727 RGB).
3. Actual executable frames were inspected for roof/feet contact, ladder
   transitions, door openings and key spill. Mountain/forest intensity and a
   weak roof fill were tuned in `data/presentation.json` after that inspection.

The accepted source is an opaque green-matte image. The engine removes green
and despills edges at import; it does not rewrite the PNG. This is not a claim
of native generated alpha or manually authored pixel-by-pixel art. The reused
mountain/forest sources retain their earlier provenance in
`tools/sourceboards/ironline-review-prompts.md`.
