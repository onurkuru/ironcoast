"""Create a self-contained Apple Silicon .app from an existing desktop build."""
import argparse,plistlib,shutil,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--binary',required=True,type=Path);p.add_argument('--output',required=True,type=Path);args=p.parse_args()
root=Path(__file__).resolve().parents[1];app=args.output/'Kiyi Hurdasi.app';contents=app/'Contents'
if app.exists():
    shutil.rmtree(app)
for part in ['MacOS','Resources','Frameworks']:(contents/part).mkdir(parents=True,exist_ok=True)
exe=contents/'MacOS/kiyi_hurdasi';shutil.copy2(args.binary,exe)
shutil.copytree(root/'assets',contents/'Resources/assets',dirs_exist_ok=True)
links=subprocess.check_output(['otool','-L',str(exe)],text=True)
sdl=next(line.strip().split(' (')[0] for line in links.splitlines() if 'libSDL2' in line)
shutil.copy2(sdl,contents/'Frameworks/libSDL2-2.0.0.dylib')
shutil.copy2('/opt/homebrew/opt/sdl2/LICENSE.txt',contents/'Resources/SDL2-LICENSE.txt')
subprocess.run(['install_name_tool','-change',sdl,'@executable_path/../Frameworks/libSDL2-2.0.0.dylib',str(exe)],check=True)
subprocess.run(['install_name_tool','-id','@executable_path/../Frameworks/libSDL2-2.0.0.dylib',str(contents/'Frameworks/libSDL2-2.0.0.dylib')],check=True)
info={'CFBundleName':'Iron Coast: Scrap Tide','CFBundleDisplayName':'Iron Coast: Scrap Tide','CFBundleIdentifier':'games.kiyihurdasi.desktop','CFBundleVersion':'0.1.0','CFBundleShortVersionString':'0.1.0','CFBundleExecutable':'kiyi_hurdasi','CFBundlePackageType':'APPL','NSHighResolutionCapable':True,'NSSupportsAutomaticGraphicsSwitching':True}
(contents/'Info.plist').write_bytes(plistlib.dumps(info))
subprocess.run(['codesign','--force','--deep','--sign','-',str(app)],check=True)
print(app)
