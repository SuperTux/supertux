#!/usr/bin/env python3
"""Real compiled engine: deferred music, races, lifecycle, and separate persistent cache."""
import argparse
import asyncio
import functools
import http.server
import json
import re
import threading
from pathlib import Path
from urllib.parse import urljoin
from playwright.async_api import async_playwright


class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


async def smoke(args, url):
    manifest = json.loads((args.build / 'asset-manifest.json').read_text())
    html = (args.build / 'supertux2.html').read_text()
    music = [e for e in manifest['assets'] if e['package'] == 'music']
    assert len(music) > 60, 'Soundtrack inventory is incomplete'
    a = next(e for e in music if e['path'].startswith('music/forest/'))
    b = next(e for e in music if e['path'].startswith('music/tropical/'))
    c = next(e for e in music if e['path'].startswith('music/castle/'))
    checks, errors, log = [], [], []
    def record(message):
        checks.append(message)
        (args.output/'report.json').write_text(json.dumps(dict(checks=checks, errors=errors), indent=2))
        print(message, flush=True)
    def console(label, msg):
        line = label + ': ' + msg.text
        log.append(line)
        with (args.output/'console.log').open('a') as stream: stream.write(line + '\n')
    async with async_playwright() as p:
        launch = dict(executable_path=args.chromium or p.chromium.executable_path, args=['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader']) if args.browser == 'chromium' else ({'executable_path':args.webkit_executable} if args.webkit_executable else {})
        browser = await getattr(p,args.browser).launch(**launch)
        context = await browser.new_context(viewport={'width':844,'height':390},has_touch=True)

        async def boot(context, label, body=html, extra_arguments=()):
            page = await context.new_page()
            page.set_default_timeout(60000)
            page.on('pageerror', lambda error: errors.append(label + ': ' + str(error)))
            page.on('console', lambda msg: console(label,msg))
            requests = []
            page.on('request', lambda request: requests.append(request.url))
            await page.add_init_script('''const OriginalAudio = window.AudioContext;
              window.assetAudio = [];
              window.AudioContext = class extends OriginalAudio {
                constructor(...args){super(...args);assetAudio.push(this);this.starts=0;
                  const create=this.createBufferSource.bind(this);
                  this.createBufferSource=()=>{const source=create(),start=source.start.bind(source);source.start=(...args)=>{++this.starts;return start(...args);};return source;};}
              };''')
            if args.browser == 'chromium':
                cdp = await context.new_cdp_session(page)
                await cdp.send('Network.enable')
                await cdp.send('Network.setCacheDisabled', {'cacheDisabled':True})
            body = body.replace('var Module = {', 'var Module = {\narguments: ' + json.dumps(['--verbose','--developer',*extra_arguments]) + ',', 1)
            await page.route('**/supertux2.html', lambda route: route.fulfill(body=body,content_type='text/html'))
            await page.goto(url)
            await page.wait_for_function('window.Module && Module.supertuxReady === true',timeout=180000)
            assert not any('/game-assets/music/' in path for path in requests), 'Music blocked initial startup'
            assert urljoin(url, manifest['packages']['wasm']['url']) not in requests, 'SDK downloaded WASM again instead of using validated cached bytes'
            await page.evaluate(r'''() => {
              window.engineMusicOpened=[];
              const open=Module.FS.open.bind(Module.FS);
              Module.FS.open=(path,...args)=>{const mode=args[0],reading=typeof mode==='number'?(mode&3)===0:typeof mode==='string'&&mode.startsWith('r');if(reading && typeof path==='string' && /\/music\/.*\.(ogg|wav)$/.test(path))engineMusicOpened.push(path);return open(path,...args);};
            }''')
            await page.locator('#start_button').tap()
            await page.wait_for_function('Module.supertuxShell.active')
            return page, requests

        async def command(page, code):
            await page.locator('#canvas').focus()
            await page.keyboard.press('Backquote',delay=100)
            await page.keyboard.type(code,delay=1)
            await page.keyboard.press('Enter',delay=100)
            await page.keyboard.press('Backquote',delay=100)

        page, requests = await boot(context,'cold')
        await page.wait_for_function('engineMusicOpened.length > 0 && assetAudio[0].starts > 0')
        record('Fresh menu ready without soundtrack requests; requested title audio decoded and played by real OpenAL')
        # Actual runtime must expose every startup dependency, including all worlds.
        assert await page.evaluate('''() => Module.supertuxAssets.manifest.assets.filter(e=>e.package==='startup').every(e=>Module.FS.stat(Module.supertuxAssets.manifest.runtimeRoot+'/'+e.path).size===e.bytes)''')
        record('Every manifest startup file is mounted with its expected size')

        release = asyncio.Event()
        started = asyncio.Event()
        async def delayed(route):
            started.set()
            await release.wait()
            await route.continue_()
        await page.route('**/' + a['url'], delayed)
        await command(page, f'play_music("/{a["path"]}");')
        await asyncio.wait_for(started.wait(),30)
        await command(page, f'play_music("/{b["path"]}");')
        await page.wait_for_function('path => engineMusicOpened.some(p=>p.endsWith(path))',arg=b['path'])
        release.set()
        await page.wait_for_function('path => Module.supertuxAssets.tracks.get(path)?.state === 0',arg=a['path'])
        await page.wait_for_timeout(1000)
        assert not await page.evaluate('path => engineMusicOpened.some(p=>p.endsWith(path))',a['path']), 'Stale track A played after B'
        record('Real engine A→B race: late A mounted/cacheable but never opened or played')

        async def partial(route):
            await route.fulfill(status=200,body=b'partial',content_type='audio/ogg')
        await page.route('**/' + c['url'],partial)
        await command(page, f'play_music("/{c["path"]}");')
        await page.wait_for_function('path => Module.supertuxAssets.tracks.get(path)?.state === 2',arg=c['path'])
        assert await page.evaluate('Module.supertuxShell.active')
        await page.unroute('**/' + c['url'],partial)
        await page.locator('#retry_music').click()
        await page.wait_for_function('path => engineMusicOpened.some(p=>p.endsWith(path))',arg=c['path'])
        record('Partial optional download never mounted; game stayed active and retry reached real decoder')

        d = next(e for e in music if e['path'].startswith('music/retro/'))
        paused_release = asyncio.Event()
        async def paused_download(route):
            await paused_release.wait()
            await route.continue_()
        await page.route('**/' + d['url'],paused_download)
        await command(page, f'pause_music(0); play_music("/{d["path"]}");')
        # pause_music before request dispatch prevents unnecessary network activity.
        await page.wait_for_timeout(600)
        assert not any(d['url'] in path for path in requests)
        await command(page,'resume_music(0);')
        await page.wait_for_function('path => Module.supertuxAssets.tracks.has(path)',arg=d['path'])
        await command(page,'pause_music(0);')
        paused_release.set()
        await page.wait_for_function('path => Module.supertuxAssets.tracks.get(path)?.state === 0',arg=d['path'])
        assert not await page.evaluate('path => engineMusicOpened.some(p=>p.endsWith(path))',d['path'])
        await command(page,'resume_music(0);')
        await page.wait_for_function('path => engineMusicOpened.some(p=>p.endsWith(path))',arg=d['path'])
        e = next(e for e in music if e['path'].startswith('music/antarctic/'))
        background_release = asyncio.Event()
        background_started = asyncio.Event()
        async def background_download(route):
            background_started.set()
            await background_release.wait()
            await route.continue_()
        await page.route('**/' + e['url'],background_download)
        await command(page,f'play_music("/{e["path"]}");')
        await asyncio.wait_for(background_started.wait(),30)
        await page.evaluate("window.dispatchEvent(new Event('blur'))")
        await page.wait_for_function('!Module.supertuxShell.active')
        background_release.set()
        await page.wait_for_function('path => Module.supertuxAssets.tracks.get(path)?.state === 0',arg=e['path'])
        assert not await page.evaluate('path => engineMusicOpened.some(p=>p.endsWith(path))',e['path'])
        assert await page.evaluate("Module.supertuxShell.audioState === 'suspended'")
        await page.evaluate("window.realResume=assetAudio[0].resume.bind(assetAudio[0]);assetAudio[0].resume=()=>Promise.reject(new Error('Audio activation blocked'));void 0")
        await page.locator('#start_button').tap()
        await page.locator('#play_muted').wait_for(state='visible')
        await page.locator('#play_muted').tap()
        await page.wait_for_function('Module.supertuxShell.active && Module.supertuxShell.muted')
        await page.wait_for_timeout(600)
        assert not await page.evaluate('path => engineMusicOpened.some(p=>p.endsWith(path))',e['path'])
        await page.evaluate("assetAudio[0].resume=window.realResume;window.dispatchEvent(new Event('blur'))")
        await page.locator('#start_button').tap()
        await page.wait_for_function("Module.supertuxShell.active && Module.supertuxShell.audioState === 'running'")
        await page.wait_for_function('path => engineMusicOpened.some(p=>p.endsWith(path))',arg=e['path'])
        record('Paused track waits despite completed download; background freezes audio and trusted Resume permits playback')

        await page.evaluate('''async () => {
          const root=Module.supertuxStorage.root;
          Module.FS.writeFile(root+'asset-cache-save-marker','saved progress');
          await Module.supertuxStorage.flush();
          await Module.supertuxAssets.cache.pending;
        }''')
        await page.close()
        # Existing cache reads must also survive quota errors in LRU accounting.
        await context.add_init_script("const put=IDBObjectStore.prototype.put;IDBObjectStore.prototype.put=function(...args){if(this.name==='metadata')throw new DOMException('Quota','QuotaExceededError');return put.apply(this,args)}")
        page, requests = await boot(context,'warm')
        await page.wait_for_function('engineMusicOpened.length > 0')
        assert not any('.data' in path or '.wasm' in path or 'supertux2.js' in path or '/game-assets/music/' in path for path in requests), requests
        assert await page.evaluate("Module.FS.readFile(Module.supertuxStorage.root+'asset-cache-save-marker',{encoding:'utf8'})") == 'saved progress'
        record('New page reuses startup and title music with HTTP cache disabled even when cache accounting exceeds quota')
        await command(page, f'play_music("/{b["path"]}");')
        await page.wait_for_function('path => engineMusicOpened.some(p=>p.endsWith(path))',arg=b['path'])
        assert not any(b['url'] in path for path in requests)
        record('Previously downloaded world music plays from persistent cache')
        await page.evaluate('''async () => {
          const db=await Module.supertuxAssets.cache.db();
          await new Promise((resolve,reject)=>{
            const tx=db.transaction('payloads','readwrite');
            tx.objectStore('payloads').put(new Uint8Array([0]).buffer,Module.supertuxAssets.manifest.packages.startup.sha256);
            tx.oncomplete=resolve;tx.onabort=tx.onerror=reject;
          });
        }''')
        await page.close()
        page, requests = await boot(context,'corrupt-startup-cache')
        assert any('.data' in path for path in requests)
        assert await page.evaluate("Module.FS.readFile(Module.supertuxStorage.root+'asset-cache-save-marker',{encoding:'utf8'})") == 'saved progress'
        record('Corrupt persistent startup bytes are rejected and redownloaded without losing saved progress')
        config_before = await page.evaluate("Module.FS.readFile(Module.supertuxStorage.root+'config',{encoding:'utf8'})")
        await page.evaluate("window.dispatchEvent(new Event('blur'))")
        await page.locator('#activation_panel summary').click()
        await page.locator('#clear_assets_cache').click()
        await page.wait_for_function("document.querySelector('#music_status').textContent.includes('Downloaded assets cleared')")
        await page.close()
        page, requests = await boot(context,'cleared')
        assert any('.data' in path for path in requests)
        assert await page.evaluate("Module.FS.readFile(Module.supertuxStorage.root+'asset-cache-save-marker',{encoding:'utf8'})") == 'saved progress'
        assert await page.evaluate("Module.FS.readFile(Module.supertuxStorage.root+'config',{encoding:'utf8'})") == config_before
        record('Clearing only downloads redownloads startup and preserves persisted save marker/settings')
        await page.close()
        await context.close()

        context = await browser.new_context(viewport={'width':844,'height':390},has_touch=True)
        page, requests = await boot(context,'music-disabled',extra_arguments=['--disable-music'])
        await command(page,'play_sound("/sounds/coin.wav");')
        await page.wait_for_function('assetAudio[0].starts > 0')
        assert not any('/game-assets/music/' in path for path in requests)
        assert not await page.evaluate('engineMusicOpened.length > 0')
        record('Native music-disabled setting downloads no soundtrack while an immediate sound effect plays')
        await context.close()

        # Independent fresh profiles exercise fallback without existing cache hits.
        for label, init in [('unavailable', "Object.defineProperty(window,'indexedDB',{get(){throw new Error('Storage disabled')}})"),
                            ('quota', "const put=IDBObjectStore.prototype.put;IDBObjectStore.prototype.put=function(...args){if(this.name==='payloads'||this.name==='metadata')throw new DOMException('Quota','QuotaExceededError');return put.apply(this,args)}")]:
            context = await browser.new_context(viewport={'width':844,'height':390},has_touch=True)
            await context.add_init_script(init)
            page, _ = await boot(context,label)
            await page.wait_for_function('engineMusicOpened.length > 0 && assetAudio[0].starts > 0')
            record(label + ' asset storage: startup and decoded music work online')
            await context.close()

        context = await browser.new_context()
        page = await context.new_page()
        bad = re.sub(r'"manifestSha256":"[a-f0-9]{64}"', '"manifestSha256":"' + '0'*64 + '"',html)
        await page.route('**/supertux2.html',lambda route:route.fulfill(body=bad,content_type='text/html'))
        await page.goto(url)
        await page.wait_for_function("document.querySelector('#status').textContent.includes('updated')")
        assert not await page.evaluate('Module.supertuxReady === true')
        record('Mismatched manifest/build identity fails before incompatible startup is mounted')
        await context.close()
        await browser.close()
    args.output.mkdir(parents=True,exist_ok=True)
    (args.output/'console.log').write_text('\n'.join(log))
    (args.output/'report.json').write_text(json.dumps(dict(checks=checks,errors=errors),indent=2))
    assert not errors, errors
    print(json.dumps(checks,indent=2))


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('build',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--chromium')
    parser.add_argument('--browser',choices=['chromium','webkit'],default='chromium')
    parser.add_argument('--webkit-executable')
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Handler,directory=str(args.build)))
    threading.Thread(target=server.serve_forever,daemon=True).start()
    try: asyncio.run(smoke(args,f'http://127.0.0.1:{server.server_port}/supertux2.html'))
    finally: server.shutdown()


if __name__ == '__main__': main()
