const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const {webcrypto, createHash} = require('node:crypto');
const {SnapshotBuffer} = require('../../mk/emscripten/coop-view.js');
function frame(sequence,time,x,extra={}) {
  return {type:'view',session:1,epoch:1,sequence,time,scene:'coop-view-v1',camera:[x,0,1,844,390],
    players:[{id:1,x,y:20,dead:0,action:'small-stand-right',frame:0},{id:2,x:x+100,y:20,dead:0,action:'small-walk-right',frame:1}],...extra};
}
test('interpolates player and shared camera positions, with no prediction',()=>{
  const buffer = new SnapshotBuffer(50);
  assert.equal(buffer.accept(frame(1,100,0),1000),true);
  assert.equal(buffer.accept(frame(2,200,100),1100),true);
  const sample = buffer.sample(1100);
  assert.equal(sample.players[0].x,50); assert.equal(sample.players[1].x,150); assert.equal(sample.camera[0],50);
  assert.equal(buffer.sample(1300).players[0].x,100);
  assert.equal(buffer.sample(2301),null);
});
test('ordering and generation epochs never blend across restarted levels or sessions',()=>{
  const buffer = new SnapshotBuffer();
  buffer.accept(frame(9,500,500),1000);
  for (const stale of [frame(9,500,0),frame(8,600,0),frame(10,400,0),frame(10,600,0,{session:0})]) assert.equal(buffer.accept(stale,1100),false);
  assert.equal(buffer.accept(frame(1,600,0,{epoch:2}),1100),true);
  assert.equal(buffer.frames.length,1);assert.equal(buffer.sample(1100).players[0].x,0);
  assert.equal(buffer.accept(frame(11,700,900,{epoch:1}),1200),false);
  assert.equal(buffer.accept(frame(1,700,20,{session:2,epoch:1}),1200),true);
  assert.equal(buffer.accept(frame(2,800,900,{session:1,epoch:3}),1300),false);
});
test('death is discrete, incomplete state never invents an entity, history is bounded',()=>{
  const buffer = new SnapshotBuffer(50);
  buffer.accept(frame(1,100,0),1000);
  const dead=frame(2,200,100); dead.players[0].dead=1;
  buffer.accept(dead,1100);
  assert.equal(buffer.sample(1100).players[0].x,100);assert.equal(buffer.sample(1100).players[0].dead,1);
  for(let sequence=3;sequence<=100;sequence++) buffer.accept(frame(sequence,sequence*100,sequence),sequence*100);
  assert.equal(buffer.frames.length,8);
  assert.equal(buffer.accept(frame(101,10100,NaN),10100),false);
  assert.equal(buffer.accept(frame(101,10100,0,{camera:[1,2,3]}),10100),false);
  buffer.clear();assert.equal(buffer.sample(10000),null);
  assert.equal(buffer.accept(frame(1,100,0),10000),false); // clear retains ordering watermark
  buffer.reset();assert.equal(buffer.accept(frame(1,100,0),10000),true);
});

test('campaign removal and tile changes are complete discrete baselines; no mixed geometry or resurrected old-epoch objects',()=>{
  const buffer=new SnapshotBuffer(50);
  const before=frame(1,100,0,{scene:'antarctica-v1',world:{draw:[[0,17]],entities:[[17,'coin']],coins:0}});
  const after=frame(2,200,100,{scene:'antarctica-v1',world:{draw:[],entities:[],coins:1}});
  assert.equal(buffer.accept(before,1000),true);assert.equal(buffer.accept(after,1100),true);
  assert.equal(buffer.sample(1100).world.coins,0);
  assert.equal(buffer.sample(1150).world.coins,1);assert.equal(buffer.sample(1150).world.entities.length,0);
  assert.deepEqual(buffer.sample(1150).camera,after.camera);
  const restart=frame(1,300,0,{scene:'antarctica-v1',epoch:2,world:before.world});
  buffer.accept(restart,1200);assert.equal(buffer.sample(1200).world.entities[0][0],17);
  assert.equal(buffer.accept(after,1300),false);
});

function texture(owner,x,y,{camera=0,scale=1,art='images/tile.png',source=0,layer=0}={}) {
  return [0,owner,layer,0,1,0,[0,0,844,390],[art,0,0,[[source,0,32,32,(x-camera)*scale,y*scale,32*scale,32*scale,0]],[1,1,1,1]],[camera,0,scale]];
}
function campaignFrame(sequence,time,x,draw,entities,extra={}) {
  return frame(sequence,time,x,{scene:'antarctica-v1',world:{draw,entities,playerUids:[1,2],phase:'playing',coins:0,checkpoint:null},...extra});
}
test('native screen geometry and moving owners interpolate once; animation and authoritative metadata stay discrete',()=>{
  const buffer=new SnapshotBuffer(50);
  const a=campaignFrame(1,100,0,[texture(2,100,20)],[[2,'moving-sprite',100,20,'small-walk-right',0]]);
  const b=campaignFrame(2,200,20,[texture(2,140,20,{camera:20,art:'images/next-frame.png'})],[[2,'moving-sprite',140,20,'small-walk-right',1]]);
  b.players[1].x=140;
  b.world.draw[0][3]=4;
  const originals=structuredClone([a,b]);buffer.accept(a,1000);buffer.accept(b,1100);
  const sample=buffer.sample(1100);
  assert.equal(sample.players[1].x,120);assert.equal(sample.camera[0],10);
  assert.equal(sample.world.draw[0][7][3][0][4],110); // Screen-space 100 → 120, without a second camera transform.
  assert.equal(sample.world.draw[0][7][0],'images/tile.png');assert.equal(sample.world.entities[0][2],100);
  assert.equal(sample.world.draw[0][3],0); // Facing switches at the discrete native pose, movement still smooths.
  assert.equal(sample.presentation.matchedCommands,1);
  assert.strictEqual(buffer.sample(1100),sample);assert.deepEqual([a,b],originals);
  assert.strictEqual(buffer.sample(1150),b);assert.strictEqual(buffer.sample(1600),b);
});
test('tile batch reordering/culling and parallax match individual geometry; additions/removals never blend',()=>{
  const buffer=new SnapshotBuffer(50);
  const a=campaignFrame(1,100,0,[texture(10,64,32),texture(10,96,32),texture(20,1000,0,{layer:-100})],[]);
  const b=campaignFrame(2,200,16,[texture(10,96,32,{camera:16}),texture(10,128,32,{camera:16}),texture(20,1008,0,{camera:16,layer:-100})],[]);
  b.world.coins=1;buffer.accept(a,1000);buffer.accept(b,1100);
  const sample=buffer.sample(1100);
  assert.equal(sample.world.draw.length,3);assert.equal(sample.world.coins,0);
  assert.equal(sample.world.draw[0][7][3][0][4],64); // Removed quad holds, never slides to its neighbour.
  assert.equal(sample.world.draw[1][7][3][0][4],88); // Stable tile matches despite command reordering.
  assert.equal(sample.world.draw[2][7][3][0][4],996); // Actual native parallax endpoints, rather than camera speed guessed twice.
  assert.strictEqual(buffer.sample(1150),b);assert.equal(buffer.sample(1150).world.coins,1);
  assert.equal(buffer.sample(1150).world.draw.length,3);
});
test('ambiguous overlapping quads and changed tile artwork stay discrete, legacy snapshots still work',()=>{
  const buffer=new SnapshotBuffer(50);
  const a=campaignFrame(1,100,0,[texture(10,64,32),texture(10,64,32),texture(11,96,32)],[]);
  const b=campaignFrame(2,200,16,[texture(10,64,32,{camera:16}),texture(11,96,32,{camera:16,source:32})],[]);
  buffer.accept(a,1000);buffer.accept(b,1100);assert.strictEqual(buffer.sample(1100),a);
  a.world.draw.forEach(c=>c.pop());b.world.draw.forEach(c=>c.pop());
  buffer.reset();buffer.accept(a,1000);buffer.accept(b,1100);assert.strictEqual(buffer.sample(1100),a);
});
test('teleports, death/respawn, growth, viewport, phase, generation and long gaps snap complete baselines',()=>{
  for(const mutate of [f=>f.players[1].x+=500,f=>f.players[1].dead=2,f=>f.players[1].visible=false,
    f=>f.players[1].action='big-walk-right',f=>f.camera[3]=900,f=>f.camera[2]=2,
    f=>f.world.phase='finishing',f=>f.generation=2,f=>f.time=800]) {
    const buffer=new SnapshotBuffer(50),a=campaignFrame(1,100,0,[texture(2,100,20)],[[2,'moving-sprite',100,20,'small-walk-right',0]]);
    const b=structuredClone(a);b.sequence=2;b.time=200;mutate(b);
    buffer.accept(a,1000);buffer.accept(b,1100);assert.equal(buffer.frames.length,1);assert.strictEqual(buffer.sample(1100),b);
  }
});
test('jitter cannot reverse sampled host time; clearing/resetting releases matching plans and timing state',()=>{
  const buffer=new SnapshotBuffer(90);buffer.accept(frame(1,100,0),1000);
  assert.equal(buffer.sample(1090).players[0].x,0);
  buffer.accept(frame(2,140,40),1091);
  assert.equal(buffer.sample(1091).players[0].x,0); // New arrival would otherwise target time 50.
  assert.equal(buffer.sample(1131).players[0].x,0);assert.equal(buffer.sample(1181).players[0].x,40);
  buffer.clear();assert.equal(buffer.lastTarget,null);assert.equal(buffer.pair,null);
  buffer.reset();buffer.accept(frame(1,1,5),2000);assert.equal(buffer.sample(2000).players[0].x,5);
});

test('guest renders native faded colors once while texture and text keep their separate transform opacity',async()=>{
  const calls=[];
  const context={setTransform(){},clearRect(){},save(){},restore(){},beginPath(){},rect(){},clip(){},
    translate(){},rotate(){},scale(){},roundRect(){},moveTo(){},lineTo(){},
    createLinearGradient(){return {addColorStop(){}};},
    drawImage(){calls.push(['texture',this.globalAlpha]);},
    fill(){calls.push(['panel',this.globalAlpha,this.fillStyle]);},
    stroke(){calls.push(['line',this.globalAlpha,this.strokeStyle]);},
    fillRect(){calls.push(['gradient',this.globalAlpha]);},
    fillText(){calls.push(['text',this.globalAlpha,this.fillStyle]);}};
  const canvas={width:844,height:390,style:{},getContext:()=>context};
  const bytes=Buffer.from('checked image'),url='coop-art/'+createHash('sha256').update(bytes).digest('hex')+'.png';
  const scene=Buffer.from(JSON.stringify({schema:1,id:'coop-view-v1',tileImage:url,actions:{},campaign:{textures:{'images/test.png':url}}}));
  const entry=data=>({bytes:data.length,sha256:createHash('sha256').update(data).digest('hex')});
  const manifest=Buffer.from(JSON.stringify({frontend:{'coop-scene.json':entry(scene),[url]:entry(bytes)}}));
  const files=new Map([['manifest.json',manifest],['coop-scene.json',scene],[url,bytes]]);
  const browser={document:{getElementById:id=>id==='guest_canvas'?canvas:{textContent:''}},
    crypto:webcrypto,TextEncoder,TextDecoder,Blob,URL,AbortSignal,performance,console,
    Image:class {constructor(){this.width=this.height=32;}async decode(){}},requestAnimationFrame(){},
    fetch:async name=>({ok:true,arrayBuffer:async()=>Uint8Array.from(files.get(name)).buffer})};
  vm.runInNewContext(fs.readFileSync(require.resolve('../../mk/emscripten/coop-view.js'),'utf8'),browser);
  await browser.SupertuxView.prepare({manifestUrl:'manifest.json',manifestSha256:entry(manifest).sha256});
  const clip=[0,0,844,390],white=[1,1,1,1];
  const commands=[
    [0,1,1,0,.5,0,clip,['images/test.png',0,0,[[0,0,32,32,0,0,32,32,0]],[1,1,1,.4]]],
    [2,1,2,0,.5,0,clip,[0,0,32,32,[1,1,1,.2],4]],
    [3,1,3,0,.5,0,clip,[0,0,32,32,[1,1,1,.25]]],
    [1,1,4,0,.5,0,clip,[0,0,32,32,white,white,0]],
    [4,1,5,0,.5,0,clip,['Text',0,0,16,0,[1,1,1,.4]]],
  ];
  assert.equal(browser.SupertuxView.accept(frame(1,100,0,{generation:0,scene:'antarctica-v1',world:{draw:commands}})),true);
  assert.deepEqual(calls,[['texture',.2],['panel',1,'rgba(255,255,255,0.2)'],
    ['line',1,'rgba(255,255,255,0.25)'],['gradient',1],['text',.5,'rgba(255,255,255,0.4)']]);
});
