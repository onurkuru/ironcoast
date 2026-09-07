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
 out.append('{'+','.join('{{'+','.join(f(n) for n in p['box'])+'},'+str(p['oneWay']).lower()+'}' for p in l['platforms'])+'},')
 for key in ['spawns','items']:
  out.append('{'+','.join('{'+f(e['x'])+','+f(e['y'])+','+str(e['kind'])+'}' for e in l[key])+'},')
 out.append('{'+','.join('{'+','.join(f(h[k]) for k in ['x','y','w','h','period','on','offset'])+','+str(h['kind'])+'}' for h in l['hazards'])+'},')
 out.append('{'+','.join(f(n) for n in l['checkpoints'])+'}},')
out+=['}; return result;','}','}']
(root/'src/campaign.cpp').write_text('\n'.join(out)+'\n')
print('Compiled six campaign stages into C++')
