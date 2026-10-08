const {test} = require('node:test');
const assert = require('node:assert/strict');
const {Cache, Loader, hash, valid} = require('../../mk/emscripten/assets.js');
global.location = new URL('https://game.example/preview/index.html');
const elements = new Map(['music_status','retry_music'].map(id => [id, {}]));
global.document = {hidden: false, getElementById: id => elements.get(id)};
const tick = () => new Promise(resolve => setImmediate(resolve));
async function until(predicate) { const deadline = Date.now() + 3000; while (!predicate()) { if (Date.now() > deadline) throw new Error('Condition did not settle'); await tick(); } }
const bytes = text => new TextEncoder().encode(text).buffer;
test('concurrent cache readers all fall back when IndexedDB open throws', async () => {
  global.indexedDB = {open(){throw new Error('Storage unavailable');}};
  const cache = new Cache();
  const results = await Promise.all([1,2,3].map(() => cache.read({sha256:'test'})));
  assert.deepEqual(results, [null,null,null]);
  assert.equal(cache.disabled, true);
  delete global.indexedDB;
});
async function fixture() {
  const a = bytes('OggSactual audio A'), b = bytes('OggSactual audio B');
  const entries = await Promise.all([a,b].map(async (data,i) => ({path: 'music/' + (i ? 'b' : 'a') + '.ogg', package:'music', bytes:data.byteLength, sha256:await hash(data), url:'assets/' + i})));
  const files = new Map(), stored = new Map(), requests = [];
  const Module = {supertuxShell:{active:true,muted:false}, FS:{mkdirTree(){},writeFile(path,value){files.set(path,value)}}};
  const cache = {epoch:0,read:async e => stored.get(e.sha256),store(e,buffer){stored.set(e.sha256,buffer)}};
  const loader = new Loader(Module, {assets:entries,runtimeRoot:'/data',inventorySha256:'identity'}, cache);
  global.fetch = url => new Promise((resolve,reject) => requests.push({url,resolve,reject}));
  return {a,b,entries,files,stored,requests,Module,loader};
}

test('concurrent callers share a download; cache reuse validates content', async () => {
  const f = await fixture();
  const first = f.loader.download(f.entries[0]), second = f.loader.download(f.entries[0]);
  assert.equal(first, second); await tick(); assert.equal(f.requests.length, 1);
  f.requests[0].resolve(new Response(f.a)); await first; await tick();
  await f.loader.download(f.entries[0]); assert.equal(f.requests.length, 1);
  assert.equal(f.loader.stats.cacheBytes, f.a.byteLength);
});

test('late A finishes after B without selecting or playing A', async () => {
  const f = await fixture();
  assert.equal(f.loader.requestTrack('music/a.ogg'), 1); await tick();
  assert.equal(f.loader.requestTrack('/music/b.ogg'), 1); await tick();
  f.requests[1].resolve(new Response(f.b)); await tick(); await tick();
  await until(() => f.loader.tracks.get('music/b.ogg').state === 0);
  assert.equal(f.loader.requestTrack('music/b.ogg'), 0);
  f.requests[0].resolve(new Response(f.a)); await tick(); await tick();
  assert.equal(f.loader.wanted, 'music/b.ogg');
  assert.equal(f.Module.supertuxShell.active, true);
  assert.equal(elements.get('music_status').textContent, '');
  await until(() => f.files.size === 2);
  assert.equal(f.files.size, 2); // JS mounts bytes; the engine alone chooses playback.
});

test('partial failure is not cached/mounted; explicit retry succeeds', async () => {
  const f = await fixture(); f.loader.requestTrack('music/a.ogg'); await tick();
  f.requests[0].resolve(new Response(bytes('partial'))); await tick(); await tick();
  await until(() => f.loader.tracks.get('music/a.ogg').state === 2);
  assert.equal(f.loader.requestTrack('music/a.ogg'), 2);
  assert.equal(f.files.size, 0); assert.equal(f.stored.size, 0);
  f.loader.retry(); await tick(); f.requests[1].resolve(new Response(f.a)); await tick(); await tick();
  await until(() => f.loader.tracks.get('music/a.ogg').state === 0);
  assert.equal(f.loader.requestTrack('music/a.ogg'), 0);
});

test('muted/inactive/background requests wait; corrupt cache falls back to network', async () => {
  const f = await fixture();
  f.Module.supertuxShell.muted = true; f.loader.requestTrack('music/a.ogg'); await tick(); assert.equal(f.requests.length, 0);
  f.Module.supertuxShell.muted = false; f.Module.supertuxShell.active = false; f.loader.requestTrack('music/a.ogg'); await tick(); assert.equal(f.requests.length, 0);
  f.Module.supertuxShell.active = true; document.hidden = true; f.loader.requestTrack('music/a.ogg'); await tick(); assert.equal(f.requests.length, 0);
  document.hidden = false; f.stored.set(f.entries[0].sha256, bytes('wrong but cached!'));
  f.loader.requestTrack('music/a.ogg'); await tick(); assert.equal(f.requests.length, 1);
  f.requests[0].resolve(new Response(f.a)); await tick(); await tick();
  await until(() => f.loader.tracks.get('music/a.ogg').state === 0);
  assert.equal(await valid(f.stored.get(f.entries[0].sha256), f.entries[0]), true);
});

test('bounded queue does not fetch every obsolete request', async () => {
  const f = await fixture();
  const extra = [];
  for (let i = 0; i < 8; ++i) {
    const entry = {...f.entries[0],path:'music/extra' + i, sha256:String(i).padStart(64,'0')};
    f.loader.entries.set(entry.path,entry); extra.push(entry);
    f.loader.requestTrack(entry.path);
  }
  await tick(); assert.equal(f.requests.length, 2); assert.equal(f.loader.queue.length, 1);
  assert.equal(f.loader.wanted, 'music/extra7');
  f.requests.forEach(r => r.reject(new Error('network unavailable'))); await tick(); await tick();
  assert.equal(f.requests.length, 3);
  f.requests[2].reject(new Error('network unavailable')); await tick();
});

test('gzip delivery validates decoded bytes; older browsers retain raw fallback', async () => {
  const {gzipSync} = require('node:zlib');
  const f = await fixture(), raw = f.entries[0];
  const entry = {...raw, encodings:{gzip:{url:'assets/compressed',bytes:30,sha256:'a'.repeat(64)}}};
  const load = f.loader.obtain(entry); await tick();
  assert.match(f.requests[0].url, /assets\/compressed$/);
  f.requests[0].resolve(new Response(gzipSync(Buffer.from(f.a))));
  assert.equal(await valid(await load,raw), true);
  f.stored.clear();
  const decompression = global.DecompressionStream;
  global.DecompressionStream = undefined;
  try {
    const fallback = f.loader.obtain(entry); await tick();
    assert.match(f.requests[1].url, /assets\/0$/);
    f.requests[1].resolve(new Response(f.a));
    assert.equal(await valid(await fallback,raw),true);
  } finally { global.DecompressionStream = decompression; }
  f.stored.clear();
  const damaged = f.loader.obtain(entry); await tick();
  f.requests[2].resolve(new Response(gzipSync(Buffer.from('wrong decoded bytes'))));
  await assert.rejects(damaged, /wrong size|incomplete or damaged/);
  assert.equal(f.stored.size,0);
});
