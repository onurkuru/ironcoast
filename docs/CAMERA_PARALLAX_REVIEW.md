# Workshop camera and harbor parallax review

This phase applies only to the playable workshop. The original main checkout
and the approved source painting are preserved. This is not a six-map release.

## Camera composition

The camera now chooses an establishing shot, a movement follow shot, a quiet
workbench detail, a two-subject combat composition, a gallery reveal or a secured
exit composition. These use pan, vertical framing and restrained zoom. They are
2D cinematic compositions, not actual 3D camera rotations.

The detail shot starts after a quiet dwell beside the workbench. Shooting,
movement and nearby opponents change the composition smoothly. The combat shot
holds the last opponent for 1.25 seconds after death, avoiding abrupt reframing.
Gallery framing follows ladder ascent and the upper landing, not ordinary jumps.
Its upward limit is derived from the source painting's top edge to avoid exposing
unpainted space above the room.
Pan, vertical movement and zoom all have bounded rates. Input is never locked.

## Distance layers

The new 2046 x 682 panorama is a continuous harbor viewed through all workshop
windows. Its camera factor is 0.22; the independent fog factor is 0.42, the room
is 1.0, and the near floor reaches 1.08. Fog drifts at four world units per second
while the harbor remains anchored to the world. There are no per-window repeats.

A glass matte is calculated once when the workshop loads, using bounded window
apertures and blue transmission. Dark window framing, the hoist and warm objects
remain in the foreground. The original exterior's darker silhouettes remain as
near silhouettes, so this is a layered adaptation of the painting, not a clean
hand-separated source scene. Matte transitions need visual review at gallery
height as well as on the floor. No source painting or actor atlas is rewritten.

The new textures add about 11.3 MiB of uncompressed RGBA storage (the runtime
window shell plus panorama). No full-screen render targets, per-frame uploads,
or custom blend shaders are introduced. Physical Vita memory and frame time
remain unmeasured.

## Files and tuning

| File | Change |
| --- | --- |
| `src/workshop_review.cpp` | Shot selection, subject hold, smooth camera targets |
| `src/game.h`, `src/game.cpp` | Camera state and isolated review routing |
| `src/render.cpp`, `src/render.h` | Runtime glass matte, texture lifecycle and interpolated scene time |
| `src/cinematic_render.cpp` | Distant panorama, fog and room compositing |
| `src/main.cpp` | Deterministic `--review-camera` capture scenario |
| `data/presentation.json` | All camera and layer tuning |
| `src/presentation_config.h` | Generated configuration |
| `tools/compile_presentation.py` | Lens, layer ordering, aperture and coverage validation |
| `tests/test_workshop.cpp` | Shot transitions, hold, speed caps and gallery behavior |
| `tests/test_render.cpp` | Wall stability, independent distance movement and deterministic frame replay |
| `assets/workshop-harbor-distance-v1.png` | Generated continuous pixel-art harbor |
| `tools/sourceboards/workshop-harbor-distance-prompt.txt` | Generation prompt and provenance |

Edit `workshopCamera`, `workshopParallax`, `workshopApertures` and `workshopView`
in `data/presentation.json`, then run `python3 tools/compile_presentation.py`
and rebuild. Matte parameters apply on the next workshop load.

## How to review

Open `../Target-Recovery-Review/Play Workshop.command`. Walk both ways while
watching a crane behind a window mullion; the crane should slide more slowly.
Stop by the workbench after the fight to see the quiet push-in. Climb either
left ladder and check the gallery reveal. Shoot the sentry, then observe the
brief framing hold after its death. All game text remains English.

For a reproducible opening, detail, movement and combat sequence:

```sh
mkdir -p ../../work/camera-parallax-frames
../../work/build-cinematic-review/kiyi_hurdasi --assets assets --review-camera \
  --frames 780 --fast --record ../../work/camera-parallax-frames --record-every 1
```

Encode the frames at 60 fps. This is an offline executable capture, not a Vita
performance benchmark. The standard test command is:

```sh
ctest --test-dir ../../work/build-cinematic-review --output-on-failure
```

Validation on the development Mac: all eight CTest suites passed, including
shot transitions, campaign stability, independent distant movement, wall/contact
plane stability and frame replay. The initial detail screenshot was visually
checked against the recovered workshop composition. After the gallery ceiling
limit was added, the workshop suite and full renderer audit passed again; the
upper-gallery screenshot was also visually checked for canvas edges and framing.
