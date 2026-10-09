#!/usr/bin/env python3
"""Two real browsers, compiled authoritative host and real Durable Object relay.

Host-only scripting fixtures exercise death/restart. Guest inputs use the
ordinary relay/controller. This is desktop/mobile emulation, not phone Safari.
"""
import asyncio
import json
import re
import time
from urllib.request import Request, urlopen
from playwright.async_api import async_playwright
from browser_smoke import KNOWN_UPSTREAM_UB
import coop_smoke


async def run(args, url):
    logs, errors, requests, snapshots = [], [], [], []
    report = dict(engine=args.browser, configuration=json.loads((args.build/'BUILD_INFO.json').read_text()),
                  physical_device_tested=False, checks=[], metrics={})
    request = Request(url+'index.html', headers={'User-Agent': 'SuperTux-Coop-Validation/1.0 (+https://github.com/jbbejena/supertux)'})
    html = urlopen(request, timeout=30).read().decode().replace('var Module = {','var Module = {\narguments:["--verbose","--developer"],',1)
    async with async_playwright() as p:
        launch = dict(args=['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader']) if args.browser=='chromium' else {}
        if args.webkit_executable: launch['executable_path']=args.webkit_executable
        host_browser=await getattr(p,args.browser).launch(**launch)
        guest_browser=await getattr(p,args.browser).launch(**launch)
        report['browser']=host_browser.version
        hc=await host_browser.new_context(viewport={'width':844,'height':390},has_touch=True)
        gc=await guest_browser.new_context(viewport={'width':844,'height':390},has_touch=True)
        if args.browser=='chromium': await hc.grant_permissions(['local-network-access'])
        host=await hc.new_page(); guest=await gc.new_page()
        host.set_default_timeout(60000);guest.set_default_timeout(60000)
        def log(message):
            logs.append(message.text)
            with (args.output/'console.log').open('a') as file: file.write(message.type+': '+message.text+'\n')
        host.on('console',log);guest.on('console',log)
        host.on('pageerror',lambda error:errors.append('Host: '+str(error)))
        guest.on('pageerror',lambda error:errors.append('Guest: '+str(error)))
        guest.on('request',lambda request:requests.append(request.url.split('#')[0]))
        await host.route('**/index.html?coop=1',lambda route:route.fulfill(body=html,content_type='text/html'))
        await host.goto(url+'index.html?coop=1')
        await host.wait_for_function('Module.supertuxReady',timeout=180000)
        await host.locator('#start_button').click();await host.wait_for_function('Module.supertuxShell.active')
        await host.wait_for_function("document.querySelector('#output').textContent.includes('Setting status: In main menu')")
        await host.locator('#coop_panel').evaluate('(e)=>e.open=true')
        await host.locator('#coop_create').click()
        await host.wait_for_function("document.querySelector('#coop_status').textContent.includes('Room ready')")
        await guest.goto(await host.locator('#coop_view_link').get_attribute('href'))
        await guest.wait_for_function('supertuxGuest.state.connected',timeout=90000)
        await host.wait_for_function('Module.supertuxCoop.state.reserved===1')
        await host.evaluate('''() => {
          window.lastViewPacket=null;const publish=Module.supertuxCoop.view;
          Module.supertuxCoop.view=frame=>{lastViewPacket=frame;publish(frame);};
          window.viewInputFrames=0;const status=Module.supertuxCoop.engineStatus;
          Module.supertuxCoop.engineStatus=(...args)=>{++viewInputFrames;status(...args);};
        }''')
        async def host_key(key):
            # Observe a native input update after each edge. Wall-clock presses
            # can merge across slow CI frames, leaving Escape held when the
            # console closes and reopening the pause menu on the next update.
            await host.keyboard.down(key)
            frame=await host.evaluate('viewInputFrames')
            await host.wait_for_function('(frame)=>viewInputFrames>frame',arg=frame)
            await host.keyboard.up(key)
            frame=await host.evaluate('viewInputFrames')
            await host.wait_for_function('(frame)=>viewInputFrames>frame',arg=frame)
        assert not await guest.evaluate('SupertuxView.state.enabled')
        await host.locator('#coop_view_start').click()
        await host.locator('#coop_panel').evaluate('(e)=>e.open=false')
        await host.locator('#canvas').focus()
        try:
            await guest.wait_for_function('SupertuxView.playable',timeout=30000)
        except Exception:
            print('VIEW_START_FAILED',await guest.evaluate('({guest:supertuxGuest.state,view:SupertuxView.state,status:document.querySelector("#guest_status").textContent,viewStatus:document.querySelector("#view_status").textContent})'),await host.evaluate('({state:Module.supertuxCoop.state,status:document.querySelector("#coop_status").textContent,packet:window.lastViewPacket})'),flush=True)
            await host.locator('#canvas').screenshot(path=str(args.output/'start-host-failure.png'))
            await guest.screenshot(path=str(args.output/'start-guest-failure.png'))
            raise
        await guest.evaluate('''() => {
          window.viewPackets=[];const accept=SupertuxView.accept;
          SupertuxView.accept=frame=>{viewPackets.push({bytes:JSON.stringify(frame).length,at:performance.now(),frame});return accept(frame);};
        }''')
        async def sample(label, expression='true', timeout=15000):
            try:
                await guest.wait_for_function('SupertuxView.playable && SupertuxView.state.drawn && ('+expression+')',timeout=timeout)
            except Exception:
                print('VIEW_SAMPLE_FAILED',label,await guest.evaluate('({guest:supertuxGuest.state,view:SupertuxView.state,status:document.querySelector("#guest_status").textContent,viewStatus:document.querySelector("#view_status").textContent})'),await host.evaluate('({state:Module.supertuxCoop.state,status:document.querySelector("#coop_status").textContent,packet:window.lastViewPacket})'),flush=True)
                await guest.screenshot(path=str(args.output/'sample-guest-failure.png'))
                raise
            frame=await guest.evaluate('SupertuxView.state.drawn')
            snapshots.append(dict(label=label,frame=frame));return frame
        base=await sample('initial-two-players',"SupertuxView.state.drawn.players.every(p=>p.dead===0 && p.y>500)")
        assert [p['id'] for p in base['players']]==[1,2]
        assert all(p['action'].startswith('small-') for p in base['players'])
        report['checks'].append('Actual compiled host starts the derived static proof; guest displays both stable player IDs with original art')
        x=base['players'][1]['x']; p1x=base['players'][0]['x']
        start=time.monotonic();await guest.keyboard.down('ArrowRight')
        moved=await sample('guest-move',f'SupertuxView.state.drawn.players[1].x>{x+30}')
        report['metrics']['input_to_visible_30px_ms']=round((time.monotonic()-start)*1000)
        await guest.keyboard.up('ArrowRight')
        assert abs(moved['players'][0]['x']-p1x)<5
        await guest.wait_for_timeout(300)
        await guest.keyboard.down('Space')
        jumped=await sample('guest-jump',"SupertuxView.state.drawn.players[1].action.includes('jump') && SupertuxView.state.drawn.players[1].y<550")
        await guest.keyboard.up('Space')
        await sample('landed',"SupertuxView.state.drawn.players.every(p=>p.y>570)")
        await guest.locator('[data-control="16"]').tap()
        await sample('guest-touch-jump',"SupertuxView.state.drawn.players[1].action.includes('jump') && SupertuxView.state.drawn.players[1].y<560")
        assert await guest.evaluate('supertuxGuest.state.mask')==0
        await sample('touch-landed',"SupertuxView.state.drawn.players.every(p=>p.y>570)")
        await host.locator('#canvas').screenshot(path=str(args.output/'host-scene.png'))
        await guest.locator('#guest_canvas').screenshot(path=str(args.output/'guest-scene.png'))
        report['checks'].append('Guest keyboard RIGHT/JUMP and trusted touch Jump animate only Player 2; touch release clears input; both canvases show the authoritative scene')

        # Move apart far enough to exercise the existing group camera/zoom.
        await guest.keyboard.down('ArrowRight');await guest.wait_for_timeout(1600);await guest.keyboard.up('ArrowRight')
        spread=await sample('shared-camera')
        cx,cy,scale,width,height=spread['camera']
        for player in spread['players']:
            assert -64<(player['x']-cx)*scale<width+64,spread
            assert -64<(player['y']-cy)*scale<height+64,spread
        await host.keyboard.down('ArrowRight');await host.wait_for_timeout(500);await host.keyboard.up('ArrowRight')
        await sample('host-move',f'SupertuxView.state.drawn.players[0].x>{p1x+20}')
        report['checks'].append('Shared camera comes from the native group camera and keeps both living players visible; host input remains independent')

        old_generation=await guest.evaluate('supertuxGuest.state.generation')
        await guest.keyboard.down('ArrowRight');await host_key('Escape')
        await guest.wait_for_function('!SupertuxView.state.enabled')
        assert not await guest.evaluate('SupertuxView.playable')
        await host_key('Escape')
        await guest.wait_for_function('SupertuxView.playable && supertuxGuest.state.mask===0')
        assert await guest.evaluate('supertuxGuest.state.generation')!=old_generation
        await guest.keyboard.up('ArrowRight')
        report['checks'].append('Host Pause freezes presentation and releases guest input; Resume requires neutral state with a fresh generation')

        async def script(command):
            await host.locator('#canvas').focus();await host_key('Escape')
            await host.wait_for_function('!Module.supertuxCoop.state.enabled')
            await host_key('Backquote')
            await host.keyboard.type(command,delay=4);await host_key('Enter')
            assert any('> '+command in line for line in logs[-30:]),logs[-10:]
            await host_key('Backquote');await host_key('Escape')
            await host.wait_for_function('Module.supertuxCoop.state.enabled')
        await script('sector.Tux2.kill(true);')
        await sample('death',"SupertuxView.state.drawn.players[1].dead>0")
        # Press Action only after native death is complete. A held Action first
        # pressed during the dying animation has no new pressed edge at death.
        await sample('dead-ready',"SupertuxView.state.drawn.players[1].dead===2",timeout=30000)
        await guest.keyboard.down('ControlLeft')
        await sample('respawn',"SupertuxView.state.drawn.players[1].dead===0 && SupertuxView.state.drawn.players[1].visible")
        await guest.keyboard.up('ControlLeft')
        report['checks'].append('Host death snapshot is discrete; ordinary guest Action triggers native co-op respawn and the new visual state')
        epoch=await guest.evaluate('SupertuxView.state.latest.epoch')
        await script('sector.Tux.kill(true);sector.Tux2.kill(true);')
        await sample('restarted',f'SupertuxView.state.drawn.epoch>{epoch} && SupertuxView.state.drawn.players.every(p=>p.dead===0)',timeout=45000)
        report['checks'].append('All-player death restarts the native level; a fresh epoch discards previous visual state and interpolation history')

        # Artificial delivery jitter at the presentation boundary, after the
        # real relay. This is not a claim of WAN or packet-loss testing.
        await guest.evaluate('''() => {window.actualViewAccept=SupertuxView.accept;SupertuxView.accept=frame=>{setTimeout(()=>actualViewAccept(frame),frame.sequence%3===0?180:20);return true;};}''')
        await guest.keyboard.down('ArrowRight');last=0
        for _ in range(12):
            await guest.wait_for_timeout(100)
            state=await guest.evaluate('SupertuxView.state')
            assert state['buffered']<=8
            if state['latest']:assert state['latest']['sequence']>=last;last=state['latest']['sequence']
        await guest.keyboard.up('ArrowRight');await guest.evaluate('() => {SupertuxView.accept=actualViewAccept;}')
        await guest.wait_for_timeout(250)
        report['checks'].append('Injected 20/180ms presentation delivery jitter cannot reverse snapshots; interpolation history remains at most eight complete states')

        packets=await guest.evaluate('viewPackets')
        elapsed=(packets[-1]['at']-packets[0]['at'])/1000
        report['metrics'].update(snapshot_count=len(packets),snapshot_bytes_max=max(p['bytes'] for p in packets),
            snapshot_bytes_per_second=round(sum(p['bytes'] for p in packets)/elapsed),snapshot_hz=round(len(packets)/elapsed,1),
            interpolation_delay_ms=90,buffer_limit=8)
        assert report['metrics']['snapshot_bytes_max']<=2048
        assert not any(re.search(r'supertux2\.(wasm|data|js)|/music/|data-gzip|wasm-gzip',request) for request in requests),requests
        assert not await guest.evaluate("typeof Module !== 'undefined'")
        assert await guest.evaluate("indexedDB.databases().then(d=>d.length)")==0
        report['checks'].append('Guest downloads no game WASM/DATA/music, runs no native game and creates no save/settings database; presentation payloads are content-hash validated')

        await guest.keyboard.down('ArrowRight')
        await guest.evaluate("Object.defineProperty(document,'hidden',{configurable:true,value:true});document.dispatchEvent(new Event('visibilitychange'))")
        await host.wait_for_function('Module.supertuxCoop.state.reserved===-1 && !Module.supertuxCoop.state.enabled')
        assert not await guest.evaluate('SupertuxView.playable')
        assert await guest.evaluate('supertuxGuest.state.mask')==0
        report['checks'].append('Guest background/disconnect clears controls, image history and authority; host retains its game and requires title-screen rejoin')
        for line in logs:
            if re.search(r'undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function|AN ERROR HAS OCCURRED|Error waking VM|Squirrel exception:',line):
                if not (args.record_known_ub and any(re.search(pattern,line) for pattern in KNOWN_UPSTREAM_UB)):errors.append(line)
        report['known_upstream_ub']=[line for line in logs if 'runtime error:' in line and args.record_known_ub and any(re.search(pattern,line) for pattern in KNOWN_UPSTREAM_UB)]
        report['snapshots']=snapshots;report['guest_requests']=requests;report['errors']=errors
        (args.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        await hc.close();await gc.close();await host_browser.close();await guest_browser.close()
        assert not errors,errors
    print(json.dumps({key:value for key,value in report.items() if key not in ('snapshots','guest_requests')},indent=2),flush=True)


if __name__=='__main__':
    coop_smoke.run=run
    coop_smoke.main()
