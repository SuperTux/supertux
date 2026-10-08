#!/usr/bin/env python3
"""Publish only missing immutable R2 objects; never buffer them in the Worker."""
import argparse
import concurrent.futures
import json
import os
import subprocess
from pathlib import Path
from package_assets import digest
from verify_artifact import verify


def objects(directory, manifest):
    result = {}
    for category, suffix, mime in [('startup', 'data', 'application/octet-stream'), ('wasm', 'wasm', 'application/wasm')]:
        entry = manifest['packages'][category]
        result[f'{suffix}/{entry["sha256"]}/supertux2.{suffix}'] = (directory / ('supertux2.' + suffix), mime)
    for entry in manifest['assets']:
        if entry['package'] == 'music':
            result[entry['url'].removeprefix('game-assets/')] = (directory / entry['url'], 'audio/ogg' if entry['url'].endswith('.ogg') else 'audio/wav')
    path = directory / 'asset-manifest.json'
    result[f'manifest/{digest(path)}/asset-manifest.json'] = (path, 'application/json')
    return result


def upload(key, payload, endpoint, bucket):
    path, mime = payload
    head = subprocess.run(['aws','s3api','head-object','--endpoint-url',endpoint,'--bucket',bucket,'--key',key], capture_output=True, text=True)
    if head.returncode == 0:
        metadata = json.loads(head.stdout)
        if metadata['ContentLength'] != path.stat().st_size or metadata.get('Metadata', {}).get('sha256', digest(path)) != digest(path):
            raise RuntimeError('Existing immutable object has incompatible metadata: ' + key)
        return 'present: ' + key
    if '(404)' not in head.stderr and 'Not Found' not in head.stderr and 'NoSuchKey' not in head.stderr:
        raise RuntimeError('Cannot check R2 object; refusing an unverified upload: ' + key)
    subprocess.run(['aws','s3','cp',str(path),f's3://{bucket}/{key}','--endpoint-url',endpoint,'--only-show-errors','--content-type',mime,'--cache-control','public,max-age=31536000,immutable','--metadata','sha256=' + digest(path)], check=True)
    return 'uploaded: ' + key


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    parser.add_argument('--source-commit', required=True)
    parser.add_argument('--bucket', required=True)
    parser.add_argument('--plan', action='store_true', help='Validate and print publication plan without credentials or writes')
    args = parser.parse_args()
    manifest = verify(args.directory, args.source_commit)
    payloads = objects(args.directory, manifest)
    if args.plan:
        print(json.dumps({k: dict(bytes=v[0].stat().st_size, mime=v[1]) for k,v in payloads.items()}, indent=2))
    else:
        endpoint = f'https://{os.environ["CLOUDFLARE_ACCOUNT_ID"]}.r2.cloudflarestorage.com'
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
            for result in pool.map(lambda item: upload(*item, endpoint, args.bucket), payloads.items()):
                print(result)
