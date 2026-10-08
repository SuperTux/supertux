/* Private input proof: the guest has no game simulation or world renderer. */
(function () {
  'use strict';
  const protocol = 1, limit = 32, buttons = {ArrowLeft: 1, KeyA: 1, ArrowRight: 2, KeyD: 2,
    ArrowUp: 4, KeyW: 4, ArrowDown: 8, KeyS: 8, Space: 16, KeyJ: 16, ControlLeft: 32, KeyK: 32, ShiftLeft: 64};

  class Connection {
    constructor(room, role, token, build, events) {
      this.events = events; this.ready = false; this.last = Date.now(); this.closed = false;
      const url = new URL(`/coop/rooms/${room}/socket`, location.href);
      url.protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
      this.socket = new WebSocket(url, ['supertux-coop-v1', `${role}.${token}`]);
      this.socket.addEventListener('open', () => {if (!this.closed) this.send({type: 'hello', protocol, build});});
      this.socket.addEventListener('message', event => {
        if (this.closed) return; // buffered events from a retired room own no input
        if (typeof event.data !== 'string' || event.data.length > 512) { this.close('Invalid relay message'); return; }
        let value;
        try { value = JSON.parse(event.data); } catch { this.close('Invalid relay message'); return; }
        if (!value || typeof value !== 'object' || Array.isArray(value)) {this.close('Invalid relay message'); return;}
        this.last = Date.now();
        this.send({type: 'seen'});
        if (value.type === 'ready') { this.ready = true; events.ready?.(); }
        events.message?.(value);
      });
      this.socket.addEventListener('close', event => this.finish(event.reason || 'Connection closed. Rejoin before starting a level.'));
      this.socket.addEventListener('error', () => this.finish('Connection rejected or unavailable. Check the room link and build.'));
      this.timer = setInterval(() => {
        if (Date.now() - this.last > 4000) this.close('Relay timed out. Rejoin before starting a level.');
        else if (this.ready) { this.send({type: 'ping'}); events.refresh?.(); }
      }, role === 'guest' ? 125 : 500);
    }
    send(message) {
      if (this.closed || this.socket.readyState !== WebSocket.OPEN) return false;
      if (this.socket.bufferedAmount > 16384) { this.close('Connection is too slow. Rejoin before starting a level.'); return false; }
      this.socket.send(JSON.stringify(message)); return true;
    }
    finish(reason) {
      if (this.closed) return;
      this.closed = true; this.ready = false; clearInterval(this.timer); this.events.closed?.(reason);
      try { this.socket.close(1000, 'Input proof stopped'); } catch {}
    }
    close(reason = 'Room closed') { this.finish(reason); }
  }

  function host(module) {
    const queue = [], status = document.getElementById('coop_status');
    let connection, creating = false, createEpoch = 0, joinRejected = false, state = {reserved: 0, enabled: false, generation: 0, sequence: 0}, lastSent = '';
    const say = text => { if (status) status.textContent = text; };
    const enqueue = value => {
      if (value[0] !== 2) queue.length = 0;
      if (queue.length >= limit) { queue.length = 0; queue.push([4, 0, 0, 0]); say('Input backlog cleared. Release controls and try again.'); return; }
      queue.push(value);
    };
    const session = () => {
      if (!connection?.ready || !state.generation) return;
      connection.send({type: 'session', generation: state.generation, enabled: state.enabled});
      connection.send({type: 'ack', sequence: state.sequence});
    };
    module.supertuxCoop = {
      enqueue, poll: () => queue.shift(),
      engineStatus(reserved, enabled, generation, sequence) {
        state = {reserved, enabled: !!enabled, generation, sequence};
        const key = `${reserved}/${enabled}/${generation}`;
        if (key !== lastSent) { lastSent = key; session(); }
        if (reserved === 1) joinRejected = false;
        if (reserved === -2) joinRejected = true;
        if (connection?.ready && reserved === 1) say(enabled ? 'Player 2 input active. Host owns the game and saves.' : 'Player 2 joined. Input waits while the host is in menus or paused.');
        if (connection?.ready && reserved === -1 && !joinRejected) say('Player 2 disconnected. Return to the title screen, rejoin, then start a level.');
        if (connection?.ready && joinRejected) say('Join requires one local player at the title screen. Return there and rejoin before starting a level; reload if local Player 2 was already configured.');
      },
      get state() { return {...state, queued: queue.length, joinRejected}; },
      get connection() { return connection; },
    };
    const panel = document.getElementById('coop_panel');
    if (panel) panel.hidden = !new URLSearchParams(location.search).has('coop');
    document.getElementById('coop_create')?.addEventListener('click', async () => {
      if (creating || !module.supertuxReady) return;
      const epoch = ++createEpoch;
      creating = true; joinRejected = false; connection?.close(); enqueue([3, 0, 0, 0]);
      try {
        const build = window.SUPERTUX_DEPLOY_CONFIG?.manifestSha256;
        const response = await fetch('/coop/rooms', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({protocol, build}), signal: AbortSignal.timeout(10000)});
        if (!response.ok) throw Error('Private input rooms are unavailable here. Start the local proof server.');
        const room = await response.json();
        if (epoch !== createEpoch || document.hidden) return;
        const join = new URL('coop-controller.html', location.href);
        join.hash = new URLSearchParams({room: room.room, token: room.guest, build: room.build});
        const link = document.getElementById('coop_link'); link.href = join.href; link.textContent = join.href;
        connection = new Connection(room.room, 'host', room.host, build, {
          ready: () => { say('Room ready. Invite Player 2 before starting a level.'); session(); },
          message: value => {
            if (value.type === 'peer') {joinRejected = false; enqueue([value.connected ? 1 : 3, 0, 0, 0]);}
            if (value.type === 'input') enqueue([2, value.generation, value.sequence, value.mask]);
          },
          refresh: session,
          closed: reason => { enqueue([3, 0, 0, 0]); say(reason); },
        });
      } catch (error) { if (epoch === createEpoch) say(error.message); }
      finally { creating = false; }
    });
    document.getElementById('coop_close')?.addEventListener('click', () => {++createEpoch; connection?.close();});
    document.getElementById('coop_antarctica')?.addEventListener('click', () => enqueue([5, 0, 0, 0]));
    document.getElementById('coop_forest')?.addEventListener('click', () => enqueue([5, 1, 0, 0]));
    // The trusted Start/Resume shell already pauses the engine. Clear the JS
    // backlog too, so an old browser event cannot reassert held movement.
    for (const event of ['blur', 'pagehide']) window.addEventListener(event, () => {++createEpoch; enqueue([4, 0, 0, 0]);});
    document.addEventListener('visibilitychange', () => { if (document.hidden) {++createEpoch; enqueue([4, 0, 0, 0]);} });
  }

  function guest() {
    const params = new URLSearchParams(location.hash.slice(1)), keys = new Map(), fingers = new Map();
    const status = document.getElementById('guest_status');
    let connection, generation = 0, sequence = 0, enabled = false, joining = false, joinEpoch = 0;
    const mask = () => [...keys.values(), ...fingers.values()].reduce((a, b) => a | b, 0);
    const send = () => {
      if (enabled && connection?.ready) connection.send({type: 'input', generation, sequence: ++sequence, mask: mask()});
    };
    const clear = () => { keys.clear(); fingers.clear(); send(); };
    const join = async () => {
      if (joining) return;
      const epoch = ++joinEpoch;
      connection?.close(); enabled = false; clear(); generation = sequence = 0;
      joining = true;
      if (connection && connection.socket.readyState !== WebSocket.CLOSED) {
        await new Promise(resolve => {
          const timer = setTimeout(resolve, 1000);
          connection.socket.addEventListener('close', () => {clearTimeout(timer); resolve();}, {once:true});
        });
      }
      if (!/^[a-f0-9]{32}$/.test(params.get('room') || '') || !/^[a-f0-9]{64}$/.test(params.get('token') || '') || !/^[a-f0-9]{64}$/.test(params.get('build') || '')) {
        joining = false; status.textContent = 'Use the private controller link shared by the host.'; return;
      }
      status.textContent = 'Joining diagnostic controller…';
      try {
        const html = await (await fetch('index.html', {cache: 'no-store', signal: AbortSignal.timeout(10000)})).text();
        const config = /window\.SUPERTUX_DEPLOY_CONFIG = (.*?);<\/script>/.exec(html);
        if (!config || JSON.parse(config[1]).manifestSha256 !== params.get('build')) throw Error('The host and controller builds differ. Ask the host for a new room link.');
      } catch (error) { joining = false; status.textContent = error.message; return; }
      if (epoch !== joinEpoch || document.hidden) {joining = false; return;}
      connection = new Connection(params.get('room'), 'guest', params.get('token'), params.get('build'), {
        ready: () => { joining = false; status.textContent = 'Joined. Wait for the host to start a level.'; },
        message: value => {
          if (value.type === 'session' && Number.isInteger(value.generation) && typeof value.enabled === 'boolean') {
            if (generation !== value.generation || enabled !== value.enabled) {
              enabled = false; keys.clear(); fingers.clear(); generation = value.generation; sequence = 0;
              enabled = value.enabled; send(); // fresh neutral state, never held-key replay
            }
            status.textContent = enabled ? 'Player 2 input active. The game is visible on the host only.' : 'Host is paused or in menus. Release controls before continuing.';
          }
          if (value.type === 'ack') document.getElementById('guest_ack').textContent = `Host accepted input ${value.sequence}.`;
        },
        refresh: send,
        closed: reason => { joining = false; enabled = false; clear(); status.textContent = reason; },
      });
    };
    window.addEventListener('keydown', event => {
      if (buttons[event.code]) { event.preventDefault(); if (enabled && !event.repeat) { keys.set(event.code, buttons[event.code]); send(); } }
    });
    window.addEventListener('keyup', event => { if (buttons[event.code]) { event.preventDefault(); keys.delete(event.code); send(); } });
    for (const button of document.querySelectorAll('[data-control]')) {
      button.addEventListener('pointerdown', event => {
        event.preventDefault(); if (!enabled) return;
        button.setPointerCapture(event.pointerId); fingers.set(event.pointerId, Number(button.dataset.control)); send();
      });
      for (const type of ['pointerup', 'pointercancel', 'lostpointercapture']) button.addEventListener(type, event => { fingers.delete(event.pointerId); send(); });
      button.addEventListener('contextmenu', event => event.preventDefault());
    }
    window.addEventListener('blur', clear);
    window.addEventListener('pagehide', () => { ++joinEpoch; clear(); connection?.close('Controller left. Rejoin from the host title screen.'); });
    document.addEventListener('visibilitychange', () => { if (document.hidden) { ++joinEpoch; clear(); connection?.close('Controller backgrounded. Rejoin from the host title screen.'); } });
    document.getElementById('guest_join').addEventListener('click', join);
    // Diagnostic state is input/connection only. It contains no world data.
    window.supertuxGuest = {get state() {return {enabled, generation, sequence, mask: mask(), connected: !!connection?.ready};}, get connection() {return connection;}};
    join();
  }
  window.SupertuxCoop = {Connection, host, guest};
  if (window.Module) host(window.Module);
  else if (document.getElementById('guest_status')) guest();
})();
