import assert from 'node:assert/strict';
import {test} from 'node:test';
import {CoopRoom, coopFetch, PROTOCOL} from '../../worker/coop.js';

const build = 'b'.repeat(64), token = 'a'.repeat(64);
function fixture() {
  const room = {host: token, guest: 'c'.repeat(64), build, origin: 'http://localhost:8787', expires: Date.now() + 900000};
  const sockets = [];
  const storage = {room, alarm: 0, async get() {return this.room;}, async put(_, value) {this.room = value;}, async setAlarm(value) {this.alarm = value;}, async deleteAll() {this.room = null;}, async deleteAlarm() {this.alarm = 0;}};
  const state = {storage, getWebSockets(role) {return sockets.filter(s => !s.closed && (!role || s.info.role === role));}};
  const object = new CoopRoom(state);
  function add(role) {
    const socket = {info: {role, hello: false, last: Date.now(), opened: Date.now(), rateStart: Date.now(), count: 0, generation: 0, sequence: 0}, messages: [],
      deserializeAttachment() {return structuredClone(this.info);}, serializeAttachment(info) {this.info = structuredClone(info);},
      send(value) {this.messages.push(JSON.parse(value));}, close(code, reason) {this.closed ||= {code, reason};}};
    sockets.push(socket); return socket;
  }
  return {object, state, add, room};
}
const send = (object, socket, message) => object.webSocketMessage(socket, JSON.stringify(message));
async function ready(f, socket) {await send(f.object, socket, {type: 'hello', protocol: PROTOCOL, build});}

test('routing is opt-in, origin-bound, bounded and build-validated', async () => {
  assert.equal((await coopFetch(new Request('http://localhost/coop/rooms'), {})).status, 404);
  assert.equal((await coopFetch(new Request('http://localhost/coop/rooms'), {COOP_ROOMS: {}})).status, 403);
  for (const body of ['invalid', 'null', '[]', JSON.stringify({protocol: 2, build}), JSON.stringify({protocol: 1, build:[build]}), JSON.stringify({protocol: 1, build, role:'host'}), 'x'.repeat(513)]) {
    const response = await coopFetch(new Request('http://localhost/coop/rooms', {method: 'POST', headers: {Origin: 'http://localhost'}, body}), {COOP_ROOMS: {}});
    assert.ok([400,413].includes(response.status));
  }
});

test('role-bound credentials, occupied role and origin reject before upgrade', async () => {
  const f = fixture(); f.add('host');
  const request = protocols => new Request('http://localhost:8787/coop/rooms/' + '1'.repeat(32) + '/socket', {headers: {Origin: f.room.origin, 'Sec-WebSocket-Protocol': protocols}});
  assert.equal((await f.object.fetch(request(`supertux-coop-v1, host.${token}`))).status, 409);
  assert.equal((await f.object.fetch(request(`supertux-coop-v1, guest.${token}`))).status, 403);
  f.room.expires = Date.now() - 1;
  assert.equal((await f.object.fetch(request(`supertux-coop-v1, host.${token}`))).status, 410);
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
  for(let i=0;i<33 && !guest.closed;i++) await send(f.object,guest,{type:'ping'});
  assert.equal(guest.closed.code,1013);
  assert.ok(guest.messages.length <= 32);
  const other=fixture(), client=other.add('guest');await ready(other,client);
  await send(other.object,client,{type:'input',generation:1,sequence:1,mask:2,role:'host'});
  assert.equal(client.closed.code,1008);
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
