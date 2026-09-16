# Ironline review asset generation

Built-in image_gen, 2026-09-15. Approved style reference:
`../../../Phase-1-Visual-Targets/02-ironline-final-target.png`.
Successful assets were normalized to 2046 pixels wide with sips. Runtime key
coverage/despill is applied only to the carriage and forest files.

## Carriage source

Initial output: `exec-769fdb4c-9121-4a7a-a0f9-b21e46d7b907.png` in the thread's
`/Users/onurkuru/.codex/generated_images/01a07b04-646e-7e62-84c5-25172b35ea51/`
directory. This output contained a baked checkerboard and was not imported.

Prompt:

Use case: stylized-concept. Asset type: production foreground train sprite, transparent PNG. Image 1 is the approved style reference. Recreate ONLY the two richly detailed side-view passenger train carriages from that reference, no people anywhere, no weapons or bullets, no landscape, no sky, no tracks, no foreground poles or foreground occluders. Truly transparent background above, below and around the train. Wide landscape canvas 3:1, preferably 2048x688. Two connected charcoal blue steel carriages with flat walkable roofs on exactly the same horizontal level; slim metal coupler footbridge spans the separation at roof level. Rust, bolts, ribbed steel panels, tiny roof vents, warm amber windows, visible undercarriage wheels. Fixed orthographic side view, no foreshortening. Carriages extend flush to left and right edges so there are no visible front noses. Roof line at 20 percent canvas height, train occupies from 20 to 92 percent canvas height, clear empty alpha above. Keep roof fixtures under 4 percent image height so characters can run past them. Cool blue rim-light, crisp detailed cinematic pixel-art clusters matching the supplied reference, no illustrations painted inside windows of people. No text, no UI, no borders, no background fill. Usable layered game asset.

## Carriage key correction

Edit target: the initial carriage output above.
Selected output: `exec-ba9dc913-89d9-4f34-9da0-05c65724ebe1.png` in the same
generated-image directory. Imported as `assets/ironline-carriages-review-v1.png`.

Prompt:

Precise object edit for a game sprite import. Preserve the two-carriage train pixel art exactly: same roof baseline, position, scale, coupler, silhouettes, windows, colors and detail. Replace ALL of the gray-and-white checkerboard with a perfectly uniform solid chroma-key green #00FF00, including the gap under the coupler and gaps beneath the carriages. No green anywhere inside the train. NO checkerboard, NO gradients, NO texture in the green, no shadows cast onto the green. Opaque PNG with solid green background for deterministic engine keying. Do not redesign or move the train. Keep the exact same wide aspect ratio and framing.

## Mountain panorama

Selected output: `exec-6c8727c1-22c4-4f79-8a3b-2c34630ec2e5.png` in the same
generated-image directory. Imported as `assets/ironline-mountains-review-v1.png`.

Prompt:

Use case: stylized-concept. Asset type: single continuous background panorama for a cinematic pixel-art train action game. Image 1 is the approved palette and landscape reference. Generate only the expansive distant blue mountain landscape: multiple layers of jagged mountains, conifer forests, fog filling valleys, a distant stone railway viaduct with sparse tiny warm amber lamps in the middle distance. No trains, no carriages, no characters, no weapons, no foreground poles, no text/UI. Wide 3:1 canvas, full bleed opaque background. The scene must be readable across its entire width, level distant horizon, cold desaturated steel blues with deep dark teal forests. Highest detail cinematic pixel-art clusters matching the reference. Mid-distance fir trees along lower third, mountains and atmospheric sky filling the top two thirds. Broad continuous distant vista suitable for slow horizontal parallax, no panels or borders. Avoid giant nearby trees obscuring the view.

## Forest layer

Selected output: `exec-74979203-3708-463e-b824-4c37b5165b3f.png` in the same
generated-image directory. Imported as `assets/ironline-forest-review-v1.png`.

Prompt:

Use case: stylized-concept. Game parallax asset: a single horizontal strip of distant spruce/fir forest silhouettes matching the dark steel blue cinematic pixel art of reference image 1. ONLY a dense layered conifer treeline with varied heights across the width, muted cool blue edge highlights and fine pixel detail. No train, no people, no mountains, no ground or sky. Trees occupy lower 80 percent of wide 3:1 canvas and extend flush to bottom and both side edges. Background behind and above all needles must be perfectly solid chroma key green #00FF00, opaque image, NOT checkerboard transparency. No green in the trees. Dense dark cool blue silhouettes, soft atmospheric detail but sharp keyed edges. For a continuous scrolling layer, make left/right tree height similar and avoid any unique focal tree. No text, UI, borders, glow or gradients in the green.
