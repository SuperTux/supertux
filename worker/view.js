// A presentation snapshot, never a guest simulation command or arbitrary asset.
const uint = x => Number.isInteger(x) && x > 0 && x < 0x80000000;
const uid = x => Number.isInteger(x) && x > 0 && x <= 0xffffffff;
const finite = (x, low, high) => typeof x === 'number' && Number.isFinite(x) && x >= low && x <= high;
const exact = (x, keys) => x && typeof x === 'object' && !Array.isArray(x) && Object.keys(x).length === keys.length && keys.every(key => Object.hasOwn(x, key));
export function validView(value) {
  if (!exact(value, ['type','session','epoch','sequence','generation','time','scene','camera','players',...(value?.scene === 'antarctica-v1' ? ['world'] : [])]) || value.type !== 'view' ||
      !uint(value.session) || !uint(value.epoch) || !uint(value.sequence) || !uint(value.generation) || !finite(value.time, 0, 1e12) ||
      !['coop-view-v1','antarctica-v1','unsupported'].includes(value.scene) || !Array.isArray(value.camera) || value.camera.length !== 5 ||
      !value.camera.slice(0,2).every(x => finite(x,-100000,100000)) || !finite(value.camera[2],0.05,10) ||
      !value.camera.slice(3).every(x => Number.isInteger(x) && x >= 100 && x <= 8192) ||
      !Array.isArray(value.players) || value.players.length > 2) return false;
  if (value.scene === 'unsupported') return value.players.length === 0;
  if (value.players.length !== 2) return false;
  const ids = new Set();
  for (const player of value.players) {
    if (!exact(player,['id','x','y','action','frame','angle','alpha','dead','visible']) || ![1,2].includes(player.id) || ids.has(player.id) ||
        !finite(player.x,-100000,100000) || !finite(player.y,-100000,100000) ||
        typeof player.action !== 'string' || !(value.scene === 'antarctica-v1' ? /^[a-z-]{1,64}$/ : /^(?:small-[a-z-]{1,48}|gameover)$/).test(player.action) ||
        !Number.isInteger(player.frame) || player.frame < 0 || player.frame > 255 ||
        !finite(player.angle,-3600,3600) || !finite(player.alpha,0,1) || ![0,1,2].includes(player.dead) || typeof player.visible !== 'boolean') return false;
    ids.add(player.id);
  }
  return value.scene !== 'antarctica-v1' || validWorld(value.world);
}

const numbers = (a,n,lo=-100000,hi=100000) => Array.isArray(a) && a.length===n && a.every(x=>finite(x,lo,hi));
const rgba = a => numbers(a,4,0,1);
function validWorld(world) {
  if (!exact(world,['draw','entities','coins','checkpoint','phase']) ||
      !Number.isInteger(world.coins) || world.coins<0 || world.coins>1000000 ||
      !(world.checkpoint===null || numbers(world.checkpoint,2)) ||
      !['playing','finishing'].includes(world.phase) || !Array.isArray(world.entities) || world.entities.length>512 ||
      !Array.isArray(world.draw) || world.draw.length>256) return false;
  const ids=new Set();
  for (const e of world.entities) {
    if (!Array.isArray(e) || e.length!==6 || !uid(e[0]) || ids.has(e[0]) ||
        typeof e[1]!=='string' || !/^[a-z_-]{1,64}$/.test(e[1]) || !finite(e[2],-100000,100000) || !finite(e[3],-100000,100000) ||
        typeof e[4]!=='string' || !/^[a-z0-9-]{1,64}$/.test(e[4]) || !Number.isInteger(e[5]) || e[5]<0 || e[5]>255) return false;
    ids.add(e[0]);
  }
  let quads=0;
  for (const c of world.draw) {
    if (!Array.isArray(c) || c.length!==8 || ![0,1,2,3,4].includes(c[0]) || !Number.isInteger(c[1]) || c[1]<0 || c[1]>0xffffffff ||
        !Number.isInteger(c[2]) || Math.abs(c[2])>100000 || ![0,2,4,6].includes(c[3]) || !finite(c[4],0,1) ||
        ![0,1,2,3].includes(c[5]) || !numbers(c[6],4) || c[6][2]<0 || c[6][3]<0 || !Array.isArray(c[7])) return false;
    const p=c[7];
    if (c[0]===0) {
      if (p.length!==5 || typeof p[0]!=='string' || p[0].length>180 || !/^\/?images\/[a-zA-Z0-9_./ -]+\.png$/.test(p[0]) || p[0].includes('..') ||
          !Number.isInteger(p[1]) || !Number.isInteger(p[2]) || p[1]<0 || p[2]<0 || p[1]>16384 || p[2]>16384 ||
          !Array.isArray(p[3]) || !rgba(p[4])) return false;
      for (const r of p[3]) {
        if (++quads>2048 || !numbers(r,9) || r.slice(0,4).some(x=>x<0) || r[6]<0 || r[7]<0 || Math.abs(r[8])>3600) return false;
      }
    } else if (c[0]===1) {
      if (p.length!==7 || !numbers(p.slice(0,4),4) || !rgba(p[4]) || !rgba(p[5]) || ![0,1,2,3].includes(p[6])) return false;
    } else if (c[0]===2) {
      if (p.length!==6 || !numbers(p.slice(0,4),4) || !rgba(p[4]) || !finite(p[5],0,10000)) return false;
    } else if (c[0]===3) {
      if (p.length!==5 || !numbers(p.slice(0,4),4) || !rgba(p[4])) return false;
    } else if (p.length!==6 || typeof p[0]!=='string' || p[0].length>2048 || !numbers(p.slice(1,4),3) ||
               p[3]<1 || p[3]>512 || ![0,1,2].includes(p[4]) || !rgba(p[5])) return false;
  }
  return true;
}
