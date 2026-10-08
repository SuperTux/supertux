#!/usr/bin/env python3
"""A later world's real textures, level scripts, movement, jump and music."""
import argparse
import asyncio
import functools
import http.server
import json
from pathlib import Path
import re
import threading
from playwright.async_api import async_playwright


class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


async def smoke(args, url):
    manifest=json.loads((args.build/'asset-manifest.json').read_text())
    html=(args.build/'supertux2.html').read_text().replace('var Module = {','var Module = {\narguments: '+json.dumps(['--verbose','--developer',manifest['runtimeRoot']+'/levels/world2/welcome_forest.stl'])+',',1)
    samples, errors, output=[],[],[]
    def console(message):
        output.append(message.text)
        match=re.search(r'AREA_POSITION=([-\d.]+),([-\d.]+),(true|false),(true|false)',message.text)
        if match:
            x,y,right,jump=match.groups();samples.append(dict(x=float(x),y=float(y),right=right=='true',jump=jump=='true'))
    async with async_playwright() as p:
        browser=await p.chromium.launch(executable_path=p.chromium.executable_path,args=['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader'])
        page=await browser.new_page(viewport={'width':960,'height':600})
        page.set_default_timeout(60000)
        page.on('console',console);page.on('pageerror',lambda e:errors.append(str(e)))
        page.on('response',lambda r:errors.append('HTTP '+str(r.status)+': '+r.url) if r.status>=400 else None)
        await page.route('**/supertux2.html',lambda route:route.fulfill(body=html,content_type='text/html'))
        await page.goto(url);await page.wait_for_function('Module.supertuxReady === true',timeout=180000)
        await page.locator('#start_button').click();await page.wait_for_function('Module.supertuxShell.active')
        await page.locator('#canvas').focus();await page.keyboard.press('Enter',delay=150)
        await page.wait_for_function("document.querySelector('#output').textContent.includes('Playing')")
        await page.keyboard.press('Escape',delay=150);await page.keyboard.press('Backquote',delay=100)
        await page.keyboard.type('areaObserver <- newthread(function(){for(local i=0;i<1000;i++){print("AREA_POSITION=" + sector.Tux.get_x() + "," + sector.Tux.get_y() + "," + sector.Tux.get_input_held("right") + "," + sector.Tux.get_input_held("jump"));wait(0.05);}});areaObserver.call();',delay=1)
        await page.keyboard.press('Enter',delay=100);await page.keyboard.press('Backquote',delay=100);await page.keyboard.press('Escape',delay=150)
        async def sample(predicate):
            start=len(samples)
            async with asyncio.timeout(10):
                while len(samples)<=start or not predicate(samples[-1]):await page.wait_for_timeout(50)
            return samples[-1]
        initial=await sample(lambda s:not s['right'] and not s['jump'])
        await page.keyboard.down('ArrowRight');moved=await sample(lambda s:s['right'] and s['x']>initial['x']+8)
        await page.keyboard.up('ArrowRight');await sample(lambda s:not s['right'])
        await page.keyboard.down('Space');jumped=await sample(lambda s:s['jump'] and s['y']<initial['y']-5)
        await page.keyboard.up('Space')
        await page.wait_for_function("[...Module.supertuxAssets.tracks.entries()].some(([path,t])=>path.startsWith('music/forest/') && t.state===0)")
        await page.screenshot(path=str(args.output/'forest.png'))
        assert not errors and not any('[FATAL]' in s or 'runtime error:' in s for s in output), errors
        (args.output/'console.log').write_text('\n'.join(output))
        (args.output/'report.json').write_text(json.dumps(dict(level='world2/welcome_forest.stl',initial=initial,moved=moved,jumped=jumped,musicReady=True,errors=errors),indent=2))
        await browser.close()
        print('Forest level: rendered, moved, jumped, and downloaded music successfully.')


def main():
    parser=argparse.ArgumentParser();parser.add_argument('build',type=Path);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Handler,directory=str(args.build)))
    threading.Thread(target=server.serve_forever,daemon=True).start()
    try:asyncio.run(smoke(args,f'http://127.0.0.1:{server.server_port}/supertux2.html'))
    finally:server.shutdown()


if __name__=='__main__':main()
