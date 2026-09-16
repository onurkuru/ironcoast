"""Key and register generated guard poses without scaling collapsed bodies up."""
import json
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
def build(destination=None):
    spec=json.loads((ROOT/'tools/sourceboards/guard-reactions-v3.json').read_text())
    rig=json.loads((ROOT/'data/presentation.json').read_text())['guardReactions']
    source=Image.open(ROOT/'tools/sourceboards/guard-reactions-v3-source.png').convert('RGBA')
    if list(source.size)!=spec['sourceSize']:raise ValueError('Unexpected source grid')
    px=source.load()
    for y in range(source.height):
        for x in range(source.width):
            r,g,b,a=px[x,y];spill=g-max(r,b)
            if spill>12:a=round(a*(1-min(1,(spill-12)/64)));g=min(g,max(r,b))
            px[x,y]=(r,g,b,a if a>=40 else 0)
    cell=int(rig['cellSize']);scale=rig['sourceScale'];baseline=rig['baseline']
    atlas=Image.new('RGBA',(cell*4,cell*5))
    for i,anchor in enumerate(spec['poses']):
        col=i%4;row=i//4
        bounds=(round(col*source.width/4),round(row*source.height/5),
                round((col+1)*source.width/4),round((row+1)*source.height/5))
        crop=source.crop(bounds)
        crop=crop.resize((round(crop.width*scale),round(crop.height*scale)),Image.Resampling.LANCZOS)
        crop.putalpha(crop.getchannel('A').point(lambda a:a if a>=64 else 0))
        x=round(cell/2-(anchor['rootX']-bounds[0])*scale)
        y=round(baseline-(anchor['groundY']-bounds[1])*scale)
        # Validate BEFORE compositing: a fixed-size cell could silently clip art.
        content=crop.getchannel('A').getbbox()
        if not content:raise ValueError(f'Empty pose {i}')
        box=(x+content[0],y+content[1],x+content[2],y+content[3])
        if box[0]<2 or box[1]<2 or box[2]>cell-2 or abs(box[3]-baseline)>2:
            raise ValueError(f'Pose {i} exceeds its registered cell: {box}')
        tile=Image.new('RGBA',(cell,cell));tile.alpha_composite(crop,(x,y))
        atlas.alpha_composite(tile,(col*cell,row*cell))
    atlas.save(destination or ROOT/'assets/guard-reactions-v3.png')
    return atlas
if __name__=='__main__':build()
