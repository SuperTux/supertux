"""Derived web-only artwork for the arena and the supported campaign level.

Original source assets are copied unchanged; only the host simulates gameplay.
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
        if token.startswith('"'): return json.loads(token, strict=False)
        return token
    return read(next(token for token in tokens if not token.startswith(';')))


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
    # Explicit Phase 7B content inventory. Tile artwork comes from the real
    # tileset entries used by this level, including tiles converted to objects.
    level_source = source / 'levels/world1/welcome_antarctica.stl'
    parsed_level = sexp(level_source.read_text())
    sector = next(item for item in parsed_level[1:] if item[0] == 'sector')
    objects = {}
    tile_ids = set()
    paths = set()
    def leaves(node):
        if isinstance(node, list):
            for item in node: yield from leaves(item)
        else: yield node
    for item in sector[1:]:
        objects[item[0]] = objects.get(item[0], 0) + 1
        if item[0] == 'tilemap':
            fields = {field[0]: field[1:] for field in item[1:]}
            encoded_tiles = iter(map(int, fields['tiles']))
            count = 0
            for value in encoded_tiles:
                run = 1
                if value < 0: run, value = -value, next(encoded_tiles)
                if value < 0: raise ValueError('Invalid run-length tile value')
                tile_ids.add(value)
                count += run
            if count != int(fields['width'][0]) * int(fields['height'][0]):
                raise ValueError('Incomplete campaign tilemap')
        for value in leaves(item):
            if value.lstrip('/').startswith('images/') and value.endswith('.png'):
                paths.add(value.lstrip('/'))
    for item in sexp((source / 'images/tiles.strf').read_text())[1:]:
        if not isinstance(item, list) or item[0] not in ('tile', 'tiles'): continue
        ids = {int(v) for field in item[1:] if isinstance(field,list) and field[0] in ('id','ids') for v in field[1:]}
        if ids & tile_ids:
            for value in leaves(item):
                if value.endswith('.png'): paths.add('images/' + value)
    # Includes all animations/bonus poses, generated projectiles, block debris,
    # death/explosion effects and checkpoint particles for the audited families.
    folders = ['creatures/tux','creatures/snowball','creatures/iceblock',
               'creatures/mr_bomb','creatures/jumpy','creatures/stalactite',
               'objects/coin','objects/bonus_block','objects/weak_block',
               'objects/resetpoints','objects/explosion','objects/bullets',
               'objects/water_drop','particles','powerups','decal/explanations','engine/hud']
    for folder in folders:
        paths.update(path.relative_to(source).as_posix() for path in (source / 'images' / folder).rglob('*.png'))
    paths.update(path.relative_to(source).as_posix() for path in (source / 'images/tiles/blocks').glob('brick_piece*.png'))
    textures = {path: image(source / path) for path in sorted(paths)}
    fixture['campaign'] = dict(id='antarctica-v1', path='levels/world1/welcome_antarctica.stl',
                              sourceSha256=hashlib.sha256(level_source.read_bytes()).hexdigest(),
                              objects=objects, tileIds=sorted(tile_ids), textures=textures)
    fixture.update(tiles=tiles, actions=actions, tileImage=image(source / 'images/tiles/snow/convex.png'),
                   tileRegions={'14': [32,32,32,32], '11': [32,64,32,32]}, schema=1)
    write(art / 'coop-scene.json', encoded(fixture))
    assets['coop-scene.json'] = dict(bytes=(art/'coop-scene.json').stat().st_size,
                                    sha256=hashlib.sha256((art/'coop-scene.json').read_bytes()).hexdigest())
    for old in art.rglob('*'):
        if old.is_file() and old.relative_to(art).as_posix() not in assets: old.unlink()
    return [level_path], assets
