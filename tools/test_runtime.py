"""Bounded full-application smoke tests with real SDL rendering/audio callbacks."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    binary = str(args.binary.resolve())
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    with tempfile.TemporaryDirectory(prefix='iron-coast-runtime-') as folder:
        work = Path(folder)
        cases = [(f'mission-{stage}', ['--stage', str(stage), '--demo', '--frames', '120'], '')
                 for stage in range(1, 7)]
        cases += [(f'menu-{screen}', ['--screen', screen, '--frames', '2'], '')
                  for screen in ('map', 'brief', 'controls')]
        cases += [(f'save-{i}', ['--frames', '2'], data) for i, data in enumerate([
            'bad magic', 'KH_SAVE_1 3', 'KH_SAVE_1 999 -123 8 2 -1 7',
            'KH_SAVE_1 999999999999999999999999999999999999 0 0 0 0 0'])]

        def run(case):
            name, flags, data = case
            save = work / f'{name}.dat'
            save.write_text(data)
            result = subprocess.run([binary, '--assets', str(root / 'assets'), '--save', str(save),
                                     '--fast', *flags], env=env, capture_output=True, text=True, timeout=60)
            if result.returncode != 0 or 'session frames=' not in result.stdout:
                raise RuntimeError(f'{name}: {result.returncode}\n{result.stdout}\n{result.stderr}')
            return f'PASS {name}: {result.stdout.strip()}'

        with ThreadPoolExecutor(max_workers=2) as pool:
            for result in pool.map(run, cases):
                print(result, flush=True)
        print(f'PASS {len(cases)} application/SDL/audio/save-startup cases')


if __name__ == '__main__':
    main()
