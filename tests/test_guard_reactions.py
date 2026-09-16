import json,sys,tempfile,unittest
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from import_guard_reactions import build
class GuardReactionTests(unittest.TestCase):
    def test_registered_geometry(self):
        rig=json.loads((ROOT/'data/presentation.json').read_text())['guardReactions']
        cell=int(rig['cellSize']);sheet=Image.open(ROOT/'assets/guard-reactions-v3.png').convert('RGBA')
        self.assertEqual(sheet.size,(cell*4,cell*5));heights=[]
        for i in range(20):
            tile=sheet.crop((i%4*cell,i//4*cell,(i%4+1)*cell,(i//4+1)*cell))
            box=tile.getchannel('A').getbbox();self.assertIsNotNone(box)
            self.assertGreaterEqual(min(box[0],box[1]),2)
            self.assertLessEqual(box[2],cell-2)
            self.assertLessEqual(abs(box[3]-rig['baseline']),2)
            self.assertFalse(any(a>64 and g>r+40 and g>b+40 for r,g,b,a in tile.getdata()))
            heights.append(box[3]-box[1])
        self.assertLess(heights[19],heights[0]*.45,'Prone body was scaled up to standing height')
        self.assertLess(heights[10],heights[8]*.85,'Kneeling pose does not lower the body')
    def test_reproducible_import(self):
        with tempfile.TemporaryDirectory() as path:
            atlas=build(Path(path)/'guard.png')
            self.assertEqual(atlas.tobytes(),Image.open(ROOT/'assets/guard-reactions-v3.png').tobytes())
if __name__=='__main__':unittest.main()
