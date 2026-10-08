"""Derived web-only fixture and authentic sprite presentation; no source edits.

The controlled scene has static solid snow tiles, two players and a normal
camera. No scripts, pickups, enemies or progression. Only the host runs it.
"""
import copy
import hashlib
import json
from pathlib import Path
import re


def sexp(text):
    tokens = iter(re.findall(r';[^\n]*|"(?:\\.|[^"\\])*"|[()]|[^\s();]+', text))
    def read(token):
        if token == '(':
            result = []
            for item in tokens:
                if item.startswith(';'): continue
                if item == ')': return result
                result.append(read(item))
            raise ValueError('Unclosed expression')
        if token.startswith('"'): return json.loads(token)
        return token
    return read(next(tokens))


def derive(source, output, write, encoded):
    fixture = json.loads((Path(__file__).parent / 'coop/scene.json').read_text())
    tiles = [0] * (fixture['width'] * fixture['height'])
    for x, y, width, height in [fixture['ground'], *fixture['platforms']]:
        for row in range(y, y + height):
            for col in range(x, x + width):
                # ID 14 is the solid snow top; 11 the solid fill, from tiles.strf.
                tiles[row * fixture['width'] + col] = 14 if row == y else 11
    sky = ' '.join(map(str, fixture['sky']))
    level = f'''(supertux-level (version 3) (name "{fixture['name']}")
      (author "SuperTux contributors") (license "CC-BY-SA 4.0 International")
      (sector (name "main") (ambient-light (color 1 1 1))
        (gradient (top_color {sky}) (bottom_color {sky}))
        (camera (mode "normal"))
        (spawnpoint (name "main") (x {fixture['spawn'][0]}) (y {fixture['spawn'][1]}))
        (tilemap (width {fixture['width']}) (height {fixture['height']})
          (solid #t) (z-pos 0) (tiles {' '.join(map(str, tiles))}))))\n'''
    level_path = output / 'startup/levels/web/coop-view.stl'
    write(level_path, level.encode())
    art = output / 'presentation'
    assets = {}
    def image(path):
        content = path.read_bytes()
        sha = hashlib.sha256(content).hexdigest()
        url = 'coop-art/' + sha + '.png'
        write(art / url, content)
        assets[url] = dict(bytes=len(content), sha256=sha)
        return url
    actions = {}
    root = source / 'images/creatures/tux'
    for item in sexp((root / 'tux.sprite').read_text())[1:]:
        if item[0] != 'action': raise ValueError('Unsupported sprite field')
        fields = {v[0]: v[1:] for v in item[1:]}
        name = fields['name'][0]
        # The fixture never grants bonuses. Include every small pose and death.
        if not (name.startswith('small-') or name == 'gameover'): continue
        action = dict(offset=list(map(float, fields.get('hitbox', ['0','0'])[:2])), flipX=False, flipY=False)
        inherited = next((key for key in ('mirror-action','flip-action','clone-action') if key in fields), None)
        if inherited:
            parent = actions[fields[inherited][0]]
            action['frames'] = copy.deepcopy(parent['frames'])
            action['flipX'], action['flipY'] = parent['flipX'], parent['flipY']
            if inherited == 'mirror-action': action['flipX'] = not action['flipX']
            if inherited == 'flip-action': action['flipY'] = not action['flipY']
            if 'hitbox' not in fields or inherited == 'clone-action': action['offset'] = parent['offset'][:]
        else:
            if 'regions' in fields or 'images' not in fields: raise ValueError('Unsupported presentation sprite')
            action['frames'] = [image(root / path) for path in fields['images']]
        actions[name] = action
    fixture.update(tiles=tiles, actions=actions, tileImage=image(source / 'images/tiles/snow/convex.png'),
                   tileRegions={'14': [32,32,32,32], '11': [32,64,32,32]}, schema=1)
    write(art / 'coop-scene.json', encoded(fixture))
    assets['coop-scene.json'] = dict(bytes=(art/'coop-scene.json').stat().st_size,
                                    sha256=hashlib.sha256((art/'coop-scene.json').read_bytes()).hexdigest())
    for old in art.rglob('*'):
        if old.is_file() and old.relative_to(art).as_posix() not in assets: old.unlink()
    return [level_path], assets
