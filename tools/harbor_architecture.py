"""Register Harbor gameplay to the integrated painting, never to overlay tiles."""
import argparse
import copy
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def apply_harbor(level):
    art=json.loads((ROOT/'data/presentation.json').read_text())
    rig=art['harborBuiltScene'];width=level['width'];height=width/rig['sourceAspect']
    world_y=lambda v:232+(v-rig['sourceFloor'])*height
    level['platforms']=[{'box':[0,232,width,40],'oneWay':False,'material':0}]
    level['buildings']=[];level['ladders']=[];level['entrances']=[]
    level['spawns']=[];level['items']=[]
    for index,gallery in enumerate(art['harborGalleries']):
        left,right=gallery['u0']*width,gallery['u1']*width;y=world_y(gallery['v'])
        level['platforms'].append({'box':[left,y,right-left,8],'oneWay':True,'material':1})
        ladders=[l['u']*width for l in art['harborLadders'] if l['gallery']==index]
        # Wall extents represent the actual bearing bay, inside the deck lip.
        level['buildings'].append({'box':[ladders[0]-26,y,ladders[-1]-ladders[0]+52,232-y],'style':0})
        level['ladders'] += [{'x':x,'top':y,'bottom':232} for x in ladders]
        level['spawns'].append({'x':(ladders[0]+ladders[-1])/2,'y':y,'kind':[0,1,0][index]})
        level['items'] += [{'x':ladders[-1]-50,'y':y-17,'kind':0},
                           {'x':ladders[0]+55,'y':y-17,'kind':[1,2,6][index]}]
    for index,door in enumerate(art['harborDoors']):
        x=(door['u']+door['w']/2)*width
        level['entrances'].append({'x':x,'y':232,'triggerX':x-130})
        for order,kind in enumerate([0,0] if index==0 else [0,1]):
            level['spawns'].append({'x':x,'y':232,'kind':kind,'entrance':index,'delay':order*1.15})
    level['spawns'].append({'x':650,'y':232,'kind':0})
    level['items'] += [{'x':1000,'y':215,'kind':6},{'x':width-560,'y':215,'kind':4},
                       {'x':width-620,'y':215,'kind':9}]
    level['checkpoints']=[525,1090]
    level['puddles']=[[170,232,100,7],[715,232,115,7],[1550,232,120,7]]
    level['buildings'].append({'box':[width-480,-72,480,304],'style':8})
    level['minY']=min(p['box'][1] for p in level['platforms'])-130
    from arcade_encounters import apply_arcade_encounters
    apply_arcade_encounters(level)

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Check registration without writing files')
    args=parser.parse_args()
    path=ROOT/'data/campaign.json';data=json.loads(path.read_text())
    previous=copy.deepcopy(data['levels'][0])
    apply_harbor(data['levels'][0])
    if args.check:
        changed=[key for key,value in data['levels'][0].items() if previous.get(key)!=value]
        if changed:
            raise SystemExit('Harbor geometry differs from its painting: '+', '.join(changed)+
                             '. Run tools/harbor_architecture.py and tools/compile_campaign.py.')
        print('Harbor galleries, ladders, doors and encounters match the painting anchors')
    else:
        path.write_text(json.dumps(data,indent=2)+'\n')
