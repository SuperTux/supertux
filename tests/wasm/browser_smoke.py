#!/usr/bin/env python3
"""Real game/browser smoke. Install playwright and Pillow; serve a complete WASM build.

The test changes only the served HTML's Module.arguments to use the existing CLI.
It does not add gameplay exports or alter the compiled game.
"""
import argparse
import asyncio
import functools
import http.server
import io
import json
import re
import threading
from pathlib import Path

from PIL import Image, ImageChops
from playwright.async_api import async_playwright

# Observed on the unchanged audit source with the pinned Debug SDK. Instrumentation
# stays enabled. This opt-in records these exact sites; new diagnostics still fail.
KNOWN_UPSTREAM_UB = [
    r"/__utility/swap\.h:43:11: runtime error: load of value \d+, which is not a valid value for type '__libcpp_remove_reference_t<bool &>' \(aka 'bool'\)",
    r"/(?:external/obstack/obstack\.c:(?:143:35|236:5|263:7)|src/util/obstackpp\.hpp:24:10): runtime error: subtraction of unsigned offset from 0x00000000 overflowed to 0x[0-9a-f]+",
]


class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


async def smoke(args, url, data_dir):
    evidence = []
    errors = []
    keyboard_samples = []
    position_pattern = re.compile(r'\[SCRIPTING\] PHASE1_POSITION=([-0-9.]+),([-0-9.]+),(true|false),(true|false),(true|false)')
    report = {"checks": [], "artifact_sizes": {}}
    for suffix in ("html", "js", "wasm", "data", "png", "ico"):
        artifact = args.build / f"supertux2.{suffix}"
        assert artifact.stat().st_size > 0, f"Missing artifact: {artifact}"
        report["artifact_sizes"][artifact.name] = artifact.stat().st_size
    html = (args.build / "supertux2.html").read_text()

    def log_console(name, message):
        line = f"{name}: {message.type}: {message.text}"
        evidence.append(line)
        with (args.output / "console.log").open("a") as log:
            log.write(line + "\n")
        match = position_pattern.search(message.text)
        if match:
            x, y, left, right, jump = match.groups()
            keyboard_samples.append(dict(x=float(x), y=float(y), left=left == 'true',
                                         right=right == 'true', jump=jump == 'true'))

    async with async_playwright() as playwright:
        launch = {}
        if args.browser == 'chromium':
            launch = dict(executable_path=args.chromium,
                          args=["--no-sandbox", "--use-angle=swiftshader", "--enable-unsafe-swiftshader"])
        elif args.webkit_executable:
            launch['executable_path'] = args.webkit_executable
        browser = await getattr(playwright, args.browser).launch(**launch)
        report["browser"] = browser.version
        report['browser_engine'] = args.browser

        async def boot(context, name, level=False, memory=False):
            page = await context.new_page()
            page.set_default_timeout(60000)
            page.on("console", lambda message: log_console(name, message))
            page.on("pageerror", lambda error: errors.append(f"{name}: {error}"))
            page.on("response", lambda response: errors.append(f"{name}: HTTP {response.status}: {response.url}")
                    if response.status >= 400 else None)
            # Observe the game's actual audio engine; do not create another
            # context or substitute playback. Count real scheduled buffer starts.
            await page.add_init_script("""(() => {
                const Audio = window.AudioContext;
                window.phase2AudioContexts = [];
                window.AudioContext = class extends Audio {
                    constructor(...args) {
                        super(...args);
                        window.phase2AudioContexts.push(this);
                        this.phase2StartedBuffers = 0;
                        const create = this.createBufferSource.bind(this);
                        this.createBufferSource = () => {
                            const source = create(), start = source.start.bind(source);
                            source.start = (...args) => { ++this.phase2StartedBuffers; return start(...args); };
                            return source;
                        };
                    }
                };
            })();""")
            arguments = ["--verbose"]
            if level:
                # Use existing read-only scripting access to measure keyboard
                # movement. No test-only exports are compiled into the game.
                arguments.append("--developer")
                arguments.append(data_dir + "/" + args.level)
            body = html.replace("var Module = {", "var Module = {\narguments: " + json.dumps(arguments) + ",", 1)
            await page.route("**/supertux2.html", lambda route: route.fulfill(body=body, content_type="text/html"))
            await page.goto(url)
            await page.wait_for_function("window.Module && Module.supertuxReady === true", timeout=180000)
            mode = await page.evaluate("Module.supertuxStorage.state")
            assert mode == ("memory" if memory else "indexeddb"), (name, mode)
            assert not await page.evaluate('Module.supertuxShell.active')
            # WebKit can report interrupted before the first gesture, including
            # when no physical audio device is present. Both states prohibit
            # audible playback; a running context still fails this gate.
            await page.wait_for_function("phase2AudioContexts.length === 1 && ['suspended','interrupted'].includes(phase2AudioContexts[0].state)")
            initial_audio = await page.evaluate('phase2AudioContexts[0].state')
            await page.locator('#start_button').click()
            await page.wait_for_function('Module.supertuxShell.active')
            await page.wait_for_function("phase2AudioContexts[0].state === 'running' && phase2AudioContexts[0].phase2StartedBuffers > 0")
            assert await page.locator("#overlay").evaluate("element => getComputedStyle(element).display") == "none"
            await page.locator("#canvas").focus()
            report["checks"].append(name + ": boot and storage " + mode + "; blocked initial audio " + initial_audio)
            return page

        async def capture(page, filename, uncovered=False):
            if uncovered:
                # Locator screenshots include covering HTML. Hide the animated
                # prompt only for readback; shell/input/simulation remain paused.
                await page.locator('#overlay').evaluate("e => e.style.visibility = 'hidden'")
            try:
                data = await page.locator("#canvas").screenshot(path=str(args.output / filename))
            finally:
                if uncovered:
                    await page.locator('#overlay').evaluate("e => e.style.visibility = ''")
            image = Image.open(io.BytesIO(data)).convert("RGB")
            assert len(image.getcolors(image.width * image.height)) > 20, "Canvas has no rendered image/font content"
            return image

        context = await browser.new_context(viewport={"width": 960, "height": 600})
        page = await boot(context, "fresh-menu")
        await capture(page, "menu.png")
        assert sum("Browser storage hydrated before main:" in line for line in evidence) == 1
        report["renderer"] = next(line for line in evidence if "SDL_Renderer:" in line)
        # C++ serializes a valid config. Commit a changed setting and a progression
        # fixture, then verify hydration on reload and C++ consumption of the setting.
        fixture = '(supertux-savegame (version 2) (save (save-version 2) (tux (num_players 1) (bonus "none") (coins 17)) (state)))\n'
        committed = await page.evaluate(r"""async fixture => {
            Module.ccall('save_config', null, [], []);
            const fs = Module.FS, root = Module.supertuxStorage.root;
            const config = fs.readFile(root + 'config', { encoding: 'utf8' });
            if (!config.includes('(music_volume ')) throw Error('No serialized music setting');
            fs.writeFile(root + 'config', config.replace(/\(music_volume [^)]+\)/, '(music_volume 37)'));
            fs.mkdirTree(root + 'profile1');
            fs.writeFile(root + 'profile1/phase1-fixture.stsg', fixture);
            localStorage.setItem('supertux2_config', config.replace(/\(music_volume [^)]+\)/, '(music_volume 12)'));
            const committed = await Module.supertuxStorage.flush();
            // This is a seeded fixture, not the running C++ config. Disarm unload
            // writes so that closing this seed page cannot replace it with defaults.
            const call = Module.ccall;
            Module.ccall = (name, ...args) => name === 'save_config' ? undefined : call(name, ...args);
            window.supertux_saveFiles = () => {};
            return committed;
        }""", fixture)
        assert committed
        await page.close()  # No reliance on unload to commit the fixture.
        page = await boot(context, "persisted-menu")
        assert await page.evaluate("""fixture => {
            Module.ccall('save_config', null, [], []);
            const fs = Module.FS, root = Module.supertuxStorage.root;
            return fs.readFile(root + 'config', { encoding: 'utf8' }).includes('(music_volume 37)') &&
                   fs.readFile(root + 'profile1/phase1-fixture.stsg', { encoding: 'utf8' }) === fixture;
        }""", fixture)
        report["checks"].append("IndexedDB setting consumed by C++; progression fixture bytes survive reload; hydrated config beats stale backup")

        # Actual game runtime must recover after both synchronous and callback errors.
        assert await page.evaluate("""async () => {
            const fs = Module.FS, sync = fs.syncfs;
            try {
                fs.syncfs = () => { throw Error('phase1 forced synchronous flush failure'); };
                if (await Module.supertuxStorage.flush()) return false;
                fs.syncfs = (_, callback) => callback(Error('phase1 forced callback flush failure'));
                if (await Module.supertuxStorage.flush()) return false;
            } finally { fs.syncfs = sync; }
            return await Module.supertuxStorage.flush();
        }""")
        report["checks"].append("Real runtime flush errors release guard and allow retry")
        await page.close()

        page = await boot(context, "keyboard-level", level=True)
        # Human-duration key events are needed: a down/up pair in one simulation
        # step can disappear from this game's pressed-action input abstraction.
        await page.keyboard.press("Enter", delay=150)
        await page.wait_for_function("document.querySelector('#output').textContent.includes('Playing')", timeout=60000)

        # Prepare one read-only observer through the normal developer console
        # while the ordinary pause menu stops enemies. Repeated console typing
        # during live play can outlast a jump or let Tux die on a slow build.
        await page.keyboard.press('Escape', delay=150)
        await page.keyboard.press('Backquote', delay=100)
        getters = ' + "," + '.join(['sector.Tux.get_x()', 'sector.Tux.get_y()'] +
            [f'sector.Tux.get_input_held("{key}")' for key in ['left', 'right', 'jump']])
        await page.keyboard.type('phase1Observer <- newthread(function(){for(local i=0;i<6000;i++){print("PHASE1_POSITION=" + ' + getters + ');wait(0.05);}});phase1Observer.call();', delay=1)
        await page.keyboard.press('Enter', delay=100)
        await page.keyboard.press('Backquote', delay=100)
        await page.keyboard.press('Escape', delay=150)

        async def snapshot(label, observe=lambda sample: True):
            start = len(keyboard_samples)
            try:
                async with asyncio.timeout(10):
                    while len(keyboard_samples) <= start or not observe(keyboard_samples[-1]):
                        await page.wait_for_timeout(50)
            except TimeoutError as error:
                raise AssertionError((label, keyboard_samples[-1] if keyboard_samples else None)) from error
            sample = keyboard_samples[-1]
            assert sample['y'] < 800, (label, 'Player died during input checks', sample)
            return sample

        async def position(label, axis):
            return (await snapshot(label))[axis]

        await snapshot('observer-ready')

        # Freeze the actual simulation with a key held; lifecycle return must
        # retain an explicit Resume gate and clear the missing key-up.
        await page.keyboard.down('ArrowRight')
        await page.wait_for_timeout(100)
        await page.evaluate("window.dispatchEvent(new Event('blur'))")
        await page.wait_for_function("Module.supertuxShell.audioState === 'suspended'")
        await page.screenshot(path=str(args.output / 'resume-prompt.png'))
        frozen = await capture(page, 'shell-paused.png', uncovered=True)
        await page.wait_for_timeout(500)
        still = await capture(page, 'shell-still-paused.png', uncovered=True)
        assert ImageChops.difference(frozen, still).getbbox() is None, 'Simulation drew/advanced while shell paused'
        await page.evaluate("window.dispatchEvent(new PageTransitionEvent('pageshow', {persisted: true}))")
        # The synthetic blur was also consumed by SDL. Model the matching window
        # focus event on return; pageshow alone does not restore SDL text focus.
        await page.evaluate("window.dispatchEvent(new Event('focus'))")
        assert not await page.evaluate('Module.supertuxShell.active')
        await page.locator('#start_button').click()
        await page.wait_for_function('Module.supertuxShell.active')
        # Do not send ArrowRight up yet. The reset must neutralize a held action
        # even when a browser never delivered that release. Allow friction to settle.
        await page.wait_for_timeout(600)
        x_reset = await position('x_reset', 'x')
        await page.wait_for_timeout(500)
        x_still = await position('x_still', 'x')
        assert abs(x_still - x_reset) < 5, (x_reset, x_still)
        await page.keyboard.up('ArrowRight')
        # Interrupt the real context while foregrounded, without browser blur.
        await page.evaluate('phase2AudioContexts[0].suspend()')
        await page.wait_for_function('!Module.supertuxShell.active')
        await page.locator('#start_button').click()
        await page.wait_for_function("Module.supertuxShell.active && Module.supertuxShell.audioState === 'running'")
        # Rejection in the real engine wrapper must remain a recoverable UX.
        await page.evaluate("""() => {
            window.dispatchEvent(new Event('blur'));
            window.dispatchEvent(new Event('focus'));
            const context = phase2AudioContexts[0];
            window.phase2Resume = context.resume.bind(context);
            context.resume = () => Promise.reject(Error('phase2 denied audio resume'));
        }""")
        await page.locator('#start_button').click()
        await page.locator('#play_muted').wait_for(state='visible')
        assert not await page.evaluate('Module.supertuxShell.active')
        await page.locator('#play_muted').click()
        await page.wait_for_function("Module.supertuxShell.active && Module.supertuxShell.audioState === 'suspended'")
        await page.evaluate("phase2AudioContexts[0].resume = window.phase2Resume")
        await page.evaluate("window.dispatchEvent(new Event('blur')); window.dispatchEvent(new Event('focus'))")
        await page.locator('#start_button').click()
        await page.wait_for_function("Module.supertuxShell.active && Module.supertuxShell.audioState === 'running'")
        report['checks'].append('Actual level frozen across blur/pageshow; repeated gesture resumes real OpenAL context; missing key-up neutralized')
        report['checks'].append('Real AudioContext rejection shows retry/muted choice; muted play stays suspended; later gesture restores audio')

        x_before = await position("x_before", "x")
        y_ground = await position("y_ground", "y")
        before = await capture(page, "level-before.png")
        await page.keyboard.down("Space")
        try:
            y_jump = (await snapshot('jump', lambda sample: sample['jump'] and sample['y'] < y_ground - 10))['y']
        finally:
            await page.keyboard.up("Space")
        assert y_jump < y_ground - 10, (y_ground, y_jump)
        await snapshot('landed', lambda sample: not sample['jump'] and sample['y'] >= y_ground - 1)
        await page.keyboard.down("ArrowLeft")
        try:
            x_after = (await snapshot('left', lambda sample: sample['left'] and sample['x'] < x_before - 50))['x']
        finally:
            await page.keyboard.up("ArrowLeft")
        assert x_after < x_before - 50, (x_before, x_after)
        report["keyboard_positions"] = {"x_before": x_before, "x_after": x_after, "y_ground": y_ground, "y_jump": y_jump}
        after = await capture(page, "level-after.png")
        assert ImageChops.difference(before, after).getbbox() is not None
        await page.keyboard.press("Escape", delay=150)
        await capture(page, "level-paused.png")
        report["checks"].append("Real level entered; keyboard movement/jump measured through existing read-only console methods; pause screenshot captured")
        await page.close()
        await context.close()

        mobile = await browser.new_context(viewport={'width': 390, 'height': 844},
                                           device_scale_factor=3, is_mobile=True, has_touch=True)
        page = await boot(mobile, 'mobile-shell-menu')
        report['mobile_layouts'] = []
        for width, height in [(390, 844), (844, 390), (844, 320), (390, 700), (390, 844)]:
            await page.set_viewport_size({'width': width, 'height': height})
            await page.wait_for_function("""size => {
                const canvas = Module.canvas, rect = document.getElementById('game_area').getBoundingClientRect();
                return canvas.width === size[0] && canvas.height === size[1] &&
                       canvas.width === Math.floor(rect.width) && canvas.height === Math.floor(rect.height);
            }""", arg=[width, height])
            dimensions = await page.evaluate("""() => ({
                width: Module.canvas.width, height: Module.canvas.height,
                css: Module.canvas.getBoundingClientRect().toJSON(), dpr: devicePixelRatio,
                scrollWidth: document.documentElement.scrollWidth, innerWidth
            })""")
            assert dimensions['width'] == width and dimensions['height'] == height, dimensions
            assert dimensions['scrollWidth'] == dimensions['innerWidth'], dimensions
            report['mobile_layouts'].append(dimensions)
            await page.wait_for_timeout(200) # Allow a real frame at the new viewport.
            rendered = await capture(page, f'mobile-{width}x{height}.png')
            # Dimension equality alone misses SDL3 scaling the viewport twice.
            # This title backdrop should span both horizontal edges, with only
            # vertical letterboxing in these sizes. Require a substantial band
            # of colored game pixels on each side, not merely one cursor pixel.
            for fraction in (0.03, 0.97):
                column = int(rendered.width * fraction)
                colored = sum(max(rendered.getpixel((column, y))) > 30 for y in range(rendered.height))
                assert colored > rendered.height * .1, (width, height, 'clipped rendered viewport', fraction, colored)
        await capture(page, 'mobile-portrait.png')
        # Test the actual CSS-padded inner area, modeling nonzero safe insets.
        await page.locator('#game_shell').evaluate("e => e.style.padding = '10px 20px 30px 40px'")
        await page.evaluate("window.dispatchEvent(new Event('resize'))")
        await page.wait_for_function('Module.canvas.width === 330 && Module.canvas.height === 804')
        await capture(page, 'mobile-safe-area-fixture.png')
        await mobile.close()
        report['checks'].append(args.browser + ' mobile emulation DPR 3: portrait/landscape/toolbar-size sequence, CSS-safe-area fixture; backing resolution uses CSS pixels')

        denied = await browser.new_context(viewport={"width": 960, "height": 600})
        await denied.add_init_script("Object.defineProperty(window, 'indexedDB', {get() { throw Error('phase1 denied IndexedDB'); }});")
        page = await boot(denied, "denied-storage-menu", memory=True)
        assert await page.locator("#storage_warning").is_visible()
        warning_box = await page.locator("#storage_warning").bounding_box()
        assert 0 <= warning_box["y"] and warning_box["y"] + warning_box["height"] <= 600
        assert not await page.evaluate("Module.supertuxStorage.flush()")
        await page.screenshot(path=str(args.output / "memory-fallback.png"))
        await denied.close()
        await browser.close()

    diagnostics = [line for line in evidence if 'runtime error:' in line]
    known = []
    if args.record_known_ub:
        known = [line for line in diagnostics if any(re.search(pattern, line) for pattern in KNOWN_UPSTREAM_UB)]
    fatal = [line for line in evidence if re.search(r"undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function", line) and line not in known]
    assert not errors and not fatal, errors + fatal
    report["known_upstream_sanitizer_diagnostics"] = known
    for line in known:
        print('::warning title=Existing upstream UBSan diagnostic::' + line, flush=True)
    report["checks"].append("No HTTP, uncaught JavaScript, unresolved-symbol, fatal game, or new sanitizer errors")
    (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    return evidence, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path)
    parser.add_argument("--output", type=Path, default=Path("wasm-browser-evidence"))
    parser.add_argument("--chromium", help="Installed executable; omit to use Playwright Chromium")
    parser.add_argument('--browser', choices=['chromium', 'webkit'], default='chromium')
    parser.add_argument('--webkit-executable', help='Optional local WebKit launcher')
    parser.add_argument("--level", default="levels/world1/welcome_antarctica.stl", help="Real level for the gameplay checks")
    parser.add_argument("--data-dir", help="Compiled virtual data directory; otherwise read build/config.h")
    parser.add_argument("--record-known-ub", action="store_true", help="Record the exact documented upstream Debug UBSan sites; fail any new site. Does not disable instrumentation.")
    args = parser.parse_args()
    args.build = args.build.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "console.log").write_text("")
    (args.output / "report.json").unlink(missing_ok=True)
    data_dir = args.data_dir
    if data_dir is None:
        data_dir = re.search(r'#define BUILD_CONFIG_DATA_DIR "([^"]+)"', (args.build / "config.h").read_text()).group(1)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(Handler, directory=str(args.build)))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    try:
        evidence, report = asyncio.run(smoke(args, f"http://127.0.0.1:{server.server_port}/supertux2.html", data_dir))
        (args.output / "console.log").write_text("\n".join(evidence) + "\n")
        print(json.dumps(report, indent=2), flush=True)
    finally:
        server.shutdown()


if __name__ == "__main__":
    main()
