/* Phase 7A: presentation only. No WASM, simulation, scripting or save storage. */
(function (root) {
  'use strict';
  class SnapshotBuffer {
    constructor(delay = 90) {this.delay = delay; this.reset();}
    reset() {this.frames = []; this.order = null; this.lastArrival = 0;}
    clear() {this.frames = []; this.lastArrival = 0;}
    accept(frame, arrival) {
      const previous = this.order;
      if (!frame || !['session','epoch','sequence'].every(key => Number.isInteger(frame[key]) && frame[key] > 0) ||
          !Number.isFinite(frame.time) || !Number.isFinite(arrival) ||
          !Array.isArray(frame.camera) || frame.camera.length !== 5 || !frame.camera.every(Number.isFinite) ||
          !Array.isArray(frame.players) || frame.players.length > 2 || !frame.players.every(p => Number.isFinite(p.x) && Number.isFinite(p.y))) return false;
      if (previous && (frame.session < previous.session || (frame.session === previous.session &&
          (frame.epoch < previous.epoch || (frame.epoch === previous.epoch && (frame.sequence <= previous.sequence || frame.time < previous.time)))))) return false;
      if (!previous || frame.session !== previous.session || frame.epoch !== previous.epoch || frame.scene !== previous.scene) this.clear();
      this.order = frame; this.lastArrival = arrival;
      this.frames.push({frame, arrival});
      if (this.frames.length > 8) this.frames.shift();
      return true;
    }
    sample(now) {
      if (!this.frames.length || now - this.lastArrival > 1200) return null;
      // Render behind the latest host time; never extrapolate or simulate.
      const latest = this.frames.at(-1);
      const target = latest.frame.time + Math.min(now - latest.arrival, this.delay) - this.delay;
      let a = this.frames[0].frame, b = a;
      for (const item of this.frames) {
        b = item.frame;
        if (b.time >= target) break;
        a = b;
      }
      const weight = b.time > a.time ? Math.max(0, Math.min(1, (target - a.time) / (b.time - a.time))) : 0;
      const mix = (x,y) => x + (y-x)*weight;
      const discrete = weight >= 1 ? b : a;
      return {...discrete, camera:a.camera.map((x,i) => i < 3 ? mix(x,b.camera[i]) : discrete.camera[i]),
        players:discrete.players.map(player => {
          const start = a.players.find(p => p.id === player.id), end = b.players.find(p => p.id === player.id);
          // Death/respawn is a discrete visual transition, never lerp a corpse.
          return start && end && start.dead === end.dead ? {...player,x:mix(start.x,end.x),y:mix(start.y,end.y)} : {...player};
        })};
    }
  }
  if (typeof module !== 'undefined' && module.exports) {module.exports = {SnapshotBuffer}; return;}
  const canvas = document.getElementById('guest_canvas');
  if (!canvas) return;
  const context = canvas.getContext('2d'), status = document.getElementById('view_status'), buffer = new SnapshotBuffer();
  let scene, images = new Map(), enabled = false, generation = 0, drawn = null, loading = null, build = null, playable = false;
  const sha = async bytes => [...new Uint8Array(await crypto.subtle.digest('SHA-256',bytes))].map(x=>x.toString(16).padStart(2,'0')).join('');
  async function checked(url, entry, max) {
    if (!entry || !Number.isInteger(entry.bytes) || entry.bytes < 0 || entry.bytes > max) throw Error('Incomplete shared view assets. Ask the host to update the preview.');
    const response = await fetch(url,{signal:AbortSignal.timeout(15000)});
    if (!response.ok) throw Error('Shared view download failed. Check your connection and tap Rejoin.');
    const bytes = await response.arrayBuffer();
    if (bytes.byteLength !== entry.bytes || await sha(bytes) !== entry.sha256) throw Error('Shared view assets changed. Reload both pages.');
    return bytes;
  }
  async function prepare(config) {
    if (build === config.manifestSha256 && scene) return;
    if (loading) return loading;
    loading = (async () => {
      status.textContent = 'Loading the shared display scene…';
      const response = await fetch(config.manifestUrl,{signal:AbortSignal.timeout(15000)});
      if (!response.ok) throw Error('Shared view manifest unavailable. Tap Rejoin to retry.');
      const bytes = await response.arrayBuffer();
      if (bytes.byteLength > 4*1024*1024 || await sha(bytes) !== config.manifestSha256) throw Error('Host and guest content differ. Request a new room link.');
      const manifest = JSON.parse(new TextDecoder().decode(bytes));
      const parsed = JSON.parse(new TextDecoder().decode(await checked('coop-scene.json',manifest.frontend['coop-scene.json'],2*1024*1024)));
      if (parsed.schema !== 1 || parsed.id !== 'coop-view-v1') throw Error('Unsupported shared view scene.');
      const urls = [...new Set([parsed.tileImage,...Object.values(parsed.actions).flatMap(action=>action.frames)])];
      const loaded = new Map(); let index = 0;
      await Promise.all(Array.from({length:4},async () => {
        while (index < urls.length) {
          const url = urls[index++];
          if (!/^coop-art\/[a-f0-9]{64}\.png$/.test(url)) throw Error('Invalid shared view asset path.');
          const bytes = await checked(url,manifest.frontend[url],2*1024*1024);
          const blobUrl = URL.createObjectURL(new Blob([bytes],{type:'image/png'}));
          try {
            const image = new Image(); image.src = blobUrl; await image.decode(); loaded.set(url,image);
          } finally {URL.revokeObjectURL(blobUrl);}
        }
      }));
      scene = parsed; images = loaded; build = config.manifestSha256;
      status.textContent = 'Shared display ready. Waiting for the host.';
    })();
    try {await loading;} finally {loading = null;}
  }
  function freeze(text) {
    if (playable) {playable = false; root.SupertuxView.onFreeze?.();}
    canvas.style.opacity = '0.55';
    status.textContent = text;
  }
  function paint(frame) {
    const [cameraX,cameraY,scale,width,height] = frame.camera;
    if (canvas.width !== width || canvas.height !== height) {canvas.width = width; canvas.height = height;}
    context.setTransform(1,0,0,1,0,0); context.globalAlpha = 1;
    context.fillStyle = 'rgb(' + scene.sky.map(x=>Math.round(255*x)).join(',') + ')'; context.fillRect(0,0,width,height);
    context.setTransform(scale,0,0,scale,-cameraX*scale,-cameraY*scale);
    const tile = images.get(scene.tileImage);
    for (let row=Math.max(0,Math.floor(cameraY/32)); row<Math.min(scene.height,Math.ceil((cameraY+height/scale)/32)); ++row) {
      for (let col=Math.max(0,Math.floor(cameraX/32)); col<Math.min(scene.width,Math.ceil((cameraX+width/scale)/32)); ++col) {
        const region = scene.tileRegions[scene.tiles[row*scene.width+col]];
        if (region) context.drawImage(tile,...region,col*32,row*32,32,32);
      }
    }
    for (const player of frame.players) {
      const action = scene.actions[player.action], image = action && images.get(action.frames[player.frame]);
      if (!action || !image) {freeze('This player pose is outside the shared display proof.'); return;}
      const x = player.x-action.offset[0], y = player.y-action.offset[1];
      if (player.visible) {
        context.save(); context.globalAlpha = player.alpha;
        context.translate(x+image.width/2,y+image.height/2); context.rotate(player.angle*Math.PI/180);
        context.scale(action.flipX ? -1:1,action.flipY ? -1:1);
        context.drawImage(image,-image.width/2,-image.height/2); context.restore();
      }
      context.save(); context.globalAlpha = 1; context.fillStyle = player.id === 1 ? '#fff' : '#ffe27a';
      context.font = 'bold 18px system-ui'; context.textAlign = 'center';
      context.fillText(player.id + (player.dead ? ' · down' : ''),player.x+16,player.y-8); context.restore();
    }
    drawn = frame; playable = true; canvas.style.opacity = '1';
    const guest = frame.players.find(player=>player.id===2);
    status.textContent = guest.dead === 2 ? 'Player 2 is down · Tap Action to rejoin Player 1' :
      guest.dead === 1 ? 'Player 2 is falling · Wait for Action respawn' : 'Shared display proof · You control Player 2';
  }
  function tick() {
    if (enabled && !document.hidden && scene) {
      const frame = buffer.sample(performance.now());
      if (!frame) freeze('Waiting for the host’s next update. Controls are released.');
      else if (frame.scene !== scene.id) freeze('This level has no shared view yet. Ask the host to start the display proof.');
      else paint(frame);
    }
    requestAnimationFrame(tick);
  }
  root.SupertuxView = {
    prepare,
    setEnabled(value, currentGeneration) {if (enabled === value && generation === currentGeneration) return; enabled = value; generation = currentGeneration; buffer.clear(); if (!enabled) freeze('Host is paused or in menus.');},
    accept(frame) {if (enabled && frame && frame.generation === generation) return buffer.accept(frame,performance.now()); return false;},
    reset() {enabled = false; generation = 0; buffer.reset(); drawn = null; freeze('Waiting for the host.'); context.clearRect(0,0,canvas.width,canvas.height);},
    get playable() {return playable && enabled && !document.hidden;},
    get state() {return {playable:this.playable,enabled,generation,buffered:buffer.frames.length,drawn,latest:buffer.order,loaded:images.size};},
  };
  requestAnimationFrame(tick);
})(typeof window === 'undefined' ? globalThis : window);
