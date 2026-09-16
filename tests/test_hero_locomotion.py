"""Actual imported pose geometry: gutters, airborne feet and reproducibility."""
import sys,tempfile,unittest,json,statistics
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from import_hero_locomotion import build
class LocomotionTests(unittest.TestCase):
    def test_muzzle_anchors_match_art(self):
        tuning=json.loads((ROOT/'data/presentation.json').read_text())
        rig=tuning['heroLocomotion'];cell=int(rig['cellSize'])
        sheet=Image.open(ROOT/'assets/hero-locomotion-v3.png').convert('RGBA')
        for phase,muzzle in enumerate(tuning['heroRunMuzzles']):
            tile=sheet.crop((phase*cell,cell,(phase+1)*cell,2*cell))
            # The horizontally extended barrel is the far-right solid silhouette.
            points=[(x,y) for y in range(cell) for x in range(cell) if tile.getpixel((x,y))[3]>=200]
            tip=max(x for x,y in points)
            height=statistics.median(y for x,y in points if x>=tip-1)
            self.assertLess(abs(tip-(cell/2+muzzle['x']*rig['sourceScale'])),2)
            self.assertLess(abs(height-(rig['baseline']-muzzle['height']*rig['sourceScale'])),2)
    def test_pose_geometry(self):
        sheet=Image.open(ROOT/'assets/hero-locomotion-v3.png').convert('RGBA')
        self.assertEqual(sheet.size,(1280,320))
        for index in range(16):
            tile=sheet.crop((index%8*160,index//8*160,index%8*160+160,index//8*160+160))
            box=tile.getchannel('A').getbbox()
            self.assertGreaterEqual(box[0],2);self.assertGreaterEqual(box[1],2)
            self.assertLessEqual(box[2],158);self.assertLessEqual(box[3],158)
            if index%4==3:self.assertLess(box[3],153,'flight pose boots were grounded by packing')
            else:self.assertGreaterEqual(box[3],154,'contact pose lost its ground anchor')
            self.assertFalse(any(a>64 and g>r+40 and g>b+40 for r,g,b,a in tile.getdata()))
        # A planted compression pose must have a narrower foot spread than contact.
        for offset in [0,8]:
            widths=[]
            for index in [offset,offset+1]:
                band=sheet.crop((index%8*160,index//8*160+132,index%8*160+160,index//8*160+160))
                box=band.getchannel('A').getbbox();widths.append(box[2]-box[0])
            self.assertLess(widths[1],widths[0]*.8)
    def test_reproducible_import(self):
        with tempfile.TemporaryDirectory() as path:
            image=build(Path(path)/'locomotion.png')
            self.assertEqual(image.tobytes(),Image.open(ROOT/'assets/hero-locomotion-v3.png').tobytes())
if __name__=='__main__':unittest.main()
