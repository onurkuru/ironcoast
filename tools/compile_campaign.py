import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
data=json.loads((root/'data/campaign.json').read_text())['levels']
q=lambda s:json.dumps(s,ensure_ascii=True)
f=lambda n:str(float(n))+'f'
out=['// Generated from data/campaign.json by tools/compile_campaign.py.','// Edit the JSON and regenerate; no runtime JSON dependency on Vita.','#include "game.h"','namespace kh {','const std::vector<Level>& campaign(){','static const std::vector<Level> result = {']
for l in data:
 strings=','.join(q(l[k]) for k in ['name','subtitle','brief1','brief2','radio','ending','bossName'])
 out.append('{'+strings+','+str(l['theme'])+','+f(l['width'])+','+f(l['vehicleX'])+','+str(l['bossKind'])+',')
 out.append('{'+','.join('{{'+','.join(f(n) for n in p['box'])+'},'+str(p['oneWay']).lower()+','+str(p.get('material',0))+'}' for p in l['platforms'])+'},')
 for key in ['spawns','items']:
  out.append('{'+','.join('{'+f(e['x'])+','+f(e['y'])+','+str(e['kind'])+(','+str(e.get('entrance',-1))+','+f(e.get('delay',0)) if key=='spawns' else '')+'}' for e in l[key])+'},')
 out.append('{'+','.join('{'+','.join(f(h[k]) for k in ['x','y','w','h','period','on','offset'])+','+str(h['kind'])+'}' for h in l['hazards'])+'},')
 out.append('{'+','.join(f(n) for n in l['checkpoints'])+'},')
 out.append('{'+','.join('{{'+','.join(f(n) for n in b['box'])+'},'+str(b['style'])+'}' for b in l['buildings'])+'},')
 out.append('{'+','.join('{'+','.join(f(a[k]) for k in ['x','top','bottom'])+'}' for a in l['ladders'])+'},')
 out.append('{'+','.join('{'+','.join(f(a[k]) for k in ['x','y','triggerX'])+'}' for a in l['entrances'])+'},')
 out.append('{'+','.join('{'+','.join(f(n) for n in a)+'}' for a in l['puddles'])+'},'+f(l['minY'])+'},')
out+=['}; return result;','}','}']
(root/'src/campaign.cpp').write_text('\n'.join(out)+'\n')
print('Compiled six campaign stages into C++')
