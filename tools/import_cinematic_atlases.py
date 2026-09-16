"""Rebuild cinematic atlases from explicit measured source rectangles.

The first import can measure alpha bounds inside reviewed row/column limits.
Checked-in manifests freeze those bounds; normal builds never guess mapping.
"""
import argparse,json
from pathlib import Path
from PIL import Image
from prepare_sprite_atlases import prepare_mapped,key_backdrop,remove_boundary_bleed
ROOT=Path(__file__).resolve().parents[1]
SOURCES=ROOT/'tools/sourceboards'
SPECS={'enemies':(8,6,96,92,.46,True),'aim':(4,3,112,108,.29,False),'climb':(4,3,96,92,.22,True),'effects':(8,4,128,124,.43,True)}
def measure(actor):
 cols,rows,cell,base,scale,alpha=SPECS[actor]
 src=Image.open(SOURCES/f'{actor}-cinematic-source.png').convert('RGBA')
 keyed=src if alpha else key_backdrop(src)
 keyed.putalpha(keyed.getchannel('A').point(lambda a:a if a>=72 else 0))
 bounds=[]
 rowcuts=[(0,264),(275,461),(470,679),(680,887)] if actor=='effects' else [(round(i*src.height/rows),round((i+1)*src.height/rows)) for i in range(rows)]
 for y0,y1 in rowcuts:
  row=[]
  for col in range(cols):
   x0,x1=round(col*src.width/cols),round((col+1)*src.width/cols)
   pose=remove_boundary_bleed(keyed.crop((x0,y0,x1,y1)),1)
   box=pose.getchannel('A').getbbox()
   if box is None:raise ValueError((actor,col,y0,'empty frame'))
   row.append([x0+box[0],y0+box[1],x0+box[2],y0+box[3]])
  bounds.append(row)
 manifest=dict(source_size=list(src.size),cell_size=cell,cols=cols,baseline=base,scale=scale,preserve_alpha=alpha,alpha_cutoff=72,clean_edges=True,rows=bounds)
 (SOURCES/f'{actor}-cinematic-frames.json').write_text(json.dumps(manifest,indent=2)+'\n')
def build(dest):
 for actor in ['hero','enemies','aim','climb','effects']:
  prepare_mapped(SOURCES,dest,actor+'-cinematic')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--measure',action='store_true');p.add_argument('--output',type=Path,default=ROOT/'assets');args=p.parse_args()
 if args.measure:
  for actor in SPECS:measure(actor)
 build(args.output)
