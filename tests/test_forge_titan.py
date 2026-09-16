import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('titan_import',ROOT/'tools/import_forge_titan.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class TitanAtlasTests(unittest.TestCase):
    def test_cells_feet_and_wreck_scale(self):
        rig=json.loads((ROOT/'data/presentation.json').read_text())['forgeTitan']
        cell=int(rig['cellSize'])
        image=Image.open(ROOT/'assets/forge-titan-v3.png').convert('RGBA')
        self.assertEqual(image.size,(cell*4,cell*2))
        heights=[];poses=[]
        for i in range(8):
            x,y=(i%4)*cell,(i//4)*cell
            pose=image.crop((x,y,x+cell,y+cell));poses.append(pose.tobytes())
            left,top,right,bottom=pose.getchannel('A').getbbox()
            self.assertGreaterEqual(left,2);self.assertLessEqual(right,cell-2)
            self.assertGreaterEqual(top,2);self.assertLessEqual(abs(bottom-rig['baseline']),2)
            heights.append(bottom-top)
            pixels=pose.tobytes()
            self.assertFalse(any(pixels[j+3]>0 and pixels[j+1]-max(pixels[j],pixels[j+2])>16 for j in range(0,len(pixels),4)))
        self.assertEqual(len(set(poses)),8,'A pose was duplicated during import')
        self.assertLess(heights[-1],heights[0]*.55,'Wreck was enlarged to fill its cell')

    def test_reproducible_from_source(self):
        with tempfile.TemporaryDirectory() as folder:
            target=Path(folder)/'titan.png';module.build(target)
            # PNG compression varies with zlib/Pillow; the decoded RGBA pixels
            # and cell dimensions must reproduce exactly on every platform.
            with Image.open(target) as generated, Image.open(ROOT/'assets/forge-titan-v3.png') as checked_in:
                self.assertEqual(generated.size, checked_in.size)
                self.assertEqual(generated.convert('RGBA').tobytes(), checked_in.convert('RGBA').tobytes())

if __name__=='__main__':unittest.main()
