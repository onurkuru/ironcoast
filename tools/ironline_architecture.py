"""Register the campaign freight route to its integrated foreground painting."""
import argparse
import copy
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def apply_ironline(level):
    art = json.loads((ROOT/'data/presentation.json').read_text())
    rig = art['ironlineBuiltScene']
    level['width'] = width = 1920
    level['vehicleX'] = width-700
    height = width/rig['sourceAspect']
    world_y = lambda v: 232+(v-rig['sourceFloor'])*height
    level['platforms'] = [{'box': [0,232,width,40], 'oneWay': False, 'material': 1}]
    for key in ('buildings','ladders','entrances','spawns','items','checkpoints'):
        level[key] = []
    for index, gallery in enumerate(art['ironlineGalleries']):
        left, right = gallery['u0']*width, gallery['u1']*width
        y = world_y(gallery['v'])
        ladders = [ladder['u']*width for ladder in art['ironlineLadders'] if ladder['gallery']==index]
        level['platforms'].append({'box': [left,y,right-left,8], 'oneWay': True, 'material': 1})
        level['buildings'].append({'box': [left,y,right-left,232-y], 'style': 2})
        level['ladders'] += [{'x': x, 'top': y, 'bottom': 232} for x in ladders]
        level['spawns'].append({'x': (ladders[0]+ladders[1])/2, 'y': y, 'kind': [0,4,1][index]})
        level['items'] += [{'x': ladders[1]-48, 'y': y-17, 'kind': 0},
                           {'x': ladders[0]+50, 'y': y-17, 'kind': [2,6,9][index]}]
        if index<2:
            level['checkpoints'].append(ladders[1]+20)
    for index, door in enumerate(art['ironlineDoors']):
        x = (door['u']+door['w']/2)*width
        level['entrances'].append({'x': x, 'y': 232, 'triggerX': x-130})
        for order, kind in enumerate(([2,0],[2,4])[index]):
            level['spawns'].append({'x': x, 'y': 232, 'kind': kind,
                                    'entrance': index, 'delay': order*1.15})
    level['spawns'].append({'x': 1010, 'y': 140, 'kind': 3})
    level['items'] += [{'x': width-620, 'y': 215, 'kind': 4},
                       {'x': width-560, 'y': 215, 'kind': 9}]
    level['hazards'] = []
    level['puddles'] = [[180,232,110,5],[725,232,110,5],[1300,232,100,5]]
    level['buildings'].append({'box': [width-480,-72,480,304], 'style': 8})
    level['minY'] = min(p['box'][1] for p in level['platforms'])-130


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    path = ROOT/'data/campaign.json'
    data = json.loads(path.read_text())
    before = copy.deepcopy(data['levels'][2])
    apply_ironline(data['levels'][2])
    if args.check:
        changed = [key for key,value in data['levels'][2].items() if before.get(key)!=value]
        if changed:
            raise SystemExit('Ironline geometry differs from its painting: '+', '.join(changed)+
                             '. Run tools/ironline_architecture.py and tools/compile_campaign.py.')
        print('Ironline roofs, ladders, doors and rescues match the train painting')
    else:
        path.write_text(json.dumps(data,indent=2)+'\n')
