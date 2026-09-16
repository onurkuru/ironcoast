"""Import the Phase 1 adult rigs with explicit reviewed row boundaries.

This is atlas packing and background keying, not procedural character drawing.
Actor scale is fixed for each complete source sheet; never normalize each pose.
"""
from pathlib import Path
import argparse,json
from PIL import Image
from prepare_sprite_atlases import prepare_mapped,key_backdrop,remove_boundary_bleed
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'tools/sourceboards'
SPECS={
 'workshop-hero':('workshop-hero-source.png',8,[(0,160),(161,326),(327,502),(503,640),(641,797),(798,956),(957,1085),(1086,1254)],.70,False),
 'workshop-guard':('workshop-guard-source.png',8,[(0,260),(261,510),(511,769)],.46,True),
 'workshop-aim':('workshop-aux-keyed-source.png',4,[(0,274),(275,526),(527,737)],.40,True),
 'workshop-climb':('workshop-aux-keyed-source.png',4,[(738,996),(997,1252),(1253,1536)],.40,True),
}
def source_image(filename,preserve):
 im=Image.open(SRC/filename).convert('RGBA')
 if not preserve:im=key_backdrop(im)
 if 'keyed' in filename:
  px=im.load()
  for y in range(im.height):
   for x in range(im.width):
    r,g,b,a=px[x,y]
    if g>90 and g>r*1.45 and g>b*1.45:px[x,y]=(0,0,0,0)
 im.putalpha(im.getchannel("A").point(lambda a:a if a>=72 else 0))
 return im

def measure(actor):
 filename,cols,cuts,scale,preserve=SPECS[actor];im=source_image(filename,preserve);rows=[]
 for row,(y0,y1) in enumerate(cuts):
  poses=[]
  for col in range(cols):
   x0,x1=round(col*im.width/cols),round((col+1)*im.width/cols)
   if actor=='workshop-hero' and row==6 and col>=5:x0,x1=791,990
   if actor=='workshop-hero' and row==4 and col==3:x1=626
   pose=remove_boundary_bleed(im.crop((x0,y0,x1,y1)),1)
   box=pose.getchannel('A').getbbox()
   if box is None:raise ValueError((actor,row,col,'empty'))
   poses.append([x0+box[0],y0+box[1],x0+box[2],y0+box[3]])
  rows.append(poses)
 # Entire lying pose is wider than an upright pose: reserve a larger cell
 # while retaining the same fixed scale and actor height.
 manifest=dict(source_size=list(im.size),cell_size=160,cols=cols,baseline=156,scale=scale,preserve_alpha=True,clean_edges=True,reanchor_cleaned=True,rows=rows)
 (SRC/(actor+'-frames.json')).write_text(json.dumps(manifest,indent=2)+'\n')

def build(dest):
 import tempfile
 for actor,(filename,cols,cuts,scale,preserve) in SPECS.items():
  with tempfile.TemporaryDirectory() as temp:
   clean=Path(temp)/'keyed.png';source_image(filename,preserve).save(clean)
   prepare_mapped(SRC,dest,actor,clean)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--measure',action='store_true');p.add_argument('--output',type=Path,default=ROOT/'assets');a=p.parse_args()
 if a.measure:
  for actor in SPECS:measure(actor)
 build(a.output)
