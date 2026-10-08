import {test} from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const {default: worker} = await import('data:text/javascript;base64,' + Buffer.from(fs.readFileSync(new URL('../../worker/index.js', import.meta.url))).toString('base64'));
const sha = 'a'.repeat(64);
function env() {
  const calls = [];
  const object = {body: new ReadableStream({start(c){c.enqueue(new Uint8Array([1,2,3]));c.close();}}),size:3,httpEtag:'"hash"',writeHttpMetadata(){}};
  return {calls, GAME_ASSETS:{get:async key => {calls.push(['get',key]);return object;},head:async key => {calls.push(['head',key]);return object;}},ASSETS:{fetch:request => new Response(request.url)}};
}
test('narrow routes stream complete content with matching MIME and immutable headers', async () => {
  for (const [path,mime] of [['data/'+sha+'/supertux2.data','application/octet-stream'],['wasm/'+sha+'/supertux2.wasm','application/wasm'],['music/'+sha+'/track.ogg','audio/ogg'],['music/'+sha+'/track.wav','audio/wav'],['manifest/'+sha+'/asset-manifest.json','application/json']]) {
    const bindings=env();const response=await worker.fetch(new Request('https://game.example/game-assets/'+path),bindings);
    assert.equal(response.status,200);assert.equal(response.headers.get('content-type'),mime);
    assert.equal(response.headers.get('content-length'),'3');assert.match(response.headers.get('cache-control'),/immutable/);
    assert.deepEqual([...new Uint8Array(await response.arrayBuffer())],[1,2,3]);
    assert.deepEqual(bindings.calls,[['get',path]]);
  }
});
test('HEAD uses R2 metadata; invalid paths and verbs never query the bucket', async () => {
  const bindings=env();let response=await worker.fetch(new Request('https://game.example/game-assets/music/'+sha+'/track.ogg',{method:'HEAD'}),bindings);
  assert.equal(await response.text(),'');assert.equal(bindings.calls[0][0],'head');
  for (const path of ['data/'+sha+'/supertux2.wasm','music/'+sha+'/other.ogg','music/short/track.ogg','../private']) {
    response=await worker.fetch(new Request('https://game.example/game-assets/'+path),env());
    if(path!=='../private') assert.equal(response.status,404);
  }
  const blocked=env();response=await worker.fetch(new Request('https://game.example/game-assets/music/'+sha+'/track.ogg',{method:'POST'}),blocked);
  assert.equal(response.status,405);assert.equal(blocked.calls.length,0);
});
test('gzip archives stream without HTTP decoding; encoded URLs cannot mix categories', async () => {
  for (const suffix of ['data','wasm','js']) {
    const response=await worker.fetch(new Request(`https://game.example/game-assets/${suffix}-gzip/${sha}/supertux2.${suffix}.gz`),env());
    assert.equal(response.status,200);assert.equal(response.headers.get('content-type'),'application/gzip');
    assert.equal(response.headers.get('content-encoding'),null);
    assert.equal((await response.arrayBuffer()).byteLength,3);
  }
  assert.equal((await worker.fetch(new Request(`https://game.example/game-assets/data-gzip/${sha}/supertux2.wasm.gz`),env())).status,404);
});
test('missing object stays 404 and conditional reads return 304', async () => {
  const missing=env();missing.GAME_ASSETS.get=async()=>null;
  assert.equal((await worker.fetch(new Request('https://game.example/game-assets/music/'+sha+'/track.ogg'),missing)).status,404);
  const cached=await worker.fetch(new Request('https://game.example/game-assets/music/'+sha+'/track.ogg',{headers:{'If-None-Match':'"hash"'}}),env());
  assert.equal(cached.status,304);
});
