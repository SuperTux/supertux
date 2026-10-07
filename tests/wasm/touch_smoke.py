#!/usr/bin/env python3
"""Compiled-game touch-only menu/level entry and multitouch/lifecycle checks.

Chromium uses trusted CDP touch injection. WebKit uses native touchscreen taps
for startup, then DOM Pointer Events through SDL3's actual browser handlers for
multitouch (Playwright has no WebKit multifinger injection). This is not a real
Safari/device certification. Only existing read-only Squirrel getters observe
play; no player positioning, bonuses, input exports, or level changes are used.
"""
import argparse
import asyncio
import functools
import http.server
import json
import re
import threading
from pathlib import Path
from playwright.async_api import async_playwright
from browser_smoke import Handler, KNOWN_UPSTREAM_UB


class Fingers:
    def __init__(self, page, cdp=None):
        self.page, self.cdp, self.points = page, cdp, {}

    async def change(self, kind, finger, pos=None):
        old = self.points.get(finger)
        if kind in ('down', 'move'):
            self.points[finger] = pos
        else:
            self.points.pop(finger, None)
        if self.cdp:
            await self.cdp.send('Input.dispatchTouchEvent', {
                'type': {'down': 'touchStart', 'move': 'touchMove', 'up': 'touchEnd'}[kind],
                # Chromium's touchEnd lists the fingers being released, not
                # the remaining contacts (verified with native pointerup events).
                'touchPoints': [dict(id=i, x=p[0], y=p[1], radiusX=8, radiusY=8, force=1)
                                for i, p in ([(finger, old)] if kind == 'up' else self.points.items())]})
        else:
            await self.page.evaluate('''([kind, id, pos]) => {
                const canvas = Module.canvas;
                canvas.dispatchEvent(new PointerEvent('pointer' + kind, {
                    pointerType: 'touch', pointerId: id, button: kind === 'move' ? -1 : 0,
                    buttons: kind === 'up' ? 0 : 1, clientX: pos[0], clientY: pos[1],
                    bubbles: true, cancelable: true, isPrimary: id === 1
                }));
                if (kind === 'up') {
                    canvas.dispatchEvent(new PointerEvent('lostpointercapture', {pointerType: 'touch', pointerId: id}));
                    canvas.dispatchEvent(new PointerEvent('pointerleave', {pointerType: 'touch', pointerId: id, clientX: pos[0], clientY: pos[1]}));
                }
            }''', [kind, finger, pos or old])

    async def down(self, finger, pos): await self.change('down', finger, pos)
    async def move(self, finger, pos): await self.change('move', finger, pos)
    async def up(self, finger): await self.change('up', finger)
    async def tap(self, pos, duration=120):
        await self.down(9, pos)
        await self.page.wait_for_timeout(duration)
        await self.up(9)
        await self.page.wait_for_timeout(120)

    async def release(self):
        for finger in list(self.points): await self.up(finger)


async def controls(page):
    # Expected physical geometry, independently derived from the documented
    # viewport limits. Screenshots plus actual actions test the full conversion.
    return await page.evaluate('''() => {
        const r = Module.canvas.getBoundingClientRect(), w = Module.canvas.width, h = Module.canvas.height;
        let scale = Math.max(1, w / 1368, h / 800);
        if (w / scale < 640 || h / scale < 480) scale = Math.min(w / 640, h / 480);
        const vw = Math.min(w, Math.floor(scale * 1368)), vh = Math.min(h, Math.floor(scale * 800));
        const x = r.left + Math.floor((w - vw) / 2), y = r.top + Math.floor((h - vh) / 2);
        const b = Math.min(Math.max(56, Math.min(96, vh * .2)), Math.max(44, vw / 6.5));
        const d = Math.max(132, 2.3 * b), small = Math.max(44, b * .75), middleY = y + vh - 12 - d / 2;
        return {right: [x + 12 + d * .83, middleY], left: [x + 12 + d * .17, middleY],
            up: [x + 12 + d / 2, y + vh - 12 - d * .83],
            down: [x + 12 + d / 2, y + vh - 12 - d * .17],
            neutral: [x + vw / 2, y + vh / 2],
            jump: [x + vw - 12 - b / 2, y + vh - 12 - b / 2],
            action: [x + vw - 24 - 1.5 * b, y + vh - 12 - b / 2],
            pause: [x + vw / 2 + 6 + small / 2, y + 12 + small / 2],
            item: [x + vw / 2 - 6 - small / 2, y + 12 + small / 2],
            viewport: {x, y, width: vw, height: vh}, buttonSize: b};
    }''')


async def run(args, url):
    report = {'checks': [], 'samples': {}, 'physical_device_tested': False}
    logs, errors, samples = [], [], []
    pattern = re.compile(r'\[SCRIPTING\] PHASE3=([-0-9.,truefals]+)')
    async with async_playwright() as p:
        launch = {}
        if args.browser == 'chromium':
            launch = dict(executable_path=args.chromium,
                          args=['--no-sandbox', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'])
        elif args.webkit_executable:
            launch['executable_path'] = args.webkit_executable
        browser = await getattr(p, args.browser).launch(**launch)
        report.update(browser=browser.version, engine=args.browser,
                      touch_injection='trusted CDP touch' if args.browser == 'chromium' else 'DOM Pointer Events through SDL3; native Start tap')
        context = await browser.new_context(viewport={'width':844, 'height':390}, has_touch=True, device_scale_factor=1)
        page = await context.new_page()
        page.set_default_timeout(60000)
        def console(message):
            logs.append(message.text)
            with (args.output / 'console.log').open('a') as f: f.write(message.type + ': ' + message.text + '\n')
            match = pattern.search(message.text)
            if match:
                values = match[1].split(',')
                samples.append(dict(x=float(values[0]), y=float(values[1]),
                    **{key: values[i+2] == 'true' for i,key in enumerate(['right','jump','action','left','up','down','item'])}))
        page.on('console', console)
        page.on('pageerror', lambda error: errors.append(str(error)))
        page.on('response', lambda response: errors.append(f'HTTP {response.status}: {response.url}') if response.status >= 400 else None)
        # Verbose logging only; no level CLI shortcut or developer mode at boot.
        html = (args.build / 'supertux2.html').read_text().replace('var Module = {', 'var Module = {\narguments: ["--verbose"],', 1)
        await page.route('**/supertux2.html', lambda route: route.fulfill(body=html,content_type='text/html'))
        await page.goto(url)
        await page.wait_for_function('Module.supertuxReady === true', timeout=180000)
        await page.locator('#start_button').tap()
        await page.wait_for_function('Module.supertuxShell.active')
        await page.wait_for_timeout(400)
        await page.screenshot(path=str(args.output / 'touch-main-menu.png'))
        cdp = await context.new_cdp_session(page) if args.browser == 'chromium' else None
        fingers = Fingers(page, cdp)
        g = await controls(page)
        # Direct tap Start Game, not the currently selected action/keyboard row.
        # The main menu has eight entries, 24 logical pixels high; center +35.
        vp = g['viewport']; logical_h = 480
        main_y = vp['y'] + (logical_h / 2 + 35 - 8 * 24 / 2 + 12) * (vp['height'] / logical_h)
        await fingers.tap([422, main_y])
        await page.wait_for_timeout(1800)
        await page.screenshot(path=str(args.output / 'story-intro.png'))
        # The existing Escape action skips the introductory cutscene.
        await fingers.tap(g['pause'])
        await page.wait_for_timeout(1500)
        await page.screenshot(path=str(args.output / 'touch-world-map.png'))
        # Starting node (42,66) → first playable node (42,68), using DOWN.
        await fingers.down(1, g['down'])
        await page.wait_for_timeout(700)
        await fingers.up(1)
        await page.wait_for_timeout(300)
        await fingers.tap(g['jump'])
        await page.wait_for_timeout(1500)
        await page.screenshot(path=str(args.output / 'touch-level-intro.png'))
        await fingers.tap(g['jump'])
        await page.wait_for_timeout(500)
        await page.screenshot(path=str(args.output / 'touch-level.png'))
        # Confirm the normal route really started the expected packaged level.
        output = await page.locator('#output').text_content()
        assert 'Setting status: Playing' in output, 'Touch-only normal entry did not start a level'
        report['checks'].append('Fresh storage → native Start tap → direct Start Game tap → skip story → world-map DOWN → JUMP → dismiss level intro, entirely by touch')

        # Enable only the existing diagnostic console after touch-only level entry.
        # The observer calls getters, never setters or gameplay/script cheats.
        await page.keyboard.press('Control+F2', delay=100)
        # F2 also opens the existing Debug menu; close it through touch before
        # observing, otherwise gameplay remains paused during the checks.
        await fingers.tap(g['pause'])
        await page.keyboard.press('Backquote', delay=100)
        getters = ' + "," + '.join(['sector.Tux.get_x()', 'sector.Tux.get_y()'] +
            [f'sector.Tux.get_input_held("{key}")' for key in ['right','jump','action','left','up','down','item']])
        await page.keyboard.type('function phase3Snapshot(){print("PHASE3=" + ' + getters + ');}phase3Observer <- newthread(function(){for(local i=0;i<6000;i++){phase3Snapshot();wait(0.1);}});phase3Observer.call();', delay=1)
        await page.keyboard.press('Enter', delay=100)
        await page.keyboard.press('Backquote', delay=100)
        await page.wait_for_function("document.querySelector('#output').textContent.includes('[SCRIPTING] PHASE3=')")
        assert samples, 'Read-only observer did not start'

        paused_checks = False
        async def state(label, expected, wait=160, observe=lambda sample: True):
            start = len(samples)
            await page.wait_for_timeout(wait)
            if paused_checks:
                # The game's normal pause stops its script scheduler. Query
                # the same read-only helper once instead of changing the clock.
                await page.keyboard.press('Backquote',delay=100)
                await page.keyboard.type('phase3Snapshot();',delay=1)
                await page.keyboard.press('Enter',delay=100)
                await page.keyboard.press('Backquote',delay=100)
            # Browser frames and the script's 100 ms simulation interval need
            # not line up with a wall-clock delay. Require a fresh matching
            # sample, with a bounded deadline rather than a timing assumption.
            deadline = asyncio.get_running_loop().time() + 2
            while not (len(samples) > start and all(samples[-1][key] == value for key,value in expected.items()) and observe(samples[-1])):
                if asyncio.get_running_loop().time() >= deadline:
                    raise AssertionError((label, expected, samples[-1] if samples else None, logs[-5:]))
                await page.wait_for_timeout(50)
            sample = samples[-1]
            assert sample['y'] < 800, (label, 'Player died during an idle test; state observation is no longer valid', sample)
            assert all(sample[key] == value for key,value in expected.items()), (label, expected, sample)
            report['samples'][label] = sample
            return sample

        async def restart(label):
            # Use the ordinary touch menu to reset enemies between unrelated
            # groups, rather than changing player state or granting immunity.
            await fingers.release()
            await fingers.tap(g['pause'])
            await fingers.tap(g['down']) # Continue → Restart Level.
            await fingers.tap(g['jump'])
            restored = await state(label, dict(left=False,right=False,jump=False,action=False), 400)
            assert abs(restored['x'] - 96) < 1 and abs(restored['y'] - 673.196) < 1, restored

        # Move near the safe starting boundary before lengthy release checks.
        await fingers.down(1, g['left'])
        await page.wait_for_timeout(700)
        await fingers.up(1)
        await state('neutral', dict(right=False, jump=False, action=False, left=False), 600)
        baseline = samples[-1]
        await fingers.down(1, g['right'])
        moved = await state('right', dict(right=True), 220, lambda sample: sample['x'] > baseline['x'] + 5)
        assert moved['x'] > baseline['x'] + 5, (baseline, moved)
        await fingers.down(2, g['jump'])
        jumped = await state('right-jump', dict(right=True, jump=True), 150, lambda sample: sample['y'] < baseline['y'] - 10)
        assert jumped['y'] < baseline['y'] - 10, (baseline, jumped)
        await fingers.down(3, g['action'])
        await state('three-fingers', dict(right=True, jump=True, action=True), 120)
        await page.screenshot(path=str(args.output / 'three-fingers.png'))
        await fingers.up(2)
        await state('jump-released', dict(right=True, jump=False, action=True), 100)
        await fingers.up(1)
        await state('direction-released', dict(right=False, jump=False, action=True), 100)
        await fingers.up(3)
        await state('all-released', dict(right=False, jump=False, action=False))
        report['checks'].append('Real Tux moves RIGHT and jumps while moving; three simultaneous actions; individual native finger releases leave remaining actions held')

        await restart('restart-before-slides')
        # Slide one owned finger across pad and out.
        await fingers.down(1, g['left'])
        await page.wait_for_timeout(600)
        await fingers.move(1, g['right'])
        await state('slide-right', dict(left=False, right=True), 100)
        await fingers.move(1, g['neutral'])
        await state('slide-out', dict(left=False, right=False))
        await fingers.move(1, g['left'])
        await state('slide-back', dict(left=True, right=False))
        await fingers.up(1)
        await state('slide-release', dict(left=False, right=False))
        # Two fingers on one action: release one, keep the other held.
        await fingers.down(2, g['action']); await fingers.down(3, g['action'])
        await state('same-button-two', dict(action=True))
        await fingers.up(2)
        await state('same-button-one', dict(action=True))
        await fingers.up(3)
        await state('same-button-none', dict(action=False))
        report['checks'].append('Sliding pad→opposite direction→outside→back and two fingers on the same button release correctly')

        await restart('restart-before-cancel')
        # One unexpected capture loss, plus subsequent stale motion/up. The
        # browser's cancellation cannot be triggered per finger through CDP.
        await fingers.down(1, g['left']); await fingers.down(3, g['action'])
        await state('before-cancel', dict(left=True, action=True))
        if cdp:
            # Observe native pointer IDs (CDP touch IDs need not equal them).
            await page.evaluate('''() => { window.phase3Pointer = null;
                Module.canvas.addEventListener('pointermove', e => { if(e.pointerType==='touch') window.phase3Pointer=e.pointerId; }, {once:true}); }''')
            await fingers.move(1, [g['left'][0]+1,g['left'][1]])
            pointer = await page.evaluate('window.phase3Pointer')
        else: pointer = 1
        assert pointer is not None
        await page.evaluate("id => Module.canvas.dispatchEvent(new PointerEvent('lostpointercapture', {pointerType:'touch',pointerId:id}))", pointer)
        await state('one-canceled', dict(left=False, action=True))
        await fingers.move(1, g['right'])
        await state('canceled-motion-ignored', dict(left=False, right=False, action=True))
        await fingers.up(1)
        await state('canceled-up-ignored', dict(left=False, right=False, action=True))
        await fingers.up(3)
        await state('cancel-cleared', dict(action=False))
        report['checks'].append('Unexpected capture loss cancels one finger; stale motion/up cannot resurrect it; other finger survives')

        await page.wait_for_timeout(600)
        ground = samples[-1]['y']
        await page.evaluate("""pos => {
            for(const kind of ['down','up','leave']) Module.canvas.dispatchEvent(new PointerEvent('pointer'+kind, {
                pointerType:'touch',pointerId:37,button:0,buttons:kind==='down'?1:0,
                clientX:pos[0],clientY:pos[1],bubbles:true,cancelable:true
            }));
        }""", g['jump'])
        quick = await state('short-tap', dict(jump=False), 150)
        assert quick['y'] < ground - 5, (ground,quick)
        await page.wait_for_timeout(600)
        report['checks'].append('DOWN/UP/normal pointerleave in one browser turn delivers a short jump exactly once')

        await restart('restart-before-item')
        # Item output and gameplay pause/resume are ordinary touch actions.
        await fingers.down(1, g['item']); await state('item', dict(item=True)); await fingers.up(1)
        await fingers.tap(g['pause'])
        await page.wait_for_timeout(300)
        await page.screenshot(path=str(args.output / 'touch-pause-menu.png'))
        await fingers.tap(g['jump']) # Continue is initially selected.
        await page.wait_for_timeout(400)
        await state('touch-resumed', dict(right=False, left=False, jump=False, action=False, item=False))
        report['checks'].append('Item, gameplay Pause, and Continue are accessible through touch')

        # Keep the level's normal pause menu open during lengthy layout/save
        # checks: an idle small Tux can otherwise be killed by approaching enemies.
        # LEFT does not activate the selected Continue row, but still exercises
        # the same player's real controller and viewport-aligned hit regions.
        await fingers.tap(g['pause'])
        paused_checks = True
        # Re-measure at orientation/toolbar/safe-area changes. No keyboard input.
        for width,height in [(390,844),(844,320),(844,390)]:
            await fingers.down(1, g['left'])
            await page.set_viewport_size({'width':width,'height':height})
            await page.wait_for_function('size => Module.canvas.width === size[0] && Module.canvas.height === size[1]', arg=[width,height])
            await fingers.move(1, [20,20])
            await state(f'resize-{width}x{height}-neutral', dict(left=False, right=False))
            await fingers.up(1)
            g = await controls(page)
            await fingers.down(1, g['left'])
            await state(f'resize-{width}x{height}-left', dict(left=True))
            await fingers.up(1)
            await page.screenshot(path=str(args.output / f'touch-{width}x{height}.png'))
        await page.locator('#game_shell').evaluate("e => e.style.padding = '10px 20px 30px 40px'")
        await page.evaluate("window.dispatchEvent(new Event('resize'))")
        await page.wait_for_function('Module.canvas.width === 784 && Module.canvas.height === 350')
        await page.wait_for_timeout(200) # Let SDL/ResizeObserver finish the padded resize.
        g = await controls(page)
        await fingers.down(1, g['left']); await state('safe-area-left', dict(left=True)); await fingers.up(1)
        await page.screenshot(path=str(args.output / 'touch-safe-area.png'))
        report['checks'].append('Portrait, landscape, toolbar-height and CSS safe-area changes keep controls aligned and reset held gestures')

        await fingers.down(1, g['left']); await fingers.down(3, g['item'])
        await page.evaluate("window.dispatchEvent(new Event('blur'))")
        await page.wait_for_function('!Module.supertuxShell.active')
        await page.evaluate("window.dispatchEvent(new Event('focus')); window.dispatchEvent(new Event('pageshow'))")
        await page.locator('#start_button').tap()
        await page.wait_for_function('Module.supertuxShell.active')
        await fingers.move(1, g['right'])
        await state('background-resumed', dict(left=False, right=False, action=False, jump=False, item=False))
        await fingers.release()
        report['checks'].append('Blur/pageshow leaves explicit native Resume; held direction/item and stale motion are neutral afterward')
        # The real in-game Controls menu exposes preferences on web/hybrid
        # devices. No synthetic setting export or replacement settings UI.
        # The layout checks left the normal pause menu open. Resume and the
        # diagnostic console can change its selected row, so tap Options
        # directly instead of assuming three DOWN presses start at Continue.
        # This fresh level has no checkpoint; developer mode adds Edit Level.
        # Its eight rows center Options 36 logical pixels below the midpoint.
        vp = g['viewport']
        await fingers.tap([vp['x'] + vp['width'] / 2,
                           vp['y'] + (480 / 2 + 36) * vp['height'] / 480])
        await page.wait_for_timeout(300) # Finish the pause → Options transition.
        # Pointer motion can select a category as this menu opens. Controls is
        # the middle tile of the five in-game categories; tap it directly rather
        # than assuming two RIGHT presses always begin on Video.
        await page.screenshot(path=str(args.output / 'touch-options-categories.png'))
        category = [vp['x'] + vp['width'] / 2, vp['y'] + vp['height'] / 2]
        await fingers.down(9, category)
        await page.wait_for_timeout(300)
        # The opening UI finger must lose ownership when the submenu changes.
        # This point is over a different Controls-menu row. Its stale motion/up
        # must leave Touch Controls selected for the subsequent RIGHT press.
        await fingers.move(9, [category[0] + 1, category[1]])
        await fingers.up(9)
        await page.wait_for_timeout(300)
        await page.screenshot(path=str(args.output / 'touch-controls-options.png'))
        await fingers.tap(g['right']) # Auto → Off; clear owned holds immediately.
        async def saved_mode():
            return await page.evaluate(r"""() => {
                Module.ccall('save_config',null,[],[]);
                const config = Module.FS.readFile(Module.supertuxStorage.root+'config',{encoding:'utf8'});
                return Number(config.match(/\(browser_touch_controls (-?\d+)\)/)[1]);
            }""")
        assert await saved_mode() == 0, 'Touch Controls menu did not select Off'
        await page.screenshot(path=str(args.output / 'touch-controls-off.png'))
        # With controls off, tapping the selected string row itself advances to
        # On. Ten 24px rows, OptionsMenu centers at half the logical screen plus 15px.
        vp = g['viewport']
        await fingers.tap([vp['x']+vp['width']/2, vp['y']+195*vp['height']/480])
        assert await saved_mode() == 1, 'Direct menu tap could not re-enable touch controls'
        assert await page.evaluate('Module.supertuxStorage.flush()')
        await page.reload()
        await page.wait_for_function('Module.supertuxReady === true',timeout=180000)
        await page.locator('#start_button').tap()
        await page.wait_for_function('Module.supertuxShell.active')
        assert await saved_mode() == 1, 'Touch preference did not survive IDBFS reload'
        report['checks'].append('Direct Controls-menu entry discards its opening UI finger; stale motion/up cannot refocus a setting; Auto→Off through D-pad, Off→On by direct tap, persisted On survives real IDBFS reload')
        await context.close()
        await browser.close()
    diagnostics = [line for line in logs if 'runtime error:' in line]
    known = [line for line in diagnostics if any(re.search(p,line) for p in KNOWN_UPSTREAM_UB)] if args.record_known_ub else []
    fatal = [line for line in logs if re.search(r'undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function|AN ERROR HAS OCCURRED|Error waking VM',line) and line not in known]
    report['known_upstream_sanitizer_diagnostics'] = known
    report['errors'] = errors + fatal
    report['passed'] = not report['errors']
    if report['passed']:
        report['checks'].append('No HTTP/page errors, missing symbols, fatal runtime errors, or new sanitizer sites')
    (args.output / 'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2),flush=True)
    assert report['passed'], report['errors']


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build',type=Path)
    parser.add_argument('--output',type=Path,default=Path('wasm-touch-evidence'))
    parser.add_argument('--chromium')
    parser.add_argument('--browser',choices=['chromium','webkit'],default='chromium')
    parser.add_argument('--webkit-executable')
    parser.add_argument('--record-known-ub',action='store_true')
    args=parser.parse_args();args.build=args.build.resolve();args.output.mkdir(parents=True,exist_ok=True)
    (args.output/'console.log').write_text('');(args.output/'report.json').unlink(missing_ok=True)
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Handler,directory=str(args.build)))
    threading.Thread(target=server.serve_forever,daemon=True).start()
    try: asyncio.run(run(args,f'http://127.0.0.1:{server.server_port}/supertux2.html'))
    finally: server.shutdown()

if __name__=='__main__':main()
