"""Verify all Phase 1 frames and reproduce each atlas from its measured map."""
import unittest,sys,tempfile
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from import_workshop_atlases import build,SPECS
class WorkshopAtlasTests(unittest.TestCase):
 def test_complete_poses_gutters_and_common_root(self):
  for actor,(_,cols,cuts,_,_) in SPECS.items():
   im=Image.open(ROOT/'assets'/f'{actor}-v2.png').convert('RGBA')
   self.assertEqual(im.size,(160*cols,160*len(cuts)))
   for row in range(len(cuts)):
    for col in range(cols):
     with self.subTest(actor=actor,row=row,col=col):
      mask=im.crop((col*160,row*160,(col+1)*160,(row+1)*160)).getchannel('A')
      box=mask.getbbox();self.assertIsNotNone(box)
      self.assertGreaterEqual(box[0],2);self.assertGreaterEqual(box[1],2)
      self.assertLessEqual(box[2],158);self.assertEqual(box[3],156)
 def test_reproducible_measured_import(self):
  with tempfile.TemporaryDirectory() as folder:
   build(Path(folder))
   for actor in SPECS:
    self.assertEqual(Image.open(Path(folder)/f'{actor}-v2.png').tobytes(),Image.open(ROOT/'assets'/f'{actor}-v2.png').tobytes())
if __name__=='__main__':unittest.main()
