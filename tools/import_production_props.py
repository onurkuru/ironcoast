"""Pack isolated prop cells with transparent gutters and registered feet."""
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]

def build(name='production-props-v1',destination=None):
    image=Image.open(ROOT/f'tools/sourceboards/{name}-source.png').convert('RGBA')
    pixels=image.load()
    for y in range(image.height):
        for x in range(image.width):
            r,g,b,a=pixels[x,y];spill=g-max(r,b)
            if spill>12:
                a=round(a*(1-min(1,(spill-12)/64)));g=max(r,b)
            pixels[x,y]=(r,g,b,a if a>=64 else 0)
    cell=128;atlas=Image.new('RGBA',(512,256))
    for i in range(8):
        col,row=i%4,i//4
        # Tall posts cross the mathematical midpoint of the structural sheet.
        # Use its measured row boundary to exclude them from deck cells.
        split=350 if name=='production-structures-v1' else round(image.height/2)
        y0,y1=(0,split) if row==0 else (split,image.height)
        tile=image.crop((round(col*image.width/4),y0,round((col+1)*image.width/4),y1))
        box=tile.getchannel('A').getbbox()
        if not box:raise ValueError('Empty prop cell')
        tile=tile.crop(box);factor=120/max(tile.size)
        tile=tile.resize((round(tile.width*factor),round(tile.height*factor)),Image.Resampling.LANCZOS)
        p=tile.load()
        for y in range(tile.height):
            for x in range(tile.width):
                r,g,b,a=p[x,y];p[x,y]=(r,min(g,max(r,b)+12),b,a if a>=64 else 0)
        atlas.alpha_composite(tile,(col*cell+(cell-tile.width)//2,row*cell+126-tile.height))
    atlas.save(destination or ROOT/f'assets/{name}.png')
    return atlas
if __name__=='__main__':build()
