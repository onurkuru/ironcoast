#!/usr/bin/env python3
"""Normalize generated sprite sheets to stable atlas cells.

The artwork is authored as pose boards. This tool only normalizes dimensions,
keys the neutral checkerboard/paper or black backdrop, keeps each resize inside
its source cell, and fills known seven-pose rows with a held final pose. It never
downloads or copies commercial game artwork.
"""
from __future__ import annotations

import argparse
import json
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


def remove_boundary_bleed(image: Image.Image, min_area: int = 24) -> Image.Image:
    """Drop disconnected pixels that leaked in from a neighboring pose cell.

    The authored pose boards occasionally let a previous character cross a
    cell boundary.  Cropping the board alone preserves those pixels, so the
    next animation frame can appear as a second character trailing the hero.
    Keep the largest connected silhouette and any interior effects, but remove
    smaller components that touch a cell edge.  This is deliberately limited to
    disconnected edge components: muzzle flashes, grenades and debris inside
    the cell remain available to the renderer.
    """
    rgba = image.convert("RGBA")
    width, height = rgba.size
    px = rgba.load()
    visited: set[tuple[int, int]] = set()
    components: list[tuple[int, tuple[int, int, int, int], list[tuple[int, int]]]] = []
    for y in range(height):
        for x in range(width):
            if (x, y) in visited or px[x, y][3] < 72:
                continue
            stack = [(x, y)]
            visited.add((x, y))
            points: list[tuple[int, int]] = []
            left = right = x
            top = bottom = y
            while stack:
                xx, yy = stack.pop()
                points.append((xx, yy))
                left, right = min(left, xx), max(right, xx)
                top, bottom = min(top, yy), max(bottom, yy)
                for nx in range(xx - 1, xx + 2):
                    for ny in range(yy - 1, yy + 2):
                        if not (0 <= nx < width and 0 <= ny < height):
                            continue
                        if (nx, ny) in visited or px[nx, ny][3] < 72:
                            continue
                        visited.add((nx, ny))
                        stack.append((nx, ny))
            if len(points) >= min_area:
                components.append((len(points), (left, top, right, bottom), points))
    if len(components) < 2:
        return rgba
    components.sort(key=lambda item: item[0], reverse=True)
    for area, (left, top, right, bottom), points in components[1:]:
        touches_edge = left == 0 or top == 0 or right == width - 1 or bottom == height - 1
        if touches_edge and area < components[0][0]:
            for xx, yy in points:
                r, g, b, _ = px[xx, yy]
                px[xx, yy] = (r, g, b, 0)
    return rgba


def normalize(source: Path, target: Path, size: tuple[int, int], cols: int, rows: int,
              black: bool = False, duplicate_rows: tuple[int, ...] = (), pad: int = 0) -> None:
    source_image = Image.open(source).convert("RGBA")
    # Resize each authored cell independently. Resizing the complete board lets
    # Lanczos sample a neighboring pose, which produces stray limbs around a
    # character at runtime.
    image = Image.new("RGBA", size, (0, 0, 0, 0))
    cell_w, cell_h = size[0] // cols, size[1] // rows
    if pad * 2 >= cell_w or pad * 2 >= cell_h:
        raise ValueError("atlas padding leaves no room for a pose")
    for row in range(rows):
        for col in range(cols):
            sx0 = col * source_image.width // cols
            sx1 = (col + 1) * source_image.width // cols
            sy0 = row * source_image.height // rows
            sy1 = (row + 1) * source_image.height // rows
            cell = source_image.crop((sx0, sy0, sx1, sy1))
            cell = key_backdrop(cell, black=black)
            cell = remove_boundary_bleed(cell)
            # Keep a transparent gutter around every authored pose. Some
            # boards place a claw, wheel or muzzle on a source-cell edge;
            # this inset makes cross-cell filtering impossible while keeping
            # the complete pose in the destination cell.
            inner = (cell_w - pad * 2, cell_h - pad * 2)
            cell = cell.resize(inner, Image.Resampling.LANCZOS)
            cell = key_backdrop(cell, black=black)
            cell = remove_boundary_bleed(cell)
            image.alpha_composite(cell, (col * cell_w + pad, row * cell_h + pad))
    for row in duplicate_rows:
        src = image.crop((6 * cell_w, row * cell_h, 7 * cell_w, (row + 1) * cell_h))
        image.paste((0, 0, 0, 0), (7 * cell_w, row * cell_h, 8 * cell_w, (row + 1) * cell_h))
        image.alpha_composite(src, (7 * cell_w, row * cell_h))
    target.parent.mkdir(parents=True, exist_ok=True)
    image.save(target, "PNG", optimize=True)


def prepare_bosses(source_dir: Path, target_dir: Path) -> None:
    board_size = (768, 768)  # 4x4 cells, 192px per authored pose
    for boss in range(6):
        source = source_dir / f"boss{boss}-source.png"
        target = target_dir / f"boss{boss}-v2.png"
        normalize(source, target, board_size, 4, 4, pad=5)


def prepare_hero(source_dir: Path, target_dir: Path) -> None:
    prepare_mapped(source_dir, target_dir, "hero")


def prepare_mapped(source_dir: Path, target_dir: Path, actor: str) -> None:
    """Pack complete irregular source poses at a single scale and foot anchor.

    Edge cleanup of uniform cells cannot recover a head/foot already cropped
    away. Source rectangles are measured from the full board before packing.
    """
    manifest = json.loads((source_dir / f"{actor}-frames.json").read_text())
    source = key_backdrop(Image.open(source_dir / f"{actor}-source.png"))
    if list(source.size) != manifest["source_size"]:
        raise ValueError(f"{actor} source dimensions changed; review the explicit pose map")
    size = manifest["cell_size"]
    atlas = Image.new("RGBA", (size * 8, size * len(manifest["rows"])))
    for row_index, poses in enumerate(manifest["rows"]):
        for col in range(8):
            pose = source.crop(tuple(poses[min(col, len(poses) - 1)]))
            width = round(pose.width * manifest["scale"])
            height = round(pose.height * manifest["scale"])
            if width > size - 4 or height > manifest["baseline"] - 2:
                raise ValueError(f"{actor} pose {row_index}:{col} exceeds its padded cell")
            pose = pose.resize((width, height), Image.Resampling.LANCZOS)
            atlas.alpha_composite(pose, (col * size + (size - width) // 2,
                                        row_index * size + manifest["baseline"] - height))
    target_dir.mkdir(parents=True, exist_ok=True)
    atlas.save(target_dir / f"{actor}-v2.png", optimize=True)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path, default=Path(__file__).resolve().parents[1] / "assets")
    parser.add_argument("--source-dir", type=Path,
                        default=Path(__file__).resolve().parent / "sourceboards")
    args = parser.parse_args()
    assets = args.assets
    prepare_hero(args.source_dir, assets)
    prepare_mapped(args.source_dir, assets, "enemies")
    normalize(args.source_dir / "vehicle-source.png", assets / "vehicle-v2.png", (768, 768), 4, 4,
              black=True, pad=5)
    prepare_bosses(args.source_dir, assets)


if __name__ == "__main__":
    main()
