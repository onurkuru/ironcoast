"""Register the eight authored titan poses; never normalize pose silhouettes."""
from pathlib import Path
import json
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]

def build(destination=None):
    rig = json.loads((ROOT/'data/presentation.json').read_text())['forgeTitan']
    source = Image.open(ROOT/'tools/sourceboards/forge-titan-v3-source.png').convert('RGBA')
    if source.size != (1774, 887):
        raise ValueError('Unexpected titan source dimensions')
    pixels = source.load()
    for y in range(source.height):
        for x in range(source.width):
            r,g,b,a = pixels[x,y]
            spill = g-max(r,b)
            if spill>12:
                a=round(a*(1-min(1,(spill-12)/64)))
                g=min(g,max(r,b))
            pixels[x,y]=(r,g,b,a if a>=64 else 0)
    cell=int(rig['cellSize'])
    atlas=Image.new('RGBA',(cell*4,cell*2))
    for frame in range(8):
        col,row=frame%4,frame//4
        bounds=(round(col*source.width/4),round(row*source.height/2),
                round((col+1)*source.width/4),round((row+1)*source.height/2))
        tile=source.crop(bounds)
        content=tile.getchannel('A').getbbox()
        if not content:raise ValueError('Empty titan pose')
        # One fixed physical scale across walking, hammer lift and wreck.
        scale=rig['sourceScale']
        ground=content[3]
        resized=tile.resize((round(tile.width*scale),round(tile.height*scale)),Image.Resampling.LANCZOS)
        resized.putalpha(resized.getchannel('A').point(lambda a:a if a>=64 else 0))
        # Lanczos can reintroduce a narrow green overshoot at keyed edges.
        # Remove that fringe after resampling, before atlas registration.
        rgba=resized.load()
        for py in range(resized.height):
            for px in range(resized.width):
                r,g,b,a=rgba[px,py]
                if g-max(r,b)>12:rgba[px,py]=(r,max(r,b),b,a)
        x=round((cell-tile.width*scale)/2)
        y=round(rig['baseline']-ground*scale)
        actual=resized.getchannel('A').getbbox()
        if x+actual[0]<2 or x+actual[2]>cell-2 or y+actual[1]<2 or abs(y+actual[3]-rig['baseline'])>2:
            raise ValueError(f'Titan pose {frame} clips: {actual}, offset {x,y}')
        atlas.alpha_composite(resized,(col*cell+x,row*cell+y))
    atlas.save(destination or ROOT/'assets/forge-titan-v3.png')
    return atlas

if __name__=='__main__':build()
