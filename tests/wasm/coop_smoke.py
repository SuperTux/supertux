#!/usr/bin/env python3
"""Real compiled co-op/input proof, with actual local workerd/Durable Object.

Gameplay observations use existing Squirrel getters. Long diagnostic scenarios
use existing host-only script fixtures for safety/death/transition; guest input
always goes through the ordinary controller, never through scripting setters.
Linux WebKit and mobile viewport emulation are not physical Safari evidence.
"""
import argparse
import asyncio
import functools
import http.server
import json
from pathlib import Path
import re
import subprocess
import threading
from urllib.request import urlopen
from playwright.async_api import async_playwright
from browser_smoke import Handler, KNOWN_UPSTREAM_UB
from touch_smoke import Fingers, controls

PAD_SCRIPT = '''window.pads=[0,1].map(index=>({index,id:'Xbox 360 Controller',mapping:'standard',connected:true,timestamp:1,axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,touched:false,value:0}))}));navigator.getGamepads=()=>pads.map(p=>p.connected?p:null);window.pad=(i,j,v)=>{pads[i].buttons[j]={pressed:!!v,touched:!!v,value:v};pads[i].timestamp=performance.now();};'''


async def run(args, url):
    report = {'engine': args.browser, 'configuration': json.loads((args.build/'BUILD_INFO.json').read_text()),
              'physical_device_tested': False, 'checks': [], 'samples': {}}
    samples, logs, errors = [], [], []
    pattern = re.compile(r'COOP=([-\d.,truefals]+)\|([-\d.,truefals]+)')
    html = urlopen(url+'index.html').read().decode().replace('var Module = {', 'var Module = {\narguments:["--verbose","--developer"],', 1)
    async with async_playwright() as p:
        launch = dict(args=['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader']) if args.browser == 'chromium' else {}
        if args.webkit_executable: launch['executable_path'] = args.webkit_executable
        browser = await getattr(p,args.browser).launch(**launch)
        guest_browser = await getattr(p,args.browser).launch(**launch) if not args.local_only else None
        report['browser'] = browser.version
        context = await browser.new_context(viewport={'width':844,'height':390},has_touch=True)
        if args.browser == 'chromium': await context.grant_permissions(['local-network-access'])
        if args.local_only: await context.add_init_script(PAD_SCRIPT)
        host = await context.new_page(); host.set_default_timeout(60000)
        def console(message):
            logs.append(message.text)
            with (args.output/'console.log').open('a') as f: f.write(message.type+': '+message.text+'\n')
            match = pattern.search(message.text)
            if match:
                players = []
                for group in match.groups():
                    values=group.split(','); players.append(dict(x=float(values[0]),y=float(values[1]),**{key: values[i+2]=='true' for i,key in enumerate(['left','right','jump','action'])}))
                samples.append(players)
        host.on('console',console);host.on('pageerror',lambda e:errors.append(str(e)))
        await host.route('**/index.html?coop=1',lambda r:r.fulfill(body=html,content_type='text/html'))
        await host.goto(url+'index.html?coop=1')
        await host.wait_for_function('Module.supertuxReady===true',timeout=180000)
        await host.locator('#start_button').click();await host.wait_for_function('Module.supertuxShell.active')
        guest = None
        if not args.local_only:
            await host.locator('#coop_panel summary').click();await host.locator('#coop_create').click()
            await host.wait_for_function("document.querySelector('#coop_status').textContent.includes('Room ready')")
            link=await host.locator('#coop_link').get_attribute('href')
            gc=await guest_browser.new_context(viewport={'width':844,'height':390},has_touch=True)
            guest=await gc.new_page();guest.set_default_timeout(60000)
            guest_requests=[];guest.on('request',lambda r:guest_requests.append(r.url.split('#')[0]))
            await guest.goto(link)
            await guest.wait_for_function('window.supertuxGuest && supertuxGuest.state.connected')
            await host.wait_for_function('Module.supertuxCoop.state.reserved===1')
            assert not await host.evaluate('Module.supertuxCoop.state.enabled')
            report['checks'].append('Two independent browsers join actual Worker/Durable Object before level; remote slot reserved without a local device; menus reject guest gameplay')
            await host.locator('#coop_panel summary').click()
        await host.locator('#canvas').focus()
        # The standard packaged campaign path, including story and world map.
        await host.keyboard.press('Enter',delay=150);await host.wait_for_timeout(1800)
        await host.keyboard.press('Escape',delay=150);await host.wait_for_timeout(1500)
        await host.keyboard.down('ArrowDown');await host.wait_for_timeout(700);await host.keyboard.up('ArrowDown')
        await host.keyboard.press('Space',delay=150);await host.wait_for_timeout(1500)
        await host.keyboard.press('Space',delay=150);await host.wait_for_timeout(600)
        await host.wait_for_function("document.querySelector('#output').textContent.includes('Setting status: Playing')")

        async def script(command, paused=True):
            await host.locator('#canvas').focus()
            if paused: await host.keyboard.press('Escape',delay=150)
            await host.keyboard.press('Backquote',delay=100)
            await host.wait_for_timeout(250)
            await host.keyboard.type(command,delay=8);await host.keyboard.press('Enter',delay=100)
            assert any('> '+command in line for line in logs[-20:]),('Script fixture was not typed',logs[-4:])
            await host.keyboard.press('Backquote',delay=100)
            if paused: await host.keyboard.press('Escape',delay=150)

        async def observer():
            await script('sector.Tux.set_is_intentionally_safe(true);sector.Tux2.set_is_intentionally_safe(true);function cp(p){return p.get_x()+","+p.get_y()+","+p.get_input_held("left")+","+p.get_input_held("right")+","+p.get_input_held("jump")+","+p.get_input_held("action");}coObserver <- newthread(function(){for(local i=0;i<6000;i++){print("COOP="+cp(sector.Tux)+"|"+cp(sector.Tux2));wait(0.05);}});coObserver.call();')
        await observer()
        async def sample(label, predicate=lambda s:True, timeout=10000):
            start=len(samples);deadline=asyncio.get_running_loop().time()+timeout/1000
            while len(samples)<=start or not predicate(samples[-1]):
                if asyncio.get_running_loop().time()>deadline:
                    await host.locator('#canvas').screenshot(path=str(args.output/'failure.png'))
                    state=await host.evaluate('({active:Module.supertuxShell.active,audio:Module.supertuxShell.audioState,remote:Module.supertuxCoop?.state,focus:document.hasFocus(),element:document.activeElement.tagName})')
                    raise AssertionError((label,state,samples[-1:] if samples else [],logs[-6:]))
                await host.wait_for_timeout(40)
            report['samples'][label]=samples[-1];return samples[-1]
        async def p2(control, down):
            if guest:
                key={'left':'ArrowLeft','right':'ArrowRight','jump':'Space','action':'ControlLeft'}[control]
                await (guest.keyboard.down(key) if down else guest.keyboard.up(key))
            else: await host.evaluate('([button,value])=>pad(1,button,value)',[{'left':14,'right':15,'jump':0,'action':2}[control],int(down)])

        start=await sample('spawn',lambda s:all(v['y']<800 for v in s))
        if guest: await guest.wait_for_function('supertuxGuest.state.enabled')
        await host.keyboard.down('ArrowRight');await p2('left',True)
        moved=await sample('independent-movement',lambda s:s[0]['right'] and not s[0]['left'] and s[1]['left'] and not s[1]['right'] and s[0]['x']>start[0]['x']+5 and s[1]['x']<start[1]['x']-5)
        await host.keyboard.up('ArrowRight');await p2('left',False)
        await sample('released',lambda s:not s[0]['right'] and not s[1]['left'])
        await p2('jump',True)
        await sample('guest-jump',lambda s:s[1]['jump'] and not s[0]['jump'] and s[1]['y']<start[1]['y']-10)
        await p2('action',True);await sample('guest-action',lambda s:s[1]['action'] and not s[0]['action'])
        await p2('jump',False);await p2('action',False)
        await sample('neutral',lambda s:all(not v[c] for v in s for c in ['left','right','jump','action']))
        await host.locator('#canvas').screenshot(path=str(args.output/'two-player-game.png'))
        report['checks'].append('Actual packaged welcome_antarctica: independent movement, jump and action on real Tux/Tux2; shared mobile viewport; ordinary held/released controls')
        geometry=await controls(host)
        fingers=Fingers(host,await context.new_cdp_session(host) if args.browser=='chromium' else None)
        await fingers.down(1,geometry['right']);await p2('left',True)
        await sample('touch-and-second-owner',lambda s:s[0]['right'] and s[1]['left'] and not s[1]['right'])
        await fingers.up(1);await p2('left',False)
        await sample('touch-released',lambda s:not s[0]['right'] and not s[1]['left'])
        report['checks'].append('Existing host SDL touch input plus independent P2 input; releasing one source leaves the other owner isolated')
        if guest:
            await host.evaluate(PAD_SCRIPT)
            await host.evaluate("for(const pad of pads){const event=new Event('gamepadconnected');event.gamepad=pad;window.dispatchEvent(event);}")
            await p2('left',True);await sample('hotplug-isolation',lambda s:s[1]['left'] and not s[1]['right'])
            await host.evaluate("for(const pad of pads){pad.connected=false;const event=new Event('gamepaddisconnected');event.gamepad=pad;window.dispatchEvent(event);}")
            await p2('left',False);await sample('hot-unplug-isolation',lambda s:not s[1]['left'])
            report['checks'].append('Actual SDL gamepad connect/disconnect events cannot claim or remove the remote slot')
            # The actual room refuses a third socket before gameplay messages.
            third = await guest.evaluate('''() => new Promise(resolve => {
                const q=new URLSearchParams(location.hash.slice(1)), s=new WebSocket(supertuxGuest.connection.socket.url,['supertux-coop-v1','guest.'+q.get('token')]);
                s.onopen=()=>{s.close();resolve(false)};s.onerror=()=>resolve(true);
            })''')
            assert third
            mismatch = await guest.evaluate('''async () => {
                const r=await (await fetch('/coop/rooms',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({protocol:1,build:'a'.repeat(64)})})).json();
                return await new Promise(resolve=>{const u=new URL('/coop/rooms/'+r.room+'/socket',location.href);u.protocol='ws:';
                    const s=new WebSocket(u,['supertux-coop-v1','host.'+r.host]);
                    s.onopen=()=>s.send(JSON.stringify({type:'hello',protocol:1,build:'b'.repeat(64)}));s.onclose=e=>resolve(e.code);});
            }''')
            assert mismatch==1008,mismatch
            report['checks'].append('Actual Durable Object rejects third client and closes a mismatched build before accepting input')
            assert not any('.wasm' in r or '.data' in r or '/music/' in r for r in guest_requests),guest_requests
            # Down/up in one delivery turn must still produce a physical jump.
            await host.wait_for_timeout(1000);ground=await sample('before-short-tap')
            await guest.keyboard.press('Space',delay=0)
            await sample('short-tap-jump',lambda s:s[1]['y']<ground[1]['y']-10)
            report['checks'].append('Short network tap preserves a jump edge; guest downloads no game world/WASM/DATA/music')
            # Delayed old held state follows a newer neutral state over real WS.
            await guest.evaluate('''() => { const socket=supertuxGuest.connection.socket, send=socket.send.bind(socket); window.delayed=[];window.rawSend=send;socket.send=value=>{const v=JSON.parse(value);if(v.type==='input'&&v.mask===2)delayed.push(value);else send(value);}; }''')
            await p2('right',True);await guest.wait_for_timeout(150);await p2('right',False)
            await guest.evaluate('''() => {supertuxGuest.connection.socket.send=rawSend;for(const value of delayed)rawSend(value);}''')
            await host.wait_for_timeout(300);await sample('late-held-rejected',lambda s:not s[1]['right'])
            await p2('right',True);await sample('before-inactivity',lambda s:s[1]['right'])
            old=await host.evaluate('Module.supertuxCoop.state.generation')
            await guest.evaluate('window.savedRefresh=supertuxGuest.connection.events.refresh;supertuxGuest.connection.events.refresh=()=>{}')
            await host.wait_for_timeout(1100)
            await sample('inactivity-neutral',lambda s:not s[1]['right'])
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await guest.evaluate('supertuxGuest.connection.events.refresh=savedRefresh')
            await p2('right',False)
            report['checks'].append('Delayed old state rejected over relay; stopped input refresh neutralizes held movement and changes generation while connection remains alive')
            await p2('right',True);await sample('before-host-pause',lambda s:s[1]['right'])
            old=await host.evaluate('Module.supertuxCoop.state.generation')
            await host.keyboard.press('Escape',delay=150)
            await host.wait_for_function('!Module.supertuxCoop.state.enabled')
            await host.keyboard.press('Escape',delay=150)
            await guest.wait_for_function('supertuxGuest.state.enabled')
            await sample('pause-neutral',lambda s:not s[1]['right'])
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await p2('right',False)
            await p2('right',True);await sample('before-shell-pause',lambda s:s[1]['right'])
            await host.evaluate("window.dispatchEvent(new Event('blur'))")
            await host.wait_for_function('!Module.supertuxShell.active && !Module.supertuxCoop.state.enabled')
            await host.evaluate("window.dispatchEvent(new Event('focus'))")
            await host.locator('#start_button').click();await guest.wait_for_function('supertuxGuest.state.enabled')
            await sample('shell-resume-neutral',lambda s:not s[1]['right'])
            await p2('right',False)
            report['checks'].append('Host menu Pause and shell blur/trusted Resume clear remote controls; held keys do not replay')

        # The existing engine's co-op death/respawn (action), not a new rule.
        await script('sector.Tux2.kill(true);')
        await host.wait_for_timeout(3300)
        await p2('action',True)
        await sample('coop-respawn',lambda s:abs(s[1]['x']-s[0]['x'])<100 and s[1]['y']<800,timeout=15000)
        await p2('action',False)
        report['checks'].append('Host-only kill fixture exercises existing Player 2 death and ordinary co-op Action respawn without killing Player 1')
        if not guest:
            await host.evaluate("pads[1].connected=false;const event=new Event('gamepaddisconnected');event.gamepad=pads[1];window.dispatchEvent(event)")
            await host.wait_for_timeout(500)
            # Existing game policy pauses after unplugging a local controller.
            await host.keyboard.press('Escape',delay=150)
            await host.evaluate("pads[1].connected=true;pads[1].timestamp=performance.now();const event=new Event('gamepadconnected');event.gamepad=pads[1];window.dispatchEvent(event)")
            await host.wait_for_timeout(500)
            await observer()
            await p2('action',True);await sample('live-rejoin-action',lambda s:s[1]['action']);await p2('action',False)
            await sample('gamepad-live-rejoin',lambda s:s[1]['y']<800)
            report['checks'].append('Actual SDL browser gamepad removal and live rejoin recreate a real P2 and preserve P1')

        # Touch the packaged checkpoint bell via a host fixture, then exercise
        # the ordinary all-dead restart path. No campaign assets/rules change.
        await script('sector.Tux.set_pos(5360,512);sector.Tux2.set_pos(5360,512);')
        await sample('checkpoint-bell',lambda s:all(v['x']>5300 and v['y']<800 for v in s))
        await host.wait_for_timeout(600)
        old = await host.evaluate('Module.supertuxCoop.state.generation') if guest else 0
        await script('sector.Tux.kill(true);sector.Tux2.kill(true);')
        await host.wait_for_timeout(4300)
        await observer()
        checkpoint=await sample('all-dead-checkpoint-restart',lambda s:all(abs(v['x']-5360)<5 and v['y']<800 for v in s))
        if guest:
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await guest.wait_for_function('supertuxGuest.state.enabled')
            await p2('right',True);await sample('input-after-restart',lambda s:s[1]['right'] and not s[0]['right'])
            await p2('right',False)
        report['checks'].append('Actual packaged bell collision records checkpoint; both players die and restart there; controller ownership/input survive restart')

        # A later, packaged level has actual door/sector mechanics. Host-only
        # load fixture keeps the same controllers; this is not campaign traversal.
        await script('load_level("levels/bonus1/area_42.stl");',paused=False)
        await host.wait_for_timeout(1500);await host.keyboard.press('Enter',delay=150);await host.wait_for_timeout(400)
        await observer()
        await script('Level.spawn("sector2","main");')
        await host.wait_for_timeout(700);await observer()
        await sample('second-sector',lambda s:all(v['y']<2000 for v in s))
        if guest:
            await guest.wait_for_function('supertuxGuest.state.enabled')
            await p2('left',True);await sample('input-after-sector',lambda s:s[1]['left'] and not s[0]['left']);await p2('left',False)
        report['checks'].append('Existing load_level/spawn fixtures run packaged area_42 and a second sector; established input source survives level/sector generations')

        # Finish the nested diagnostic level, then complete the campaign level
        # with its existing sequence. Temporary script controllers borrow safely.
        await script('Level.finish(false);',paused=False)
        await host.wait_for_timeout(1000)
        await script('sector.Tux.trigger_sequence("endsequence");',paused=False)
        await host.wait_for_timeout(7500)
        await host.keyboard.press('Space',delay=150);await host.wait_for_timeout(600)
        if guest: assert not await host.evaluate('Module.supertuxCoop.state.enabled')
        report['checks'].append('Existing end sequence temporarily controls players and returns to the world map; guest cannot control host progression')

        if guest:
            # Membership is absent from the host's existing save schema.
            saved=await host.evaluate('''() => {Module.ccall('save_config',null,[],[]);const fs=Module.FS,root=Module.supertuxStorage.root;
              const files=fs.readdir(root+'profile1').filter(n=>n.endsWith('.stsg'));return files.map(n=>fs.readFile(root+'profile1/'+n,{encoding:'utf8'}));}''')
            assert saved and all('(num_players 1)' in value and 'coop/rooms' not in value for value in saved),saved
            await guest.evaluate("supertuxGuest.connection.close('Acceptance test disconnect')")
            await host.wait_for_function('Module.supertuxCoop.state.reserved===-1')
            report['checks'].append('Host progression save retains existing schema with one persistent local player; real socket disconnect clears remote source')
        else:
            report['checks'].append('Two independently mapped browser gamepads and keyboard P1 remain local sources')
        for line in logs:
            if 'runtime error:' in line or 'Aborted(' in line or 'ERROR:' in line:
                if args.record_known_ub and any(re.search(pattern,line) for pattern in KNOWN_UPSTREAM_UB): continue
                errors.append(line)
        assert not errors,errors[:10]
        await context.close();await browser.close()
        if guest_browser: await guest_browser.close()
    (args.output/'report.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2),flush=True)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('build',type=Path);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--browser',choices=['chromium','webkit'],default='chromium');parser.add_argument('--webkit-executable')
    parser.add_argument('--url');parser.add_argument('--local-only',action='store_true');parser.add_argument('--record-known-ub',action='store_true');args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True);relay=None;server=None
    if args.url: url=args.url.rstrip('/')+'/'
    elif args.local_only:
        server=http.server.ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Handler,directory=str(args.build)))
        threading.Thread(target=server.serve_forever,daemon=True).start();url=f'http://127.0.0.1:{server.server_port}/'
    else:
        root=Path(__file__).parents[2]
        relay=subprocess.Popen(['node',str(root/'tools/web/coop/preview.mjs'),str(args.build.resolve()),'0'],stdout=subprocess.PIPE,stderr=(args.output/'relay.log').open('w'),text=True)
        while True:
            line=relay.stdout.readline()
            if line.startswith('INPUT_PROOF_READY '):url=line.split()[1].split('index.html')[0];break
            if relay.poll() is not None:raise RuntimeError('Local relay failed; see relay.log')
    try:asyncio.run(run(args,url))
    finally:
        if relay:relay.terminate();relay.wait(timeout=20)
        if server:server.shutdown()


if __name__=='__main__':main()
