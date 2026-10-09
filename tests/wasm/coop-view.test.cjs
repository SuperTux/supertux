const {test} = require('node:test');
const assert = require('node:assert/strict');
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
  assert.equal(buffer.sample(1100).players[0].x,0);assert.equal(buffer.sample(1150).players[0].dead,1);
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
