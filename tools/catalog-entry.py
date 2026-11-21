#!/usr/bin/env python3
"""Generate metadata only; does not upload files or contact the CDN."""
import argparse
import hashlib
import json
from pathlib import Path
from urllib.parse import quote, urlsplit

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('project', type=Path)
parser.add_argument('base_url')
parser.add_argument('--id', required=True)
parser.add_argument('--author', required=True)
parser.add_argument('--license', required=True)
args = parser.parse_args()
root = args.project.resolve()
url = urlsplit(args.base_url)
if url.scheme != 'https' or not url.netloc or url.username or url.query or url.fragment:
    parser.error('base URL must be an HTTPS directory URL without credentials, query or fragment')
manifest = root / 'wallpaper.json'
if manifest.is_file():
    doc = json.loads(manifest.read_text())
    if doc.get('format') != 'wpe-native' or doc.get('version') not in (1, 2):
        parser.error('unsupported native document')
    kind, title = 'native', doc['title']
    paths = {'wallpaper.json'}
    paths.update(layer['source'] for layer in doc['layers'] if layer['type'] in ('image', 'video'))
    entry = None
else:
    doc = json.loads((root / 'project.json').read_text())
    kind, title, entry = doc['type'], doc['title'], doc['file']
    if kind not in ('image', 'video'):
        parser.error('only native, image and video projects are supported')
    paths = {entry}
files = []
for name in sorted(paths):
    path = (root / name).resolve()
    if not path.is_relative_to(root) or not path.is_file() or '\\' in name or ':' in name or '..' in Path(name).parts:
        parser.error(f'invalid asset path: {name}')
    size = path.stat().st_size
    if not 0 < size <= 256 * 1024 * 1024:
        parser.error(f'asset exceeds 256 MiB or is empty: {name}')
    digest = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            digest.update(chunk)
    files.append(dict(path=name, url=args.base_url.rstrip('/') + '/' + quote(name, safe='/'), bytes=size, sha256=digest.hexdigest()))
if len(files) > 128 or sum(f['bytes'] for f in files) > 1024**3:
    parser.error('project exceeds catalog file or size limits')
result = dict(id=args.id, title=title, author=args.author, license=args.license, type=kind, files=files)
if entry:
    result['entry'] = entry
print(json.dumps(result, indent=2))
