"""Authored playable architecture. Roofs, ladders and facade art share coordinates."""
# x, width, roof elevation, structure style (warehouse, stilts, train, factory,
# relay, hangar, container rack, bridge). All routes retain a vehicle corridor.
BLOCKS = [
 [(300,270,152,0),(760,350,112,0),(1240,250,152,6),(1680,400,232,7),(2220,500,128,0)],
 [(320,260,152,1),(780,380,112,1),(1390,300,152,7),(1830,420,96,1),(2710,450,128,1)],
 [(280,320,152,2),(730,360,152,2),(1220,400,112,2),(1780,350,152,2),(2330,320,112,2),(2810,470,152,2)],
 [(300,300,136,3),(850,400,96,3),(1440,450,112,3),(2060,350,72,3),(2620,470,128,3)],
 [(300,330,128,4),(790,440,48,4),(1430,460,-48,4),(2070,360,48,4),(2690,470,112,4)],
 [(300,340,136,5),(820,420,112,5),(1430,420,72,5),(2050,440,112,5),(2700,570,96,5)],
]

def apply_architecture(levels):
 for index, level in enumerate(levels):
  level['platforms']=[{'box':[0,232,level['width'],40],'oneWay':False,'material':index % 3}]
  level['buildings']=[];level['ladders']=[];level['entrances']=[];level['puddles']=[]
  level['spawns']=[];level['items']=[]
  roofs=[]
  for n,(x,w,top,style) in enumerate(BLOCKS[index]):
   level['buildings'].append({'box':[x,top,w,232-top],'style':style})
   if top < 232:
    level['platforms'].append({'box':[x,top,w,8],'oneWay':True,'material':1})
    roofs.append((x,w,top))
    # Roof access and return are explicit, never inferred from sprite bounds.
    level['ladders'] += [{'x':x+26,'top':top,'bottom':232},
                         {'x':x+w-26,'top':top,'bottom':232}]
    # Each facade has a finite pair of visible reinforcements, staggered.
    door=len(level['entrances']);dx=x+w*.55
    level['entrances'].append({'x':dx,'y':232,'triggerX':x-100})
    for delay in (0,.85):
     level['spawns'].append({'x':dx,'y':232,'kind':0 if delay==0 else (2 if n%2 else 1),'entrance':door,'delay':delay})
    level['spawns'].append({'x':x+w*.7,'y':top,'kind':1 if n%2==0 else 0})
    # One clearly authored intermediate service balcony on tall buildings.
    if top < 80:
     mid=(top+232)/2
     level['platforms'].append({'box':[x+60,mid,w-120,8],'oneWay':True,'material':1})
   if n%2==0:
    level['puddles'].append([x+40,232,min(w-80,150),10])
   level['items'] += [{'x':x-45,'y':215,'kind':[1,2,6,3,9,1][n]},
                      {'x':x+w-70,'y':232,'kind':7 if n%2==0 else 8}]
  # Introductory guard and explicitly placed airborne threats.
  level['spawns'].insert(0,{'x':240,'y':232,'kind':0})
  for x in (650,1730,2550):
   level['spawns'].append({'x':x,'y':110,'kind':3})
  for x,w,y in roofs[:3]:
   level['items'].append({'x':x+w*.38,'y':y-17,'kind':0})
  level['items'] += [{'x':level['width']-560,'y':215,'kind':4},
                     {'x':level['width']-620,'y':215,'kind':9}]
  level['checkpoints']=[float(x-90) for x,_,_,_ in BLOCKS[index][1:]]
  # Arena backdrop has no visible roof or collision across the escape route.
  level['buildings'].append({'box':[level['width']-480,-72,480,304],'style':8})
  level['minY']=min(0,min(y for _,_,y,_ in BLOCKS[index])-130)
  # Hazards remain bounded and timed, with a clear supported floor beneath.
  if index==0:level['hazards']=[]
