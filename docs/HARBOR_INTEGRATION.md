# Harbor integrated architecture review

The Harbor route uses the edges of `assets/harbor-integrated-v3.png` as its
playable surfaces. Three galleries, six ladders and two reinforcement doors are
registered in `data/presentation.json`. Collision platforms are derived from
those coordinates instead of drawing a second platform kit over the painting.
The original painting contains the wall brackets, gallery fascias, rails, lamps
and door frames. Only each measured door panel animates over its opening.

## Continuous traversal

`--harbor-review` is a reproducible host review of all three galleries. After its
initial spawn at the first ladder, it supplies normal movement, fire and climb
inputs. It never teleports the actor or removes enemies. Training invulnerability
is enabled for this review; it is not a difficulty or balance benchmark.

The route climbs the left ladder, traverses the gallery while firing, rescues its
worker, descends the other ladder and walks to the next gallery. It finishes on
the quay after **1,928 simulation frames (32.13 seconds)**. All six painted ladders,
three workers and both finite reinforcement encounters are exercised. The
regression test also rejects abrupt camera changes during the uninterrupted run.

The motion review exposed a boarding hint that appeared while the player was
climbing above the vehicle. The hint and interaction now share the same height,
distance and ladder eligibility check; it is shown only when boarding is possible.

```sh
cmake -S . -B build-qa -DKH_VITA=OFF
cmake --build build-qa -j4
ctest --test-dir build-qa --output-on-failure
build-qa/kiyi_hurdasi --assets assets --harbor-review
```

For a 34-second offline capture, create `build-qa/harbor-frames` first, then run:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build-qa/kiyi_hurdasi \
  --assets assets --harbor-review --fast --frames 2040 \
  --record build-qa/harbor-frames --record-every 2 \
  --record-audio build-qa/harbor-audio.s16le
```

The PNG sequence is 30 fps; audio is mono signed 16-bit little-endian PCM at
32 kHz. Offline capture speed does not measure real-time Vita performance.
To play the Harbor with normal controls, use `--stage 1` instead of
`--harbor-review`. The ordinary campaign still starts with the workshop sequence.

## Keeping the painting and game in sync

After adjusting the measured anchors in `data/presentation.json`, run:

```sh
python3 tools/harbor_architecture.py
python3 tools/compile_campaign.py
python3 tools/compile_presentation.py
```

CTest now checks both links in that pipeline: authored Harbor geometry must
match its painting anchors, and compiled campaign C++ must match the JSON. Both
checks are read-only. A later edit can no longer silently leave the renderer and
the collision surfaces using different versions of the geometry.

The Titan and production-prop regeneration tests compare decoded RGBA pixels
and dimensions exactly. PNG compression bytes may vary across platforms while
the art stays identical; differences in actual color or alpha still fail.

## Scope of this review

The development Mac passed all 19 CTest checks. The source also cross-compiles
to Vita, and the package validator accepts its SELF, metadata and 75 resources.
This does not establish hardware frame rate, memory headroom, or controller feel.

The visual review covers the Harbor's gallery contacts, ladder alignment, camera
ascent/descent, worker placement and integrated door openings. The scene remains
a painted 2D environment with limited perspective and weather effects. Other
maps still need their own integrated architecture pass; this is not approval of
the entire campaign's art or a new published release.
