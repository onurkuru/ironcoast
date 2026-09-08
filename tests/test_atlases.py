"""Regression checks for complete, reproducible player poses with safe gutters."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from prepare_sprite_atlases import prepare_auxiliary, prepare_bosses, prepare_mapped


class HeroAtlasTests(unittest.TestCase):
    def test_all_poses_have_transparent_gutters(self):
        for actor, rows in [("hero", 8), ("enemies", 6)]:
            self.check_gutters(actor, rows)

    def check_gutters(self, actor, rows):
        atlas = Image.open(ROOT / f"assets/{actor}-v2.png").convert("RGBA")
        self.assertEqual(atlas.size, (768, rows * 96))
        for frame in range(rows * 8):
            with self.subTest(actor=actor, frame=frame):
                col, row = frame % 8, frame // 8
                mask = atlas.crop((col * 96, row * 96, (col + 1) * 96, (row + 1) * 96)).getchannel("A")
                box = mask.getbbox()
                self.assertIsNotNone(box)
                self.assertGreaterEqual(box[0], 2)
                self.assertGreaterEqual(box[1], 2)
                self.assertLessEqual(box[2], 94)
                self.assertEqual(box[3], 92, "Each packed pose must share the foot anchor")

    def test_original_full_pose_size_is_preserved(self):
        data = json.loads((ROOT / "tools/sourceboards/hero-frames.json").read_text())
        # A uniform 8x8 crop split these full source silhouettes. These bounds
        # retain both feet and the gun instead of trimming the overflow away.
        self.assertEqual([len(row) for row in data["rows"]], [7, 8, 8, 8, 8, 8, 7, 7])
        for left, top, right, bottom in data["rows"][0]:
            self.assertGreater(right - left, 120)
            self.assertGreater(bottom - top, 135)

    def test_checked_in_atlas_matches_source_map(self):
        with tempfile.TemporaryDirectory() as directory:
            for actor in ["hero", "enemies"]:
                prepare_mapped(ROOT / "tools/sourceboards", Path(directory), actor)
                with Image.open(Path(directory) / f"{actor}-v2.png") as regenerated:
                    with Image.open(ROOT / f"assets/{actor}-v2.png") as checked_in:
                        self.assertEqual(regenerated.tobytes(), checked_in.tobytes())

    def test_vehicle_and_boss_cells_have_no_cross_pose_alpha(self):
        # Source poses cross the old uniform 4x4 boundaries. All are now
        # imported from full measured rectangles with a shared bottom anchor.
        for actor in ["vehicle"] + [f"boss{i}" for i in range(6)]:
            atlas = Image.open(ROOT / f"assets/{actor}-v2.png").convert("RGBA")
            self.assertEqual(atlas.size, (768, 768))
            for frame in range(16):
                col, row = frame % 4, frame // 4
                mask = atlas.crop((col * 192, row * 192, (col + 1) * 192,
                                   (row + 1) * 192)).getchannel("A")
                box = mask.getbbox()
                with self.subTest(actor=actor, frame=frame):
                    self.assertIsNotNone(box)
                    self.assertGreaterEqual(box[0], 5)
                    self.assertGreaterEqual(box[1], 5)
                    self.assertLessEqual(box[2], 187)
                    self.assertLessEqual(box[3], 187)

    def test_directional_cells_have_no_cross_pose_alpha(self):
        for actor, cols, rows in [("aim", 4, 3)]:
            atlas = Image.open(ROOT / f"assets/{actor}-v2.png").convert("RGBA")
            self.assertEqual(atlas.size, (448, 336))
            for frame in range(cols * rows):
                col, row = frame % cols, frame // cols
                mask = atlas.crop((col * 112, row * 112, (col + 1) * 112,
                                   (row + 1) * 112)).getchannel("A")
                box = mask.getbbox()
                with self.subTest(actor=actor, frame=frame):
                    self.assertIsNotNone(box)
                    self.assertGreaterEqual(box[0], 5)
                    self.assertGreaterEqual(box[1], 5)
                    self.assertLessEqual(box[2], 107)
                    self.assertLessEqual(box[3], 106)
                if row == 0:
                    self.assertLess(abs((box[0] + box[2]) / 2 - 56), 1.1,
                                    "Aiming must not drift across the player root")

    def test_checked_in_vehicle_and_boss_atlases_are_reproducible(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            prepare_mapped(ROOT / "tools/sourceboards", target, "vehicle")
            prepare_bosses(ROOT / "tools/sourceboards", target, target)
            self.assertEqual((target / "boss_muzzles.h").read_bytes(), (ROOT / "src/boss_muzzles.h").read_bytes())
            for actor in ["vehicle"] + [f"boss{i}" for i in range(6)]:
                with Image.open(target / f"{actor}-v2.png") as regenerated:
                    with Image.open(ROOT / f"assets/{actor}-v2.png") as checked_in:
                        self.assertEqual(regenerated.tobytes(), checked_in.tobytes())
                if actor.startswith("boss"):
                    self.assertEqual((target / f"{actor}-v2.anchors").read_bytes(),
                                     (ROOT / f"assets/{actor}-v2.anchors").read_bytes())
            prepare_auxiliary(ROOT / "tools/sourceboards", target)
            for actor in ["aim"]:
                with Image.open(target / f"{actor}-v2.png") as regenerated:
                    with Image.open(ROOT / f"assets/{actor}-v2.png") as checked_in:
                        self.assertEqual(regenerated.tobytes(), checked_in.tobytes())

    def test_full_boss_poses_and_core_anchors(self):
        # These particular feet used to be cut at y=313 and y=627.
        manifest = json.loads((ROOT / "tools/sourceboards/boss5-frames.json").read_text())
        self.assertTrue(all(box[3] >= 330 for box in manifest["rows"][0]))
        self.assertTrue(all(box[3] >= 646 for box in manifest["rows"][1]))
        for actor in [f"boss{i}" for i in range(6)]:
            anchors = (ROOT / f"assets/{actor}-v2.anchors").read_text().splitlines()
            self.assertEqual(len(anchors), 16)
            atlas = Image.open(ROOT / f"assets/{actor}-v2.png").convert("RGBA")
            for frame, line in enumerate(anchors[:12]):
                x, y = map(float, line.split())
                px, py = frame % 4 * 192 + round(x * 192), frame // 4 * 192 + round(y * 192)
                self.assertGreater(atlas.getpixel((px, py))[3], 128,
                                   f"{actor}:{frame} core light must sit on the painted body")


if __name__ == "__main__":
    unittest.main()
