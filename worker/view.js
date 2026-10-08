// A presentation snapshot, never a guest simulation command or arbitrary asset.
const uint = x => Number.isInteger(x) && x > 0 && x < 0x80000000;
const finite = (x, low, high) => typeof x === 'number' && Number.isFinite(x) && x >= low && x <= high;
const exact = (x, keys) => x && typeof x === 'object' && !Array.isArray(x) && Object.keys(x).length === keys.length && keys.every(key => Object.hasOwn(x, key));
export function validView(value) {
  if (!exact(value, ['type','session','epoch','sequence','generation','time','scene','camera','players']) || value.type !== 'view' ||
      !uint(value.session) || !uint(value.epoch) || !uint(value.sequence) || !uint(value.generation) || !finite(value.time, 0, 1e12) ||
      !['coop-view-v1','unsupported'].includes(value.scene) || !Array.isArray(value.camera) || value.camera.length !== 5 ||
      !value.camera.slice(0,2).every(x => finite(x,-100000,100000)) || !finite(value.camera[2],0.05,10) ||
      !value.camera.slice(3).every(x => Number.isInteger(x) && x >= 100 && x <= 8192) ||
      !Array.isArray(value.players) || value.players.length > 2) return false;
  if (value.scene === 'unsupported') return value.players.length === 0;
  if (value.players.length !== 2) return false;
  const ids = new Set();
  for (const player of value.players) {
    if (!exact(player,['id','x','y','action','frame','angle','alpha','dead','visible']) || ![1,2].includes(player.id) || ids.has(player.id) ||
        !finite(player.x,-100000,100000) || !finite(player.y,-100000,100000) ||
        typeof player.action !== 'string' || !/^(?:small-[a-z-]{1,48}|gameover)$/.test(player.action) ||
        !Number.isInteger(player.frame) || player.frame < 0 || player.frame > 255 ||
        !finite(player.angle,-3600,3600) || !finite(player.alpha,0,1) || ![0,1,2].includes(player.dead) || typeof player.visible !== 'boolean') return false;
    ids.add(player.id);
  }
  return true;
}
