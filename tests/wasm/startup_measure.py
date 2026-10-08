#!/usr/bin/env python3
"""Comparable Chromium measurements; warm launch bypasses HTTP cache to test persistence."""
import argparse
import asyncio
import functools
import http.server
import json
import re
import threading
import tempfile
from pathlib import Path
from playwright.async_api import async_playwright


class Handler(http.server.SimpleHTTPRequestHandler):
    html = b''
    def log_message(self, *_):
        pass
    def do_GET(self):
        if self.path.split('?')[0] == '/supertux2.html':
            self.send_response(200)
            self.send_header('Content-Type','text/html')
            self.send_header('Content-Length',str(len(self.html)))
            self.end_headers()
            self.wfile.write(self.html)
        else:
            super().do_GET()


async def measure(args, url):
    with tempfile.TemporaryDirectory(prefix='supertux-benchmark-') as profile, args.output.with_suffix('.profile-note').open('w') as note:
      note.write('New empty profile for cold visit; same disk profile after full browser restart for warm visit.\n')
      async with async_playwright() as p:
        results = { 'viewport': '844x390 mobile emulation (not physical Safari)',
                   'network': '50 Mbit/s down, 40 ms latency; HTTP cache disabled for both visits',
                   'cpu': '2x slowdown', 'level': args.level, 'persistent_profile': 'Browser process restarted between visits', 'launches': []}
        for condition in ('cold', 'warm-persistent'):
            context = await p.chromium.launch_persistent_context(profile, executable_path=p.chromium.executable_path, args=['--no-sandbox', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'], viewport={'width':844,'height':390},is_mobile=True,has_touch=True)
            results['browser'] = context.browser.version
            page = await context.new_page()
            cdp = await context.new_cdp_session(page)
            await cdp.send('Network.enable')
            await cdp.send('Network.setCacheDisabled', {'cacheDisabled': True})
            await cdp.send('Network.emulateNetworkConditions', dict(offline=False, latency=40, downloadThroughput=6250000, uploadThroughput=1250000))
            await cdp.send('Emulation.setCPUThrottlingRate', {'rate': 2})
            urls, received, console = {}, [], []
            cdp.on('Network.requestWillBeSent', lambda e: urls.update({e['requestId']: e['request']['url']}))
            cdp.on('Network.loadingFinished', lambda e: received.append(dict(url=urls.get(e['requestId'], ''), bytes=e['encodedDataLength'], timestamp=e['timestamp'])))
            page.on('console', lambda message: console.append(message.text))
            start = asyncio.get_running_loop().time()
            await page.goto(url, timeout=180000)
            await page.wait_for_function('window.Module && Module.supertuxReady === true', timeout=240000)
            ready = asyncio.get_running_loop().time() - start
            mandatory_received = list(received)
            resources = await page.evaluate("performance.getEntriesByType('resource').filter(r=>r.name.startsWith('http')).map(r=>({url:r.name,encoded:r.encodedBodySize,decoded:r.decodedBodySize,responseEnd:r.responseEnd}))")
            ready_ms = await page.evaluate('performance.now()')
            await page.locator('#start_button').tap()
            await page.wait_for_function('Module.supertuxShell.active')
            if args.level:
                await page.keyboard.press('Enter',delay=150)
                await page.wait_for_function("document.querySelector('#output').textContent.includes('Playing')", timeout=60000)
                await page.keyboard.down('ArrowRight')
                await page.wait_for_timeout(250)
                await page.keyboard.up('ArrowRight')
            playable = asyncio.get_running_loop().time() - start
            await page.wait_for_timeout(2000)
            network = [r for r in received if r['url'].startswith('http')]
            data = [r for r in network if '.data' in r['url']]
            results['launches'].append(dict(condition=condition, ready_seconds=round(ready, 3), usable_seconds=round(playable, 3),
                mandatory_http_bytes=sum(r['bytes'] for r in mandatory_received if r['url'].startswith('http')), post_download_seconds=round((ready_ms-max((r['responseEnd'] for r in resources),default=ready_ms))/1000,3), resources=resources, http_bytes=sum(r['bytes'] for r in network), data_http_bytes=sum(r['bytes'] for r in data), requests=network,
                errors=[s for s in console if 'Error' in s or 'abort' in s]))
            args.output.write_text(json.dumps(results, indent=2))
            print(condition, results['launches'][-1], flush=True)
            await page.evaluate('async () => { if (Module.supertuxAssets) await Module.supertuxAssets.cache.pending; }')
            await context.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('build', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--level', help='Virtual path to a real level, for separate gameplay boot measurements')
    args = parser.parse_args()
    arguments = ['--verbose'] + ([args.level] if args.level else [])
    Handler.html = (args.build / 'supertux2.html').read_text().replace('var Module = {', 'var Module = {\narguments: ' + json.dumps(arguments) + ',', 1).encode()
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), functools.partial(Handler, directory=str(args.build)))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    try:
        asyncio.run(measure(args, f'http://127.0.0.1:{server.server_port}/supertux2.html'))
    finally:
        server.shutdown()


if __name__ == '__main__':
    main()
