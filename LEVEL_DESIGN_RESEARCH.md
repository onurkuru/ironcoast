# Metal Slug level research and Iron Coast map revision

September 8, 2026 · Iron Coast revision reviewed: `dfc8fe7` / 0.2.2

**Historical research, with an implementation update:** v0.3.0 now includes an architectural interpretation across all six maps, ladder routes, a vertical camera, door encounters and masked lighting. See the [implementation report](docs/ARCHITECTURE_UPDATE.md). The original proposals below are retained as research; they are not an exact specification of the shipped geometry. Elevators and sloped collision remain unimplemented.

## Findings and scope

Iron Coast needs architecture that shapes movement and encounters: streets, warehouses, bridges, railway cars and factories with surfaces, enemy positions and entrances that belong to those places. Background detail alone cannot supply this structure. Metal Slug's city street shows that a mostly flat route can still have depth through substantial facades, balconies, alleys, vehicles and threats at different heights.

The review covers the six missions of Metal Slug 1 and the main and alternate routes of Metal Slug 3's five missions, not the entire series. Sources include direct map-image inspection, firsthand walkthroughs, object research and SNK's history. Stitched images are not collision or spawn databases. No exact original-game pixel coordinates or exhaustive enemy counts are claimed. All Iron Coast coordinates below are original proposals.

The VGMaps set lacks Metal Slug 1 Mission 4; Mission 3 and Final Mission images are partial. Walkthroughs supplement those gaps. Metal Slug 3's finale was reviewed in two images. Sources: [map index](https://vgmaps.com/Atlas/Neo-Geo/index.htm), [Metal Slug walkthrough](https://gamefaqs.gamespot.com/ps/573212-metal-slug/faqs/7), [Metal Slug 3 walkthrough](https://gamefaqs.gamespot.com/arcade/577440-metal-slug-3/faqs/38376).

## Metal Slug 1: architecture

| Mission | Observed structure | Application to Iron Coast |
|---|---|---|
| 1 | Ruins, wreckage, shallow water, wooden stilt structures and a rising rocky waterfall approach. | Height changes should follow buildings and terrain. Distinguish hut floors, balconies and rocks. |
| 2 | Broken railway, bridges above water, long wooden ramps and ascending tracks. | Gaps should represent visible damage, with supports and water establishing their purpose. |
| 3 | Opposing rock ledges, wooden links and a snowy base in the partial image. | Vertical progression requires camera support and deliberately connected ledges. |
| 4 | The walkthrough describes a bar interior, upper floor, hills and caves; no complete map image was available. | Interiors and outdoor terrain need different encounters. This row is not a visual measurement. |
| 5 | Stone street, thick facades, recessed windows, balconies, alleys, vehicles and a two-level passage. | Keep an accessible main street while building a substantial layered environment around it. |
| Final | Jungle approach, rope bridge and a large two-level bridge joined by a ramp in the partial image. | A break above can become a route choice when another path exists below. The image does not cover the entire final battle. |

Images: [Mission 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission1.png), [Mission 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission2.png), [Mission 3](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission3.png), [Mission 5](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission5.png), [Final Mission bridge](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-FinalMission.png).

### Enemy placement and entrances

| Mission | Encounter relationships |
|---|---|
| 1 | Divers below, bombers on huts and tanks on the rising approach combine frontal and elevated pressure. |
| 2 | Paratroopers above, boats below and soldiers around ramp crests attack from distinct directions. |
| 3 | Shield and rocket soldiers occupy ledges; elevated attacks lead into Allen O'Neil's encounter. |
| 4 | Balcony rockets, hillside bombers and road tanks make elevation matter. |
| 5 | Window positions and troop-producing vehicles support street tanks and attacks through a two-level passage. |
| Final | Upper and lower bridge threats lead to aerial attacks during the boat section and combat after landing. |

Source: [achtungnight's firsthand walkthrough](https://gamefaqs.gamespot.com/ps/573212-metal-slug/faqs/7). These summaries do not enumerate every spawn or difficulty variation.

Window bombers and rope-bound soldiers also have distinct object behaviors: positioning is more than changing an infantry actor's `y` coordinate. The [object catalog](https://randomhoohaas.flyingomelette.com/msmia/1/ob.html) does not imply that every listed object appears in every mission.

## Metal Slug 3: route structure

SNK explicitly highlights branching maps. Routes should lead through different spaces and encounters before rejoining, rather than merely increasing decorative density. [Official SNK history](https://www.snk-corp.co.jp/us/anniversary/metalslug30th/history/).

| Mission | Route structure | Spatial lesson |
|---|---|---|
| 1 | Surface/boat, underwater and sewer approaches converge toward the boss. | Branches descend and return through readable physical connections. |
| 2 | Mountain route with a returning ice-cave branch. | Slopes and two-level caves create different movement rhythms. |
| 3 | Underwater pipe branches and surface base access lead into the factory and boss. | Boxes, stairs, beams and factory shafts make upper levels actual destinations. |
| 4 | Upper pyramid and lower base/underground routes include mummy, insect-tunnel and cellar sections. | Building silhouettes, hatches and shafts explain route choices. |
| Final | Air approach, base, space travel, mothership, escape and final encounter. | Distinct movement sections replace one excessively extended platform. |

Reviewed images: [Mission 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission1.png), [Mission 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission2.png), [Mission 3](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission3.png), [Mission 4](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission4.png), [Final part 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-FinalMission(Part1).png), [Final part 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-FinalMission(Part2).png). This is a summary of major connections, not a room-by-room technical inventory.

Coastal ground/air mixtures, upper and lower cave positions, multi-floor factory guards, ceiling and floor threats in tunnels, and pursuing pressure in final corridors connect enemy composition to route selection. [Mission walkthrough](https://gamefaqs.gamespot.com/arcade/577440-metal-slug-3/faqs/38376).

## Existing Iron Coast audit

Counts come directly from `data/campaign.json` at the reviewed revision. Every main floor is at `y=232`; gaps are 48–64 pixels wide. Upper-surface counts include generated intermediate steps and do not alone establish that a platform is defective.

| Map | Width | Floor gaps | Upper surfaces | Main-floor enemies | Upper ground enemies | Drones |
|---|---:|---:|---:|---:|---:|---:|
| RUSTED HARBOR | 3600 | 3 | 10 | 13 | 3 | 2 |
| TOXIC MARSH | 3840 | 5 | 10 | 12 | 4 | 5 |
| IRONLINE | 4000 | 6 | 10 | 13 | 3 | 3 |
| EMBER FOUNDRY | 3680 | 3 | 10 | 11 | 3 | 3 |
| STORM RELAY | 3840 | 4 | 13 | 10 | 4 | 5 |
| FINAL WAVE | 4200 | 4 | 11 | 13 | 4 | 4 |
| **Total** | **23160** | **25** | **64** | **72** | **21** | **22** |

Of 115 initial enemies, 93 are ground units; about 77% of those begin at the same main-floor height. Bosses and subsequently generated units are excluded.

- `tools/create_campaign.py` assigns some upper guards by platform index modulo three instead of an authored doorway, window or encounter purpose. Props reuse x positions; checkpoints follow floor segments after gaps.
- `src/game.h` models platforms as rectangles with `oneWay`, without building, roof, window, entrance or wave identities.
- `src/game.cpp` creates enemies at level start and activates them near the camera. It lacks visible door-opening/exit sequences and follows the player horizontally only.
- `src/render.cpp` uses repeated foreground platform strips. Distant buildings do not provide connected, playable facades and roofs.

This is a level-data, scene-layout and behavior limitation, not a limitation inherent to SDL. Lighting and parallax cannot independently fix missing architecture and encounter structure.

## Proposed six-mission redesign

These are original Iron Coast proposals, not copies of Metal Slug maps.

| Mission | Spatial sequence | Playable architecture and encounters |
|---|---|---|
| RUSTED HARBOR | Harbor street → warehouse → container yard → canal bridge → crane yard | Balcony and roof alternate route; door infantry, window grenadier and yard shield unit. Replace the three gaps with continuous routes and a supported bridge. |
| TOXIC MARSH | Pump settlement → stilt houses → treatment basin → pumping station | Supported walkways, pipe bridge and maintenance balcony. Recoverable lower service route after a water fall; visible drone emergence from a pump chimney. |
| IRONLINE | Loading station → freight cars → passenger/ammunition car → locomotive | Car interiors and roofs form two routes. Couplers and service steps replace blind gaps; enemies enter through doors or neighboring cars. |
| EMBER FOUNDRY | Factory gate → production hall → casting line → furnace room | Supported mezzanines, press room and maintenance stairs; upper grenadiers and lower armor. Clearly marked molten channels provide justified hazards. |
| STORM RELAY | Administration building → maintenance floors → connecting bridge → antenna roof | Actual ascent requires vertical camera support and elevators. Windows provide positions, doors provide reinforcements, and the roof silhouette previews the destination. |
| FINAL WAVE | Dock → hangar → command building → evacuation deck | Lower hangar and upper maintenance routes rejoin. Finite waves use visible entrances; the final deck keeps escape space and floor edges clear. |

### First implementation proposal: Rusted Harbor

Coordinates target the existing 3600-pixel world. Positive `y` points downward; feet align with surface tops. The current jump rises approximately 51 pixels, so intermediate steps use 40-pixel height differences. Geometry alone does not prove accessibility: actor width and movement tests still need validation.

| World range | Structure / surface | Enemy placement and trigger |
|---|---|---|
| 0–480 | Continuous street at `y232`; loading step x270–355 at `y192`, balcony x355–475 at `y152`. | Introduce one street guard. The next visible doorway supplies reinforcements without unexplained rear spawns. |
| 480–1120 | Warehouse lower corridor at `y232`; roof x560–1050 at `y152`, steps at both ends at `y192`. | Two infantry emerge below; a window grenadier visibly prepares above. Stagger activation with player entry. |
| 1120–1680 | Containers x1200–1330 at `y192`, x1330–1450 at `y152`, exit step x1450–1535 at `y192`. | Shield unit near the exit; containers allow a changed firing angle. Preserve vehicle and melee access. |
| 1680–2280 | Continuous canal bridge at `y232`, visible supports and water, optional service ledge. | Opposite-bank position and distant, visible air reinforcement. No first-mission death pit. |
| 2280–3180 | Shipping building, loading ramp and open crane yard with a wide vehicle route. | Rolling door introduces finite waves. Teach the upper threat before combining it with ground units. |
| 3180–3600 | Continuous boss deck with crane foundations, background hangar and rails. | Arena boundaries control boss entry and camera lock. Keep retreat space free of arbitrary gaps and obstructive decoration. |

Initial door-wave target: approximately 0.6 seconds of visible preparation before the exit animation, no spawning on the player and a bounded active-threat count. These are original tuning proposals, not timings measured from Metal Slug.

## Art, lighting and collision

Each building kit should include a facade, side face, roof edge, door, window, supports and foundation contact. Initial kits: harbor warehouse, worker building, containers, bridge supports and crane hangar. **All player-facing signage, dialogue, interface text and documentation use English for a global audience.**

Layer order: distant city → middle-distance structures → playable facade → actors → sparse foreground edges. Playable roofs and facades must share world movement and collision coordinates; do not apply independent parallax to a supporting surface. Dark alley interiors and clear facade edges should establish depth. Visible floor tops and collision surfaces need one shared data source.

Attach lights to windows, doors, street lamps, vehicles and machinery. Keep sources understandable and feet readable; reduce fog and bloom that conceal contact. Foundation shadows should connect buildings to the ground. A REPLACED-inspired atmosphere must sit on this coherent physical scene. This document does not claim a new technical study of REPLACED.

## Implementation order and acceptance criteria

1. Build the first harbor block with named structures/surfaces, a visible warehouse and balcony, a continuous street and authored enemy positions.
2. Add door/window entry behavior, warning animations and finite waves triggered by the scene.
3. Complete both harbor routes, bridge, vehicle access and boss deck; author the other five missions individually.
4. Implement vertical camera movement, elevators and ladders if required. These systems cannot be delivered through campaign JSON changes alone.

Acceptance checks:

- Simulated movement reaches every main and reward route, including return paths from roofs.
- Character, enemy and vehicle feet align with visible surfaces. Camera movement keeps facades, collision and lights together.
- Balcony headroom has no invisible blocking geometry; enemy bullets do not strike their own decorative facade.
- Reinforcement triggers cannot repeat indefinitely. Checkpoint resets reconstruct encounters consistently.
- Every gap has a visible boundary, spatial purpose and defined outcome. The first harbor mission has no mandatory death pit.
- Verify moving gameplay on the host. Report Vita compilation and physical-device performance separately.

This document records research, the 0.2.2 map audit and proposed designs. It does not indicate that the redesigned maps are present in a playable release.
