"""Playable routes measured against the seven cinematic paintings.

Geometry is authored in world pixels. Every raised route is a visible gallery,
car roof or service deck. The ground corridor remains safe for the vehicle.
"""
WIDTHS = [1920,1920,1920,1600,1760,1920]
BLOCKS = [
 [(12,147,58,0),(360,200,120,0),(754,200,50,0),(1080,220,102,6)],
 [(12,315,51,1),(398,340,26,1),(790,265,40,1)],
 [(60,355,69,2),(466,483,48,2),(1000,300,48,2)],
 [(12,340,38,3),(545,300,17,3),(905,185,113,3)],
 [(12,312,-40,4),(526,406,-96,4),(1013,243,66,4)],
 [(12,340,-25,5),(442,468,32,5),(972,410,-12,5)],
]
# Map-specific composition: rifles in the dock, gas crews in the marsh,
# shield screens on the train, fixed guns at presses, drones in the relay.
ROOF_KINDS = [(0,0,1,0),(1,3,1),(4,2,0),(4,1,4),(3,4,3),(2,4,1)]
WAVES = [((0,0),(0,1)),((1,0),(1,2)),((2,0),(2,4)),((1,2),(4,1)),((3,3),(3,0)),((2,1),(0,4))]
AIR = [[],[(600,102)],[(1170,120)],[],[(340,84),(730,-5),(1190,142)],[(1090,120)]]

def apply_architecture(levels):
 for index, level in enumerate(levels):
  level['width']=WIDTHS[index];level['vehicleX']=WIDTHS[index]-700
  level['platforms']=[{'box':[0,232,level['width'],40],'oneWay':False,'material':index % 3}]
  level['buildings']=[];level['ladders']=[];level['entrances']=[];level['puddles']=[]
  level['spawns']=[{'x':250,'y':232,'kind':0}];level['items']=[]
  for n,(x,w,top,style) in enumerate(BLOCKS[index]):
   level['buildings'].append({'box':[x,top,w,232-top],'style':style})
   level['platforms'].append({'box':[x,top,w,8],'oneWay':True,'material':2 if index==1 else 1})
   level['ladders'] += [{'x':x+26,'top':top,'bottom':232}, {'x':x+w-26,'top':top,'bottom':232}]
   level['spawns'].append({'x':x+w*.7,'y':top,'kind':ROOF_KINDS[index][n]})
   if n < 3:
    level['items'].append({'x':x+w*.38,'y':top-17,'kind':0})
   level['items'].append({'x':x+w*.8,'y':top-17,'kind':[1,2,6,3,9,1][index]})
   # Only two authored reinforcement doors; upper and lower routes have
   # different lines of fire and a deliberate quiet space before the boss.
   if n>0:
    door=len(level['entrances']);dx=x+w*.55
    level['entrances'].append({'x':dx,'y':232,'triggerX':x-90})
    for j,kind in enumerate(WAVES[index][(n-1) % len(WAVES[index])]):
     level['spawns'].append({'x':dx,'y':232 if kind!=3 else 150,'kind':kind,'entrance':door,'delay':j*1.15})
   if index in (0,1,2,4):level['puddles'].append([x+w*.2,232,min(w*.4,120),7])
   level['items'].append({'x':x+w-40,'y':215,'kind':7 if n%2==0 else 8})
  if index==0:level['items'].append({'x':1050,'y':215,'kind':6})
  for x,y in AIR[index]:level['spawns'].append({'x':x,'y':y,'kind':3})
  level['items'] += [{'x':level['width']-560,'y':215,'kind':4},{'x':level['width']-620,'y':215,'kind':9}]
  level['checkpoints']=[float(x-65) for x,_,_,_ in BLOCKS[index][1:]]
  level['buildings'].append({'box':[level['width']-480,-72,480,304],'style':8})
  level['minY']=min(0,min(y for _,_,y,_ in BLOCKS[index])-130)
  locations=[[],[450,830,1080],[],[405,880,1110],[440,980,1240],[390,945,1400]][index]
  originals=level['hazards']
  level['hazards']=[dict(originals[n%len(originals)],x=x) for n,x in enumerate(locations)] if originals else []
  if index==2:level['brief2']='MIRA: Take the carriage roofs to flank the shields. The service deck leads to the locomotive cannon.'
  if index>=3:
   level['brief2']=["MIRA: Open the three gallery coolant valves to shut down the flame vents. Each valve stocks a grenade.","MIRA: Cut the three gallery breakers to ground the electric arcs. Each breaker stocks a grenade.","DENIZ: Override the three gallery defense terminals to silence the deck traps. Then take down the Grid."][index-3]
   from arcade_encounters import apply_arcade_encounters
   apply_arcade_encounters(level)
 from harbor_architecture import apply_harbor
 apply_harbor(levels[0])
 from marsh_architecture import apply_marsh
 apply_marsh(levels[1])
 from ironline_architecture import apply_ironline
 apply_ironline(levels[2])
