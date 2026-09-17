"""Register the pump-station route to the integrated Marsh painting."""
import argparse
import copy
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def apply_marsh(level):
    art = json.loads((ROOT/'data/presentation.json').read_text())
    rig = art['marshBuiltScene']
    level['width'] = width = 1920
    level['vehicleX'] = width-700
    height = width/rig['sourceAspect']
    world_y = lambda v: 232+(v-rig['sourceFloor'])*height
    level['platforms'] = [{'box': [0,232,width,40], 'oneWay': False, 'material': 1}]
    for key in ('buildings','ladders','entrances','spawns','items'):
        level[key] = []
    for index, gallery in enumerate(art['marshGalleries']):
        left, right = gallery['u0']*width, gallery['u1']*width
        y = world_y(gallery['v'])
        ladders = [ladder['u']*width for ladder in art['marshLadders'] if ladder['gallery']==index]
        level['platforms'].append({'box': [left,y,right-left,8], 'oneWay': True, 'material': 1})
        level['buildings'].append({'box': [ladders[0]-26,y,ladders[1]-ladders[0]+52,232-y], 'style': 1})
        level['ladders'] += [{'x': x, 'top': y, 'bottom': 232} for x in ladders]
        level['spawns'].append({'x': (ladders[0]+ladders[1])/2, 'y': y, 'kind': 1})
        level['items'] += [{'x': ladders[1]-48, 'y': y-17, 'kind': 0},
                           {'x': ladders[0]+55, 'y': y-17, 'kind': [2,6,9][index]}]
    for index, door in enumerate(art['marshDoors']):
        x = (door['u']+door['w']/2)*width
        level['entrances'].append({'x': x, 'y': 232, 'triggerX': x-130})
        for order, kind in enumerate(([1,0],[1,2])[index]):
            level['spawns'].append({'x': x, 'y': 232, 'kind': kind,
                                    'entrance': index, 'delay': order*1.15})
    level['spawns'].append({'x': 980, 'y': 122, 'kind': 3})
    level['items'] += [{'x': width-620, 'y': 215, 'kind': 4},
                       {'x': width-560, 'y': 215, 'kind': 9}]
    # Timed pressure vents occupy the exposed lower service lane.
    # No outlet is placed at a ladder landing.
    level['hazards'] = [dict(x=x,y=225,w=50,h=8,period=period,on=1.1,offset=offset,kind=0)
                        for x,period,offset in [(560,3.8,0),(1120,4.2,1),(1545,3.6,2)]]
    level['checkpoints'] = [525, 1130]
    level['puddles'] = [[230,232,110,7],[890,232,110,7],[1600,232,130,7]]
    level['buildings'].append({'box': [width-480,-72,480,304], 'style': 8})
    level['minY'] = min(p['box'][1] for p in level['platforms'])-130


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    path = ROOT/'data/campaign.json'
    data = json.loads(path.read_text())
    before = copy.deepcopy(data['levels'][1])
    apply_marsh(data['levels'][1])
    if args.check:
        changed = [key for key,value in data['levels'][1].items() if before.get(key)!=value]
        if changed:
            raise SystemExit('Marsh geometry differs from its painting: '+', '.join(changed)+
                             '. Run tools/marsh_architecture.py and tools/compile_campaign.py.')
        print('Marsh route, doors, pressure vents and rescue positions match the painting')
    else:
        path.write_text(json.dumps(data,indent=2)+'\n')
