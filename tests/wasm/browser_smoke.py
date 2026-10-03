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
    r"/src/object/player\.cpp:325:17: runtime error: reference binding to null pointer of type 'Climbable'",
    r"/src/object/player\.cpp:2264:13: runtime error: load of value \d+, which is not a valid value for type 'bool'",
    r"/(?:external/obstack/obstack\.c:(?:143:35|236:5|263:7)|src/util/obstackpp\.hpp:24:10): runtime error: subtraction of unsigned offset from 0x00000000 overflowed to 0x[0-9a-f]+",
]


class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


async def smoke(args, url, data_dir):
    evidence = []
    errors = []
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

    async with async_playwright() as playwright:
        browser = await playwright.chromium.launch(
            executable_path=args.chromium,
            args=["--no-sandbox", "--use-angle=swiftshader", "--enable-unsafe-swiftshader"],
        )
        report["browser"] = browser.version

        async def boot(context, name, level=False, memory=False):
            page = await context.new_page()
            page.set_default_timeout(60000)
            page.on("console", lambda message: log_console(name, message))
            page.on("pageerror", lambda error: errors.append(f"{name}: {error}"))
            page.on("response", lambda response: errors.append(f"{name}: HTTP {response.status}: {response.url}")
                    if response.status >= 400 else None)
            arguments = ["--verbose"]
            if level:
                # Use existing read-only scripting access to measure keyboard
                # movement. No test-only exports are compiled into the game.
                arguments.append("--developer")
                arguments.append(data_dir + "/levels/world1/welcome_antarctica.stl")
            body = html.replace("var Module = {", "var Module = {\narguments: " + json.dumps(arguments) + ",", 1)
            await page.route("**/supertux2.html", lambda route: route.fulfill(body=body, content_type="text/html"))
            await page.goto(url)
            await page.wait_for_function("window.Module && Module.supertuxReady === true", timeout=180000)
            mode = await page.evaluate("Module.supertuxStorage.state")
            assert mode == ("memory" if memory else "indexeddb"), (name, mode)
            assert await page.locator("#overlay").evaluate("element => getComputedStyle(element).display") == "none"
            await page.locator("#canvas").focus()
            report["checks"].append(name + ": boot and storage " + mode)
            return page

        async def capture(page, filename):
            data = await page.locator("#canvas").screenshot(path=str(args.output / filename))
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

        async def prepare_position(label, axis):
            await page.keyboard.press("Backquote", delay=100)
            await page.keyboard.type(f'print("PHASE1_{label}=" + sector.Tux.get_{axis}());', delay=1)
            await page.keyboard.press("Backquote", delay=100)

        async def position(label, axis, prepared=False):
            if not prepared:
                await prepare_position(label, axis)
            await page.keyboard.press("Backquote", delay=100)
            await page.keyboard.press("Enter", delay=100)
            pattern = r"\[SCRIPTING\] PHASE1_" + label + r"=([-0-9.]+)"
            await page.wait_for_function("pattern => new RegExp(pattern).test(document.querySelector('#output').textContent)", arg=pattern)
            text = await page.locator("#output").text_content()
            value = float(re.findall(pattern, text)[-1])
            await page.keyboard.press("Backquote", delay=100)
            return value

        x_before = await position("x_before", "x")
        y_ground = await position("y_ground", "y")
        before = await capture(page, "level-before.png")
        await page.keyboard.down("ArrowRight")
        await page.wait_for_timeout(1400)
        await page.keyboard.up("ArrowRight")
        x_after = await position("x_after", "x")
        assert x_after > x_before + 50, (x_before, x_after)
        # Prepare the read-only query before jumping; typing it during the jump
        # can take longer than the full jump on an instrumented Debug build.
        await prepare_position("y_jump", "y")
        await page.keyboard.press("Space", delay=100)
        y_jump = await position("y_jump", "y", prepared=True)
        assert y_jump < y_ground - 10, (y_ground, y_jump)
        report["keyboard_positions"] = {"x_before": x_before, "x_after": x_after, "y_ground": y_ground, "y_jump": y_jump}
        after = await capture(page, "level-after.png")
        assert ImageChops.difference(before, after).getbbox() is not None
        await page.keyboard.press("Escape", delay=150)
        await capture(page, "level-paused.png")
        report["checks"].append("Welcome to Antarctica entered; keyboard movement/jump measured through existing read-only console methods; pause screenshot captured")
        await page.close()
        await context.close()

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
