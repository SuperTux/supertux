import test from 'node:test';
import assert from 'node:assert/strict';
import {PassThrough, Writable} from 'node:stream';
import {performance} from 'node:perf_hooks';
import http from 'node:http';
import {shape, startProxy} from '../../tools/web/coop/relay_proxy.mjs';

test('ordered shaping delays bytes and enforces bandwidth without losing a half-close', async () => {
  const source = new PassThrough(), chunks = [], start = performance.now();
  const target = new Writable({write(chunk, _, done) {chunks.push(Buffer.from(chunk)); done();}});
  const completed = new Promise(resolve => target.on('finish', resolve));
  shape(source, target, {rtt_ms:40, jitter_ms:0, mbps:.08});
  source.write(Buffer.alloc(1000, 1)); source.end(Buffer.alloc(1000, 2));
  await completed;
  assert.deepEqual(Buffer.concat(chunks), Buffer.concat([Buffer.alloc(1000,1), Buffer.alloc(1000,2)]));
  assert.ok(performance.now() - start >= 210, '20 ms propagation + 200 ms byte budget');
});

test('benchmark proxy rejects remote origins and invalid shaping before listening', async () => {
  await assert.rejects(startProxy('https://example.com', {rtt_ms:0,jitter_ms:0,mbps:0}), /local HTTP/);
  await assert.rejects(startProxy('http://127.0.0.1:1234', {rtt_ms:NaN,jitter_ms:0,mbps:0}), /Invalid/);
});

test('a slow consumer bounds the shaping queue while preserving every byte', async () => {
  const source=new PassThrough(), chunks=[];
  const target=new Writable({highWaterMark:16384,write(chunk, _, done) {
    chunks.push(Buffer.from(chunk));setTimeout(done,1);
  }});
  const complete=new Promise(resolve=>target.on('finish',resolve));
  const proxy=shape(source,target,{rtt_ms:50,jitter_ms:20,mbps:0});
  const payload=Buffer.alloc(1024*1024,7);
  for(let offset=0;offset<payload.length;offset+=16384)source.write(payload.subarray(offset,offset+16384));
  assert.ok(proxy.queuedBytes<=320*1024,'Pause upstream before accumulating the entire stream');
  source.end();await complete;
  assert.deepEqual(Buffer.concat(chunks),payload);
});

test('HTTP observations count streamed bodies while preserving headers and origin checks', async () => {
  const upstream=http.createServer((request,response)=>{
    response.writeHead(200,{'Content-Type':'image/png','Cache-Control':'public,max-age=31536000,immutable'});
    response.end(request.headers.origin || 'artwork');
  });
  await new Promise(resolve=>upstream.listen(0,'127.0.0.1',resolve));
  const upstreamOrigin=`http://127.0.0.1:${upstream.address().port}`;
  let proxy;
  try {
    proxy=await startProxy(upstreamOrigin,{rtt_ms:0,jitter_ms:0,mbps:0});
    const response=await fetch(proxy.url+'coop-art/'+'a'.repeat(64)+'.png');
    assert.equal(response.headers.get('content-type'),'image/png');
    assert.match(response.headers.get('cache-control'),/immutable/);
    assert.equal(await response.text(),'artwork');
    const stats=await(await fetch(proxy.url+'__benchmark_stats')).json();
    assert.equal(stats.art_requests,1);assert.equal(stats.art_response_body_bytes,7);
    assert.equal(await(await fetch(proxy.url,{headers:{Origin:new URL(proxy.url).origin}})).text(),upstreamOrigin);
    assert.equal(await(await fetch(proxy.url,{headers:{Origin:'https://foreign.example'}})).text(),'https://foreign.example');
  } finally {
    await proxy?.dispose();await new Promise(resolve=>upstream.close(resolve));
  }
});
