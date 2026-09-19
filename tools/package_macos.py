"""Bundle an existing host build with its SDL runtime and game resources."""
import argparse
import json
import plistlib
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def command(*args):
    return subprocess.check_output([str(a) for a in args], text=True)


def dependencies(binary):
    return [line.strip().split(' (compatibility', 1)[0]
            for line in command('otool', '-L', binary).splitlines()[1:]]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary, app = args.binary.resolve(), args.output.resolve()
    if app.suffix != '.app' or app.exists():
        parser.error('--output must be a new .app directory')
    sdl_paths = [p for p in dependencies(binary) if 'libSDL2' in p]
    if len(sdl_paths) != 1 or not Path(sdl_paths[0]).is_file():
        parser.error('binary must link to one installed SDL2 library')
    sdl = Path(sdl_paths[0]).resolve()
    license_file = sdl.parent.parent / 'LICENSE.txt'
    if not license_file.is_file():
        parser.error('SDL2 LICENSE.txt must accompany the installed library')
    minimum = re.search(r'\bminos\s+(\S+)', command('vtool', '-show-build', binary))
    if not minimum:
        parser.error('cannot determine minimum macOS from executable')
    architecture = command('lipo', '-archs', binary).strip()
    contents = app / 'Contents'
    executable = contents / 'MacOS' / 'iron-coast'
    frameworks = contents / 'Frameworks'
    resources = contents / 'Resources'
    executable.parent.mkdir(parents=True)
    frameworks.mkdir()
    resources.mkdir()
    shutil.copy2(binary, executable)
    bundled_sdl = frameworks / sdl.name
    shutil.copy2(sdl, bundled_sdl)
    shutil.copytree(ROOT / 'assets', resources / 'assets')
    shutil.copytree(ROOT / 'licenses', resources / 'licenses')
    shutil.copy2(license_file, resources / 'licenses' / 'SDL2.txt')
    command('install_name_tool', '-change', sdl_paths[0],
            '@executable_path/../Frameworks/' + sdl.name, executable)
    command('install_name_tool', '-id', '@rpath/' + sdl.name, bundled_sdl)
    for target in (executable, bundled_sdl):
        for dependency in dependencies(target):
            if not dependency.startswith(('/usr/lib/', '/System/Library/', '@executable_path/', '@rpath/')):
                raise RuntimeError(f'Unbundled dependency: {dependency}')
    with (contents / 'Info.plist').open('wb') as stream:
        plistlib.dump(dict(CFBundleExecutable='iron-coast', CFBundleName='Iron Coast',
                          CFBundleDisplayName='Iron Coast', CFBundleIdentifier='dev.ironcoast.playtest',
                          CFBundlePackageType='APPL', CFBundleVersion='40',
                          CFBundleShortVersionString='0.4.0',
                          LSMinimumSystemVersion=minimum.group(1), NSHighResolutionCapable=True), stream)
    (resources / 'Build.json').write_text(json.dumps(dict(
        architecture=architecture, minimumMacOS=minimum.group(1),
        sourceCommit=command('git', '-C', ROOT, 'rev-parse', 'HEAD').strip(),
        signing='ad-hoc local playtest; not notarized'), indent=2) + '\n')
    command('codesign', '--force', '--sign', '-', bundled_sdl)
    command('codesign', '--force', '--sign', '-', app)
    command('codesign', '--verify', '--deep', '--strict', app)
    print(f'Packaged {app}: {architecture}, macOS {minimum.group(1)}+')


if __name__ == '__main__':
    main()
