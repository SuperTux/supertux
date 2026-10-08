#!/usr/bin/env python3
"""Deterministic browser inventory. Originals and native/Android inputs are untouched."""
import argparse
import base64
import hashlib
import gzip
import json
import re
import shutil
from pathlib import Path, PurePosixPath
import zlib


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def write(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_bytes() != content:
        temporary = path.with_name(path.name + '.tmp')
        temporary.write_bytes(content)
        temporary.replace(path)


def encoded(value):
    return (json.dumps(value, sort_keys=True, separators=(',', ':')) + '\n').encode()


def prepare(source, output, full=False, source_commit='unknown', configuration='Release', runtime_root='/supertux/data', presentation=False):
    if output.resolve().is_relative_to(source.resolve()):
        raise ValueError('Derived packaging must be outside the source data tree')
    files = sorted(p for p in source.rglob('*') if p.is_file())
    audio = set()
    for path in files:
        if path.is_symlink():
            raise ValueError(f'Symlink in asset inputs: {path}')
        relative = path.relative_to(source).as_posix()
        with path.open('rb') as stream:
            magic = stream.read(12)
        # Inspect signatures, not suffixes; only soundtrack audio is deferred.
        if relative.startswith('music/') and (magic.startswith(b'OggS') or (magic.startswith(b'RIFF') and magic[8:] == b'WAVE')):
            audio.add(relative)
    # All descriptor companion references must exist, including shared tracks.
    for path in files:
        if path.suffix == '.music':
            for raw in re.findall(r'\(file\s+"([^"\n]+)"\)', path.read_text()):
                resolved = (path.parent / raw).resolve()
                resolved.relative_to(source.resolve())
                if not resolved.is_file():
                    raise ValueError(f'Missing music companion: {path}: {raw}')
    startup = output / 'startup'
    startup.mkdir(parents=True, exist_ok=True)
    entries = []
    wanted = set()
    for path in files:
        relative = path.relative_to(source).as_posix()
        sha = digest(path)
        deferred = not full and relative in audio
        entry = dict(path=relative, bytes=path.stat().st_size, sha256=sha, package='music' if deferred else 'startup')
        if deferred:
            key = f'music/{sha}/track' + ('.ogg' if path.read_bytes()[:4] == b'OggS' else '.wav')
            entry['url'] = 'game-assets/' + key
            target = output / 'delivery' / entry['url']
        else:
            target = startup / relative
            wanted.add(relative)
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists() or digest(target) != sha:
            shutil.copyfile(path, target)
        entries.append(entry)
    if presentation:
        from coop_presentation import derive
        levels, _ = derive(source, output, write, encoded)
        for level in levels:
            relative = level.relative_to(startup).as_posix()
            wanted.add(relative)
            entries.append(dict(path=relative, bytes=level.stat().st_size, sha256=digest(level), package='startup'))
        entries.sort(key=lambda entry: entry['path'])
    for old in startup.rglob('*'):
        if old.is_file() and old.relative_to(startup).as_posix() not in wanted:
            old.unlink()
    # Drop removed/changed derived music; a rebuild never publishes obsolete payloads.
    delivery = output / 'delivery'
    music_urls = {e.get('url') for e in entries if e['package'] == 'music'}
    if delivery.exists():
        for old in delivery.rglob('*'):
            if old.is_file() and old.relative_to(delivery).as_posix() not in music_urls:
                old.unlink()
    inventory = dict(schema=1, sourceCommit=source_commit, configuration=configuration,
                     toolchain='emscripten-6.0.11', mode='full-preload' if full else 'deferred-music',
                     runtimeRoot=runtime_root, assets=entries)
    write(output / 'inventory.json', encoded(inventory))
    identity = dict(sourceCommit=source_commit, configuration=configuration, inventorySha256=digest(output / 'inventory.json'))
    write(output / 'identity.js', ('if (!Module["supertuxAssets"] || Module["supertuxAssets"].identity !== ' + json.dumps(identity['inventorySha256']) + ') throw new Error("Game assets changed. Reload this page to continue.");\n').encode())
    return inventory


def compressed_package(build, path, category):
    """Reuse derived gzip by raw identity; addresses always hash encoded bytes."""
    derived = build / 'web-assets/compressed'
    index = derived / (category + '.json')
    identity = dict(rawSha256=digest(path), zlib=zlib.ZLIB_VERSION, level=6)
    if index.exists():
        try:
            previous = json.loads(index.read_text())
            payload = derived / previous['file']
            if previous['identity'] == identity and payload.exists() and digest(payload) == previous['sha256']:
                return payload
        except (ValueError, KeyError):
            pass # Interrupted derived metadata is regenerated from originals.
    derived.mkdir(parents=True, exist_ok=True)
    temporary = derived / (category + '.tmp')
    with path.open('rb') as source, temporary.open('wb') as target:
        with gzip.GzipFile(fileobj=target, mode='wb', filename='', mtime=0, compresslevel=6) as archive:
            shutil.copyfileobj(source, archive)
    sha = digest(temporary)
    payload = derived / (sha + '.gz')
    temporary.replace(payload)
    write(index, encoded(dict(identity=identity, file=payload.name, sha256=sha)))
    return payload


def assemble(build, output, cloudflare=False, compression=True):
    inventory_path = build / 'web-assets/inventory.json'
    manifest = json.loads((inventory_path if inventory_path.exists() else build / 'asset-manifest.json').read_text())
    if inventory_path.exists():
        manifest['inventorySha256'] = digest(inventory_path)
    previous_packages = manifest.get('packages', {})
    manifest['packages'] = {}
    output.mkdir(parents=True, exist_ok=True)
    for category, suffix in [('startup', 'data'), ('wasm', 'wasm'), ('javascript', 'js')]:
        path = build / ('supertux2.' + suffix)
        sha = digest(path)
        url = f'/game-assets/{suffix}/{sha}/supertux2.{suffix}' if cloudflare and suffix in ('data', 'wasm') else 'supertux2.' + suffix
        manifest['packages'][category] = dict(bytes=path.stat().st_size, sha256=sha, url=url)
        if compression:
            existing = previous_packages.get(category, {}).get('encodings', {}).get('gzip')
            if not inventory_path.exists() and existing:
                compressed = build / existing['url'].lstrip('/')
                if digest(compressed) != existing['sha256']:
                    raise ValueError('Reused compressed package is damaged')
            else:
                compressed = compressed_package(build, path, category)
            compressed_sha = digest(compressed)
            compressed_url = f'game-assets/{suffix}-gzip/{compressed_sha}/supertux2.{suffix}.gz'
            manifest['packages'][category]['encodings'] = dict(gzip=dict(
                bytes=compressed.stat().st_size, sha256=compressed_sha, url=compressed_url))
            # Explicit gzip archives work with ordinary HTTP servers as well as R2.
            target = output / compressed_url
            if not cloudflare or output.resolve() == build.resolve():
                target.parent.mkdir(parents=True, exist_ok=True)
                if compressed.resolve() != target.resolve():
                    shutil.copyfile(compressed, target)
        # Compatibility with deployment identity probes.
        if suffix in ('data', 'wasm'):
            manifest[suffix + 'Sha256'] = sha
            manifest[suffix + 'Url'] = url
        if (suffix == 'js' or not cloudflare) and path.resolve() != (output / path.name).resolve():
            shutil.copyfile(path, output / path.name)
    derived = build / 'web-assets/compressed'
    if derived.exists():
        retained = {json.loads(p.read_text())['file'] for p in derived.glob('*.json')}
        for old in derived.glob('*.gz'):
            if old.name not in retained:
                old.unlink()
    payloads = build / 'web-assets/delivery'
    if payloads.exists() and not cloudflare:
        shutil.copytree(payloads, output, dirs_exist_ok=True)
    elif not cloudflare:
        for entry in manifest['assets']:
            if entry['package'] == 'music':
                source, target = build / entry['url'], output / entry['url']
                if source.resolve() != target.resolve():
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(source, target)
    if not cloudflare and (output / 'game-assets').exists():
        wanted = {e['url'] for e in manifest['assets'] if e['package'] == 'music'}
        wanted.update(e['encodings']['gzip']['url'] for e in manifest['packages'].values() if 'encodings' in e)
        for old in (output / 'game-assets').rglob('*'):
            if old.is_file() and old.relative_to(output).as_posix() not in wanted:
                old.unlink()
    bootstrap = build / 'assets.js' if not inventory_path.exists() else Path(__file__).parents[2] / 'mk/emscripten/assets.js'
    manifest['bootstrapSha256'] = digest(bootstrap)
    # The optional input proof is shipped with previews and verified artifact
    # reuse. It never changes the soundtrack inventory or production bindings.
    frontend = build if not inventory_path.exists() else Path(__file__).parents[2] / 'mk/emscripten'
    manifest['frontend'] = {}
    names = ['coop.js', 'coop-controller.html', 'coop-view.js', 'coop-view.html']
    presentation = build / 'web-assets/presentation' if inventory_path.exists() else build
    names += ['coop-scene.json'] + [path.relative_to(presentation).as_posix() for path in sorted((presentation / 'coop-art').glob('*.png'))]
    for name in names:
        source = (presentation if name == 'coop-scene.json' or name.startswith('coop-art/') else frontend) / name
        manifest['frontend'][name] = dict(bytes=source.stat().st_size, sha256=digest(source))
        if source.resolve() != (output / name).resolve():
            (output / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, output / name)
    for old in (output / 'coop-art').glob('*.png'):
        if old.relative_to(output).as_posix() not in names: old.unlink()
    for name in ('coop-controller.html', 'coop-view.html'):
        page = (output / name).read_text()
        for script in ('coop.js', 'coop-view.js'):
            page = re.sub(r'<script src="' + re.escape(script) + r'"[^>]*>', '<script src="' + script + '" integrity="sha256-' + base64.b64encode(bytes.fromhex(manifest['frontend'][script]['sha256'])).decode() + '">', page)
        write(output / name, page.encode())
        manifest['frontend'][name] = dict(bytes=(output / name).stat().st_size, sha256=digest(output / name))
    # The manifest binds code, configuration, inventory, and every payload.
    manifest_bytes = encoded(manifest)
    write(output / 'asset-manifest.json', manifest_bytes)
    manifest_sha = hashlib.sha256(manifest_bytes).hexdigest()
    config = dict(manifestUrl=f'/game-assets/manifest/{manifest_sha}/asset-manifest.json' if cloudflare else 'asset-manifest.json', manifestSha256=manifest_sha,
                  dataUrl=manifest['dataUrl'], wasmUrl=manifest['wasmUrl'])
    marker = '<!-- SUPERTUX_DEPLOY_CONFIG -->'
    html = (build / ('template.html' if inventory_path.exists() else 'index.html')).read_text()
    if marker not in html:
        html, count = re.subn(r'<script>window\.SUPERTUX_DEPLOY_CONFIG = .*?;</script>', marker, html, count=1)
        if count != 1: raise ValueError('Asset bootstrap marker missing from generated HTML')
    html = html.replace(marker, '<script>window.SUPERTUX_DEPLOY_CONFIG = ' + json.dumps(config, separators=(',', ':')) + ';</script>')
    html = re.sub(r'<script src="assets.js"[^>]*>', '<script src="assets.js" integrity="sha256-' + base64.b64encode(bytes.fromhex(manifest['bootstrapSha256'])).decode() + '">', html)
    html = re.sub(r'<script src="coop.js"[^>]*>', '<script src="coop.js" integrity="sha256-' + base64.b64encode(bytes.fromhex(manifest['frontend']['coop.js']['sha256'])).decode() + '">', html)
    for name in ('index.html', 'supertux2.html'):
        write(output / name, html.encode())
    for name in ('supertux2.png', 'supertux2.ico', 'supertux2_bkg.png'):
        if (build / name).resolve() != (output / name).resolve():
            shutil.copyfile(build / name, output / name)
    if bootstrap.resolve() != (output / 'assets.js').resolve():
        shutil.copyfile(bootstrap, output / 'assets.js')
    write(output / 'BUILD_INFO.json', encoded({k: manifest[k] for k in ('sourceCommit', 'configuration', 'toolchain', 'mode', 'inventorySha256')}))
    return manifest


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest='command', required=True)
    prepare_parser = sub.add_parser('prepare')
    prepare_parser.add_argument('--source', type=Path, required=True)
    prepare_parser.add_argument('--output', type=Path, required=True)
    prepare_parser.add_argument('--full-preload', action='store_true')
    prepare_parser.add_argument('--source-commit', required=True)
    prepare_parser.add_argument('--configuration', required=True)
    prepare_parser.add_argument('--runtime-root', required=True)
    prepare_parser.add_argument('--presentation', action='store_true')
    assemble_parser = sub.add_parser('assemble')
    assemble_parser.add_argument('--build', type=Path, required=True)
    assemble_parser.add_argument('--output', type=Path, required=True)
    assemble_parser.add_argument('--cloudflare', action='store_true')
    assemble_parser.add_argument('--no-compression', action='store_true')
    args = parser.parse_args()
    if args.command == 'prepare':
        result = prepare(args.source, args.output, args.full_preload, args.source_commit, args.configuration, args.runtime_root, args.presentation)
    else:
        result = assemble(args.build, args.output, args.cloudflare, not args.no_compression)
    print(json.dumps(dict(mode=result['mode'], startupBytes=sum(a['bytes'] for a in result['assets'] if a['package']=='startup'), deferredBytes=sum(a['bytes'] for a in result['assets'] if a['package']=='music'))))


if __name__ == '__main__':
    main()
