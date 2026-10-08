// Private input relay. No game simulation or world snapshots live here.
export const PROTOCOL = 1;
export const ROOM_MS = 15 * 60 * 1000;
const TOKEN = /^[a-f0-9]{64}$/;
const BUILD = /^[a-f0-9]{64}$/;
const UINT = value => Number.isInteger(value) && value > 0 && value < 0x80000000;
const FIELDS = {hello: ['type','protocol','build'], ping: ['type'], seen: ['type'], session: ['type','generation','enabled'], ack: ['type','sequence'], input: ['type','generation','sequence','mask']};
const json = (body, status = 200) => new Response(JSON.stringify(body), {
  status, headers: { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' },
});
const secret = () => Array.from(crypto.getRandomValues(new Uint8Array(32)), x => x.toString(16).padStart(2, '0')).join('');

export async function coopFetch(request, env) {
  if (!env.COOP_ROOMS) return new Response('Private input proof is not enabled on this server.', {status: 404});
  const url = new URL(request.url);
  // Browser credentials are origin-bound. No permissive CORS or URL tokens.
  if (request.headers.get('Origin') !== url.origin) return json({error: 'Origin rejected'}, 403);
  if (url.pathname === '/coop/rooms' && request.method === 'POST') {
    if (Number(request.headers.get('Content-Length')) > 512) return json({error: 'Request too large'}, 413);
    const reader = request.body?.getReader();
    if (!reader) return json({error: 'Invalid request'}, 400);
    const chunks = []; let bytes = 0;
    while (true) {
      const {value, done} = await reader.read();
      if (done) break;
      bytes += value.byteLength;
      if (bytes > 512) {await reader.cancel(); return json({error: 'Request too large'}, 413);}
      chunks.push(value);
    }
    const data = new Uint8Array(bytes); let offset = 0;
    for (const chunk of chunks) {data.set(chunk, offset); offset += chunk.byteLength;}
    const body = new TextDecoder().decode(data);
    let value;
    try { value = JSON.parse(body); } catch { return json({error: 'Invalid request'}, 400); }
    if (value.protocol !== PROTOCOL || !BUILD.test(value.build)) return json({error: 'Unsupported build/protocol'}, 400);
    const room = crypto.randomUUID().replaceAll('-', ''), host = secret(), guest = secret();
    const stub = env.COOP_ROOMS.get(env.COOP_ROOMS.idFromName(room));
    const response = await stub.fetch('https://room.internal/create', {
      method: 'POST', body: JSON.stringify({host, guest, build: value.build, origin: url.origin, expires: Date.now() + ROOM_MS}),
    });
    if (!response.ok) return json({error: 'Room creation failed'}, 503);
    return json({room, host, guest, protocol: PROTOCOL, build: value.build, expiresIn: ROOM_MS / 1000}, 201);
  }
  const match = /^\/coop\/rooms\/([a-f0-9]{32})\/socket$/.exec(url.pathname);
  if (match && request.method === 'GET' && request.headers.get('Upgrade')?.toLowerCase() === 'websocket')
    return env.COOP_ROOMS.get(env.COOP_ROOMS.idFromName(match[1])).fetch(request);
  return json({error: 'Not found'}, 404);
}

export class CoopRoom {
  constructor(state) { this.state = state; }

  async fetch(request) {
    if (new URL(request.url).pathname === '/create') {
      if (await this.state.storage.get('room')) return json({error: 'Already initialized'}, 409);
      const room = await request.json();
      await this.state.storage.put('room', room);
      await this.state.storage.setAlarm(Date.now() + ROOM_MS);
      return json({created: true});
    }
    const room = await this.state.storage.get('room');
    if (!room || Date.now() >= room.expires) return json({error: 'Room expired'}, 410);
    if (request.headers.get('Origin') !== room.origin) return json({error: 'Origin rejected'}, 403);
    const protocols = (request.headers.get('Sec-WebSocket-Protocol') || '').split(',').map(x => x.trim());
    const credential = /^(host|guest)\.([a-f0-9]{64})$/.exec(protocols[1] || '');
    if (protocols.length !== 2 || protocols[0] !== 'supertux-coop-v1' || !credential ||
        !TOKEN.test(credential[2]) || room[credential[1]] !== credential[2])
      return json({error: 'Credentials rejected'}, 403);
    const role = credential[1];
    if (this.state.getWebSockets(role).length || this.state.getWebSockets().length >= 2)
      return json({error: 'Room role already occupied'}, 409);
    const [client, server] = Object.values(new WebSocketPair());
    this.state.acceptWebSocket(server, [role]);
    server.serializeAttachment({role, hello: false, last: Date.now(), opened: Date.now(), rateStart: Date.now(), count: 0, generation: 0, sequence: 0, inFlight: 0});
    await this.arm(room);
    return new Response(null, {status: 101, webSocket: client, headers: {'Sec-WebSocket-Protocol': 'supertux-coop-v1'}});
  }

  send(socket, value) {
    const info = socket.deserializeAttachment();
    if ((info.inFlight || 0) >= 32) {this.close(socket, 1013, 'Receiver fell behind'); return false;}
    info.inFlight = (info.inFlight || 0) + 1;
    socket.serializeAttachment(info);
    try { socket.send(JSON.stringify(value)); return true; }
    catch { try { socket.close(1011, 'Relay send failed'); } catch {} return false; }
  }
  peer(role) { return this.state.getWebSockets(role)[0]; }
  close(socket, code, reason) { try { socket.close(code, reason); } catch {} }

  async webSocketMessage(socket, raw) {
    const room = await this.state.storage.get('room');
    const info = socket.deserializeAttachment();
    if (!room || Date.now() >= room.expires) { this.close(socket, 1008, 'Room expired'); return; }
    if (typeof raw !== 'string' || new TextEncoder().encode(raw).length > 512) { this.close(socket, 1009, 'Message too large'); return; }
    const now = Date.now();
    if (now - info.rateStart >= 1000) { info.rateStart = now; info.count = 0; }
    if (++info.count > 60) { this.close(socket, 1008, 'Input rate exceeded'); return; }
    let message;
    try { message = JSON.parse(raw); } catch { this.close(socket, 1008, 'Malformed message'); return; }
    if (!message || typeof message !== 'object' || Array.isArray(message)) { this.close(socket, 1008, 'Malformed message'); return; }
    if (!FIELDS[message.type] || Object.keys(message).some(key => !FIELDS[message.type].includes(key))) {
      this.close(socket, 1008, 'Unknown message fields or role change'); return;
    }
    if (!info.hello) {
      if (message.type !== 'hello' || message.protocol !== PROTOCOL || message.build !== room.build) {
        this.close(socket, 1008, 'Build or protocol mismatch'); return;
      }
      info.hello = true;
      this.send(socket, {type: 'ready', role: info.role, protocol: PROTOCOL});
    } else if (message.type === 'ping') {
      this.send(socket, {type: 'pong'});
    } else if (message.type === 'seen') {
      // Application-level receive credit, independent of runtime-specific
      // bufferedAmount support. Never queue more than 32 outbound messages.
      const current = socket.deserializeAttachment();
      current.inFlight = Math.max(0, (current.inFlight || 0) - 1);
      socket.serializeAttachment(current);
    } else if (info.role === 'host' && message.type === 'session' && UINT(message.generation) && typeof message.enabled === 'boolean') {
      info.session = {type: 'session', generation: message.generation, enabled: message.enabled};
      const peer = this.peer('guest');
      if (peer?.deserializeAttachment().hello) this.send(peer, info.session);
    } else if (info.role === 'host' && message.type === 'ack' && Number.isInteger(message.sequence) && message.sequence >= 0 && message.sequence < 0x80000000) {
      const peer = this.peer('guest');
      if (peer?.deserializeAttachment().hello) this.send(peer, {type: 'ack', sequence: message.sequence});
    } else if (info.role === 'guest' && message.type === 'input' && UINT(message.generation) && UINT(message.sequence) &&
               Number.isInteger(message.mask) && message.mask >= 0 && message.mask <= 127) {
      const host = this.peer('host'), session = host?.deserializeAttachment().session;
      if (host?.deserializeAttachment().hello && session?.enabled && session.generation === message.generation &&
          (info.generation !== message.generation || message.sequence > info.sequence)) {
        info.generation = message.generation; info.sequence = message.sequence;
        this.send(host, {type: 'input', generation: message.generation, sequence: message.sequence, mask: message.mask});
      }
    } else { this.close(socket, 1008, 'Message not allowed for role'); return; }
    info.last = now;
    info.inFlight = socket.deserializeAttachment().inFlight;
    socket.serializeAttachment(info);
    if (message.type === 'hello') {
      const other = this.peer(info.role === 'host' ? 'guest' : 'host');
      if (other?.deserializeAttachment().hello) {
        this.send(socket, {type: 'peer', connected: true});
        this.send(other, {type: 'peer', connected: true});
        const session = this.peer('host')?.deserializeAttachment().session;
        if (session) this.send(this.peer('guest'), session);
      }
    }
    await this.arm(room);
  }

  async webSocketClose(socket, code, reason) {
    const role = socket.deserializeAttachment().role;
    this.close(socket, code === 1005 ? 1000 : code, reason);
    if (role === 'guest') {
      const host = this.peer('host');
      if (host) this.send(host, {type: 'peer', connected: false});
    } else {
      for (const peer of this.state.getWebSockets()) this.close(peer, 1012, 'Host left; create a new room');
      await this.state.storage.deleteAll();
      await this.state.storage.deleteAlarm();
    }
  }
  async webSocketError(socket) { await this.webSocketClose(socket, 1011, 'Connection error'); }

  async arm(room) {
    let next = room.expires;
    for (const socket of this.state.getWebSockets()) {
      const info = socket.deserializeAttachment();
      next = Math.min(next, info.hello ? info.last + 2500 : info.opened + 5000);
    }
    await this.state.storage.setAlarm(Math.max(Date.now() + 100, next));
  }
  async alarm() {
    const room = await this.state.storage.get('room');
    if (!room) return;
    if (Date.now() >= room.expires) {
      for (const socket of this.state.getWebSockets()) this.close(socket, 1008, 'Room expired');
      await this.state.storage.deleteAll(); return;
    }
    for (const socket of this.state.getWebSockets()) {
      const info = socket.deserializeAttachment();
      if ((!info.hello && Date.now() - info.opened >= 5000) || (info.hello && Date.now() - info.last >= 2500)) {
        await this.webSocketClose(socket, 1001, 'Connection timed out');
      }
    }
    if (await this.state.storage.get('room')) await this.arm(room);
  }
}
