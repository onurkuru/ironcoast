#!/usr/bin/env python3
"""Compile the single presentation tuning source; no runtime parser on Vita."""
import argparse, json, math, pathlib, re
ROOT = pathlib.Path(__file__).resolve().parents[1]
def compile_config():
    data=json.loads((ROOT/'data/presentation.json').read_text())
    if len(data['maps']) != 6 or len(data['weapons']) != 6 or len(data['secrets']) > 32:
        raise ValueError('Expected six map/weapon profiles and at most 32 discoveries')
    if len(data['productionPlates']) != 6:raise ValueError('Expected six production plates')
    harbor=data['harborBuiltScene']
    if harbor['sourceFloor']!=data['productionPlates'][0]['sourceFloor'] or harbor['sourceAspect']!=data['productionPlates'][0]['sourceAspect']:
        raise ValueError('Harbor painting and geometry registration disagree')
    if len(data['harborGalleries'])!=3 or len(data['harborLadders'])!=6 or len(data['harborDoors'])!=2:
        raise ValueError('Incomplete integrated harbor architecture')
    for gallery in data['harborGalleries']:
        if not 0<=gallery['u0']<gallery['u1']<=1 or not 0<gallery['v']<harbor['sourceFloor']:
            raise ValueError('Invalid painted gallery')
    for ladder in data['harborLadders']:
        if ladder['gallery'] not in range(3):raise ValueError('Invalid gallery index')
        gallery=data['harborGalleries'][ladder['gallery']]
        if not gallery['u0']<ladder['u']<gallery['u1']:raise ValueError('Ladder outside its painted deck')
    for door in data['harborDoors']:
        if not 0<door['u']<door['u']+door['w']<1 or not 0<door['v']<door['v']+door['h']<=harbor['sourceFloor']:
            raise ValueError('Door panel leaves painted opening')
    for section in ['productionProps','productionStructures','campaignPresentation']:
        for key,value in data[section].items():
            if key=='enabled' and value in (0,1):continue
            if not isinstance(value,(int,float)) or not math.isfinite(value) or value<=0:
                raise ValueError('Invalid '+section+'.'+key)
    health=data['campaignPresentation']['health']
    if health!=int(health) or health>12:raise ValueError('Campaign health must be an integer from 1 to 12')
    if data['productionStructures']['postInset']*2>=data['productionStructures']['deckWidth']:
        raise ValueError('Platform supports exceed the deck bay')
    for plate in data['productionPlates']:
        if not 0<plate['sourceFloor']<1 or plate['sourceAspect']<=0:raise ValueError('Invalid scene registration')
    depth=data['productionDepth']
    if not 1<=depth['nearDepth']<=1.15 or depth['stripHeight']<=0 or not 0<depth['floorMargin']<.1:raise ValueError('Invalid production perspective')
    for weather in data['productionWeather']:
        if not 0<=weather['u0']<weather['u1']<=1 or not 0<=weather['v0']<weather['v1']<=1:raise ValueError('Invalid weather region')
        if weather['count']<=0 or weather['count']>128 or weather['speed']<=0:raise ValueError('Invalid weather budget')
    for light in data['productionLights']:
        if light['map'] not in range(6) or not 0<=light['u']<=1 or not 0<=light['v']<=1 or light['radius']<=0:raise ValueError('Invalid registered lamp')
    titan=data['forgeTitan']
    if len(data['forgeTitanPoses'])!=16 or len(data['forgeTitanCores'])!=8:raise ValueError('Incomplete titan pose map')
    if not 0<titan['baseline']<titan['cellSize'] or titan['sourceScale']<=0:raise ValueError('Invalid titan registration')
    for pose in data['forgeTitanPoses']:
        if pose['frame']!=int(pose['frame']) or not 0<=pose['frame']<8:raise ValueError('Invalid titan frame')
    for core in data['forgeTitanCores']:
        if not 0<core['x']<titan['cellSize'] or not 0<core['y']<titan['baseline']:raise ValueError('Invalid titan core')
    for key in ['shaftStep','fogSpacing','aoWidth','fogHeight']:
        if data['lighting'][key] <= 0: raise ValueError(key+' must be positive')
    for row in data['weapons']:
        for key in ['magazine','reload','flashDuration','fireDuration','casingLife']:
            if row[key] <= 0: raise ValueError(key+' must be positive')
        if row['magazine'] != int(row['magazine']): raise ValueError('Magazine must be integral')
    for row in data['maps']:
        if not 0 <= row['fogDensity'] <= 1: raise ValueError('Fog density must be 0..1')
    view=data['workshopView']
    impact=data['guardImpact']
    for key in ['sparkCount','sparkLife','sparkSpeed','sparkTrailTime','sparkTrailMax','dustCount','dustLife','dustSpeed','dustLift','dustSize','drag','landingFrequency','landingDuration','landingDecay']:
        if impact[key]<=0:raise ValueError('Invalid guard impact '+key)
    for key in ['sparkCount','dustCount']:
        if impact[key]!=int(impact[key]) or impact[key]>16:raise ValueError('Guard particle budget exceeded')
    if not 0<impact['landingPhase']<1 or not 0<=impact['landingNoise']<=1 or not 0<impact['landingGain']<=1 or not 0<=impact['dustOpacity']<=255:raise ValueError('Invalid landing effect profile')
    action=data['guardAction']
    for key in ['cellSize','renderSize','strideLength','attackDuration','flashDuration','lightRadius']:
        if action[key]<=0:raise ValueError('Invalid guard action '+key)
    if not 0<action['baseline']<action['cellSize'] or not 0<action['lightStrength']<=1:raise ValueError('Invalid guard action baseline/light')
    if len(data['guardPoseRoots'])!=16 or len(data['guardFireAnchors'])!=4:raise ValueError('Invalid guard action anchors')
    end=0
    for tip in data['guardFireAnchors']:
        if not 0<=tip['x']<action['cellSize'] or not 0<=tip['y']<action['baseline'] or tip['end']<=end:raise ValueError('Invalid guard barrel timeline')
        end=tip['end']
    if end!=action['attackDuration'] or action['flashDuration']>end:raise ValueError('Guard attack timeline mismatch')
    guard=data['guardReactions']
    for key in ['cellSize','sourceScale','renderSize','headDuration','torsoDuration','legDuration','flashDuration','collapseDuration','fadeDuration']:
        if guard[key]<=0:raise ValueError('Invalid guard '+key)
    if not 0<guard['baseline']<guard['cellSize'] or guard['corpseHold']<0:raise ValueError('Invalid guard baseline/hold')
    if guard['flashDuration']>min(guard[k] for k in ['headDuration','torsoDuration','legDuration']):raise ValueError('Guard flash hides reaction')
    for key,count,group,limit in [('guardHitTimeline',12,4,12),('guardDeathTimeline',8,8,20)]:
        rows=data[key]
        if len(rows)!=count:raise ValueError('Invalid '+key+' length')
        for start in range(0,count,group):
            end=0
            for row in rows[start:start+group]:
                if not end<row['end']<=1 or not 0<=row['frame']<limit or row['frame']!=int(row['frame']):raise ValueError('Invalid '+key+' pose')
                end=row['end']
            if end!=1:raise ValueError('Incomplete '+key)
    hero=data['heroLocomotion']
    if len(data['heroRunMuzzles'])!=8: raise ValueError('Run muzzle map must cover eight phases')
    for key in ['cellSize','sourceScale','renderSize','strideLength','acceleration','braking']:
        if hero[key]<=0: raise ValueError(key+' must be positive')
    if not 0<hero['baseline']<hero['cellSize']: raise ValueError('Invalid locomotion baseline')
    rail=data['ironlineReview']
    if not 0 < rail['bridgeStart'] < rail['bridgeEnd'] < rail['width']:
        raise ValueError('Invalid carriage coupler extents')
    if not rail['roofY'] <= rail['bridgeY'] < rail['roofY']+30:
        raise ValueError('Coupler step exceeds the reviewed traversal height')
    if not 0 < rail['mountainDepth'] < rail['forestDepth'] < 1:
        raise ValueError('Invalid train parallax depth ordering')
    if not 1 <= view['nearDepth'] <= 1.15: raise ValueError('Near depth must be 1..1.15')
    if not data['workshop']['sourceFloor'] < view['depthStart'] < 1: raise ValueError('Depth must start below the walking plane')
    for key in ['stripHeight','leadResponse','maxCameraSpeed']:
        if view[key] <= 0: raise ValueError(key+' must be positive')
    camera=data['workshopCamera']
    for key in ['establishZoom','trackingZoom','detailZoom','combatZoom','galleryZoom','exitZoom']:
        if not 1 <= camera[key] <= 1.12: raise ValueError(key+' must be 1..1.12')
    for key in ['panResponse','verticalResponse','zoomResponse','maxZoomSpeed','maxVerticalSpeed','idleDelay','combatHold']:
        if camera[key] <= 0: raise ValueError(key+' must be positive')
    if not 0 < camera['safeLeft'] < camera['safeRight'] < 480: raise ValueError('Invalid camera safe region')
    if not 0 <= camera['combatSubjectWeight'] <= 1: raise ValueError('Invalid subject weight')
    if not camera['detailStart'] < camera['detailEnd']: raise ValueError('Invalid detail region')
    parallax=data['workshopParallax']
    if not 0 < parallax['depth'] < parallax['fogDepth'] < 1: raise ValueError('Invalid distance ordering')
    for key in ['edgeFeather','chromaFeather','darkFeather','fogSpacing','fogCount']:
        if parallax[key] <= 0: raise ValueError(key+' must be positive')
    if not 0 <= parallax['transmission'] <= 1: raise ValueError('Invalid glass transmission')
    if parallax['originX'] > 0 or parallax['originX']+parallax['width']-(data['workshop']['width']-480)*parallax['depth'] < 480:
        raise ValueError('Panorama exposes horizontal canvas edge')
    for pane in data['workshopApertures']:
        if not (0 <= pane['x'] < pane['x']+pane['w'] <= 1 and 0 <= pane['y'] < pane['y']+pane['h'] <= 1):
            raise ValueError('Invalid window aperture')
    for row in data['secrets']:
        if row['map'] not in range(6) or row['radius'] <= 0: raise ValueError('Invalid discovery')
    out=['// Generated by tools/compile_presentation.py. Edit data/presentation.json only.', '#pragma once', '#include <array>', '#include <cstdint>', 'namespace kh::tuning {']
    for section, raw in data.items():
        rows=raw if isinstance(raw,list) else [raw]
        if not rows: raise ValueError('Empty section')
        keys=list(rows[0]); name=section[0].upper()+section[1:]
        out.append('struct '+name+' {')
        for key in keys:
            if not re.fullmatch('[a-zA-Z][a-zA-Z0-9]*',key): raise ValueError(key)
            out.append(('  uint32_t ' if isinstance(rows[0][key],str) else '  float ')+key+';')
        out.append('};')
        values=[]
        for row in rows:
            if list(row)!=keys: raise ValueError('Profile fields differ')
            fields=[]
            for key,value in row.items():
                if isinstance(value,str):
                    if not re.fullmatch('[A-F0-9]{8}',value): raise ValueError(value)
                    fields.append('0x'+value+'u')
                else:
                    if not math.isfinite(value): raise ValueError('Non-finite tuning')
                    fields.append(str(float(value))+'f')
            values.append('{'+', '.join(fields)+'}')
        if isinstance(raw,list):out.append(f'inline constexpr std::array<{name}, {len(rows)}> {section} = {{{{'+',\n'.join(values)+'}};')
        else:out.append(f'inline constexpr {name} {section} = '+values[0]+';')
    out.append('} // namespace kh::tuning\n')
    return '\n'.join(out)
if __name__=='__main__':
    parser=argparse.ArgumentParser(); parser.add_argument('--check',action='store_true'); args=parser.parse_args()
    dest=ROOT/'src/presentation_config.h'; content=compile_config()
    if args.check:
        if not dest.exists() or dest.read_text()!=content: raise SystemExit('Stale presentation config: run tools/compile_presentation.py')
        print('Presentation config is current.')
    else:dest.write_text(content)
