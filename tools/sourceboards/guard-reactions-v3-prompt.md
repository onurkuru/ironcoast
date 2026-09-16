# Guard reactions v3 provenance

Generated with the built-in image generator using the imagegen skill.
Identity reference: `assets/workshop-guard-v2.png`.
Selected original: `/Users/onurkuru/.codex/generated_images/01a07b04-646e-7e62-84c5-25172b35ea51/exec-0a97f461-f68c-4698-86e2-0a296e9bc31b.png`.
Preserved source: `guard-reactions-v3-source.png` (1374 x 1145).
Runtime atlas: `assets/guard-reactions-v3.png` (640 x 800, 20 cells).

## Exact prompt

Use case: stylized-concept.
Asset type: production cinematic pixel-art enemy reaction animation sprite sheet.
Reference image 1: identity/outfit/detail guide ONLY. Preserve the same adult masked industrial guard: black armored helmet with slim amber visor, charcoal segmented armor, dark cargo trousers, heavy brown-black boots, compact black rifle. Full-body adult realistic proportions, detailed pixel clusters, subtle cool rim and warm metal highlights.
Create exactly FOUR COLUMNS and FIVE ROWS, 20 separate frames total, on a uniform pure chroma green #00FF00 background. No text, labels, gridlines, shadows, effects, blood, muzzle flash or scenery. All characters face LEFT. Same body scale in every frame, including collapsed poses; never enlarge the prone body. Generous gutters and complete boots/rifle in every cell. Ground contact is on the same baseline within each cell. Pose progression is critical: no duplicated static hurt poses.
Row 1 frames 1-4: head impact and recovery: helmet snaps backward RIGHT with chin raised; guarded recoil with shoulders following; lowering head/regaining balance; return to ready stance.
Row 2 frames 5-8: torso impact and recovery: chest recoils RIGHT; strong hunched compression with hand toward ribs; regain footing; recover to ready rifle stance.
Row 3 frames 9-12: leg impact and recovery: front LEFT knee buckles; drop onto one knee with other boot planted; push up from planted leg; stand again. Keep body anatomy the same, no vertical scaling.
Rows 4 and 5 frames 13-20 ONE CONTINUOUS EIGHT-FRAME DEATH, reading order: 13 standing hit/chest arches backward to RIGHT; 14 knees fold and upper body tilts RIGHT; 15 low stagger RIGHT reaching out; 16 hip falling right toward ground; 17 body lands on RIGHT hip; 18 shoulders fall right and legs extend; 19 almost prone on back with small settling motion; 20 fully settled prone with head RIGHT, boots LEFT. No reverse motion back to standing, no detached limbs. Place a clear standing-to-fall transition before any prone frame. Same scale throughout. Flat left-facing game side view. This sheet must match the reference guard's silhouette and armor, not redesign it.

## Selection and import

The generated rows start with ready poses. The runtime hit timelines deliberately
start at the second pose so damage does not show a neutral frame first. The leg
recovery reuses a kneeling pose before standing. The eight death cells form a
single collapse; mirrored playback supplies the opposite hit direction, not a
separately drawn forward-fall performance. `import_guard_reactions.py` keys and
packs the generated raster at one fixed scale, with measured floor/root anchors.
No limb interpolation or new drawings are produced by the importer.
