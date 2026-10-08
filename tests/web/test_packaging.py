import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('packaging', Path(__file__).parents[2] / 'tools/web/package_assets.py')
packaging = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packaging)


class PackagingTests(unittest.TestCase):
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
