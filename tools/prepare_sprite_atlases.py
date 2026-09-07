#!/usr/bin/env python3
"""Normalize generated sprite sheets to stable atlas cells.

The artwork is authored as pose boards. This tool only normalizes dimensions,
keys the neutral checkerboard/paper or black backdrop, and assembles the six
boss boards into one 4x24 atlas. It never downloads or copies commercial game
artwork.
"""
from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


def key_backdrop(image: Image.Image, black: bool = False) -> Image.Image:
    rgba = image.convert("RGBA")
    px = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            r, g, b, a = px[x, y]
            if black:
                remove = max(r, g, b) < 28
            else:
                lo, hi = min(r, g, b), max(r, g, b)
                # ImageGen pose boards use a neutral grey/white checkerboard.
                remove = lo > 168 and hi - lo < 24
            if remove:
                px[x, y] = (r, g, b, 0)
    return rgba


def normalize(source: Path, target: Path, size: tuple[int, int], cols: int, rows: int,
              black: bool = False) -> None:
    source_image = Image.open(source).convert("RGBA")
    # Resize each authored cell independently. Resizing the complete board lets
    # Lanczos sample a neighboring pose, which produces stray limbs around a
    # character at runtime.
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    cell_w, cell_h = size[0] // cols, size[1] // rows
    for row in range(rows):
        for col in range(cols):
            sx0 = col * source_image.width // cols
            sx1 = (col + 1) * source_image.width // cols
            sy0 = row * source_image.height // rows
            sy1 = (row + 1) * source_image.height // rows
            cell = source_image.crop((sx0, sy0, sx1, sy1))
            cell = key_backdrop(cell, black=black)
            cell = cell.resize((cell_w, cell_h), Image.Resampling.LANCZOS)
            cell = key_backdrop(cell, black=black)
            image.alpha_composite(cell, (col * cell_w, row * cell_h))
    target.parent.mkdir(parents=True, exist_ok=True)
    image.save(target, "PNG", optimize=True)


def prepare_bosses(source_dir: Path, target_dir: Path) -> None:
    board_size = (768, 768)  # 4x4 cells, 192px per authored pose
    for boss in range(6):
        source = source_dir / f"boss{boss}-source.png"
        target = target_dir / f"boss{boss}-v2.png"
        normalize(source, target, board_size, 4, 4)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path, default=Path(__file__).resolve().parents[1] / "assets")
    parser.add_argument("--source-dir", type=Path,
                        default=Path(__file__).resolve().parent / "sourceboards")
    args = parser.parse_args()
    assets = args.assets
    normalize(args.source_dir / "hero-source.png", assets / "hero-v2.png", (768, 768), 8, 8)
    normalize(args.source_dir / "enemies-source.png", assets / "enemies-v2.png", (768, 576), 8, 6)
    normalize(args.source_dir / "vehicle-source.png", assets / "vehicle-v2.png", (768, 768), 4, 4,
              black=True)
    prepare_bosses(args.source_dir, assets)


if __name__ == "__main__":
    main()
