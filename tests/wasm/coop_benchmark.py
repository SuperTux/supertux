#!/usr/bin/env python3
"""Repeatable, exact-build private co-op measurements; not physical-phone evidence.

The host uses position/safety fixtures on the original level. Guest movement
uses ordinary keyboard input. Response ends when the authoritative guest canvas
state first moves one world pixel, not when an input acknowledgment arrives.
"""
import argparse
import asyncio
from contextlib import AsyncExitStack
import hashlib
import json
from pathlib import Path
import re
import selectors
import subprocess
import sys
import tempfile
import time
from urllib.request import Request, urlopen
from playwright.async_api import async_playwright
from browser_smoke import KNOWN_UPSTREAM_UB

ROOT = Path(__file__).parents[2]
sys.path.insert(0, str(ROOT / 'tools/web'))
from coop_metrics import summarize
from verify_artifact import verify

PROFILES = {'loopback': dict(rtt_ms=0, jitter_ms=0, mbps=0),
            'constrained': dict(rtt_ms=150, jitter_ms=30, mbps=5)}

GUEST_OBSERVER = r'''() => {
  performance.setResourceTimingBufferSize(5000);
  const empty = () => ({input_to_first_draw_ms:[], input_to_first_frame_ms:[],
    arrival_interval_ms:[], draw_interval_ms:[], receive_to_draw_ms:[], draw_callback_ms:[],
    view_count:0, view_bytes:0, largest_view_bytes:0, max_guest_buffered_bytes:0,
    presentation_draws:0, interpolated_draws:0});
  const state = window.coopBenchmark = {active:false, pending:null, received:new Map(), data:empty()};
  const clock=()=>performance.now(), raf=requestAnimationFrame;
  window.requestAnimationFrame = callback => raf.call(window, time => {
    const previous=window.SupertuxView?.state.drawn, begin=clock();
    callback(time);
    const frame=window.SupertuxView?.state.drawn, end=clock();
    if (!state.active || callback.name!=='tick' || !frame || frame===previous) return;
    const data=state.data;
    ++data.presentation_draws;
    if(frame.presentation)++data.interpolated_draws;
    data.draw_callback_ms.push(end-begin);
    if (state.lastDraw!==undefined) data.draw_interval_ms.push(end-state.lastDraw);
    state.lastDraw=end;
    // For a synthesized frame, age belongs to the newest required endpoint.
    const newest=state.received.get(frame.presentation?.to ?? frame.sequence);
    if (newest!==undefined) data.receive_to_draw_ms.push(end-newest);
    const pending=state.pending, player=frame.players.find(p=>p.id===2);
    if (pending && frame.session===pending.session && frame.epoch===pending.epoch &&
        player && (player.x-pending.x)*pending.direction>=1) {
      data.input_to_first_draw_ms.push(end-pending.at); state.pending=null;
    }
  });
  window.addEventListener('keydown', event => {
    const direction=event.code==='ArrowRight'?1:event.code==='ArrowLeft'?-1:0;
    const frame=window.SupertuxView?.state.drawn;
    if (state.active && direction && !event.repeat && frame && SupertuxView.playable) {
      state.pending={at:clock(), direction, x:frame.players.find(p=>p.id===2).x,
        session:frame.session, epoch:frame.epoch, received:false};
    }
  }, true);
  state.start=()=>{state.active=true;state.started=clock();state.data=empty();
    state.received.clear();state.lastArrival=state.lastDraw=undefined;};
  state.finish=()=>{state.active=false;return {...state.data,window_ms:clock()-state.started};};
}'''

ATTACH_OBSERVER = r'''() => {
  const state=coopBenchmark, accept=SupertuxView.accept;
  SupertuxView.accept=frame=>{
    const at=performance.now(), accepted=accept(frame);
    if (!state.active || !accepted || frame.scene!=='antarctica-v1') return accepted;
    const data=state.data, bytes=new TextEncoder().encode(JSON.stringify(frame)).length;
    ++data.view_count;data.view_bytes+=bytes;data.largest_view_bytes=Math.max(data.largest_view_bytes,bytes);
    if (state.lastArrival!==undefined) data.arrival_interval_ms.push(at-state.lastArrival);
    state.lastArrival=at;state.received.set(frame.sequence,at);
    if (state.received.size>16) state.received.delete(state.received.keys().next().value);
    const pending=state.pending, player=frame.players.find(p=>p.id===2);
    if (pending && !pending.received && frame.session===pending.session && frame.epoch===pending.epoch &&
        player && (player.x-pending.x)*pending.direction>=1) {
      data.input_to_first_frame_ms.push(at-pending.at);pending.received=true;
    }
    return accepted;
  };
  const connection=supertuxGuest.connection, send=connection.send.bind(connection);
  connection.send=value=>{const result=send(value);
    if(state.active)state.data.max_guest_buffered_bytes=Math.max(state.data.max_guest_buffered_bytes,connection.socket.bufferedAmount);
    return result;};
}'''

HOST_OBSERVER = r'''() => {
  const state=window.hostBenchmark={active:false, intervals:[], nativeIntervals:[], buffered:0}, connection=Module.supertuxCoop.connection;
  const send=connection.send.bind(connection);
  connection.send=value=>{const result=send(value);
    if(state.active)state.buffered=Math.max(state.buffered,connection.socket.bufferedAmount);return result;};
  const status=Module.supertuxCoop.engineStatus;
  Module.supertuxCoop.engineStatus=(...args)=>{
    if(state.active){const now=performance.now();if(state.lastNative!==undefined)state.nativeIntervals.push(now-state.lastNative);state.lastNative=now;}
    status(...args);
  };
  requestAnimationFrame(function observe(){
    // SDK wrappers need not supply the native callback's timestamp argument.
    const now=performance.now();
    if(state.active){if(state.last!==undefined)state.intervals.push(now-state.last);state.last=now;}
    requestAnimationFrame(observe);
  });
}'''


def redact(value):
    return re.sub(r'#room=[^\s\"\'<>]+', '#[private invitation]', str(value))


def start_process(command, marker, log):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=log, text=True)
    deadline = time.monotonic() + 180
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)
        try:
            while time.monotonic() < deadline:
                if not selector.select(1):
                    if process.poll() is not None: break
                    continue
                line = process.stdout.readline()
                if line.startswith(marker): return process, line.split()[1].split('index.html')[0]
                if process.poll() is not None: break
            raise RuntimeError('Benchmark server failed to start; inspect its log')
        except BaseException:
            stop_process(process)
            raise


def stop_process(process):
    process.terminate()
    try: process.wait(timeout=20)
    except subprocess.TimeoutExpired: process.kill(); process.wait()
    process.stdout.close()


async def resources(page):
    # Same-origin Resource Timing includes response/header sizes when supported.
    # Zero sizes in an engine with no positive samples cannot establish caching.
    return await page.evaluate(r'''() => {
      const entries=performance.getEntriesByType('resource').filter(e=>!e.name.startsWith('blob:'));
      const art=entries.filter(e=>e.name.includes('/coop-art/'));
      const packages=entries.filter(e=>/supertux2\.(data|wasm)|\/(data|wasm)-gzip\//.test(e.name));
      const totals=items=>({requests:items.length, transfer_bytes:items.reduce((s,e)=>s+e.transferSize,0),
        encoded_body_bytes:items.reduce((s,e)=>s+e.encodedBodySize,0), decoded_body_bytes:items.reduce((s,e)=>s+e.decodedBodySize,0),
        cached_responses:items.filter(e=>e.encodedBodySize>0 && e.transferSize===0).length,
        sizes_available:items.some(e=>e.encodedBodySize>0)});
      return {all:totals(entries),art:totals(art),startup_packages:totals(packages)};
    }''')


async def trial(args, url, playwright, index, html):
    launch = dict(args=['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader']) if args.browser=='chromium' else {}
    if args.webkit_executable: launch['executable_path']=args.webkit_executable
    async with AsyncExitStack() as stack:
        profiles=stack.enter_context(tempfile.TemporaryDirectory(prefix='supertux-coop-benchmark-'))
        engine=getattr(playwright,args.browser)
        contexts=[]
        for role in ('host','guest'):
            if args.ephemeral:
                browser=await engine.launch(**launch);stack.push_async_callback(browser.close)
                context=await browser.new_context(viewport=dict(width=844,height=390),has_touch=True)
            else:
                context=await engine.launch_persistent_context(str(Path(profiles)/role),
                    viewport=dict(width=844,height=390),has_touch=True,**launch)
            stack.push_async_callback(context.close);contexts.append(context)
        hc,gc=contexts
        version=hc.browser.version if hc.browser else await hc.pages[0].evaluate('navigator.userAgent')
        # WebKit's persistent launcher owns a default tab. Keep it alive:
        # closing every tab can terminate that browser before measurement.
        await hc.add_init_script('performance.setResourceTimingBufferSize(5000)')
        await gc.add_init_script('('+GUEST_OBSERVER+')()')
        return await measure_trial(args,url,index,html,hc,gc,version)


async def network_counts(args):
    if not args.observer_url: return None
    def read():
        request=Request(args.observer_url+'__benchmark_stats',headers={'User-Agent':'SuperTux-Coop-Validation/Phase8'})
        return json.loads(urlopen(request,timeout=30).read())
    return await asyncio.to_thread(read)


def network_delta(before, after, category):
    if before is None: return dict(backend_observed=False)
    return dict(backend_observed=True, requests=after[category+'_requests']-before[category+'_requests'],
                response_body_bytes=after[category+'_response_body_bytes']-before[category+'_response_body_bytes'])


async def measure_trial(args,url,index,html,hc,gc,version):
    errors, logs = [], []
    result = dict(run=index+1, browser=version, loading={}, http={}, observations={}, errors=errors)
    host=guest=None
    stage='boot'
    def console(message):
        line=redact(message.text); logs.append(line)
        if re.search(r'undefined symbol|Aborted\(|\[FATAL\]|runtime error:|missing function|AN ERROR HAS OCCURRED|Error waking VM|Squirrel exception:|Shared view artwork missing:|Co-op presentation rejected|Co-op presentation exceeded',line):
            if not (args.record_known_ub and any(re.search(pattern,line) for pattern in KNOWN_UPSTREAM_UB)):errors.append(line)
    try:
        if args.browser=='chromium': await hc.grant_permissions(['local-network-access'])
        host=await hc.new_page();host.set_default_timeout(60000)
        host.on('console',console);host.on('pageerror',lambda error:errors.append('Host: '+redact(error)))
        # Developer console fixtures need the existing --developer launch flag.
        # Routing disables the host's HTTP cache; warm host reuse tests its
        # persistent asset database instead. Guest requests remain un-routed.
        await host.route('**/index.html?coop=1',lambda route:route.fulfill(body=html,content_type='text/html'))
        async def boot_host(warm):
            before=await network_counts(args)
            start=time.monotonic()
            if warm: await host.reload()
            else: await host.goto(url+'index.html?coop=1')
            await host.wait_for_function('Module.supertuxReady',timeout=180000)
            kind='warm' if warm else 'cold'
            result['loading']['host_'+kind+'_ready_ms']=round((time.monotonic()-start)*1000,2)
            result['http']['host_'+kind]=await resources(host)
            result['http']['host_'+kind]['network_startup']=network_delta(before,await network_counts(args),'startup')
            await host.locator('#start_button').click()
            await host.wait_for_function('Module.supertuxShell.active')
            await host.wait_for_function("document.querySelector('#output').textContent.includes('Setting status: In main menu')")
            await host.locator('#coop_panel').evaluate('(e)=>e.open=true')
            await host.locator('#coop_create').click()
            await host.wait_for_function("document.querySelector('#coop_status').textContent.includes('Room ready')")
        async def boot_guest(kind):
            before=await network_counts(args)
            guest=await gc.new_page();guest.set_default_timeout(60000)
            guest.on('console',console);guest.on('pageerror',lambda error:errors.append('Guest: '+redact(error)))
            start=time.monotonic()
            await guest.goto(await host.locator('#coop_view_link').get_attribute('href'))
            await guest.wait_for_function('window.supertuxGuest?.state.connected && SupertuxView.state.loaded>0',timeout=120000)
            result['loading']['guest_'+kind+'_ready_ms']=round((time.monotonic()-start)*1000,2)
            result['http']['guest_'+kind]=await resources(guest)
            result['http']['guest_'+kind]['network_art']=network_delta(before,await network_counts(args),'art')
            assert result['http']['guest_'+kind]['startup_packages']['requests']==0, 'Guest requested host game payloads'
            assert not await guest.evaluate("typeof Module!=='undefined'")
            assert await guest.evaluate('indexedDB.databases().then(d=>d.length)')==0
            await host.wait_for_function('Module.supertuxCoop.state.reserved===1')
            return guest
        await boot_host(False)
        cold_guest=await boot_guest('cold')
        await cold_guest.evaluate("supertuxGuest.connection.close('Benchmark warm reload')")
        await host.wait_for_function('Module.supertuxCoop.state.reserved===-1')
        await cold_guest.close()
        await boot_host(True)
        guest=await boot_guest('warm')
        result['guest_loaded_images']=await guest.evaluate('SupertuxView.state.loaded')
        await guest.evaluate(ATTACH_OBSERVER)
        await host.evaluate('''() => {window.benchmarkInputFrames=0;const status=Module.supertuxCoop.engineStatus;
          Module.supertuxCoop.engineStatus=(...args)=>{++benchmarkInputFrames;status(...args);};}''')
        async def key(key):
            for action in (host.keyboard.down,host.keyboard.up):
                await action(key);frame=await host.evaluate('benchmarkInputFrames')
                await host.wait_for_function('(frame)=>benchmarkInputFrames>frame',arg=frame)
        stage='campaign selection'
        start=time.monotonic()
        await host.locator('#coop_antarctica').click()
        await host.locator('#coop_panel').evaluate('(e)=>e.open=false')
        await host.locator('#canvas').focus()
        await guest.wait_for_function("SupertuxView.playable && SupertuxView.state.drawn.scene==='antarctica-v1'",timeout=60000)
        result['loading']['selection_to_ready_ms']=round((time.monotonic()-start)*1000,2)
        # Safe, repeatable terrain near spawn. This is a measurement fixture,
        # not evidence that someone completed the level on a physical phone.
        stage='measurement fixture'
        await key('Escape');await host.wait_for_function('!Module.supertuxCoop.state.enabled')
        await key('Backquote')
        command='sector.Tux.set_is_intentionally_safe(true);sector.Tux2.set_is_intentionally_safe(true);sector.Tux.set_pos(128,672);sector.Tux2.set_pos(240,672);sector.Tux2.set_velocity(0,0);'
        await host.keyboard.type(command,delay=4);await key('Enter')
        assert any('> '+command in line for line in logs), 'Measurement fixture was not consumed'
        await key('Backquote');await key('Escape')
        await guest.wait_for_function('SupertuxView.playable && supertuxGuest.state.mask===0')
        async def settled():
            await guest.evaluate('window.benchmarkSettled={sequence:0,x:null,count:0}')
            await guest.wait_for_function('''() => {
              const frame=SupertuxView.state.drawn,s=benchmarkSettled,p=frame?.players.find(p=>p.id===2);
              if(!p || frame.sequence===s.sequence)return false;
              s.count=s.x!==null && Math.abs(p.x-s.x)<.1?s.count+1:0;s.x=p.x;s.sequence=frame.sequence;
              return SupertuxView.playable && s.count>=2;
            }''',timeout=15000)
        stage='initial settling'
        await settled();await host.evaluate(HOST_OBSERVER)
        await host.evaluate('hostBenchmark.active=true');await guest.evaluate('coopBenchmark.start()')
        for sample in range(args.samples):
            stage=f'sample {sample+1} settling'
            key_name='ArrowRight' if sample%2==0 else 'ArrowLeft'
            await settled()
            stage=f'sample {sample+1} first draw'
            await guest.keyboard.down(key_name)
            try:
                await guest.wait_for_function('(count)=>coopBenchmark.data.input_to_first_draw_ms.length>count',arg=sample,timeout=10000)
            finally: await guest.keyboard.up(key_name)
            assert await guest.evaluate('supertuxGuest.state.mask')==0
        result['observations']=await guest.evaluate('coopBenchmark.finish()')
        host_data=await host.evaluate('() => {hostBenchmark.active=false;return hostBenchmark}')
        result['observations'].update(host_raf_interval_ms=host_data['intervals'],
            host_native_input_update_interval_ms=host_data['nativeIntervals'],max_host_buffered_bytes=host_data['buffered'])
        assert len(result['observations']['input_to_first_frame_ms'])==args.samples
        assert len(result['observations']['input_to_first_draw_ms'])==args.samples
        assert result['observations']['largest_view_bytes']<=65536
        result['known_upstream_ub']=[line for line in logs if 'runtime error:' in line and any(re.search(p,line) for p in KNOWN_UPSTREAM_UB)]
        assert not errors, errors
        return result
    except Exception as error:
        result['failure']=redact(error)
        result['failure_stage']=stage
        for role,page in (('host',host),('guest',guest)):
            if page is None:continue
            try:
                result[role+'_failure_state']=await page.evaluate(r'''() => {
                  const view=window.SupertuxView?.state, frame=view?.drawn;
                  const coop=window.Module?.supertuxCoop ?? window.supertuxGuest;
                  return {visibility:document.visibilityState,focus:document.activeElement?.id,
                    input:coop?.state, connected:coop?.connection?.ready,
                    status:document.querySelector('#coop_status,#status')?.textContent,
                    packet_age_ms:Date.now()-(coop?.connection?.last ?? Date.now()),
                    view:view && {playable:view.playable,enabled:view.enabled,buffered:view.buffered,
                      latest_sequence:view.latest?.sequence, sequence:frame?.sequence,
                      session:frame?.session,epoch:frame?.epoch,players:frame?.players,camera:frame?.camera},
                    settled:window.benchmarkSettled,measurements:window.coopBenchmark?.data,
                    host_intervals:window.hostBenchmark && {
                      raf_max_ms:Math.max(0,...hostBenchmark.intervals),
                      native_max_ms:Math.max(0,...hostBenchmark.nativeIntervals)}};
                }''')
                await page.screenshot(path=str(args.output/f'run-{index+1}-{role}-failure.png'))
            except Exception as diagnostic_error:
                result[role+'_diagnostic_error']=redact(diagnostic_error)
        raise
    finally:
        (args.output/f'run-{index+1}.json').write_text(json.dumps(result,indent=2)+'\n')
        (args.output/f'run-{index+1}-console.log').write_text('\n'.join(logs)+'\n')


async def run(args, url):
    request=lambda name: urlopen(Request(url+name,headers={'User-Agent':'SuperTux-Coop-Validation/Phase8'}),timeout=30).read()
    remote=json.loads(request('asset-manifest.json'))
    identity=json.loads((args.build/'BUILD_INFO.json').read_text())
    assert all(remote[key]==identity[key] for key in identity), 'Hosted/runtime artifact identity mismatch'
    html=request('index.html').decode().replace('var Module = {','var Module = {\narguments:["--verbose","--developer"],',1)
    report=dict(schema=1, runtime=identity, engine=args.browser, physical_device_tested=False,
        requested_runs=args.runs,samples_per_run=args.samples,complete=False,
        measurement_tool_sha256={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
          ('tests/wasm/coop_benchmark.py','tools/web/coop_metrics.py','tools/web/coop/relay_proxy.mjs')},
        profile=dict(name=args.profile,**PROFILES[args.profile],scope='ordered local WebSocket byte stream; HTTP assets unshaped'),
        conditions=dict(viewport=dict(width=844,height=390),device_scale_factor=1,has_touch=True,
          separate_browser_processes=True, fresh_profiles_per_run=True,
          browser_profile='ephemeral contexts' if args.ephemeral else 'persistent temporary profiles',
          host_http_cache='disabled by developer HTML route; warm persistent asset store retained',
          guest_http_cache='enabled, cold fresh context then warm page in same context',
          render_endpoint='authoritative drawn-state change observed after Canvas2D callback; not hardware photon timing',
          statistics='median and nearest-rank p95 of completed trials; failed trials retained separately'),trials=[],failed_trials=[])
    async with async_playwright() as playwright:
        for index in range(args.runs):
            print(f'Benchmark {args.browser} {args.profile}: run {index+1}/{args.runs}',flush=True)
            try:
                report['trials'].append(await trial(args,url,playwright,index,html))
            except Exception as error:
                # Independent fresh profiles allow remaining trials to run.
                # Never classify a partial/failed trial as a passing sample.
                report['failed_trials'].append(json.loads((args.output/f'run-{index+1}.json').read_text()))
                print(f'Trial {index+1} failed: {redact(error)}',flush=True)
            report['summary']=summarize(report['trials']) if report['trials'] else {}
            report['complete']=len(report['trials'])==args.runs and not report['failed_trials']
            (args.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report['summary'],indent=2),flush=True)
    if not report['complete']:
        raise RuntimeError(f'Incomplete benchmark: {len(report["failed_trials"])} of {args.runs} trials failed; inspect retained reports')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build',type=Path);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--browser',choices=['chromium','webkit'],default='chromium')
    parser.add_argument('--webkit-executable');parser.add_argument('--url')
    parser.add_argument('--profile',choices=PROFILES,default='loopback')
    parser.add_argument('--runs',type=int,default=3);parser.add_argument('--samples',type=int,default=10)
    parser.add_argument('--record-known-ub',action='store_true')
    parser.add_argument('--ephemeral',action='store_true',help='Diagnostic private contexts; WebKit may disable HTTP caching')
    args=parser.parse_args()
    if not 1<=args.runs<=10 or not 1<=args.samples<=100: parser.error('Use 1–10 runs and 1–100 samples per run')
    if args.output.exists() and any(args.output.iterdir()): parser.error('Output must be a new or empty directory')
    identity=json.loads((args.build/'BUILD_INFO.json').read_text())
    verify(args.build,identity['sourceCommit'],identity['configuration'])
    args.output.mkdir(parents=True,exist_ok=True)
    processes=[]
    try:
        with (args.output/'relay.log').open('w') as relay_log, (args.output/'proxy.log').open('w') as proxy_log:
            url=args.url.rstrip('/')+'/' if args.url else None
            if not url:
                process,url=start_process(['node',str(ROOT/'tools/web/coop/preview.mjs'),str(args.build.resolve()),'0'],'INPUT_PROOF_READY ',relay_log)
                processes.append(process)
            args.observer_url=None
            if not args.url or args.profile!='loopback':
                process,url=start_process(['node',str(ROOT/'tools/web/coop/relay_proxy.mjs'),url,json.dumps(PROFILES[args.profile])],'BENCHMARK_PROXY_READY ',proxy_log)
                processes.append(process)
                args.observer_url=url
            asyncio.run(run(args,url))
    except Exception as error:
        (args.output/'failure.txt').write_text(redact(error)+'\n')
        raise RuntimeError(redact(error)) from None
    finally:
        for process in reversed(processes): stop_process(process)


if __name__=='__main__': main()
