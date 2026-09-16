"""Guard against cell bleed and floating props in the production packs."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('props_import', ROOT/'tools/import_production_props.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class ProductionPackTests(unittest.TestCase):
    def test_registered_isolated_cells(self):
        for name in ('production-props-v1', 'production-structures-v1'):
            with self.subTest(pack=name):
                atlas = Image.open(ROOT/f'assets/{name}.png').convert('RGBA')
                self.assertEqual(atlas.size, (512, 256))
                poses = []
                for i in range(8):
                    x, y = (i%4)*128, (i//4)*128
                    tile = atlas.crop((x, y, x+128, y+128))
                    bounds = tile.getchannel('A').getbbox()
                    self.assertIsNotNone(bounds)
                    left, top, right, bottom = bounds
                    self.assertGreaterEqual(left, 4)
                    self.assertLessEqual(right, 124)
                    self.assertGreaterEqual(top, 6)
                    self.assertEqual(bottom, 126, 'Feet shifted from the common baseline')
                    poses.append(tile.tobytes())
                    pixels=tile.tobytes()
                    self.assertFalse(any(pixels[j+3] and pixels[j+1]-max(pixels[j],pixels[j+2])>12
                                         for j in range(0,len(pixels),4)),
                                     'Visible chroma-key spill')
                    if name=='production-structures-v1':
                        if i<6:
                            # The snowy bay includes long authored icicles (1.69:1).
                            self.assertGreater((right-left)/(bottom-top), 1.5,
                                               'A post leaked into a horizontal deck cell')
                        else:
                            # Foot plates widen the isolated posts to roughly 2.3:1.
                            self.assertGreater((bottom-top)/(right-left), 2,
                                               'A deck leaked into a support cell')
                self.assertEqual(len(set(poses)), 8)

    def test_source_regeneration(self):
        with tempfile.TemporaryDirectory() as folder:
            for name in ('production-props-v1', 'production-structures-v1'):
                target = Path(folder)/f'{name}.png'
                module.build(name, target)
                self.assertEqual(target.read_bytes(), (ROOT/f'assets/{name}.png').read_bytes())

if __name__=='__main__':
    unittest.main()
