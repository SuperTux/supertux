import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import gzip
import shutil
import sys
sys.path.insert(0, str(Path(__file__).parents[2] / 'tools/web'))
from verify_artifact import verify

spec = importlib.util.spec_from_file_location('packaging', Path(__file__).parents[2] / 'tools/web/package_assets.py')
packaging = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packaging)


class PackagingTests(unittest.TestCase):
    def test_complete_compressed_preview_reuse_and_cloudflare(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory) / 'build'
            source = Path(directory) / 'data'
            source.mkdir()
            (source/'font').write_bytes(b'font data' * 20)
            (source/'music').mkdir()
            (source/'music/track.ogg').write_bytes(b'OggSaudio')
            original = Path(__file__).parents[2] / 'data/images'
            shutil.copytree(original/'creatures/tux',source/'images/creatures/tux')
            (source/'images/tiles/snow').mkdir(parents=True)
            shutil.copyfile(original/'tiles/snow/convex.png',source/'images/tiles/snow/convex.png')
            sha = 'a' * 40
            inventory = packaging.prepare(source,build/'web-assets',source_commit=sha,presentation=True)
            for suffix, data in [('data',b'0'*sum(e['bytes'] for e in inventory['assets'] if e['package']=='startup')),('js',b'code'*100),('wasm',b'wasm'*100)]:
                (build/('supertux2.'+suffix)).write_bytes(data)
            (build/'template.html').write_text('<script src="assets.js"></script><!-- SUPERTUX_DEPLOY_CONFIG -->')
            for name in ('supertux2.png','supertux2.ico','supertux2_bkg.png'):
                (build/name).write_bytes(b'icon')
            preview=Path(directory)/'preview'
            manifest=packaging.assemble(build,preview)
            verify(preview,sha)
            for category,suffix in [('startup','data'),('wasm','wasm'),('javascript','js')]:
                variant=manifest['packages'][category]['encodings']['gzip']
                self.assertEqual(gzip.decompress((preview/variant['url']).read_bytes()),(build/('supertux2.'+suffix)).read_bytes())
            reused=Path(directory)/'reuse'
            self.assertEqual(manifest,packaging.assemble(preview,reused))
            verify(reused,sha)
            scene=json.loads((reused/'coop-scene.json').read_text())
            self.assertEqual(scene['actions']['small-stand-left']['frames'],scene['actions']['small-stand-right']['frames'])
            self.assertTrue(scene['actions']['small-stand-left']['flipX'])
            self.assertEqual(len(scene['tiles']),scene['width']*scene['height'])
            self.assertEqual(scene['tiles'][19*scene['width']],14)
            self.assertEqual(scene['tiles'][20*scene['width']],11)
            cloud=Path(directory)/'cloud'
            cf=packaging.assemble(preview,cloud,cloudflare=True)
            self.assertFalse((cloud/'supertux2.data').exists())
            self.assertFalse((cloud/'game-assets').exists())
            self.assertTrue(cf['packages']['startup']['url'].startswith('/game-assets/data/'))
            packaging.assemble(preview,preview,cloudflare=True)
            verify(preview,sha)
            bad=reused/manifest['packages']['startup']['encodings']['gzip']['url']
            bad.write_bytes(b'partial')
            with self.assertRaises(AssertionError): verify(reused,sha)
            art=reused/scene['tileImage'];art.write_bytes(b'partial')
            with self.assertRaises(AssertionError): verify(reused,sha)
            packaging.assemble(build,build)
            packaging.assemble(build,build,compression=False)
            self.assertFalse(list((build/'game-assets').rglob('*.gz')))
            verify(build,sha)

    def test_signatures_metadata_rollback_and_obsolete_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'data'
            output = Path(directory) / 'derived'
            (source / 'music').mkdir(parents=True)
            (source / 'levels').mkdir()
            (source / 'music/track.bin').write_bytes(b'OggSsome audio')
            (source / 'music/track.music').write_text('(supertux-music (file "track.bin") (loop-begin 2))')
            (source / 'levels/world.stl').write_bytes(b'world')
            args = dict(source_commit='a' * 40, configuration='Release')
            result = packaging.prepare(source, output, **args)
            self.assertEqual([e['path'] for e in result['assets'] if e['package'] == 'music'], ['music/track.bin'])
            self.assertTrue((output / 'startup/music/track.music').exists())
            self.assertFalse((output / 'startup/music/track.bin').exists())
            first = (output / 'inventory.json').read_bytes()
            packaging.prepare(source, output, **args)
            self.assertEqual(first, (output / 'inventory.json').read_bytes())
            packaging.prepare(source, output, full=True, **args)
            self.assertTrue((output / 'startup/music/track.bin').exists())
            self.assertEqual(list((output / 'delivery').rglob('*.ogg')), [])
            packaging.prepare(source, output, **args)
            self.assertFalse((output / 'startup/music/track.bin').exists())
            self.assertEqual((source / 'music/track.bin').read_bytes(), b'OggSsome audio')
            (source / 'music/track.music').write_text('(supertux-music (file "absent.ogg"))')
            with self.assertRaises(ValueError):
                packaging.prepare(source, output, **args)

    def test_hashes_follow_bytes_and_companions_cannot_escape(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'data'
            source.mkdir()
            (source / 'music').mkdir()
            raw = source / 'music/track.ogg'
            raw.write_bytes(b'OggSone')
            one = packaging.prepare(source, Path(directory) / 'one')['assets'][0]
            raw.write_bytes(b'OggStwo')
            two = packaging.prepare(source, Path(directory) / 'two')['assets'][0]
            self.assertNotEqual(one['url'], two['url'])
            (source / 'music/escape.music').write_text('(file "../../outside")')
            with self.assertRaises(ValueError):
                packaging.prepare(source, Path(directory) / 'invalid')


if __name__ == '__main__':
    unittest.main()
