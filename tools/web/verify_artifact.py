#!/usr/bin/env python3
"""Fail closed before reusing or publishing an explicit web artifact."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
from package_assets import digest, encoded


def verify(directory, source_commit, configuration='Release'):
    manifest = json.loads((directory / 'asset-manifest.json').read_text())
    assert manifest['sourceCommit'] == source_commit, 'Artifact source SHA mismatch'
    assert manifest['configuration'] == configuration, 'Artifact configuration mismatch'
    assert manifest['toolchain'] == 'emscripten-6.0.11', 'Artifact toolchain mismatch'
    inventory = {k: manifest[k] for k in ('schema', 'sourceCommit', 'configuration', 'toolchain', 'mode', 'runtimeRoot', 'assets')}
    assert hashlib.sha256(encoded(inventory)).hexdigest() == manifest['inventorySha256'], 'Inventory identity mismatch'
    paths = set()
    for entry in manifest['assets']:
        path = PurePosixPath(entry['path'])
        assert not path.is_absolute() and '..' not in path.parts and str(path) == entry['path'], 'Noncanonical asset path'
        assert entry['path'] not in paths, 'Duplicate runtime path'
        paths.add(entry['path'])
        if entry['package'] == 'music':
            payload = directory / entry['url']
            assert entry['url'].startswith('game-assets/music/' + entry['sha256'] + '/'), 'Incorrect deferred URL'
            assert payload.stat().st_size == entry['bytes'] and digest(payload) == entry['sha256'], 'Deferred asset mismatch'
        else:
            assert entry['package'] == 'startup', 'Unknown package membership'
    assert sum(e['bytes'] for e in manifest['assets'] if e['package'] == 'startup') == manifest['packages']['startup']['bytes'], 'Incomplete startup package'
    for category, suffix in [('startup', 'data'), ('wasm', 'wasm'), ('javascript', 'js')]:
        path = directory / ('supertux2.' + suffix)
        entry = manifest['packages'][category]
        assert path.stat().st_size == entry['bytes'] and digest(path) == entry['sha256'], 'Package hash mismatch: ' + category
    assert digest(directory / 'assets.js') == manifest['bootstrapSha256'], 'Bootstrap hash mismatch'
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    parser.add_argument('--source-commit', required=True)
    parser.add_argument('--configuration', default='Release')
    args = parser.parse_args()
    manifest = verify(args.directory, args.source_commit, args.configuration)
    print('Verified complete', manifest['mode'], args.configuration, 'artifact for', args.source_commit)
