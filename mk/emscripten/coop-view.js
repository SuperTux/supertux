/* Host-authoritative shared view: presentation only. No WASM, simulation, scripting or save storage. */
(function (root) {
  'use strict';
  const distance = (x,y) => Math.hypot(x[0]-y[0],x[1]-y[1]);
  function continuous(a,b) {
    return a.generation===b.generation && a.camera[3]===b.camera[3] && a.camera[4]===b.camera[4] &&
      distance(a.camera,b.camera)<=128 && b.camera[2]/a.camera[2]>=.8 && b.camera[2]/a.camera[2]<=1.25 &&
      b.time-a.time<=500 && a.world?.phase===b.world?.phase && a.players.length===b.players.length &&
      a.players.every(p=>b.players.some(q=>p.id===q.id && p.dead===q.dead && p.visible===q.visible &&
        p.action?.split('-')[0]===q.action?.split('-')[0] && distance([p.x,p.y],[q.x,q.y])<=128));
  }
  // Match individual quads, not command indices: tile batches/culling reorder
  // requests, and a UID can draw several sprites. Native transform metadata
  // recovers local anchors; screen destinations are interpolated exactly once.
  function geometry(frame) {
    const entities=new Map(frame.world.entities.map(e=>[e[0],e])), entries=[];
    frame.world.draw.forEach((c,command)=>{
      if (![0,4].includes(c[0]) || !c[1] || !Array.isArray(c[8]) || c[8].length!==3 ||
          !c[8].every(Number.isFinite) || c[8][2]<=0) return;
      const entity=entities.get(c[1]), [tx,ty,scale]=c[8], p=c[7];
      const quads=c[0]===0 ? p[3] : [[0,0,0,0,p[1],p[2],p[3],p[3]]];
      quads.forEach((q,quad)=>{
        const x=q[4]/scale+tx-c[6][0]-(entity?.[2] || 0), y=q[5]/scale+ty-c[6][1]-(entity?.[3] || 0);
        const w=q[6]/scale,h=q[7]/scale;
        // Sprite animation/art remains discrete. Moving owners can keep their
        // anchor across animation frames; static tiles must keep exact artwork.
        const art=c[0]===4 ? [p[0],p[4]] : entity ? q.slice(2,4) : [p[0],p[1],p[2],...q.slice(0,4)];
        const key=JSON.stringify([c[0],c[1],c[2],c[3],c[5],c[6],entity?.[1],art,Math.round(w),Math.round(h)]);
        entries.push({command,quad,key,x,y,w,h,c,q,entity,radius:Math.min(32,Math.max(.1,Math.min(w,h)/4))});
      });
    });
    return entries;
  }
  function indexGeometry(entries) {
    const index=new Map();
    for (const e of entries) {
      const key=e.key+'/'+Math.floor(e.x/32)+'/'+Math.floor(e.y/32);
      if (!index.has(key)) index.set(key,[]);
      index.get(key).push(e);
    }
    return index;
  }
  function uniqueMatch(e,index) {
    let match=null;
    for (let x=-1;x<=1;++x) for (let y=-1;y<=1;++y) {
      for (const candidate of index.get(e.key+'/'+(Math.floor(e.x/32)+x)+'/'+(Math.floor(e.y/32)+y)) || []) {
        if (Math.abs(e.w-candidate.w)>.1 || Math.abs(e.h-candidate.h)>.1 ||
            distance([e.x,e.y],[candidate.x,candidate.y])>Math.min(e.radius,candidate.radius)) continue;
        if (match) return null; // Ambiguous overlapping geometry stays discrete.
        match=candidate;
      }
    }
    return match;
  }
  function campaignPlan(a,b) {
    const start=geometry(a),end=geometry(b),ia=indexGeometry(start),ib=indexGeometry(end),draw=new Map(),owners=new Set();
    for (const e of start) {
      const target=uniqueMatch(e,ib);
      if (!target || uniqueMatch(target,ia)!==e || (e.entity && (!target.entity ||
          distance(e.entity.slice(2,4),target.entity.slice(2,4))>128))) continue;
      if (!draw.has(e.command)) draw.set(e.command,new Map());
      draw.get(e.command).set(e.quad,target);
      if (e.c[0]===0 && e.entity) owners.add(e.c[1]);
    }
    return {draw,owners};
  }
  function campaignSample(a,b,weight,plan) {
    if (!plan.draw.size) return a; // Older snapshots without metadata remain usable.
    const mix=(x,y)=>x+(y-x)*weight;
    const draw=a.world.draw.map((c,index)=>{
      const matched=plan.draw.get(index);
      if (!matched) return c;
      const p=c[7].slice();
      if (c[0]===0) p[3]=p[3].map((q,i)=>{
        const target=matched.get(i);
        return target ? q.map((v,j)=>j>=4 && j<=7 ? mix(v,target.q[j]) : v) : q;
      });
      else {const target=matched.get(0);for(let j=1;j<=3;++j)p[j]=mix(p[j],target.c[7][j]);}
      return [...c.slice(0,7),p,c[8]];
    });
    return {...a,camera:a.camera.map((v,i)=>i<3?mix(v,b.camera[i]):v),
      players:a.players.map(p=>{
        const q=b.players.find(q=>q.id===p.id),uid=a.world.playerUids?.[p.id-1];
        return q && uid && uid===b.world.playerUids?.[p.id-1] && plan.owners.has(uid) ?
          {...p,x:mix(p.x,q.x),y:mix(p.y,q.y)} : p;
      }),world:{...a.world,draw},
      presentation:{from:a.sequence,to:b.sequence,weight,matchedCommands:plan.draw.size}};
  }
  class SnapshotBuffer {
    constructor(delay = 90) {this.delay = delay; this.reset();}
    reset() {this.clear(); this.order = null;}
    clear() {this.frames = []; this.lastArrival = 0; this.lastTarget=null; this.pair=null;}
    accept(frame, arrival) {
      const previous = this.order;
      if (!frame || !['session','epoch','sequence'].every(key => Number.isInteger(frame[key]) && frame[key] > 0) ||
          !Number.isFinite(frame.time) || !Number.isFinite(arrival) ||
          !Array.isArray(frame.camera) || frame.camera.length !== 5 || !frame.camera.every(Number.isFinite) ||
          !Array.isArray(frame.players) || frame.players.length > 2 || !frame.players.every(p => Number.isFinite(p.x) && Number.isFinite(p.y))) return false;
      if (previous && (frame.session < previous.session || (frame.session === previous.session &&
          (frame.epoch < previous.epoch || (frame.epoch === previous.epoch && (frame.sequence <= previous.sequence || frame.time < previous.time)))))) return false;
      if (!previous || frame.session !== previous.session || frame.epoch !== previous.epoch || frame.scene !== previous.scene ||
          arrival-this.lastArrival>1200 || !continuous(previous,frame)) this.clear();
      this.order = frame; this.lastArrival = arrival;
      this.frames.push({frame, arrival});
      if (this.frames.length > 8) this.frames.shift();
      return true;
    }
    sample(now) {
      if (!this.frames.length || now - this.lastArrival > 1200) return null;
      // Render behind the latest host time; never extrapolate or simulate.
      const latest = this.frames.at(-1);
      const target = Math.min(latest.frame.time,Math.max(this.lastTarget ?? -Infinity,
        latest.frame.time + Math.min(Math.max(0,now - latest.arrival), this.delay) - this.delay));
      this.lastTarget=target; // Jitter/new arrivals never move the picture backwards.
      let a = this.frames[0].frame, b = a;
      for (const item of this.frames) {
        b = item.frame;
        if (b.time >= target) break;
        a = b;
      }
      const weight = b.time > a.time ? Math.max(0, Math.min(1, (target - a.time) / (b.time - a.time))) : 0;
      const mix = (x,y) => x + (y-x)*weight;
      const discrete = weight >= 1 ? b : a;
      if (discrete.world) {
        if (a===b || weight<=0 || weight>=1 || !a.world || !b.world) return discrete;
        if (this.pair?.a!==a || this.pair?.b!==b) this.pair={a,b,plan:campaignPlan(a,b)};
        if (this.pair.weight!==weight) {
          this.pair.weight=weight;this.pair.sample=campaignSample(a,b,weight,this.pair.plan);
        }
        return this.pair.sample;
      }
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
  let outcome = null, readyKey = null, tintCache = new Map(), scene, images = new Map(), enabled = false, generation = 0, drawn = null, loading = null, build = null, playable = false;
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
      const urls = [...new Set([parsed.tileImage,...Object.values(parsed.actions).flatMap(action=>action.frames),...Object.values(parsed.campaign.textures)])];
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
  const css = c => `rgba(${c[0]*255},${c[1]*255},${c[2]*255},${c[3]})`;
  function tinted(image, color, key) {
    if (color.slice(0,3).every(x=>x===1)) return image;
    const identity=key+'/'+color.slice(0,3).join('/');
    if (tintCache.has(identity)) return tintCache.get(identity);
    const result=document.createElement('canvas'); result.width=image.width; result.height=image.height;
    const ctx=result.getContext('2d');ctx.drawImage(image,0,0);
    ctx.globalCompositeOperation='multiply';ctx.fillStyle=css([...color.slice(0,3),1]);ctx.fillRect(0,0,result.width,result.height);
    ctx.globalCompositeOperation='destination-in';ctx.drawImage(image,0,0);
    tintCache.set(identity,result);
    if (tintCache.size>16) tintCache.delete(tintCache.keys().next().value);
    return result;
  }
  function paintWorld(frame) {
    const [, , , width,height]=frame.camera;
    if (canvas.width!==width || canvas.height!==height) {canvas.width=width;canvas.height=height;}
    context.setTransform(1,0,0,1,0,0); context.globalAlpha=1; context.globalCompositeOperation='source-over';
    context.clearRect(0,0,width,height);
    for (const [kind,owner,layer,flip,alpha,blend,clip,p] of frame.world.draw) {
      context.save(); context.beginPath();context.rect(...clip);context.clip();
      // SDL texture/text requests apply transform alpha separately. Filled
      // rectangles/lines already bake it into their color; gradients use the
      // supplied colors directly. Applying it twice changes native fades.
      context.globalAlpha=kind===0 || kind===4 ? alpha : 1;
      context.globalCompositeOperation=['source-over','lighter','multiply','source-over'][blend];
      if (kind===0) {
        const url=scene.campaign.textures[p[0].replace(/^\//,'')], raw=images.get(url);
        if (!raw) {console.warn('Shared view artwork missing:',p[0]);context.restore();freeze('This scene needs artwork outside the supported level. Return to the title screen.');return false;}
        const image=tinted(raw,p[4],url);context.globalAlpha*=p[4][3];
        for (const [sx,sy,sw,sh,x,y,w,h,angle] of p[3]) {
          if (!sw || !sh || !w || !h) continue;
          context.save();context.translate(x+w/2,y+h/2);context.rotate(angle*Math.PI/180);
          context.scale(flip&4?-1:1,flip&2?-1:1);
          context.drawImage(image,p[1]+sx,p[2]+sy,sw,sh,-w/2,-h/2,w,h);context.restore();
        }
      } else if (kind===1) {
        const [x,y,w,h,top,bottom,direction]=p;
        const gradient=context.createLinearGradient(x,y,direction%2?x+w:x,direction%2?y:y+h);
        gradient.addColorStop(0,css(top));gradient.addColorStop(1,css(bottom));context.fillStyle=gradient;context.fillRect(x,y,w,h);
      } else if (kind===2) {
        context.fillStyle=css(p[4]);context.beginPath();context.roundRect(...p.slice(0,4),Math.min(p[5],Math.max(0,Math.min(p[2],p[3])/2)));context.fill();
      } else if (kind===3) {
        context.strokeStyle=css(p[4]);context.beginPath();context.moveTo(p[0],p[1]);context.lineTo(p[2],p[3]);context.stroke();
      } else if (kind===4) {
        context.fillStyle=css(p[5]);context.font=`${p[3]}px system-ui`;context.textAlign=['left','center','right'][p[4]];
        context.textBaseline='top';p[0].split('\n').forEach((line,i)=>context.fillText(line,p[1],p[2]+i*p[3]));
      }
      context.restore();
    }
    context.globalAlpha=1;context.globalCompositeOperation='source-over';
    drawn=frame;canvas.style.opacity='1';
    return true;
  }
  function paint(frame) {
    if (frame.scene==='antarctica-v1') {
      if (drawn===frame && playable===enabled) return;
      if (!paintWorld(frame)) return;
      playable=enabled;
      const p2=frame.players.find(p=>p.id===2);
      status.textContent=frame.world.phase==='finishing' ? 'Level ending · Following the host' : p2.dead===2 ? 'Player 2 is down · Tap Action to rejoin Player 1' : 'Welcome to Antarctica · You control Player 2';
      return;
    }
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
    if (!outcome && !document.hidden && scene) {
      const frame = buffer.sample(performance.now());
      if (!frame) freeze('Waiting for the host’s next update. Controls are released.');
      else if (frame.scene !== scene.id && frame.scene !== scene.campaign.id) freeze('This level has no shared view yet. Ask the host to start Welcome to Antarctica.');
      else if (enabled || frame.world?.phase==='finishing') paint(frame);
    }
    requestAnimationFrame(tick);
  }
  root.SupertuxView = {
    prepare,
    setEnabled(value, currentGeneration) {if (enabled === value && generation === currentGeneration) return; enabled = value; generation = currentGeneration; buffer.clear(); if (!enabled && !outcome) freeze('Host is paused or in menus.');},
    accept(frame) {
      if (!frame || frame.generation!==generation) return false;
      if (frame.scene==='antarctica-v1' && scene && frame.world) {
        // Preparation is exact-build checked before connecting. A complete
        // baseline acknowledgment is allowed during the engine loading gate.
        if (!buffer.accept(frame,performance.now())) return false;
        const key=`${frame.session}/${frame.epoch}`;
        if (readyKey!==key) {
          if (!paintWorld(frame)) return false;
          readyKey=key;outcome=null;root.SupertuxView.onReady?.(frame);
        }
        return true;
      }
      if (enabled) return buffer.accept(frame,performance.now());
      return false;
    },
    result(value) {
      const latest=buffer.order;
      if (!latest || value.session!==latest.session || value.epoch!==latest.epoch) return;
      outcome=value;enabled=false;freeze(value.win?'Level complete · The host chooses what to play next':'Level ended · Waiting for the host');
    },
    reset() {outcome=null;readyKey=null;tintCache.clear();enabled = false; generation = 0; buffer.reset(); drawn = null; freeze('Waiting for the host.'); context.clearRect(0,0,canvas.width,canvas.height);},
    get playable() {return playable && enabled && !document.hidden;},
    get state() {return {playable:this.playable,enabled,generation,buffered:buffer.frames.length,drawn,latest:buffer.order,loaded:images.size,outcome};},
  };
  requestAnimationFrame(tick);
})(typeof window === 'undefined' ? globalThis : window);
