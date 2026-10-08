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
import selectors
import subprocess
import threading
import time
from urllib.request import urlopen
from playwright.async_api import async_playwright
from browser_smoke import Handler, KNOWN_UPSTREAM_UB
from touch_smoke import Fingers, controls

PAD_SCRIPT = '''window.pads=[0,1].map(index=>({index,id:'Xbox 360 Controller',mapping:'standard',connected:true,timestamp:1,axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,touched:false,value:0}))}));navigator.getGamepads=()=>pads.map(p=>p.connected?p:null);window.pad=(i,j,v)=>{pads[i].buttons[j]={pressed:!!v,touched:!!v,value:v};pads[i].timestamp=performance.now();};void 0;'''


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
        async def status_after(text, start=0):
            for _ in range(600):
                if any('Setting status: '+text in line for line in logs[start:]):return
                await host.wait_for_timeout(100)
            await host.locator('#canvas').screenshot(path=str(args.output/'transition-failure.png'))
            state=await host.evaluate('({active:Module.supertuxShell.active,focus:document.hasFocus(),element:document.activeElement.tagName})')
            raise AssertionError(('Screen transition',text,state,logs[-8:]))
        async def key_until_status(key, text):
            # SDL consumes ordinary keyboard state once per game frame. A
            # short down/up can collapse in one slow software-rendered frame;
            # hold the host menu action until its actual transition is seen.
            start=len(logs)
            await host.keyboard.down(key)
            try:await status_after(text,start)
            finally:await host.keyboard.up(key)
            await host.wait_for_timeout(500)
        await status_after('In main menu');await host.wait_for_timeout(500)
        guest = None
        if not args.local_only:
            await host.locator('#coop_panel summary').click();await host.locator('#coop_create').click()
            await host.wait_for_function("document.querySelector('#coop_status').textContent.includes('Room ready')")
            link=await host.locator('#coop_link').get_attribute('href')
            gc=await guest_browser.new_context(viewport={'width':844,'height':390},has_touch=True)
            guest=await gc.new_page();guest.set_default_timeout(60000)
            guest.on('pageerror',lambda e:errors.append('Guest: '+str(e)))
            guest_requests=[];guest.on('request',lambda r:guest_requests.append(r.url.split('#')[0]))
            await guest.goto(link)
            await guest.wait_for_function('window.supertuxGuest && supertuxGuest.state.connected')
            await host.wait_for_function('Module.supertuxCoop.state.reserved===1')
            await host.evaluate("window.proofCloses=[];Module.supertuxCoop.connection.socket.addEventListener('close',e=>proofCloses.push({code:e.code,reason:e.reason,time:Date.now()}))")
            await guest.evaluate("window.proofCloses=[];supertuxGuest.connection.socket.addEventListener('close',e=>proofCloses.push({code:e.code,reason:e.reason,time:Date.now()}))")
            assert not await host.evaluate('Module.supertuxCoop.state.enabled')
            report['checks'].append('Two independent browsers join actual Worker/Durable Object before level; remote slot reserved without a local device; menus reject guest gameplay')
            # A briefly stalled receiver must coalesce diagnostic status, not
            # disconnect or accumulate an unbounded history. Input edges keep
            # their separate strict receive window.
            await guest.evaluate('''() => {const socket=supertuxGuest.connection.socket,send=socket.send.bind(socket);
                window.heldCredits=[];window.creditSend=send;socket.send=value=>{
                    if(JSON.parse(value).type==='seen')heldCredits.push(value);else send(value);};}''')
            await host.evaluate("for(let i=0;i<40;i++)Module.supertuxCoop.connection.send({type:'ack',sequence:0})")
            await guest.wait_for_function('heldCredits.length===32',timeout=5000)
            assert await guest.evaluate('supertuxGuest.state.connected')
            await guest.evaluate('''() => {const socket=supertuxGuest.connection.socket;socket.send=creditSend;
                for(const value of heldCredits)creditSend(value);window.heldCredits=[];}''')
            await guest.wait_for_timeout(200)
            assert await guest.evaluate('supertuxGuest.state.connected')
            report['checks'].append('Actual relay fills its 32-message receive window; latest diagnostic status drains after delayed credits without disconnecting or queuing history')
            await host.locator('#coop_panel summary').click()
        async def guest_active():
            try:await guest.wait_for_function('supertuxGuest.state.enabled',timeout=15000)
            except Exception:
                diagnostic={'host':await host.evaluate('({state:Module.supertuxCoop.state,status:document.querySelector("#coop_status").textContent,closes:proofCloses})'),
                            'guest':await guest.evaluate('({state:supertuxGuest.state,status:document.querySelector("#guest_status").textContent,closes:proofCloses})')}
                (args.output/'timeout-state.json').write_text(json.dumps(diagnostic,indent=2))
                raise AssertionError(diagnostic)
        await host.locator('#canvas').focus()
        # The standard packaged campaign path, including story and world map.
        # Shell Start/room clicks can leave the mouse hovering another menu
        # row. Select Start Game directly instead of assuming Enter selects it.
        entry_fingers=Fingers(host,await context.new_cdp_session(host) if args.browser=='chromium' else None)
        entry_geometry=await controls(host);vp=entry_geometry['viewport']
        main_y=vp['y']+(480/2+35-8*24/2+12)*(vp['height']/480)
        entry_start=len(logs)
        await entry_fingers.tap([vp['x']+vp['width']/2,main_y])
        # Story is a GameSession ("Playing"); "Watching a cutscene" is the
        # separate LevelIntro screen, which appears only after map entry.
        await status_after('Playing',entry_start);await host.wait_for_timeout(500)
        await key_until_status('Escape','In worldmap')
        await host.keyboard.down('ArrowDown');await host.wait_for_timeout(1000);await host.keyboard.up('ArrowDown')
        await host.wait_for_timeout(500)
        await key_until_status('Space','Watching a cutscene')
        await key_until_status('Space','Playing')

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
            await script('sector.Tux.set_is_intentionally_safe(true);sector.Tux2.set_is_intentionally_safe(true);function cp(p){return p.get_x()+","+p.get_y()+","+p.get_input_held("left")+","+p.get_input_held("right")+","+p.get_input_held("jump")+","+p.get_input_held("action");}coObserver <- newthread(function(){for(local i=0;i<6000;i++){try{print("COOP="+cp(sector.Tux)+"|"+cp(sector.Tux2));}catch(e){return;}wait(0.05);}});coObserver.call();')
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
                key={'left':'ArrowLeft','right':'ArrowRight','jump':'Space','action':'ControlLeft','up':'ArrowUp'}[control]
                await (guest.keyboard.down(key) if down else guest.keyboard.up(key))
            else: await host.evaluate('([button,value])=>pad(1,button,value)',[{'left':14,'right':15,'jump':0,'action':2,'up':12}[control],int(down)])

        start=await sample('spawn',lambda s:all(v['y']<800 for v in s))
        if guest: await guest_active()
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
        fingers=entry_fingers
        await fingers.down(1,geometry['right']);await p2('left',True)
        await sample('touch-and-second-owner',lambda s:s[0]['right'] and s[1]['left'] and not s[1]['right'])
        await fingers.up(1);await p2('left',False)
        await sample('touch-released',lambda s:not s[0]['right'] and not s[1]['left'])
        report['checks'].append('Existing host SDL touch input plus independent P2 input; releasing one source leaves the other owner isolated')
        if guest:
            button=guest.locator('[data-control="2"]')
            await button.scroll_into_view_if_needed()
            await button.evaluate("e=>e.addEventListener('pointerdown',v=>window.guestPointer=v.pointerId)")
            box=await button.bounding_box()
            await guest.mouse.move(box['x']+box['width']/2,box['y']+box['height']/2)
            await guest.mouse.down();await sample('guest-button-held',lambda s:s[1]['right'] and not s[0]['right'])
            await button.evaluate("e=>e.dispatchEvent(new PointerEvent('pointercancel',{pointerId:guestPointer,bubbles:true}))")
            await sample('guest-button-cancelled',lambda s:not s[1]['right']);await guest.mouse.up()
            await guest.mouse.down();await sample('guest-capture-held',lambda s:s[1]['right'])
            assert await button.evaluate('e=>e.hasPointerCapture(guestPointer)'),await guest.evaluate('supertuxGuest.state')
            await button.evaluate("e=>e.dispatchEvent(new PointerEvent('lostpointercapture',{pointerId:guestPointer,bubbles:true}))")
            await guest.mouse.move(box['x']-20,box['y']-20)
            await sample('guest-capture-released',lambda s:not s[1]['right']);await guest.mouse.up()
            report['checks'].append('Diagnostic guest buttons use real Pointer Events/capture; injected cancellation and lost capture release P2 without affecting P1')
            await host.evaluate(PAD_SCRIPT)
            await host.evaluate("for(const pad of pads){const event=new Event('gamepadconnected');event.gamepad=pad;window.dispatchEvent(event);}")
            await p2('left',True);await sample('hotplug-isolation',lambda s:s[1]['left'] and not s[1]['right'])
            await host.evaluate("for(const pad of pads){pad.connected=false;const event=new Event('gamepaddisconnected');event.gamepad=pad;window.dispatchEvent(event);}")
            await host.wait_for_function('!Module.supertuxCoop.state.enabled')
            await p2('left',False)
            await host.keyboard.press('Escape',delay=150)
            await guest_active()
            await sample('hot-unplug-isolation',lambda s:not s[1]['left'])
            report['checks'].append('Actual SDL gamepad connect/disconnect events cannot claim or remove the remote slot')
            # The actual room refuses a third socket before gameplay messages.
            third = await guest.evaluate('''() => new Promise(resolve => {
                const q=new URLSearchParams(location.hash.slice(1)), s=new WebSocket(supertuxGuest.connection.socket.url,['supertux-coop-v2','guest.'+q.get('token')]);
                s.onopen=()=>{s.close();resolve(false)};s.onerror=()=>resolve(true);
            })''')
            assert third
            mismatch = await guest.evaluate('''async () => {
                const r=await (await fetch('/coop/rooms',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({protocol:2,build:'a'.repeat(64)})})).json();
                return await new Promise(resolve=>{const u=new URL('/coop/rooms/'+r.room+'/socket',location.href);u.protocol='ws:';
                    const s=new WebSocket(u,['supertux-coop-v2','host.'+r.host]);
                    s.onopen=()=>s.send(JSON.stringify({type:'hello',protocol:2,build:'b'.repeat(64)}));s.onclose=e=>resolve(e.code);});
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
            await guest_active()
            await sample('pause-neutral',lambda s:not s[1]['right'])
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await p2('right',False)
            await p2('right',True);await sample('before-shell-pause',lambda s:s[1]['right'])
            await host.evaluate("window.dispatchEvent(new Event('blur'))")
            await host.wait_for_function('!Module.supertuxShell.active && !Module.supertuxCoop.state.enabled')
            await host.evaluate("window.dispatchEvent(new Event('focus'))")
            await host.locator('#start_button').click();await guest_active()
            await sample('shell-resume-neutral',lambda s:not s[1]['right'])
            await p2('right',False)
            report['checks'].append('Host menu Pause and shell blur/trusted Resume clear remote controls; held keys do not replay')

        # The existing engine's co-op death/respawn (action), not a new rule.
        # Death uses g_game_time, which can lag wall time in Debug/software
        # rendering. Wait on the same simulation clock before pressing Action.
        await script('sector.Tux2.kill(true);coopDeathWait <- newthread(function(){wait(3.1,true);print("COOP_DEATH_READY");});coopDeathWait.call();')
        await host.wait_for_function("document.querySelector('#output').textContent.includes('[SCRIPTING] COOP_DEATH_READY')",timeout=30000)
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
        await script('sector.Tux.kill(true);sector.Tux2.kill(true);coopRestartWait <- newthread(function(){wait(3.4,true);print("COOP_RESTART_READY");});coopRestartWait.call();')
        await host.wait_for_function("document.querySelector('#output').textContent.includes('[SCRIPTING] COOP_RESTART_READY')",timeout=30000)
        await observer()
        checkpoint=await sample('all-dead-checkpoint-restart',lambda s:all(abs(v['x']-5360)<5 and v['y']<800 for v in s))
        if guest:
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await guest_active()
            await p2('right',True);await sample('input-after-restart',lambda s:s[1]['right'] and not s[0]['right'])
            await p2('right',False)
        report['checks'].append('Actual packaged bell collision records checkpoint; both players die and restart there; controller ownership/input survive restart')

        # Complete the actual campaign level via its existing end sequence.
        # It temporarily borrows script controllers, then returns to the map.
        end_start=len(logs)
        await script('sector.Tux.trigger_sequence("endsequence");',paused=False)
        for _ in range(600):
            if any('Setting status: In worldmap' in line for line in logs[end_start:]): break
            await host.wait_for_timeout(100)
        assert any('Setting status: In worldmap' in line for line in logs[end_start:]),logs[-6:]
        await host.wait_for_timeout(500)
        if guest: assert not await host.evaluate('Module.supertuxCoop.state.enabled')
        report['checks'].append('Existing end sequence temporarily controls players and returns to the world map; guest cannot control host progression')
        title_start=len(logs)
        await host.keyboard.press('Escape',delay=150);await host.wait_for_timeout(500)
        await host.keyboard.press('ArrowUp',delay=150);await host.wait_for_timeout(250)
        await host.keyboard.press('Enter',delay=150)
        for _ in range(100):
            if any('Setting status: In main menu' in line for line in logs[title_start:]): break
            await host.wait_for_timeout(100)
        assert any('Setting status: In main menu' in line for line in logs[title_start:]),logs[-6:]

        # Normal GameManager/Levelset entry from the title screen, rather than
        # nesting another GameSession inside a live campaign level.
        await host.locator('#coop_panel').evaluate('(e)=>e.open=true')
        await host.locator('#coop_forest').click()
        await host.wait_for_timeout(2000)
        await host.locator('#coop_panel').evaluate('(e)=>e.open=false')
        await host.locator('#canvas').focus()
        await observer()
        await sample('forest-entry',lambda s:all(v['y']>800 and v['x']<500 for v in s))
        await script('sector.Tux.set_pos(9760,800);sector.Tux2.set_pos(9760,800);')
        await sample('at-forest-door',lambda s:all(v['x']>9700 for v in s))
        old = await host.evaluate('Module.supertuxCoop.state.generation') if guest else 0
        if guest: await guest_active()
        await p2('up',True)
        await host.wait_for_timeout(2200);await p2('up',False)
        await observer()
        await sample('second-sector',lambda s:all(750<v['x']<1000 and v['y']>2500 for v in s))
        if guest:
            assert await host.evaluate('Module.supertuxCoop.state.generation')!=old
            await guest_active()
        await p2('left',True);await sample('input-after-sector',lambda s:s[1]['left'] and not s[0]['left']);await p2('left',False)
        report['checks'].append('Packaged tux_builder: ordinary P2 Up activates a real door and moves both players to mountain; input ownership survives sector generation')

        if not guest:
            # Existing host menus really pop the controller, beyond the hotplug
            # path which only despawns its Player. Flush-before-pop is exercised
            # while a compiled level and its script observer still exist.
            async def menu_key(key):
                await host.keyboard.press(key,delay=150);await host.wait_for_timeout(400)
            await menu_key('Escape')
            await menu_key('ArrowUp');await menu_key('ArrowUp');await menu_key('Enter') # Options
            for _ in range(3):await menu_key('ArrowRight') # Video -> Extras
            await menu_key('Enter');await menu_key('Enter') # Extras -> Multiplayer
            for _ in range(3):await menu_key('ArrowDown')
            await menu_key('Enter') # Manage Players
            await menu_key('ArrowUp');await menu_key('ArrowUp');await menu_key('Enter') # Remove last
            await menu_key('Enter') # existing confirmation, default Yes
            await host.locator('#canvas').screenshot(path=str(args.output/'controller-removed.png'))
            await menu_key('ArrowDown');await menu_key('ArrowDown');await menu_key('Enter') # Add controller
            await menu_key('Escape');await menu_key('Escape') # menu exit, then Resume
            await host.evaluate("pads[1].connected=false;const event=new Event('gamepaddisconnected');event.gamepad=pads[1];window.dispatchEvent(event)")
            await host.wait_for_timeout(400);await menu_key('Escape') # existing unplug pause
            await host.evaluate("pads[1].connected=true;pads[1].timestamp=performance.now();const event=new Event('gamepadconnected');event.gamepad=pads[1];window.dispatchEvent(event)")
            await host.wait_for_timeout(500);await observer()
            await p2('action',True);await sample('controller-recreated-respawn',lambda s:abs(s[1]['x']-s[0]['x'])<100 and 2500<s[1]['y']<3400);await p2('action',False)
            before=await sample('controller-recreated-neutral')
            await p2('right',True);await sample('controller-recreated',lambda s:s[1]['right'] and not s[0]['right'] and s[1]['x']>before[1]['x']+5);await p2('right',False)
            report['checks'].append('Existing Remove Last Player confirmation destroys P2 before controller pop; Add Controller and real device rebind recreate independent P2')

        if guest:
            await p2('right',True);await sample('before-disconnect',lambda s:s[1]['right'])
            await guest.evaluate("supertuxGuest.connection.close('Acceptance test disconnect')")
            await host.wait_for_function('Module.supertuxCoop.state.reserved===-1')
            await sample('disconnect-neutral',lambda s:not s[1]['right'])
            await guest.locator('#guest_join').click()
            await host.wait_for_function('Module.supertuxCoop.state.joinRejected')
            assert not await guest.evaluate('supertuxGuest.state.enabled')
            report['checks'].append('Held-button socket disconnect clears P2; in-level rejoin is rejected instead of reclaiming a live player')
        await script('Level.finish(true);',paused=False)
        await host.wait_for_timeout(1800)
        if guest:
            await guest.locator('#guest_join').click()
            await host.wait_for_function('Module.supertuxCoop.state.reserved===1')
            await host.locator('#coop_panel').evaluate('(e)=>e.open=true')
            await host.locator('#coop_antarctica').click()
            await host.wait_for_timeout(1800)
            await host.locator('#coop_panel').evaluate('(e)=>e.open=false')
            await host.locator('#canvas').focus();await observer()
            await guest_active()
            await sample('rejoin-neutral',lambda s:not s[1]['right'])
            await p2('right',False);await p2('right',True)
            await sample('rejoin-fresh-input',lambda s:s[1]['right'] and not s[0]['right'])
            await guest.evaluate("Object.defineProperty(document,'hidden',{configurable:true,get:()=>true});document.dispatchEvent(new Event('visibilitychange'))")
            await host.wait_for_function('Module.supertuxCoop.state.reserved===-1')
            await sample('guest-background-neutral',lambda s:not s[1]['right'])
            await script('Level.finish(true);',paused=False);await host.wait_for_timeout(1600)
            report['checks'].append('Return to title, explicit rejoin and new level accept fresh P2 input; guest background closes socket and neutralizes held input')

        # Verify the existing persistence path for both ordinary local co-op
        # and transient remote membership, including a real page reload.
        await host.evaluate("Module.ccall('save_config',null,[],[])")
        snapshot='''() => {const fs=Module.FS,root=Module.supertuxStorage.root;
          const saves=Object.fromEntries(fs.readdir(root+'profile1').filter(n=>n.endsWith('.stsg')).map(n=>[n,fs.readFile(root+'profile1/'+n,{encoding:'utf8'})]));
          return {saves,config:fs.readFile(root+'config',{encoding:'utf8'})};}'''
        saved=await host.evaluate(snapshot)
        players=1 if guest else 2
        assert saved['saves'] and all(f'(num_players {players})' in value and 'coop/rooms' not in value for value in saved['saves'].values()),saved
        assert await host.evaluate('Module.supertuxStorage.flush()')
        if guest:
            await guest.evaluate("supertuxGuest.connection.close('Acceptance test disconnect')")
            await host.wait_for_function('Module.supertuxCoop.state.reserved===-1')
            report['checks'].append('Host progression save retains existing schema with one persistent local player; real socket disconnect clears remote source')
        else:
            report['checks'].append('Two independently mapped browser gamepads and keyboard P1 remain local sources')
        await host.reload();await host.wait_for_function('Module.supertuxReady===true',timeout=180000)
        assert await host.evaluate(snapshot)==saved
        assert await host.evaluate('Module.supertuxCoop.state.reserved')==0
        report['checks'].append('Existing IndexedDB hydration restores exact host config and progression saves after reload; network membership is absent')
        known=[line for line in logs if 'runtime error:' in line and args.record_known_ub and
               any(re.search(pattern,line) for pattern in KNOWN_UPSTREAM_UB)]
        report['known_upstream_sanitizer_diagnostics']=known
        for line in logs:
            if re.search(r'undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function|AN ERROR HAS OCCURRED|Error waking VM|Squirrel exception:',line) and line not in known:
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
        deadline=time.monotonic()+180
        selector=selectors.DefaultSelector();selector.register(relay.stdout,selectors.EVENT_READ)
        try:
            while True:
                if time.monotonic()>deadline:raise RuntimeError('Local relay startup timed out; see relay.log')
                if not selector.select(1):continue
                line=relay.stdout.readline()
                if line.startswith('INPUT_PROOF_READY '):url=line.split()[1].split('index.html')[0];break
                if relay.poll() is not None:raise RuntimeError('Local relay failed; see relay.log')
        except BaseException:
            relay.terminate();relay.wait(timeout=20);raise
        finally:selector.close()
    try:asyncio.run(run(args,url))
    finally:
        if relay:relay.terminate();relay.wait(timeout=20)
        if server:server.shutdown()


if __name__=='__main__':main()
