#!/usr/bin/env python3
"""Exact-build readiness for the separate co-op Worker; large payloads use HEAD."""
import argparse
import concurrent.futures
import hashlib
import json
import re
import time
import tomllib
import urllib.request
from pathlib import Path
from urllib.parse import urlsplit
from upload_assets import objects
from verify_artifact import verify

WORKER = 'supertux-private-input-proof'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def check_config(path):
    config = tomllib.loads(path.read_text())
    require(config.get('name') == WORKER, 'Staging must use the separate co-op Worker name')
    require(config.get('main') == 'worker/index.js', 'Staging must publish the tested Worker entrypoint')
    require(not any(key in config for key in ('route', 'routes', 'env', 'services', 'dispatch_namespaces', 'unsafe')),
            'Staging must not change routes, environments, services or dispatch namespaces')
    assets = config.get('assets', {})
    require(assets.get('binding') == 'ASSETS' and assets.get('directory') == './dist', 'Unexpected staging frontend binding')
    require(assets.get('html_handling') == 'none', 'Staging must preserve canonical frontend paths')
    require(set(assets.get('run_worker_first', [])) == {'/coop/*', '/game-assets/*', '/coop-art/*'}, 'Incomplete staging routing')
    require(config.get('r2_buckets') == [{'binding': 'GAME_ASSETS', 'bucket_name': 'supertux-assets'}], 'Unexpected private R2 binding')
    require(config.get('durable_objects', {}).get('bindings') == [{'name': 'COOP_ROOMS', 'class_name': 'CoopRoom'}], 'Missing co-op room binding')
    require(any('CoopRoom' in migration.get('new_sqlite_classes', []) for migration in config.get('migrations', [])), 'Missing SQLite room migration')


def deployment_url(log):
    matches = re.findall(r'https://' + WORKER + r'\.[a-z0-9-]+\.workers\.dev\b', log)
    require(bool(matches), 'Wrangler did not report the separate co-op workers.dev URL')
    return matches[-1]


def origin(url):
    parts = urlsplit(url)
    require(parts.scheme == 'https' and re.fullmatch(WORKER + r'\.[a-z0-9-]+\.workers\.dev', parts.netloc) and
            parts.path in ('', '/') and not parts.query and not parts.fragment, 'Expected the separate HTTPS co-op origin')
    return 'https://' + parts.netloc


def fetch(base, path, deadline, method='GET', limit=4 * 1024 * 1024, data=None):
    remaining = deadline - time.monotonic()
    require(remaining > 0, 'Staging readiness deadline exceeded')
    headers = {'Accept-Encoding': 'identity',
               'User-Agent': 'SuperTux-Coop-Validation/1.0 (+https://github.com/jbbejena/supertux)'}
    if data is not None:
        headers.update({'Content-Type': 'application/json', 'Origin': base})
    request = urllib.request.Request(base + '/' + path.lstrip('/'), headers=headers, data=data, method=method)
    with urllib.request.urlopen(request, timeout=min(10, remaining)) as response:
        require(urlsplit(response.url).netloc == urlsplit(base).netloc, 'Unexpected cross-origin redirect')
        body = b'' if method == 'HEAD' else response.read(limit + 1)
        require(len(body) <= limit, 'Unexpectedly large staging response')
        return response.status, response.headers, body


def checked_file(base, path, entry, deadline):
    _, headers, body = fetch(base, path, deadline, limit=entry['bytes'])
    require(len(body) == entry['bytes'] and hashlib.sha256(body).hexdigest() == entry['sha256'], 'Hosted frontend content mismatch: ' + path)
    expected = 'image/png' if path.endswith('.png') else 'application/json' if path.endswith('.json') else 'text/html' if path.endswith('.html') else 'javascript'
    mime = headers.get('Content-Type', '').split(';')[0].lower()
    require(mime in ('text/javascript', 'application/javascript') if expected == 'javascript' else mime == expected, 'Frontend MIME mismatch: ' + path)
    if path.startswith('coop-art/'):
        require('immutable' in headers.get('Cache-Control', ''), 'Presentation artwork must be immutable')


def checked_payload(base, item, deadline):
    key, (path, mime) = item
    _, headers, _ = fetch(base, 'game-assets/' + key, deadline, method='HEAD')
    require(headers.get('Content-Length') == str(path.stat().st_size), 'Incomplete hosted payload: ' + key)
    require(headers.get('Content-Type', '').split(';')[0].lower() == mime, 'Payload MIME mismatch: ' + key)
    require('immutable' in headers.get('Cache-Control', ''), 'Payload must be immutable: ' + key)


def verify_once(base, directory, frontend, expected, payloads, deadline):
    _, _, remote = fetch(base, 'asset-manifest.json', deadline)
    require(json.loads(remote) == expected, 'Hosted manifest does not match the exact tested build')
    _, headers, page = fetch(base, '', deadline, limit=2 * 1024 * 1024)
    require(headers.get('Content-Type', '').startswith('text/html'), 'Host page is not HTML')
    match = re.search(r'window\.SUPERTUX_DEPLOY_CONFIG\s*=\s*(\{[^;]+\});', page.decode())
    config = json.loads(match[1]) if match else {}
    expected_page = (frontend / 'index.html').read_text()
    expected_match = re.search(r'window\.SUPERTUX_DEPLOY_CONFIG\s*=\s*(\{[^;]+\});', expected_page)
    require(expected_match and config == json.loads(expected_match[1]), 'Hosted bootstrap selects a different build')
    _, _, immutable_manifest = fetch(base, config['manifestUrl'], deadline)
    require(hashlib.sha256(immutable_manifest).hexdigest() == config['manifestSha256'] and json.loads(immutable_manifest) == expected,
            'Immutable manifest identity mismatch')
    checked_file(base, 'assets.js', {'bytes': (directory / 'assets.js').stat().st_size, 'sha256': expected['bootstrapSha256']}, deadline)
    checked_file(base, 'supertux2.js', expected['packages']['javascript'], deadline)
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(lambda item: checked_file(base, *item, deadline), expected['frontend'].items()))
        list(pool.map(lambda item: checked_payload(base, item, deadline), payloads.items()))
    # Exercise the binding and origin policy without exposing role credentials.
    status, _, body = fetch(base, 'coop/rooms', deadline, method='POST', limit=4096,
                            data=json.dumps({'protocol': 2, 'build': config['manifestSha256']}).encode())
    room = json.loads(body)
    require(status == 201 and room.get('protocol') == 2 and room.get('build') == config['manifestSha256'] and
            re.fullmatch('[a-f0-9]{32}', room.get('room', '')) and
            all(re.fullmatch('[a-f0-9]{64}', room.get(role, '')) for role in ('host', 'guest')) and
            room['host'] != room['guest'], 'Co-op room creation or role binding failed')
    return {'url': base, 'sourceCommit': expected['sourceCommit'], 'frontendFiles': len(expected['frontend']),
            'immutablePayloads': len(payloads), 'roomCreation': True}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check-config', type=Path)
    parser.add_argument('--deployment-log', type=Path)
    parser.add_argument('--url')
    parser.add_argument('--directory', type=Path)
    parser.add_argument('--frontend', type=Path)
    args = parser.parse_args()
    if args.check_config:
        check_config(args.check_config)
        print('Verified isolated co-op Worker configuration')
    elif args.deployment_log:
        print('url=' + deployment_url(args.deployment_log.read_text()))
    else:
        require(args.url and args.directory and args.frontend, 'Provide URL, complete artifact and published frontend')
        base = origin(args.url)
        expected = json.loads((args.frontend / 'asset-manifest.json').read_text())
        require(verify(args.directory, expected['sourceCommit']) == expected, 'Published frontend differs from the complete verified artifact')
        payloads = objects(args.directory, expected)
        deadline = time.monotonic() + 300
        while True:
            try:
                print(json.dumps(verify_once(base, args.directory, args.frontend, expected, payloads, deadline)), flush=True)
                break
            except (OSError, ValueError) as error:
                if deadline - time.monotonic() <= 3:
                    raise
                print('Waiting for the exact staging build:', error, flush=True)
                time.sleep(min(3, deadline - time.monotonic()))


if __name__ == '__main__':
    main()
