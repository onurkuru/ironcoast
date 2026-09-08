"""Authoritative six-stage campaign. Coordinates are authored, never randomized."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
rows=[
 dict(name='RUSTED HARBOR',subtitle='01 / SHIPYARD GATE',width=3600,vehicleX=2450,
 brief1='DENIZ: Efe, I have your signal. The shipyard gates are locked. I am coming.',
 brief2='MIRA: The Iron Grid hijacked every work machine. Rescue the workers and shut down the crane core.',
 radio='EFE: Catch the train... Commands are coming from the northern foundry.',ending='The crane is down. Efe is alive. Cross the marsh to reach the northern rail line.',bossName='CLAW CRANE',
 ground=[(0,760),(816,1600),(1664,2450),(2506,3600)],
 upper=[(370,188,112),(570,148,96),(1110,190,128),(1280,150,128),(1870,184,112),(2080,144,112),(2740,184,144)],
 enemies=[(320,0),(550,0),(690,1),(1010,0),(1190,3),(1440,2),(1530,0),(1770,1),(1960,0),(2120,3),(2350,4),(2590,0),(2730,2),(2900,1),(3040,4)],
 items=[(225,215,1),(640,133,0),(1160,173,2),(1320,135,0),(1500,215,6),(1940,215,4),(2130,128,0),(2680,215,3),(2860,167,9),(3010,215,4)],hazards=[]),
 dict(name='TOXIC MARSH',subtitle='02 / NORTH MARSH',width=3840,vehicleX=2570,
 brief1='MIRA: The marsh pipes are poisoning the sea. Efe hid the train route here.',
 brief2='DENIZ: I will get the workers out. Then we break the dredger pump.',
 radio='EFE: The Grid is copying human voices. Captain Sarp is listening to nobody.',ending='The poison pump is broken. The shipment code can stop the armored train.',bossName='ASH DREDGER',
 ground=[(0,560),(624,1056),(1120,1616),(1680,2112),(2176,2720),(2784,3840)],
 upper=[(360,188,128),(850,184,128),(1180,192,144),(1380,148,144),(1820,188,112),(2270,184,144),(2480,144,128),(2950,184,112)],
 enemies=[(350,0),(700,1),(870,3),(980,0),(1260,2),(1420,3),(1570,1),(1790,0),(1960,3),(2070,4),(2310,2),(2530,1),(2660,3),(2850,4),(3020,2),(3170,1),(3270,3)],
 items=[(220,215,2),(900,168,0),(1460,130,0),(1600,215,6),(1790,215,1),(2040,215,4),(2530,128,0),(2980,215,3),(3200,215,9),(3330,215,4)],
 hazards=[(740,225,80,8,3.8,1.4,0,0),(1710,225,70,8,4.2,1.4,1,0),(2840,225,76,8,3.6,1.1,2,0)]),
 dict(name='IRONLINE',subtitle='03 / MOUNTAIN FREIGHT',width=4000,vehicleX=2720,
 brief1='DENIZ: I am aboard the moving freight. Every car carries another weapon.',
 brief2='MIRA: The locomotive cannon seals the pass. Watch the gaps and cut the brake line.',
 radio='EFE: There are no soldiers in the cargo. It is a city worth of power cores.',ending='The train is stopped. The stolen cores are headed for the foundry. The Iron Grid is building an army.',bossName='BLACK LOCOMOTIVE',
 ground=[(0,496),(544,1008),(1064,1552),(1608,2096),(2152,2640),(2696,3160),(3216,4000)],
 upper=[(300,188,128),(720,184,160),(1260,188,144),(1460,148,80),(1800,184,144),(2330,184,144),(2840,184,160),(3070,144,112)],
 enemies=[(350,0),(650,3),(820,0),(930,4),(1200,1),(1400,2),(1640,3),(1850,0),(2020,1),(2230,4),(2400,2),(2600,3),(2840,0),(3060,1),(3290,4),(3410,2)],
 items=[(210,215,1),(780,166,0),(1310,170,3),(1500,130,0),(1740,215,6),(1900,215,4),(2390,168,2),(3090,128,0),(3200,215,9),(3380,215,3)],hazards=[]),
 dict(name='EMBER FOUNDRY',subtitle='04 / UNDER THE MOUNTAIN',width=3680,vehicleX=2430,
 brief1='MIRA: The plant is critical. The presses are still running. Wait for the safe rhythm.',
 brief2='DENIZ: Efe sabotaged the cores. The Grid trapped him below the tower. I will cool the furnace first.',
 radio='SARP: One city can be spent for a coast. Deniz, turn back.',ending='The giant furnace is cold. Sarp will launch from the storm tower. Efe is being held there.',bossName='FORGE TITAN',
 ground=[(0,720),(776,1512),(1568,2304),(2360,3680)],
 upper=[(430,188,128),(1010,190,128),(1230,144,128),(1760,190,128),(1950,146,144),(2590,184,144),(2820,144,128)],
 enemies=[(310,0),(570,2),(670,1),(900,4),(1120,0),(1380,3),(1650,1),(1850,2),(2070,4),(2200,3),(2450,1),(2700,2),(2980,4),(3110,3)],
 items=[(220,215,3),(485,171,0),(1060,173,1),(1290,127,0),(1700,215,6),(1820,215,4),(2010,130,2),(2870,128,0),(3030,215,9),(3140,215,3)],
 hazards=[(810,184,22,48,3.4,1.1,0,1),(1480,188,22,44,3.8,1.2,1,1),(2310,180,22,52,4.0,1.4,0,1),(2770,188,22,44,3.4,1.0,1.5,1)]),
 dict(name='STORM RELAY',subtitle='05 / LAST RELAY',width=3840,vehicleX=2510,
 brief1='EFE: Deniz! The relay can hear me. Cross the lightning and open the three tower caps.',
 brief2='MIRA: Cut the signal to the sea net and the command platform will be exposed.',
 radio='EFE: I am free. But Sarp took the backup command core to sea!',ending='The tower is silent and Efe is safe. The final platform lights burn beyond the storm.',bossName='FOUR POLES',
 ground=[(0,624),(680,1232),(1296,1904),(1968,2576),(2640,3840)],
 upper=[(340,188,128),(900,190,128),(1130,144,128),(1430,190,128),(1650,144,144),(2090,190,128),(2310,144,128),(2840,188,160),(3070,144,112)],
 enemies=[(350,3),(530,0),(800,2),(1040,3),(1200,1),(1450,4),(1700,3),(1800,2),(2050,0),(2240,3),(2450,4),(2770,1),(2990,3),(3210,2),(3310,4)],
 items=[(220,215,1),(960,173,0),(1190,128,0),(1490,215,4),(1730,128,3),(2000,215,6),(2150,215,2),(3110,128,0),(3200,215,9),(3340,215,3)],
 hazards=[(740,184,18,48,3.4,1,0,2),(1350,184,18,48,3.4,1,1,2),(2010,184,18,48,3.4,1,2,2),(2900,144,18,44,3.4,.8,0,2)]),
 dict(name='FINAL WAVE',subtitle='06 / COMMAND PLATFORM',width=4200,vehicleX=2790,
 brief1='EFE: The Iron Grid core is in Captain Sarp armor. Fire when it opens.',
 brief2='DENIZ: The whole coast can hear us. We get the workers out, then end this war.',
 radio='MIRA: Evac ships are ready. There is no retreat now.',ending='The Iron Grid is silent. The coast belongs to its people again. Deniz and Efe sail home.',bossName='IRON GRID / SARP',
 ground=[(0,704),(760,1472),(1528,2240),(2296,3040),(3096,4200)],
 upper=[(390,188,144),(960,190,144),(1180,146,128),(1650,184,144),(1860,140,128),(2450,184,144),(2980,144,128),(3290,184,128)],
 enemies=[(320,2),(560,3),(670,1),(910,4),(1090,2),(1330,3),(1580,1),(1780,2),(2040,4),(2180,3),(2420,2),(2620,1),(2840,4),(3010,3),(3190,2),(3410,4),(3560,1)],
 items=[(210,215,3),(470,172,0),(1010,174,1),(1230,130,0),(1700,215,4),(1920,124,2),(2300,215,6),(2500,168,3),(3050,128,0),(3400,215,9),(3510,215,3),(3610,215,4)],
 hazards=[(790,196,22,36,4,1.2,0,1),(2110,190,18,42,3.8,1.2,1,2),(3230,190,22,42,4,1.2,1,1)])
]
levels=[]
for idx,r in enumerate(rows):
 platforms=[{'box':[a,232,b-a,40],'oneWay':False} for a,b in r.pop('ground')]
 uppers=r.pop('upper')
 platforms += [{'box':[x,y,w,12],'oneWay':True} for x,y,w in uppers]
 # Tall ledges get an authored intermediate step so the 51 px jump arc
 # reaches them without turning decorative geometry into a dead end.
 for x,y,w in uppers:
  if y < 180:
   platforms.append({'box':[max(0,x-40),y+44,min(96,w),10],'oneWay':True})
 spawns=[]
 for x,k in r.pop('enemies'):
  spawns.append({'x':x,'y':140 if k==3 else 232,'kind':k})
 # Additional elevated sentries are explicitly anchored to existing platforms.
 for j,p in enumerate(platforms):
  if p['oneWay'] and j%3==0:spawns.append({'x':p['box'][0]+p['box'][2]*.6,'y':p['box'][1],'kind':0})
 items=[dict(zip(['x','y','kind'],a)) for a in r.pop('items')]
 for x in [420,1090,1810,2660]:
  if any(p['box'][0]<=x<=p['box'][0]+p['box'][2] and not p['oneWay'] for p in platforms): items.append({'x':x,'y':232,'kind':7})
 for x in [600,1400,2020,2920]:
  if any(p['box'][0]<=x<=p['box'][0]+p['box'][2] and not p['oneWay'] for p in platforms): items.append({'x':x,'y':232,'kind':8})
 hazards=[dict(zip(['x','y','w','h','period','on','offset','kind'],a)) for a in r.pop('hazards')]
 ground=[p for p in platforms if not p['oneWay']]
 checkpoints=[float(p['box'][0]+70) for p in ground[1:]]
 levels.append(dict(**r,theme=idx,bossKind=idx,platforms=platforms,spawns=spawns,items=items,hazards=hazards,checkpoints=checkpoints))
(root/'data/campaign.json').write_text(json.dumps({'title':'Iron Coast: Scrap Tide','version':1,'levels':levels},ensure_ascii=False,indent=2))
print('Authored',len(levels),'levels')
