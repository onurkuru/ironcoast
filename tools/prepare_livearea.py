"""Convert original cover/icon masters to Vita's indexed PNG packaging format.

Requires Pillow. This is deterministic resizing/encoding, not art generation.
Committed sce_sys files allow ordinary Vita builds without Python or Pillow.
"""
from pathlib import Path
from PIL import Image, ImageOps

ROOT = Path(__file__).resolve().parents[1]


def opaque(image):
    background = Image.new('RGBA', image.size, (9, 22, 33, 255))
    background.alpha_composite(image.convert('RGBA'))
    return background.convert('RGB')


def indexed(image, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    image.quantize(colors=256, method=Image.Quantize.MEDIANCUT).save(path, bits=8)


def main():
    icon = opaque(Image.open(ROOT / 'docs/art/icon-source.png'))
    cover = opaque(Image.open(ROOT / 'docs/art/cover-source.png'))
    bubble = Image.new('RGB', (128, 128), (9, 22, 33))
    bubble.paste(ImageOps.contain(icon, (100, 100), Image.Resampling.LANCZOS), (14, 14))
    indexed(bubble, ROOT / 'sce_sys/icon0.png')
    for name, size in [('pic0.png', (960, 544)),
                       ('livearea/contents/bg0.png', (840, 500)),
                       ('livearea/contents/startup.png', (280, 158))]:
        indexed(ImageOps.fit(cover, size, Image.Resampling.LANCZOS), ROOT / 'sce_sys' / name)
    ImageOps.contain(cover, (1280, 720), Image.Resampling.LANCZOS).save(ROOT / 'docs/art/cover.jpg', quality=90)
    print('Vita icon, loading image, LiveArea background and gate image generated')


if __name__ == '__main__':
    main()
