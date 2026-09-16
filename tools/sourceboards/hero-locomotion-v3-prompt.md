# Hero locomotion v3 provenance

Generated with the built-in image generator using the imagegen skill.
Identity reference: `assets/workshop-hero-v2.png`.
Selected original: `/Users/onurkuru/.codex/generated_images/01a07b04-646e-7e62-84c5-25172b35ea51/exec-831961dc-9f3f-4a30-9e3f-b139152a1da7.png`.
Preserved source: `hero-locomotion-v3-source.png` (1254 x 1254).
Imported runtime atlas: `../../assets/hero-locomotion-v3.png` (1280 x 320).

## Generation prompt

Use case: stylized-concept. Production locomotion animation sprite sheet for this exact adult pixel-art hero. Reference image 1 is IDENTITY, OUTFIT and DETAIL guide: dark short hair, charcoal jacket/trousers, teal scarf, brown boots, compact black carbine. Redraw as ONE precisely spaced 4 columns by 4 rows sprite sheet on perfectly uniform solid chroma green #00FF00, no checkerboard, no gridlines, no text. Every cell same size, body scale constant, full-body seven-head adult proportions, all face RIGHT, feet fully visible with generous margins. 16 frames are TWO complete 8-frame run cycles in reading order. Cells 1-8: weapon low-ready RUN. Cells 9-16: identical synchronized leg phases but weapon shouldered horizontally right RUN-AND-FIRE, no baked muzzle flash. CRITICAL genuinely sequential leg biomechanics, NOT eight copies of one stride. Exact phases per 8-frame cycle: 1 right heel forward on ground and left toe back; 2 right foot planted under center with bent knee, left foot lifting behind; 3 weight on right forefoot moving behind center while left knee passes forward, narrow silhouette; 4 flight with left knee raised forward, right leg folded behind, BOTH feet visibly above the common ground line; 5 LEFT heel reaches forward and right toe back (opposite leading limb from frame 1); 6 left planted under center with knee compression, right foot lifting; 7 weight on left forefoot behind center, right knee passing forward, narrow silhouette; 8 flight with right knee raised and left leg folded behind. Distinguish near/far limbs by lighter near-side trouser shading. Same pelvis center X in every cell, coherent hip and head arcs, 2-4 pixel body bounce. Gun, face, clothing and anatomy must remain consistent, no morphing. Neutral moody blue rim light and warm subtle highlights, crisp detailed cinematic pixel-art clusters at exactly the visual fidelity of the reference, strong readable silhouettes. Grid exactly 4x4 on square canvas, all cells generous empty margins. No environment or ground shadow. This is a reviewed animation asset, not concept art.

## Import and review

`tools/import_hero_locomotion.py` removes the chroma background, suppresses edge
spill, and packs poses using the measured pelvis and common row ground anchors
in `hero-locomotion-v3.json`. It does not generate or paint intermediate poses.
The output preserves foot clearance in flight poses. Both cycles still have
similar near/far-leg silhouettes in their second half; this source is a review
iteration, not a guarantee of the prompt's complete anatomical specification.
