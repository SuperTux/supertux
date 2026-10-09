const {test} = require('node:test');
const assert = require('node:assert/strict');
const {readFileSync} = require('node:fs');
const vm = require('node:vm');
const source = readFileSync('mk/emscripten/coop.js', 'utf8');

function target(extra = {}) {
  const listeners = new Map();
  return Object.assign({addEventListener(name, fn) {listeners.set(name, fn);},
    async fire(name, value = {}) {return listeners.get(name)?.(value);}}, extra);
}
function fixture(guest = false, fetch = async () => {throw Error('Unexpected fetch');}) {
  const elements = new Map(['coop_status','coop_panel','coop_create','coop_close','coop_link',
    'coop_antarctica','coop_forest',...(guest ? ['guest_status','guest_join','guest_ack'] : [])]
    .map(id => [id, target({textContent: ''})]));
  const sockets = [];
  class Socket {
    static OPEN = 1; static CLOSED = 3;
    constructor() {Object.assign(this, target()); this.readyState = 1; this.bufferedAmount = 0; this.sent = []; sockets.push(this);}
    send(value) {this.sent.push(JSON.parse(value));}
    close() {this.readyState = 3;}
  }
  const document = target({hidden: false, getElementById: id => elements.get(id), querySelectorAll: () => []});
  const window = target({document, TextEncoder, URL, URLSearchParams, WebSocket: Socket, AbortSignal, fetch,
    location: {href: 'http://localhost/index.html', protocol: 'http:', search: '?coop=1',
      hash: '#'+new URLSearchParams({room:'a'.repeat(32),token:'b'.repeat(64),build:'c'.repeat(64)}).toString()},
    SUPERTUX_DEPLOY_CONFIG: {manifestSha256: 'c'.repeat(64)},
    setInterval: () => 1, clearInterval: () => {}, setTimeout, clearTimeout});
  if (!guest) window.Module = {supertuxReady: true};
  window.window = window;
  vm.runInNewContext(source, window);
  return {window, document, elements, sockets};
}

test('host queue preserves taps and fails neutral on overflow', () => {
  const f = fixture(), input = f.window.Module.supertuxCoop;
  input.enqueue([2,1,1,16]); input.enqueue([2,1,2,0]);
  assert.deepEqual(Array.from(input.poll()), [2,1,1,16]);
  assert.deepEqual(Array.from(input.poll()), [2,1,2,0]);
  for (let i=0;i<33;i++) input.enqueue([2,1,i+1,i%2]);
  assert.deepEqual(Array.from(input.poll()), [4,0,0,0]); assert.equal(input.poll(), undefined);
});

test('retired sockets ignore late callbacks and outgoing backpressure closes', async () => {
  const f = fixture(); let received = 0;
  const connection = new f.window.SupertuxCoop.Connection('room','host','token','build',{message:()=>received++});
  connection.socket.bufferedAmount = 131073;
  assert.equal(connection.send({type:'ping'}), false); assert.equal(connection.closed, true);
  await connection.socket.fire('message',{data:JSON.stringify({type:'input',mask:2})});
  assert.equal(received,0);
  const failing = new f.window.SupertuxCoop.Connection('room','host','token','build',{});
  failing.socket.send = () => {throw Error('Socket closed during send');};
  assert.equal(failing.send({type:'ping'}),false); assert.equal(failing.closed,true);
});

test('host pagehide prevents a pending create response opening a late socket', async () => {
  let complete;
  const f = fixture(false, () => new Promise(resolve => complete = resolve));
  const creating = f.elements.get('coop_create').fire('click');
  await f.window.fire('pagehide');
  complete({ok:true,json:async()=>({room:'a'.repeat(32),guest:'b'.repeat(64),host:'d'.repeat(64),build:'c'.repeat(64)})});
  await creating;
  assert.equal(f.sockets.length,0);
  assert.deepEqual(Array.from(f.window.Module.supertuxCoop.poll()),[4,0,0,0]);
});

test('guest background prevents a pending identity fetch opening a late socket', async () => {
  let complete;
  const f = fixture(true, () => new Promise(resolve => complete = resolve));
  f.document.hidden = true; await f.document.fire('visibilitychange');
  complete({text:async()=>'<script>window.SUPERTUX_DEPLOY_CONFIG = '+JSON.stringify({manifestSha256:'c'.repeat(64)})+';</script>'});
  await new Promise(resolve=>setImmediate(resolve));
  assert.equal(f.sockets.length,0); assert.equal(f.window.supertuxGuest.state.enabled,false);
});

test('one normal large host baseline does not cancel control/status traffic; guest inputs retain their small buffer bound',()=>{
  const f=fixture();
  const host=new f.window.SupertuxCoop.Connection('room','host','token','build',{});
  host.socket.bufferedAmount=65536;assert.equal(host.send({type:'ping'}),true);
  host.socket.bufferedAmount=131073;assert.equal(host.send({type:'ping'}),false);
  const guest=new f.window.SupertuxCoop.Connection('room','guest','token','build',{});
  guest.socket.bufferedAmount=16385;assert.equal(guest.send({type:'input'}),false);
});

test('native campaign load identity survives old title/background draws; restart requires a fresh matching acknowledgment',async()=>{
  const f=fixture(false,async()=>({ok:true,json:async()=>({room:'a'.repeat(32),guest:'b'.repeat(64),host:'d'.repeat(64),build:'c'.repeat(64)})}));
  await f.elements.get('coop_create').fire('click');const socket=f.sockets[0],engine=f.window.Module.supertuxCoop;
  await socket.fire('message',{data:JSON.stringify({type:'ready'})});
  await socket.fire('message',{data:JSON.stringify({type:'peer',connected:true,view:true})});
  engine.engineStatus(1,false,1,0);assert.equal(engine.sceneReady(3,1),false);
  await socket.fire('message',{data:JSON.stringify({type:'view-ready',session:3,epoch:1})});assert.equal(engine.sceneReady(3,1),true);
  engine.view({session:1,epoch:1,scene:'unsupported'});assert.equal(engine.sceneReady(3,1),true);
  assert.equal(engine.sceneReady(3,2),false);
  await socket.fire('message',{data:JSON.stringify({type:'view-ready',session:3,epoch:1})});assert.equal(engine.sceneReady(3,2),false);
  await socket.fire('message',{data:JSON.stringify({type:'view-ready',session:3,epoch:2})});assert.equal(engine.sceneReady(3,2),true);
});
