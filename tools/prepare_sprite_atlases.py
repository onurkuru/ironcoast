#!/usr/bin/env python3
"""Normalize generated sprite sheets to stable atlas cells.

The artwork is authored as irregular pose boards. Every active actor is packed
from measured full-pose rectangles at a fixed source scale, with transparent
gutters and a common bottom anchor. Boss core coordinates are transformed with
the same map. Commercial game artwork is not downloaded or copied.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw


def key_backdrop(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    px = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            r, g, b, a = px[x, y]
            lo, hi = min(r, g, b), max(r, g, b)
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




def prepare_bosses(source_dir: Path, target_dir: Path, header_dir: Path | None = None) -> None:
    muzzles = []
    for boss in range(6):
        prepare_mapped(source_dir, target_dir, f"boss{boss}")
        manifest = json.loads((source_dir / f"boss{boss}-frames.json").read_text())
        left, top, right, bottom = manifest["rows"][1][3]
        mx, my = manifest["attack_muzzle"]
        width, height = round((right - left) * manifest["scale"]), round((bottom - top) * manifest["scale"])
        cell = manifest["cell_size"]
        draw = 142 if boss == 4 else 152
        x = (((cell - width) // 2 + (mx - left) * width / (right - left)) / cell - .5) * draw
        y = ((my - top) * height / (bottom - top) - height) / cell * draw + (7 if boss == 4 else 0)
        muzzles.append(f"  {{{x:.6f}f, {y:.6f}f}}")
    if header_dir:
        header_dir.mkdir(parents=True, exist_ok=True)
        (header_dir / "boss_muzzles.h").write_text(
            "// Generated from boss attack-pose maps by prepare_sprite_atlases.py.\n"
            "#pragma once\nnamespace kh {\ninline constexpr float BOSS_MUZZLES[6][2] = {\n" +
            ",\n".join(muzzles) + "\n};\n}\n")


def prepare_hero(source_dir: Path, target_dir: Path) -> None:
    prepare_mapped(source_dir, target_dir, "hero")


def prepare_auxiliary(source_dir: Path, target_dir: Path) -> None:
    """Directional poses need a common root, not the board's drifting columns."""
    prepare_mapped(source_dir, target_dir, "aim")


def prepare_mapped(source_dir: Path, target_dir: Path, actor: str,
                   source_path: Path | None = None) -> None:
    """Pack complete irregular source poses at a single scale and foot anchor.

    Edge cleanup of uniform cells cannot recover a head/foot already cropped
    away. Source rectangles are measured from the full board before packing.
    """
    manifest = json.loads((source_dir / f"{actor}-frames.json").read_text())
    source = Image.open(source_path or source_dir / f"{actor}-source.png").convert("RGBA")
    if not manifest.get("preserve_alpha", False):
        original = source
        source = key_backdrop(source)
        if "cores" in manifest:
            # White-hot centers are part of the painted machine, not the
            # neutral checkerboard. Protect the measured emissive windows
            # while keying the board; otherwise the brightest pixels become
            # transparent holes precisely at the light source.
            protected = Image.new("L", source.size)
            draw = ImageDraw.Draw(protected)
            for cx, cy in manifest["cores"][:12]:
                draw.ellipse((cx - 12, cy - 12, cx + 12, cy + 12), fill=255)
            source.paste(original, (0, 0), protected)
    if list(source.size) != manifest["source_size"]:
        raise ValueError(f"{actor} source dimensions changed; review the explicit pose map")
    size = manifest["cell_size"]
    cols = manifest.get("cols", 8)
    atlas = Image.new("RGBA", (size * cols, size * len(manifest["rows"])))
    anchors = []
    for row_index, poses in enumerate(manifest["rows"]):
        for col in range(cols):
            pose = source.crop(tuple(poses[min(col, len(poses) - 1)]))
            if manifest.get("clean_edges", False):
                pose = remove_boundary_bleed(pose, min_area=1)
            width = round(pose.width * manifest["scale"])
            height = round(pose.height * manifest["scale"])
            if width > size - 4 or height > manifest["baseline"] - 2:
                raise ValueError(f"{actor} pose {row_index}:{col} exceeds its padded cell")
            pose = pose.resize((width, height), Image.Resampling.LANCZOS)
            atlas.alpha_composite(pose, (col * size + (size - width) // 2,
                                        row_index * size + manifest["baseline"] - height))
            if "cores" in manifest:
                cx, cy = manifest["cores"][row_index * cols + col]
                left, top, right, bottom = poses[col]
                anchors.append((((size - width) // 2 + (cx - left) * width / (right - left)) / size,
                                (manifest["baseline"] - height + (cy - top) * height / (bottom - top)) / size))
    target_dir.mkdir(parents=True, exist_ok=True)
    atlas.save(target_dir / f"{actor}-v2.png", optimize=True)
    if anchors:
        (target_dir / f"{actor}-v2.anchors").write_text(
            "".join(f"{x:.7f} {y:.7f}\n" for x, y in anchors))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path, default=Path(__file__).resolve().parents[1] / "assets")
    parser.add_argument("--source-dir", type=Path,
                        default=Path(__file__).resolve().parent / "sourceboards")
    parser.add_argument("--headers", type=Path, default=Path(__file__).resolve().parents[1] / "src")
    args = parser.parse_args()
    assets = args.assets
    prepare_hero(args.source_dir, assets)
    prepare_mapped(args.source_dir, assets, "enemies")
    prepare_mapped(args.source_dir, assets, "vehicle")
    prepare_bosses(args.source_dir, assets, args.headers)
    prepare_auxiliary(args.source_dir, assets)
    prepare_mapped(args.source_dir, assets, "climb")


if __name__ == "__main__":
    main()
