"""Regression checks for complete, reproducible player poses with safe gutters."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from prepare_sprite_atlases import prepare_mapped


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


if __name__ == "__main__":
    unittest.main()
