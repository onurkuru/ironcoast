"""Pack generated poses around reviewed pelvis/ground anchors, not their bounds.

This only keys and packs existing artwork; it does not draw/interpolate poses.
"""
import json
from pathlib import Path
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
def build(destination=None):
    spec=json.loads((ROOT/'tools/sourceboards/hero-locomotion-v3.json').read_text())
    tuning=json.loads((ROOT/'data/presentation.json').read_text())['heroLocomotion']
    source=Image.open(ROOT/'tools/sourceboards/hero-locomotion-v3-source.png').convert('RGBA')
    if list(source.size)!=spec['sourceSize']: raise ValueError('Source grid changed')
    pixels=source.load()
    for y in range(source.height):
        for x in range(source.width):
            r,g,b,a=pixels[x,y];spill=g-max(r,b)
            if spill>12:
                a=round(a*(1-min(1,(spill-12)/64)));g=min(g,max(r,b))
            pixels[x,y]=(r,g,b,a if a>=40 else 0)
    cell=int(tuning['cellSize']);baseline=int(tuning['baseline']);scale=tuning['sourceScale']
    result=Image.new('RGBA',(cell*8,cell*2))
    for index,pose in enumerate(spec['poses']):
        col=index%4;row=index//4
        box=(round(col*source.width/4),round(row*source.height/4),
             round((col+1)*source.width/4),round((row+1)*source.height/4))
        crop=source.crop(box)
        crop=crop.resize((round(crop.width*scale),round(crop.height*scale)),Image.Resampling.LANCZOS)
        crop.putalpha(crop.getchannel('A').point(lambda a:a if a>=64 else 0))
        x=round(cell/2-(pose['rootX']-box[0])*scale)
        y=round(baseline-(pose['groundY']-box[1])*scale)
        tile=Image.new('RGBA',(cell,cell));tile.alpha_composite(crop,(x,y))
        bounds=tile.getchannel('A').getbbox()
        if not bounds or min(bounds[0],bounds[1])<2 or bounds[2]>cell-2 or bounds[3]>baseline+2:
            raise ValueError(f'Frame {index} spills beyond its reviewed cell: {bounds}')
        result.alpha_composite(tile,((index%8)*cell,(index//8)*cell))
    output=Path(destination) if destination else ROOT/'assets/hero-locomotion-v3.png'
    result.save(output)
    return result
if __name__=='__main__': build()
