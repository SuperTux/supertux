import assert from 'node:assert/strict';
import {test} from 'node:test';
import {CoopRoom, coopFetch, PROTOCOL} from '../../worker/coop.js';
import {validView} from '../../worker/view.js';

const build = 'b'.repeat(64), token = 'a'.repeat(64);
function fixture() {
  const room = {host: token, guest: 'c'.repeat(64), build, origin: 'http://localhost:8787', expires: Date.now() + 900000};
  const sockets = [];
  const storage = {room, alarm: 0, async get() {return this.room;}, async put(_, value) {this.room = value;}, async setAlarm(value) {this.alarm = value;}, async deleteAll() {this.room = null;}, async deleteAlarm() {this.alarm = 0;}};
  const state = {storage, getWebSockets(role) {return sockets.filter(s => !s.closed && (!role || s.info.role === role));}};
  const object = new CoopRoom(state);
  function add(role) {
    const socket = {info: {role, hello: false, last: Date.now(), opened: Date.now(), rateStart: Date.now(), count: 0, generation: 0, sequence: 0}, messages: [],
      deserializeAttachment() {return structuredClone(this.info);}, serializeAttachment(info) {assert.ok(new TextEncoder().encode(JSON.stringify(info)).length<=2048,'Cloudflare WebSocket attachment exceeds 2 KiB');this.info = structuredClone(info);},
      send(value) {this.messages.push(JSON.parse(value));}, close(code, reason) {this.closed ||= {code, reason};}};
    sockets.push(socket); return socket;
  }
  return {object, state, add, room};
}
const send = (object, socket, message) => object.webSocketMessage(socket, JSON.stringify(message));
function view(sequence=1,extra={}) {
  return {type:'view',session:1,epoch:1,sequence,generation:1,time:sequence*100,scene:'coop-view-v1',camera:[0,0,1,844,390],
    players:[1,2].map(id=>({id,x:id*100,y:100,action:'small-stand-right',frame:0,angle:0,alpha:1,dead:0,visible:true})),...extra};
}

test('presentation schema rejects arbitrary entities, asset paths and nonfinite or oversized state',()=>{
  assert.equal(validView(view()),true);
  assert.equal(validView(view(1,{scene:'unsupported',players:[]})),true);
  for (const invalid of [view(1,{scene:'campaign'}),view(1,{players:[]}),view(1,{camera:[0,0,0,844,390]}),view(1,{generation:0}),view(1,{script:'run'}),view(1,{players:[view().players[0],view().players[0]]})]) assert.equal(validView(invalid),false);
  for (const patch of [{x:Infinity},{action:'../sprite.png'},{frame:256},{visible:1},{dead:3},{extra:true}]) {
    const invalid=view();Object.assign(invalid.players[0],patch);assert.equal(validView(invalid),false);
  }
});

test('only the host may publish snapshots; only a view-capable guest receives ordered current-generation state',async()=>{
  const f=fixture(),host=f.add('host'),guest=f.add('guest');
  await ready(f,host);
  await send(f.object,guest,{type:'hello',protocol:PROTOCOL,build,view:true});
  await send(f.object,host,{type:'session',generation:1,enabled:true});
  for(const packet of [view(),view(),view(2,{generation:2}),view(2),view(3,{epoch:2}),view(4,{epoch:1}),view(1,{session:2}),view(9)]) await send(f.object,host,packet);
  assert.deepEqual(guest.messages.filter(x=>x.type==='view').map(x=>[x.session,x.epoch,x.sequence]),[[1,1,1],[1,1,2],[1,2,3],[2,1,1]]);
  await send(f.object,guest,view());assert.equal(guest.closed.code,1008);
  const legacy=fixture(),h=legacy.add('host'),g=legacy.add('guest');await ready(legacy,h);await ready(legacy,g);
  await send(legacy.object,h,{type:'session',generation:1,enabled:true});await send(legacy.object,h,view());
  assert.equal(g.messages.filter(x=>x.type==='view').length,0);
});

test('slow viewer coalesces one complete latest snapshot instead of a state backlog',async()=>{
  const f=fixture(),host=f.add('host'),guest=f.add('guest');await ready(f,host);
  await send(f.object,guest,{type:'hello',protocol:PROTOCOL,build,view:true});
  await send(f.object,host,{type:'session',generation:1,enabled:true});
  for(let i=1;i<=45;i++) await send(f.object,host,view(i));
  assert.equal(guest.info.inFlight,32);assert.equal(guest.info.pending.view.sequence,45);
  assert.equal(guest.closed,undefined);assert.equal(Object.keys(guest.info.pending).length,1);
  await send(f.object,guest,{type:'seen'});assert.equal(guest.messages.at(-1).sequence,45);
  assert.equal(guest.info.pending.view,undefined);
});
async function ready(f, socket) {await send(f.object, socket, {type: 'hello', protocol: PROTOCOL, build});}

test('routing is opt-in, origin-bound, bounded and build-validated', async () => {
  assert.equal((await coopFetch(new Request('http://localhost/coop/rooms'), {})).status, 404);
  assert.equal((await coopFetch(new Request('http://localhost/coop/rooms'), {COOP_ROOMS: {}})).status, 403);
  for (const body of ['invalid', 'null', '[]', JSON.stringify({protocol: 3, build}), JSON.stringify({protocol: 1, build:[build]}), JSON.stringify({protocol: 1, build, role:'host'}), 'x'.repeat(513)]) {
    const response = await coopFetch(new Request('http://localhost/coop/rooms', {method: 'POST', headers: {Origin: 'http://localhost'}, body}), {COOP_ROOMS: {}});
    assert.ok([400,413].includes(response.status));
  }
});

test('role-bound credentials, occupied role and origin reject before upgrade', async () => {
  const f = fixture(); f.add('host');
  const request = protocols => new Request('http://localhost:8787/coop/rooms/' + '1'.repeat(32) + '/socket', {headers: {Origin: f.room.origin, 'Sec-WebSocket-Protocol': protocols}});
  assert.equal((await f.object.fetch(request(`supertux-coop-v2, host.${token}`))).status, 409);
  assert.equal((await f.object.fetch(request(`supertux-coop-v2, guest.${token}`))).status, 403);
  f.room.expires = Date.now() - 1;
  assert.equal((await f.object.fetch(request(`supertux-coop-v2, host.${token}`))).status, 410);
});

test('validated input belongs to the current host session; ordering never reverses', async () => {
  const f = fixture(), host = f.add('host'), guest = f.add('guest');
  await ready(f, host); await ready(f, guest);
  await send(f.object, host, {type: 'session', generation: 4, enabled: true});
  for (const [generation, sequence, mask] of [[4,1,16],[4,2,0],[4,1,2],[3,5,2]])
    await send(f.object, guest, {type: 'input', generation, sequence, mask});
  assert.deepEqual(host.messages.filter(x => x.type === 'input').map(x => x.mask), [16,0]);
  await send(f.object, host, {type: 'session', generation: 5, enabled: false});
  await send(f.object, guest, {type: 'input', generation: 5, sequence: 1, mask: 2});
  assert.equal(host.messages.filter(x => x.type === 'input').length, 2);
  await send(f.object, guest, {type: 'session', generation: 5, enabled: true});
  assert.equal(guest.closed.code, 1008);
});

test('mismatch, menu bits, oversized and malformed messages cannot enter gameplay', async () => {
  for (const value of [{type: 'hello', protocol: 1, build: 'd'.repeat(64)}, {type: 'input', generation:1,sequence:1,mask:128}]) {
    const f = fixture(), socket = f.add('guest');
    if (value.type === 'input') await ready(f, socket);
    await send(f.object, socket, value); assert.equal(socket.closed.code, 1008);
  }
  for (const value of ['x'.repeat(513), '{', new ArrayBuffer(16)]) {
    const f = fixture(), socket = f.add('guest');
    await f.object.webSocketMessage(socket, value); assert.ok(socket.closed);
  }
  for (const type of ['__proto__','constructor','toString',null,[]]) {
    const f=fixture(), socket=f.add('guest');await ready(f,socket);
    await send(f.object,socket,{type});assert.equal(socket.closed.code,1008);
  }
});

test('rate, idle/auth timeout, disconnect and expiry clean up without buffering', async () => {
  let f = fixture(), host = f.add('host'), guest = f.add('guest');
  await ready(f,host);await ready(f,guest);
  for(let i=0;i<61 && !guest.closed;i++) {await send(f.object,guest,{type:'ping'});await send(f.object,guest,{type:'seen'});}
  assert.equal(guest.closed.code,1008);
  await f.object.webSocketClose(guest,1000,'left');
  assert.deepEqual(host.messages.at(-1),{type:'peer',connected:false});
  f = fixture(); guest=f.add('guest');guest.info.opened=Date.now()-6000;
  await f.object.alarm();assert.equal(guest.closed.code,1001);
  f = fixture();host=f.add('host');await ready(f,host);host.info.last=Date.now()-3000;
  await f.object.alarm();assert.equal(host.closed.code,1001);assert.equal(f.state.storage.room,null);
  f = fixture();host=f.add('host');f.room.expires=Date.now()-1;
  await f.object.alarm();assert.equal(host.closed.code,1008);assert.equal(f.state.storage.room,null);
});

test('unacknowledged receiver has a fixed message window; role fields are rejected', async () => {
  const f = fixture(), host = f.add('host'), guest = f.add('guest');
  await ready(f,host);await ready(f,guest);
  await send(f.object,host,{type:'session',generation:1,enabled:true});
  for(let i=0;i<33 && !host.closed;i++) await send(f.object,guest,{type:'input',generation:1,sequence:i+1,mask:i%2});
  assert.equal(host.closed.code,1013);
  assert.ok(host.messages.length <= 32);
  const other=fixture(), client=other.add('guest');await ready(other,client);
  await send(other.object,client,{type:'input',generation:1,sequence:1,mask:2,role:'host'});
  assert.equal(client.closed.code,1008);
});

test('slow status receiver retains only three latest values and drains on credit', async () => {
  const f=fixture(), host=f.add('host'), guest=f.add('guest');
  await ready(f,host);await ready(f,guest);
  for(let i=1;i<=29;i++) {
    await send(f.object,host,{type:'session',generation:i,enabled:false});
    await send(f.object,host,{type:'ack',sequence:i});
  }
  await send(f.object,guest,{type:'ping'});
  assert.equal(guest.closed,undefined);
  assert.equal(guest.info.inFlight,32);
  assert.deepEqual(Object.keys(guest.info.pending).sort(),['ack','pong','session']);
  for(let i=0;i<3;i++) await send(f.object,guest,{type:'seen'});
  assert.deepEqual(guest.messages.slice(-3),[
    {type:'session',generation:29,enabled:false}, {type:'ack',sequence:29}, {type:'pong'},
  ]);
  assert.deepEqual(guest.info.pending,{});
  assert.equal(guest.info.inFlight,32);
  assert.equal(guest.closed,undefined);
});

test('only a neutral host gets bounded loading grace, not active play or guests', async () => {
  for (const [role, enabled, idle, closes] of [
    ['host',false,4000,false], ['host',false,16000,true],
    ['host',true,3000,true], ['guest',false,3000,true],
  ]) {
    const f=fixture(), socket=f.add(role);await ready(f,socket);
    if (role==='host') await send(f.object,socket,{type:'session',generation:1,enabled});
    socket.info.last=Date.now()-idle;
    await f.object.alarm();assert.equal(!!socket.closed,closes);
  }
});

test('valid receive credits do not double-charge the client command budget', async () => {
  const f=fixture(), guest=f.add('guest');await ready(f,guest);
  // 45 commands with 45 valid responses are below the 60-command limit,
  // including when a resumed receiver processes them within one time window.
  for(let i=0;i<45;i++) {
    await send(f.object,guest,{type:'ping'});
    await send(f.object,guest,{type:'seen'});
    assert.equal(guest.closed,undefined);
  }
  for(let i=0;i<20 && !guest.closed;i++) {
    await send(f.object,guest,{type:'ping'});
    await send(f.object,guest,{type:'seen'});
  }
  assert.equal(guest.closed.reason,'Input rate exceeded');
});

test('unsolicited receive credits cannot bypass rate or backpressure limits', async () => {
  const f=fixture(), host=f.add('host');await ready(f,host);
  await send(f.object,host,{type:'seen'});assert.equal(host.info.inFlight,0);
  await send(f.object,host,{type:'seen'});
  assert.equal(host.closed.reason,'Unexpected receive credit');
});

function campaign(sequence=1,extra={}) {
  return view(sequence,{scene:'antarctica-v1',world:{draw:[
    [0,123,50,0,1,0,[0,0,844,390],['images/objects/coin/coin-0.png',0,0,[[0,0,32,32,10,20,32,32,0]],[1,1,1,1]]]
  ],entities:[[123,'coin',10,20,'normal',0]],coins:42,checkpoint:null,phase:'playing'},...extra});
}
test('campaign complete state has bounded geometry, stable unique IDs and no executable or remote resource fields',()=>{
  assert.equal(validView(campaign()),true);
  const mirrored=campaign();mirrored.world.draw[0][3]=4;assert.equal(validView(mirrored),true);
  const large=campaign();large.world.draw[0][7][3]=Array.from({length:2048},()=>[0,0,32,32,0,0,32,32,0]);
  assert.equal(validView(large),true);large.world.draw[0][7][3].push([0,0,32,32,0,0,32,32,0]);assert.equal(validView(large),false);
  for (const mutate of [
    f=>f.world.entities.push(f.world.entities[0]),f=>f.world.draw[0][7][0]='images/../secret.png',
    f=>f.world.draw[0][7][0]='https://other.invalid/a.png',f=>f.world.draw[0][7][3][0][4]=NaN,
    f=>f.world.draw[0][7][4][0]=2,f=>f.world.phase='arbitrary',f=>f.world.checkpoint=[NaN,0],
    f=>f.world.entities[0][4]='run(script)',f=>f.world.script='execute',
  ]) {const frame=campaign();mutate(frame);assert.equal(validView(frame),false);}
  const bonus=campaign();bonus.players[0].action='big-walk-right';assert.equal(validView(bonus),true);
});
test('optional native geometry metadata is bounded and player identities belong to the complete world',()=>{
  const value=campaign();value.world.draw[0].push([10,20,1]);
  value.world.entities.push([124,'moving-sprite',40,20,'small-walk-right',0]);value.world.playerUids=[123,124];
  assert.equal(validView(value),true);
  for(const mutate of [f=>f.world.draw[0][8][2]=0,f=>f.world.draw[0][8][0]=NaN,
    f=>f.world.draw[0][8].push(1),f=>f.world.playerUids=[123,123],f=>f.world.playerUids=[123,125],
    f=>f.world.playerUids.push(124)]) {
    const bad=structuredClone(value);mutate(bad);assert.equal(validView(bad),false);
  }
});
test('only matching guest baselines acknowledge readiness; authoritative completion survives receiver backpressure',async()=>{
  const f=fixture(),host=f.add('host'),guest=f.add('guest');await ready(f,host);
  await send(f.object,guest,{type:'hello',protocol:PROTOCOL,build,view:true});
  await send(f.object,host,{type:'session',generation:1,enabled:false});await send(f.object,host,campaign());
  await send(f.object,guest,{type:'view-ready',session:1,epoch:2});
  assert.equal(host.messages.some(p=>p.type==='view-ready'),false);
  await send(f.object,guest,{type:'view-ready',session:1,epoch:1});
  assert.equal(host.messages.at(-1).type,'view-ready');
  // Exercise the actual full receive window: completion must remain separate
  // from a coalesced visual frame and survive a hibernation reconstruction.
  guest.serializeAttachment({...guest.info,inFlight:32});
  await send(f.object,host,{type:'result',session:1,epoch:1,generation:1,win:true});
  assert.equal(guest.info.pending.result.win,true);
  await send(new CoopRoom(f.state),guest,{type:'seen'});
  assert.equal(guest.messages.at(-1).win,true);
  await send(f.object,guest,{type:'result',session:1,epoch:1,generation:1,win:true});assert.equal(guest.closed.code,1008);
});

test('large visual baseline never enters the 2 KiB WebSocket attachment; hibernation safely requests the next full frame',async()=>{
  const f=fixture(),h=f.add('host'),g=f.add('guest');await ready(f,h);await send(f.object,g,{type:'hello',protocol:PROTOCOL,build,view:true});
  await send(f.object,h,{type:'session',generation:1,enabled:true});
  for (let seq=1;seq<=45;seq++) {
    const packet=campaign(seq);packet.world.draw[0][7][3]=Array.from({length:300},()=>[0,0,32,32,0,0,32,32,0]);
    await send(f.object,h,packet);
  }
  assert.equal(f.object.pendingViews.size,1);assert.equal(g.info.pending.view.sequence,45);
  const resumed=new CoopRoom(f.state); // reconstruct like a hibernation wake
  await send(resumed,g,{type:'seen'});
  assert.equal(g.info.pending.view,undefined);
  await send(resumed,h,campaign(46));assert.equal(g.messages.at(-1).sequence,46);
});
