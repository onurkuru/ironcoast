"""Validate distributable Vita metadata, indexed launcher art and bundled resources."""
import argparse
import hashlib
from pathlib import Path, PurePosixPath
import struct
import xml.etree.ElementTree as ET
import zipfile

ROOT = Path(__file__).resolve().parents[1]
IMAGES = {
    'sce_sys/icon0.png': (128, 128),
    'sce_sys/pic0.png': (960, 544),
    'sce_sys/livearea/contents/bg0.png': (840, 500),
    'sce_sys/livearea/contents/startup.png': (280, 158),
}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sfo_fields(data):
    magic, version, keys, values, count = struct.unpack_from('<5I', data)
    require(magic == 0x46535000 and version == 0x101, 'Invalid SFO header')
    result = {}
    for i in range(count):
        key, fmt, size, capacity, offset = struct.unpack_from('<HHIII', data, 20 + i * 16)
        require(size <= capacity and values + offset + size <= len(data), 'Invalid SFO range')
        name = data[keys + key:].split(b'\0', 1)[0].decode()
        value = data[values + offset:values + offset + size]
        result[name] = value.rstrip(b'\0').decode() if fmt == 0x204 else value
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('vpk', type=Path)
    args = parser.parse_args()
    with zipfile.ZipFile(args.vpk) as archive:
        names = archive.namelist()
        require(len(names) == len(set(names)), 'Duplicate archive entries')
        require(archive.testzip() is None, 'Archive CRC failed')
        require(all(not PurePosixPath(n).is_absolute() and '..' not in PurePosixPath(n).parts
                    for n in names), 'Unsafe archive path')
        require(not any(n.endswith(('.dmg', '.dylib', '.exe')) or '.app/' in n for n in names),
                'Non-Vita executable in package')
        require(archive.read('eboot.bin')[:4] == b'SCE\0', 'Missing Vita SELF executable')
        fields = sfo_fields(archive.read('sce_sys/param.sfo'))
        require(fields.get('TITLE_ID') == 'KHYI00001', 'Unexpected title ID')
        require(fields.get('APP_VER') == '00.25', 'Unexpected Vita version')
        require(fields.get('TITLE') == 'Iron Coast: Scrap Tide', 'Unexpected application title')
        for name, size in IMAGES.items():
            data = archive.read(name)
            require(data[:8] == b'\x89PNG\r\n\x1a\n', f'{name}: invalid PNG')
            w, h, depth, color, compression, filtering, interlace = struct.unpack_from('>IIBBBBB', data, 16)
            require((w, h) == size and depth == 8 and color == 3 and interlace == 0,
                    f'{name}: wrong size or indexed PNG encoding')
            offset, palette = 8, False
            while offset < len(data):
                length = struct.unpack_from('>I', data, offset)[0]
                kind = data[offset + 4:offset + 8]
                if kind == b'PLTE':
                    palette = length == 768
                require(kind != b'tRNS', f'{name}: unexpected transparency')
                offset += 12 + length
            require(palette, f'{name}: expected 256-color palette')
        base = 'sce_sys/livearea/contents/'
        template = ET.fromstring(archive.read(base + 'template.xml'))
        require(template.tag == 'livearea' and template.get('style') == 'a1', 'Invalid LiveArea template')
        for path in ('livearea-background/image', 'gate/startup-image'):
            node = template.find(path)
            require(node is not None and base + node.text in names, 'Missing LiveArea image reference')
        checked = 0
        for folder in ('assets', 'sce_sys', 'licenses'):
            for source in (ROOT / folder).rglob('*'):
                if source.is_file():
                    name = source.relative_to(ROOT).as_posix()
                    require(archive.read(name) == source.read_bytes(), f'{name}: stale packaged resource')
                    checked += 1
        print(f'PASS: SELF, title {fields["TITLE_ID"]}, version {fields["APP_VER"]}, four indexed PNGs, XML, {checked} resources')
        print(f'Installed file bytes: {sum(i.file_size for i in archive.infolist())}')
    print(f'SHA256 {hashlib.sha256(args.vpk.read_bytes()).hexdigest()}')


if __name__ == '__main__':
    main()
