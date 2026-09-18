"""Finite authored squads shared by all six architecture generators."""


def apply_arcade_encounters(level):
    # Called once after rebuilding the base architecture, never at runtime.
    # Keep every class-specific sentry and add wingmen on supported surfaces.
    for platform in level['platforms']:
        if not platform['oneWay']:
            continue
        x, y, width, _ = platform['box']
        for u in (.22, .82):
            level['spawns'].append({'x': x+width*u, 'y': y, 'kind': 0})
    for index, door in enumerate(level['entrances']):
        squad = [s for s in level['spawns'] if s.get('entrance') == index]
        for order, spawn in enumerate(squad):
            spawn['delay'] = order*.65
        for order, kind in enumerate((0, 1, 0), start=len(squad)):
            level['spawns'].append({'x': door['x'], 'y': door['y'], 'kind': kind,
                                    'entrance': index, 'delay': order*.65})
        # A visible cache on the road supports sustained fire after each wave.
        level['items'].append({'x': door['x']+64, 'y': 215, 'kind': 1})
        level['items'].append({'x': door['x']+105, 'y': 215, 'kind': 4})
    level['items'].append({'x': 90, 'y': 215, 'kind': 1})
    for x in (300, 790, 1120):
        if x < level['width']-500:
            level['spawns'].append({'x': x, 'y': 232, 'kind': 0})
