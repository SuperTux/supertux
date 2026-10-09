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
        async def capture(label):
            # Software GPU screenshot readback can block the host event loop.
            # Capture a deliberately paused scene rather than manufacture a
            # relay timeout by collecting evidence during active simulation.
            await host_key('Escape')
            await guest.wait_for_function('!SupertuxView.state.enabled')
            await host.locator('#canvas').screenshot(path=str(args.output/(label+'-host.png')))
            await guest.locator('#guest_canvas').screenshot(path=str(args.output/(label+'-guest.png')))
            await host_key('Escape')
            await guest.wait_for_function('SupertuxView.playable && supertuxGuest.state.mask===0')
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
          SupertuxView.accept=frame=>{viewPackets.push({bytes:JSON.stringify(frame).length,at:performance.now(),scene:frame.scene});return accept(frame);};
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
        await capture('scene')
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

        async def script(command, active=True, paused=True):
            await host.locator('#canvas').focus()
            if paused:
                await host_key('Escape')
                await host.wait_for_function('!Module.supertuxCoop.state.enabled')
            await host_key('Backquote')
            await host.keyboard.type(command,delay=4);await host_key('Enter')
            assert any('> '+command in line for line in logs[-30:]),logs[-10:]
            await host_key('Backquote')
            if active and paused: await host_key('Escape')
            if active:
                try:
                    await host.wait_for_function('Module.supertuxCoop.state.enabled')
                except Exception:
                    print('VIEW_SCRIPT_FAILED',command,await guest.evaluate('({guest:supertuxGuest.state,view:SupertuxView.state,status:document.querySelector("#guest_status").textContent,viewStatus:document.querySelector("#view_status").textContent})'),await host.evaluate('({state:Module.supertuxCoop.state,status:document.querySelector("#coop_status").textContent,packet:window.lastViewPacket})'),flush=True)
                    await host.locator('#canvas').screenshot(path=str(args.output/'script-host-failure.png'))
                    await guest.screenshot(path=str(args.output/'script-guest-failure.png'))
                    raise
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

        # Phase 7B uses the untouched packaged campaign level. Host-only
        # position/bonus fixtures select real collisions; never guest simulation.
        await script('Level.finish(true);',active=False)
        await host.wait_for_timeout(1200)
        await host.locator('#coop_panel').evaluate('(e)=>e.open=true')
        selection=time.monotonic()
        await host.locator('#coop_antarctica').click()
        await host.locator('#coop_panel').evaluate('(e)=>e.open=false')
        await host.locator('#canvas').focus()
        campaign=await sample('campaign-baseline',"SupertuxView.state.drawn.scene==='antarctica-v1'",timeout=60000)
        report['metrics']['campaign_selection_to_ready_ms']=round((time.monotonic()-selection)*1000)
        assert campaign['world']['draw'] and campaign['world']['entities']
        assert {'coin','bonusblock','brick','firefly','snowball','weak_block'} <= {e[1] for e in campaign['world']['entities']}
        report['checks'].append('Untouched Welcome to Antarctica starts only after a matching guest baseline acknowledgment; real native terrain, backgrounds and stable object identities are presented')
        x=campaign['players'][1]['x']
        movement=time.monotonic()
        await guest.keyboard.down('ArrowRight')
        await sample('campaign-remote-move',f'SupertuxView.state.drawn.players[1].x>{x+30}')
        report['metrics']['campaign_input_to_30_world_px_ms']=round((time.monotonic()-movement)*1000)
        await guest.keyboard.up('ArrowRight')
        await script('sector.Tux.set_is_intentionally_safe(true);sector.Tux2.set_is_intentionally_safe(true);')
        state=await sample('campaign-objects')
        coin=next(e for e in state['world']['entities'] if e[1]=='coin')
        coins=state['world']['coins']
        await script(f'sector.Tux.set_pos({coin[2]+48},{coin[3]});sector.Tux2.set_pos({coin[2]},{coin[3]});')
        collected=await sample('native-coin-collected',f'SupertuxView.state.drawn.world.coins>{coins} && !SupertuxView.state.drawn.world.entities.some(e=>e[0]==={coin[0]})')
        assert not any(c[1]==coin[0] for c in collected['world']['draw'])
        report['checks'].append('A real coin collision increases authoritative shared coins; the collected UID and all its draw commands disappear together')
        await capture('campaign')
        state=await sample('before-enemy-contact')
        enemy=min((e for e in state['world']['entities'] if e[1]=='snowball'),key=lambda e:abs(e[2]-state['players'][1]['x']))
        await script(f'sector.Tux.set_pos({enemy[2]-96},{enemy[3]});sector.Tux2.set_pos({enemy[2]-64},{enemy[3]});sector.Tux.set_velocity(0,0);sector.Tux2.set_velocity(0,0);')
        active=await sample('near-active-enemy',f'SupertuxView.state.drawn.world.entities.some(e=>e[0]==={enemy[0]} && ["left","right"].includes(e[4]))')
        enemy=next(e for e in active['world']['entities'] if e[0]==enemy[0])
        # Account for the moving enemy's actual direction and presentation
        # delay when positioning a falling player. Collision itself is native.
        direction=1 if 'right' in enemy[4] else -1
        await script(f'sector.Tux2.set_pos({enemy[2]+direction*24},{enemy[3]-48});sector.Tux2.set_velocity(0,0);')
        defeated=await sample('native-enemy-stomp',f'!SupertuxView.state.drawn.world.entities.some(e=>e[0]==={enemy[0]})',timeout=12000)
        await host_key('ControlLeft')
        await sample('host-action-rejoin',"SupertuxView.state.drawn.players.every(p=>p.dead===0)",timeout=15000)
        assert not any(c[1]==enemy[0] for c in defeated['world']['draw'])
        report['checks'].append('Host-authoritative contact stomps a real moving snowball; its stable UID is removed from both object and visual baselines')
        state=await sample('before-block-hit')
        block=next(e for e in state['world']['entities'] if e[1]=='bonusblock' and e[2]==1280)
        await script('sector.Tux.set_pos(1200,672);sector.Tux2.set_pos(1280,672);sector.Tux2.set_velocity(0,0);')
        await sample('under-growth-block',"SupertuxView.state.drawn.players[1].y>=671")
        await guest.keyboard.down('Space')
        await sample('bonus-block-used',f'SupertuxView.state.drawn.world.entities.some(e=>e[0]==={block[0]} && e[4]==="empty")')
        await guest.keyboard.up('Space')
        power=await sample('egg-spawn',"SupertuxView.state.drawn.world.entities.some(e=>SupertuxView.state.drawn.world.draw.some(c=>c[0]===0 && c[1]===e[0] && c[7][0].includes('powerups/egg/egg')))")
        egg=next(e for e in power['world']['entities'] if any(c[0]==0 and c[1]==e[0] and 'powerups/egg/egg' in c[7][0] for c in power['world']['draw']))
        await script(f'sector.Tux.set_pos({egg[2]-96},{egg[3]});sector.Tux2.set_pos({egg[2]},{egg[3]});')
        await sample('egg-collected',"SupertuxView.state.drawn.players[1].action.startsWith('big-')")
        report['checks'].append('Ordinary guest Jump hits the real growup block; the used block, spawned egg, collection and big-player pose agree in the guest presentation')
        await script('sector.Tux.set_pos(1264,672);sector.Tux2.set_pos(1344,640);sector.Tux2.set_velocity(0,0);')
        await guest.keyboard.down('Space')
        await sample('native-info-message',"SupertuxView.state.drawn.world.draw.some(c=>c[0]===4 && c[7][0].includes('egg makes Tux'))")
        await guest.keyboard.up('Space')
        await capture('info')
        report['checks'].append('Ordinary guest Jump opens the real information block; native wrapped text, image and rounded message panels remain present on both screens')
        brick=next(e for e in (await sample('before-brick'))['world']['entities'] if e[1]=='brick' and e[2]==416)
        await script('sector.Tux.set_pos(320,672);sector.Tux2.set_pos(416,640);sector.Tux2.set_velocity(0,0);')
        await sample('under-brick',"SupertuxView.state.drawn.players[1].y>=639")
        await guest.keyboard.down('Space')
        broken=await sample('native-brick-broken',f'!SupertuxView.state.drawn.world.entities.some(e=>e[0]==={brick[0]})')
        await guest.keyboard.up('Space')
        assert not any(c[1]==brick[0] for c in broken['world']['draw'])
        report['checks'].append('Big Player 2 breaks a real wooden brick with ordinary Jump; removal and native debris are visible without replaying a guest collision')
        await script('sector.Tux.set_pos(3960,512);sector.Tux2.set_pos(4032,400);sector.Tux2.set_bonus("fireflower");sector.Tux2.set_velocity(0,0);')
        await sample('fire-player',"SupertuxView.state.drawn.players[1].action.startsWith('fire-')")
        await guest.keyboard.down('ArrowRight');await guest.wait_for_timeout(120);await guest.keyboard.up('ArrowRight')
        await guest.keyboard.down('ControlLeft')
        await sample('native-fireball',"SupertuxView.state.drawn.world.draw.some(c=>c[0]===0 && c[7][0].includes('bullets/fire'))")
        await guest.keyboard.up('ControlLeft')
        report['checks'].append('Host fixture grants a fire bonus; ordinary guest Action creates a native fireball and original fire-player/projectile artwork is rendered')
        await sample('native-ice-melt',"!SupertuxView.state.drawn.world.entities.some(e=>e[1]==='weak_block' && e[2]===4128 && e[3]===416)",timeout=20000)
        report['checks'].append('The same real projectile melts a native ice weak block; its animation and eventual removal are authoritative')

        await script('sector.Tux.set_pos(8448,672);sector.Tux2.set_pos(8480,640);sector.Tux2.set_velocity(0,0);')
        cover=await sample('secret-cover',"SupertuxView.state.drawn.world.draw.some(c=>c[0]===0 && c[2]===110)")
        cover_ids={c[1] for c in cover['world']['draw'] if c[0]==0 and c[2]==110}
        await script('sector.Tux2.set_pos(8544,600);sector.Tux2.set_velocity(0,0);')
        await sample('native-secret-message',"SupertuxView.state.drawn.world.draw.some(c=>c[0]===4 && c[7][0].includes('secret area'))")
        await sample('native-secret-fade',f'SupertuxView.state.drawn.world.draw.every(c=>!{json.dumps(sorted(cover_ids))}.includes(c[1]) || c[4]<=0.01)',timeout=15000)
        report['checks'].append('Native secret-area contact shows its message and fades the exact covering tilemap; the guest retains no stale opaque tiles')

        await script('sector.Tux.set_pos(5360,512);sector.Tux2.set_pos(5360,512);')
        bell=await sample('native-checkpoint',"SupertuxView.state.drawn.world.checkpoint!==null")
        epoch=bell['epoch']
        await script('sector.Tux.kill(true);sector.Tux2.kill(true);')
        restarted=await sample('campaign-checkpoint-restart',f'SupertuxView.state.drawn.epoch>{epoch} && SupertuxView.state.drawn.players.every(p=>p.dead===0 && Math.abs(p.x-5360)<5)',timeout=45000)
        report['checks'].append('Real checkpoint bell state is replicated; all-player death rebuilds both players at the checkpoint with a new acknowledged epoch')
        # Enter the actual endsequence trigger, then follow native scripted
        # walking to the real stoptux trigger. Do not call Level.finish here.
        await script('sector.Tux.set_pos(8960,600);sector.Tux2.set_pos(8960,600);',active=False,paused=False)
        await guest.wait_for_function("SupertuxView.state.latest.world.phase==='finishing'",timeout=20000)
        await guest.wait_for_function('SupertuxView.state.outcome?.win===true',timeout=90000)
        assert not await guest.evaluate('SupertuxView.playable')
        assert await guest.evaluate("indexedDB.databases().then(d=>d.length)")==0
        await host.wait_for_timeout(1500)
        assert await host.evaluate("Module.FS.readdir(Module.supertuxStorage.root+'profile1').includes('world1.stsg')")
        report['checks'].append('Native end sequence completes the campaign level; the guest receives authoritative completion and releases controls; the host owns progression')
        campaign_packets=[p for p in await guest.evaluate('viewPackets') if p['scene']=='antarctica-v1']
        report['metrics']['campaign_snapshot_bytes_max']=max(p['bytes'] for p in campaign_packets)
        campaign_elapsed=(campaign_packets[-1]['at']-campaign_packets[0]['at'])/1000
        report['metrics']['campaign_snapshot_hz']=round(len(campaign_packets)/campaign_elapsed,1)
        report['metrics']['campaign_snapshot_bytes_per_second']=round(sum(p['bytes'] for p in campaign_packets)/campaign_elapsed)
        assert report['metrics']['campaign_snapshot_bytes_max']<=65536

        await guest.keyboard.down('ArrowRight')
        await guest.evaluate("Object.defineProperty(document,'hidden',{configurable:true,value:true});document.dispatchEvent(new Event('visibilitychange'))")
        await host.wait_for_function('Module.supertuxCoop.state.reserved===-1 && !Module.supertuxCoop.state.enabled')
        assert not await guest.evaluate('SupertuxView.playable')
        assert await guest.evaluate('supertuxGuest.state.mask')==0
        report['checks'].append('Guest background/disconnect clears controls, image history and authority; host retains its game and requires title-screen rejoin')
        for line in logs:
            if re.search(r'undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function|AN ERROR HAS OCCURRED|Error waking VM|Squirrel exception:|Shared view artwork missing:|Co-op presentation rejected|Co-op presentation exceeded',line):
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
