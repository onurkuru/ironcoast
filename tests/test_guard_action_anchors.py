"""Check runtime registration metadata against the retained raster, not a redraw."""
import json,statistics,unittest
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
class GuardActionAnchors(unittest.TestCase):
    def test_anchor_pixels(self):
        config=json.loads((ROOT/'data/presentation.json').read_text())
        atlas=Image.open(ROOT/'assets/workshop-guard-v2.png').convert('RGBA')
        for frame,root in enumerate(config['guardPoseRoots']):
            tile=atlas.crop((frame%8*160,frame//8*160,(frame%8+1)*160,(frame//8+1)*160))
            centers=[]
            # At belt height the rightmost connected solid span is the pelvis;
            # the low-ready barrel to its left must not pull the pivot sideways.
            for y in range(109,118):
                spans=[];start=None
                for x in range(160):
                    solid=tile.getpixel((x,y))[3]>180
                    if solid and start is None:start=x
                    if start is not None and (not solid or x==159):
                        if x-start>7:spans.append((start,x))
                        start=None
                if spans:centers.append(sum(spans[-1])/2)
            self.assertTrue(centers)
            self.assertLessEqual(abs(statistics.median(centers)-root['x']),.5)
            self.assertEqual(tile.getchannel('A').getbbox()[3],config['guardAction']['baseline'])
            if frame>=12:
                tip=config['guardFireAnchors'][frame-12]
                points=[(x,y) for y in range(68,87) for x in range(25,75)
                        if (lambda p:p[3]>160 and max(p[:3])<115 and abs(p[0]-p[1])<30)(tile.getpixel((x,y)))]
                left=min(x for x,y in points)
                tipY=statistics.median(y for x,y in points if x<=left+1)
                self.assertLessEqual(abs(left-tip['x']),1)
                self.assertLessEqual(abs(tipY-tip['y']),1)
if __name__=='__main__':unittest.main()
