// Benchmark-only ordered WebSocket byte-stream shaping. HTTP assets are streamed
// without shaping. No production routing, credentials or game rules live here.
import http from 'node:http';
import {performance} from 'node:perf_hooks';
import {pathToFileURL} from 'node:url';

export function shape(source, target, profile, seed = 1) {
  const queue = []; let bytes = 0, timer, due = 0, blocked = false, stopped = false, ended = false;
  const random = () => {seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0; return seed / 2**32;};
  const stop = () => {stopped = true; clearTimeout(timer); queue.length = 0; bytes = 0;};
  const pump = () => {
    if (stopped || blocked) return;
    if (!queue.length) {if (ended) target.end(); return;}
    const delay = queue[0].due - performance.now();
    if (delay > 0) {timer = setTimeout(pump, delay); return;}
    const item = queue.shift(); bytes -= item.chunk.length;
    blocked = !target.write(item.chunk);
    if (bytes < 128 * 1024 && !blocked) source.resume();
    if (blocked) source.pause();
    else pump();
  };
  source.on('data', chunk => {
    const propagation = Math.max(0, profile.rtt_ms / 2 + (random() - .5) * profile.jitter_ms);
    const transmission = profile.mbps ? chunk.length * 8 / (profile.mbps * 1000) : 0;
    due = Math.max(performance.now() + propagation, due) + transmission;
    queue.push({chunk, due}); bytes += chunk.length;
    if (bytes >= 256 * 1024) source.pause();
    clearTimeout(timer); pump();
  });
  target.on('drain', () => {blocked = false; if (bytes < 128 * 1024) source.resume(); pump();});
  // TCP half-close must follow queued bytes, not truncate a WebSocket close.
  source.on('end', () => {ended = true; pump();});
  source.on('error', () => target.destroy()); target.on('error', () => source.destroy());
  target.on('close', () => {stop(); source.destroy();});
  source.on('close', () => {if (!ended) {stop(); target.destroy();}});
  return {get queuedBytes() {return bytes;}};
}

export async function startProxy(upstream, profile) {
  upstream = new URL(upstream);
  if (upstream.protocol !== 'http:' || upstream.hostname !== '127.0.0.1') throw Error('Shaping is limited to a local HTTP relay');
  for (const key of ['rtt_ms', 'jitter_ms', 'mbps'])
    if (!Number.isFinite(profile[key]) || profile[key] < 0) throw Error('Invalid shaping profile');
  const sockets = new Set();
  const stats={art_requests:0,art_response_body_bytes:0,startup_requests:0,startup_response_body_bytes:0};
  let origin;
  const options = request => ({hostname:upstream.hostname, port:upstream.port, path:request.url, method:request.method,
    headers:{...request.headers, host:upstream.host, ...(request.headers.origin === origin ? {origin:upstream.origin} : {})}});
  const server = http.createServer((request, response) => {
    if(request.method==='GET' && request.url==='/__benchmark_stats') {
      response.writeHead(200,{'Content-Type':'application/json','Cache-Control':'no-store'});
      response.end(JSON.stringify(stats));return;
    }
    const category=request.method==='GET' && (/^\/coop-art\/[a-f0-9]{64}\.png$/.test(request.url)?'art':
      /\/game-assets\/(data|wasm)(-gzip)?\//.test(request.url)?'startup':null);
    if(category)++stats[category+'_requests'];
    const remote = http.request(options(request), reply => {
      if(category)reply.on('data',chunk=>{stats[category+'_response_body_bytes']+=chunk.length;});
      response.writeHead(reply.statusCode, reply.headers); reply.pipe(response);
    });
    remote.on('error', () => {if (!response.headersSent) response.writeHead(502); response.end();});
    request.on('aborted', () => remote.destroy()); response.on('close', () => remote.destroy()); request.pipe(remote);
  });
  server.on('connection', socket => {sockets.add(socket); socket.on('close', () => sockets.delete(socket));});
  server.on('upgrade', (request, client, head) => {
    const remote = http.request(options(request));
    remote.on('upgrade', (reply, socket, remoteHead) => {
      sockets.add(socket); socket.on('close', () => sockets.delete(socket));
      client.write(`HTTP/1.1 101 Switching Protocols\r\n${Object.entries(reply.headers).map(([k,v])=>`${k}: ${v}\r\n`).join('')}\r\n`);
      shape(client, socket, profile, 1); shape(socket, client, profile, 2);
      if (head.length) socket.write(head); if (remoteHead.length) client.write(remoteHead);
    });
    remote.on('response', reply => {client.end(`HTTP/1.1 ${reply.statusCode} Rejected\r\nConnection: close\r\n\r\n`); reply.resume();});
    remote.on('error', () => client.destroy()); client.on('error', () => remote.destroy()); remote.end();
  });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  origin = `http://127.0.0.1:${server.address().port}`;
  return {url:origin+'/', async dispose() {for (const socket of sockets) socket.destroy(); await new Promise(resolve => server.close(resolve));}};
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const proxy = await startProxy(process.argv[2], JSON.parse(process.argv[3]));
  for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, async () => {await proxy.dispose(); process.exit(0);});
  console.log(`BENCHMARK_PROXY_READY ${proxy.url}`);
}
