"""The full architecture generator must preserve each integrated scene."""
import copy
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from architecture import apply_architecture

original = json.loads((ROOT/'data/campaign.json').read_text())['levels']
regenerated = copy.deepcopy(original)
apply_architecture(regenerated)
for index in range(3):
    if original[index] != regenerated[index]:
        changed = [key for key in original[index] if original[index][key] != regenerated[index].get(key)]
        raise SystemExit(f'Integrated scene {index+1} changes during full regeneration: {changed}')
print('Full architecture regeneration preserves all three integrated campaign scenes')
