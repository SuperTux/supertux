#!/usr/bin/env python3
"""Conservative, testable PR selection. Unknown files receive platform coverage."""
import argparse
import json
import os
import subprocess

PLATFORMS = ('linux', 'windows', 'macos', 'android', 'freebsd')


def select(paths, full=False):
    result = dict.fromkeys(('tests', 'lint', 'web', 'debug', 'full_native', *PLATFORMS), False)
    if full:
        return dict.fromkeys(result, True)
    for path in paths:
        if path.startswith(('docs/', '.github/ISSUE_TEMPLATE/')) or path.endswith(('.md', '.rst')):
            continue
        result['tests'] = True
        if path.startswith(('worker/', 'tests/worker/')) or path == 'wrangler.toml':
            continue
        if path.startswith(('mk/emscripten/', 'tests/wasm/', 'tools/web/')) or path.endswith(('mobile-web-preview.yml', 'mobile-web-deploy.yml', 'wasm.yml')):
            result['web'] = True
            # Audio/cache and packaging changes must receive Debug before merge.
            if path.startswith('tools/web/') or path.endswith(('assets.js', 'assets_boot.js')):
                result['debug'] = True
            continue
        if path == 'mk/cmake/SuperTux/Emscripten.cmake':
            result['web'] = result['debug'] = result['linux'] = True
            continue
        if path.startswith('data/'):
            result['web'] = result['linux'] = True
            continue
        platform = next((p for p in PLATFORMS if path.startswith('mk/' + p + '/') or path.endswith('/' + ('gnulinux' if p == 'linux' else p) + '.yml')), None)
        if platform:
            result[platform] = True
            continue
        if path.endswith('/lint.yml') or path.startswith('mk/cppcheck/'):
            result['lint'] = True
            continue
        if path.startswith(('src/', 'tests/')):
            result['lint'] = result['web'] = result['linux'] = True
            if path.startswith('src/audio/'):
                result['debug'] = True
            continue
        if path.startswith(('.github/actions/', '.github/workflows/', 'tools/ci/')):
            # Workflow syntax and selection tests run, with affected shared jobs.
            result['web'] = result['lint'] = result['linux'] = True
            continue
        # CMake, external code, manifests, shared tooling, and unknown files.
        result.update(dict.fromkeys(('web', 'debug', 'lint', 'full_native', *PLATFORMS), True))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base')
    parser.add_argument('--head', default='HEAD')
    parser.add_argument('--full', action='store_true')
    args = parser.parse_args()
    # Include both sides of renames: deleting a shared dependency still matters.
    paths = subprocess.check_output(['git', 'diff', '--no-renames', '--name-only', '-z', args.base, args.head]).decode().split('\0') if args.base else []
    result = select([p for p in paths if p], args.full or not args.base)
    print(json.dumps(result, sort_keys=True))
    if os.environ.get('GITHUB_OUTPUT'):
        with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
            for key, value in result.items():
                output.write(f'{key}={str(value).lower()}\n')


if __name__ == '__main__':
    main()
