# Launcher artwork — 8 September 2026

Created using the built-in image generation tool, not the CLI/API fallback. These are original promotional assets, not gameplay screenshots. No reference image or commercial game artwork was supplied.

## Icon prompt

Use case: logo-brand. Create a square original PS Vita homebrew game bubble icon for IRON COAST: SCRAP TIDE. No text. A bold rust-orange armored industrial crane claw curled around a luminous cyan power core, with a subtle harbor wave at its base, on very dark navy. Crisp premium pixel-art illustration with controlled blocky edges and a strong readable silhouette at 128x128. Center all essential shapes within the middle 70% safe circle because Vita crops to a bubble. High contrast orange/cyan highlights, very few large shapes, no fine scenery, no border, no letters, no other game characters, no console mockup. Output one square image.

Master: `icon-source.png`. Packaged output: `../../sce_sys/icon0.png`.

## Cover prompt

Use case: ads-marketing. Asset type: original PS Vita LiveArea game key art, wide landscape around 16:9. IRON COAST: SCRAP TIDE is an original pixel-art run-and-gun about Deniz rescuing Efe from the Iron Grid in a stormy industrial harbor. Create richly painted cinematic pixel art: a small full-body brown-haired mechanic/soldier in orange-brown jacket and dark trousers carrying a compact rifle at lower left, distant rust-orange crane walker with glowing cyan core on right, layered harbor warehouses, gantries, dark blue sea haze, warm lamps and cyan industrial light. Bold readable title in upper left exactly 'IRON COAST', small subtitle exactly 'SCRAP TIDE'. Preserve clear central-lower space for the Vita launch button. No UI, no health bars, no console mockup, no other franchise characters or logos, no additional text. Strong silhouettes, restrained light bloom, sophisticated night atmosphere. This is promotional art, not a gameplay screenshot.

Master: `cover-source.png`. Outputs: `cover.jpg`, `../../sce_sys/pic0.png`, LiveArea `bg0.png` and `startup.png`.

## Packaging

`tools/prepare_livearea.py` performs deterministic resizing, opaque compositing and 256-color PNG encoding. It does not redraw the artwork. The bubble has an inset to protect the main silhouette from circular clipping. Master images remain unchanged.

Dimensions and indexed-image constraints follow the first-hand [LiveArea specifications](https://github.com/hammerill/livearea-specs); XML uses the minimal structure in the [VitaSDK sample](https://github.com/vitasdk/samples/blob/master/hello_world/sce_sys/livearea/contents/template.xml). Package validation checks image headers, palette sizes, XML references and archived file bytes. Appearance on a physical Vita remains unverified.

## v0.3.0 gameplay architecture and climbing

The built-in image-generation tool created the six-panel wall material atlas and a new climbing board using the original hero as its identity reference. Exact prompts and import details are recorded in [architecture-prompts.json](../../tools/sourceboards/architecture-prompts.json). Runtime assets are [architecture-v1.png](../../assets/architecture-v1.png) and [climb-v2.png](../../assets/climb-v2.png). The original climbing source and measured pose map are committed alongside the other source boards. No external game sprites were used.
