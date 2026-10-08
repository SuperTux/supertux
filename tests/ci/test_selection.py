import importlib.util
import pathlib
import unittest

spec = importlib.util.spec_from_file_location('selection', pathlib.Path(__file__).parents[2] / 'tools/ci/select_checks.py')
selection = importlib.util.module_from_spec(spec)
spec.loader.exec_module(selection)


class SelectionTests(unittest.TestCase):
    def test_change_classes(self):
        cases = [
            ('docs/WEB.md', set()),
            ('mk/emscripten/browser.js', {'tests', 'web'}),
            ('worker/index.js', {'tests'}),
            ('wrangler.toml', {'tests'}),
            ('tools/web/package_assets.py', {'tests', 'web', 'debug'}),
            ('tests/web/test_packaging.py', {'tests', 'web'}),
            ('src/audio/sound_manager.cpp', {'tests', 'lint', 'linux', 'web', 'debug'}),
            ('src/object/player.cpp', {'tests', 'lint', 'linux', 'web'}),
            ('vcpkg.json', set(selection.select([], True))),
            ('mk/cmake/SuperTux/Emscripten.cmake', {'tests', 'web', 'debug', 'linux'}),
            ('CMakeLists.txt', set(selection.select([], True))),
            ('external/SDL_ttf', set(selection.select([], True))),
            ('mk/android/build.gradle', {'tests', 'android'}),
            ('.github/workflows/windows.yml', {'tests', 'windows'}),
            ('.github/workflows/wasm.yml', {'tests', 'web'}),
            ('.github/workflows/validation.yml', {'tests', 'lint', 'linux', 'web'}),
            ('unclassified-file', set(selection.select([], True))),
        ]
        for path, wanted in cases:
            with self.subTest(path=path):
                self.assertEqual({k for k, v in selection.select([path]).items() if v}, wanted)

    def test_union_and_full(self):
        result = selection.select(['worker/index.js', 'src/object/player.cpp', 'mk/windows/foo'])
        self.assertTrue(result['linux'] and result['windows'] and result['web'])
        self.assertTrue(all(selection.select(['docs/README.md'], True).values()))


if __name__ == '__main__':
    unittest.main()
