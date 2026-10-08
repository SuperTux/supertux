/* Downloaded assets have their own database; storage.js owns player data. */
(function(root) {
  'use strict';
  const DB_NAME = 'supertux-downloaded-assets-v1';
  const LIMIT = 384 * 1024 * 1024;
  const hex = bytes => Array.from(new Uint8Array(bytes), b => b.toString(16).padStart(2, '0')).join('');
  const hash = async bytes => hex(await root.crypto.subtle.digest('SHA-256', bytes));
  const valid = async (bytes, entry) => bytes instanceof ArrayBuffer && bytes.byteLength === entry.bytes && await hash(bytes) === entry.sha256;

  class Cache {
    constructor() { this.database = null; this.opening = null; this.disabled = false; this.epoch = 0; this.pending = Promise.resolve(); }
    async db() {
      if (this.disabled) return null;
      if (this.database) return this.database;
      if (this.opening) return this.opening;
      try {
        this.opening = this.bounded((resolve, reject) => {
          const request = root.indexedDB.open(DB_NAME, 1);
          request.onupgradeneeded = () => {
            request.result.createObjectStore('payloads');
            request.result.createObjectStore('metadata', {keyPath: 'sha256'});
          };
          request.onsuccess = () => {
            const db = request.result;
            if (this.disabled) { db.close(); return; }
            db.onversionchange = () => { db.close(); this.database = null; this.opening = null; };
            resolve(db);
          };
          request.onerror = () => reject(request.error);
          request.onblocked = () => reject(new Error('Asset storage is blocked'));
        }).catch(() => { this.disabled = true; return null; });
        this.database = await this.opening;
        return this.database;
      } catch (_) { this.disabled = true; return null; }
    }
    async touch(entry, epoch) {
      const db = await this.db();
      if (!db || epoch !== this.epoch) return;
      try {
        const tx = db.transaction('metadata', 'readwrite');
        const store = tx.objectStore('metadata'), request = store.get(entry.sha256);
        request.onsuccess = () => { try { if (request.result && epoch === this.epoch) store.put({...request.result, used:Date.now()}); } catch (_) { tx.abort(); } };
        tx.onerror = tx.onabort = () => {};
      } catch (_) { /* Cache accounting is best effort. */ }
    }
    bounded(operation) {
      return new Promise((resolve, reject) => {
        const timer = setTimeout(() => reject(new Error('Asset storage timed out')), 3000);
        const finish = fn => value => { clearTimeout(timer); fn(value); };
        try { operation(finish(resolve), finish(reject)); } catch (error) { clearTimeout(timer); reject(error); }
      });
    }
    async read(entry) {
      const db = await this.db();
      if (!db) return null;
      try {
        return await this.bounded((resolve, reject) => {
          const transaction = db.transaction('payloads', 'readonly');
          const request = transaction.objectStore('payloads').get(entry.sha256);
          request.onsuccess = () => resolve(request.result || null);
          transaction.onabort = transaction.onerror = () => reject(transaction.error);
        });
      } catch (_) { return null; }
    }
    store(entry, bytes, epoch) {
      // A completed network load is usable immediately. Storage is best effort.
      this.pending = this.pending.then(async () => {
        const db = await this.db();
        if (!db || epoch !== this.epoch) return;
        let quota = null;
        if (root.navigator?.storage?.estimate) {
          try { quota = await this.bounded((resolve,reject) => root.navigator.storage.estimate().then(resolve,reject)); } catch (_) { /* Use the conservative fixed limit. */ }
        }
        try {
          await this.bounded((resolve, reject) => {
            const tx = db.transaction(['metadata', 'payloads'], 'readwrite');
            const meta = tx.objectStore('metadata'), payloads = tx.objectStore('payloads');
            const request = meta.getAll();
            request.onsuccess = () => {
              try {
              let total = entry.bytes;
              const existing = request.result.filter(e => e.sha256 !== entry.sha256);
              const accounted = request.result.reduce((n,e) => n + (Number.isSafeInteger(e.bytes) ? e.bytes : 0),0);
              const budget = quota && Number.isFinite(quota.quota)
                ? Math.max(0,Math.min(LIMIT,quota.quota - Math.max(0,(Number.isFinite(quota.usage) ? quota.usage : accounted) - accounted) - Math.max(16*1024*1024,quota.quota*.1))) : LIMIT;
              if (entry.bytes > budget) return; // Preserve useful small cached tracks when startup cannot fit.
              // Keep one startup package; unchanged music remains reusable across builds.
              const obsolete = existing.filter(e => entry.kind === 'startup' && e.kind === 'startup');
              const retained = existing.filter(e => !obsolete.includes(e)).sort((a, b) => a.used - b.used);
              total += retained.reduce((n, e) => n + e.bytes, 0);
              while (total > budget && retained.length) { const e = retained.shift(); total -= e.bytes; obsolete.push(e); }
              for (const e of obsolete) { meta.delete(e.sha256); payloads.delete(e.sha256); }
              if (entry.bytes <= budget) {
                payloads.put(bytes, entry.sha256);
                meta.put({sha256: entry.sha256, bytes: entry.bytes, kind: entry.kind, used: Date.now()});
              }
              } catch (error) { tx.abort(); reject(error); }
            };
            tx.oncomplete = resolve;
            tx.onabort = tx.onerror = () => reject(tx.error);
          });
        } catch (error) { console.info('Downloaded asset could not be cached; online play remains available.', error.name); }
      }).catch(() => {});
      return this.pending;
    }
    async clear() {
      ++this.epoch; // In-flight downloads cannot repopulate a cache being cleared.
      await this.pending;
      const db = await this.db();
      if (!db) return false;
      return this.bounded((resolve, reject) => {
        const tx = db.transaction(['metadata', 'payloads'], 'readwrite');
        tx.objectStore('metadata').clear(); tx.objectStore('payloads').clear();
        tx.oncomplete = () => resolve(true);
        tx.onerror = tx.onabort = () => reject(tx.error);
      }).catch(() => false);
    }
  }

  class Loader {
    constructor(Module, manifest, cache = new Cache()) {
      this.Module = Module; this.manifest = manifest; this.cache = cache;
      this.identity = manifest.inventorySha256;
      this.entries = new Map(manifest.assets.map(e => [e.path, e]));
      this.tracks = new Map(); this.inflight = new Map(); this.queue = []; this.active = 0; this.wanted = '';
      this.stats = {networkBytes: 0, cacheBytes: 0, failures: 0};
    }
    async obtain(entry, progress) {
      const epoch = this.cache.epoch;
      const cached = await this.cache.read(entry);
      if (cached && await valid(cached, entry)) { this.stats.cacheBytes += entry.bytes; if (this.cache.touch) this.cache.touch(entry, epoch); return cached; }
      const gzip = typeof root.DecompressionStream === 'function' && entry.encodings?.gzip;
      const url = new URL(gzip ? gzip.url : entry.url, root.location.href);
      if (url.origin !== root.location.origin) throw new Error('Asset URL must use the game origin');
      const response = await root.fetch(url.href, {cache: 'no-store'});
      if (response.status !== 200) throw new Error('Asset download failed (HTTP ' + response.status + ')');
      const bytes = new Uint8Array(entry.bytes);
      const stream = gzip ? response.body.pipeThrough(new root.DecompressionStream('gzip')) : response.body;
      const reader = stream.getReader();
      let offset = 0;
      for (;;) {
        const {done, value} = await reader.read();
        if (done) break;
        if (offset + value.length > bytes.length) { await reader.cancel(); throw new Error('Asset download has the wrong size'); }
        bytes.set(value, offset); offset += value.length;
        if (progress) progress(offset, entry.bytes);
      }
      if (!await valid(bytes.buffer, entry)) throw new Error('Asset download is incomplete or damaged');
      this.stats.networkBytes += entry.bytes;
      this.cache.store(entry, bytes.buffer, epoch);
      return bytes.buffer;
    }
    allowed() {
      const shell = this.Module.supertuxShell;
      return !!shell && shell.active && !shell.muted && !root.document.hidden;
    }
    pump() {
      if (!this.allowed()) return;
      while (this.active < 2 && this.queue.length) {
        const item = this.queue.shift(); ++this.active;
        this.obtain({...item.entry, kind: 'music'}).then(item.resolve, item.reject).finally(() => { --this.active; this.inflight.delete(item.entry.sha256); this.pump(); });
      }
    }
    download(entry) {
      if (this.inflight.has(entry.sha256)) return this.inflight.get(entry.sha256);
      const promise = new Promise((resolve, reject) => this.queue.push({entry, resolve, reject}));
      this.inflight.set(entry.sha256, promise); this.pump(); return promise;
    }
    // Called/polled on the engine's main thread. JS never holds a SoundManager pointer.
    requestTrack(path) {
      path = path.replace(/^\/+/, '');
      this.wanted = path;
      if (!this.allowed()) return 1;
      this.pump();
      const entry = this.entries.get(path);
      if (!entry || entry.package === 'startup') { this.status(''); return 0; }
      const track = this.tracks.get(path);
      if (track) { this.status(track.state === 1 ? 'Downloading music…' : track.state === 2 ? 'Music could not download. You can keep playing.' : '', track.state === 2); return track.state; }
      // A burst of track changes never queues an entire soundtrack. Active
      // downloads may finish/cache, but queued obsolete requests are dropped.
      for (const item of this.queue.splice(0)) { this.inflight.delete(item.entry.sha256); const error = new Error('Music request superseded'); error.superseded = true; item.reject(error); }
      const state = {state: 1}; this.tracks.set(path, state); this.status('Downloading music…');
      this.download(entry).then(buffer => {
        const fs = this.Module.FS;
        const target = this.manifest.runtimeRoot + '/' + path;
        fs.mkdirTree(target.slice(0, target.lastIndexOf('/')));
        fs.writeFile(target, new Uint8Array(buffer), {canOwn: true});
        state.state = 0;
        if (this.wanted === path) this.status('');
      }).catch(error => {
        if (error.superseded) { this.tracks.delete(path); return; }
        state.state = 2; ++this.stats.failures;
        console.info('Optional music download failed.', path, error.message);
        if (this.wanted === path) this.status('Music could not download. You can keep playing.', true);
      });
      return 1;
    }
    retry() { if (this.tracks.get(this.wanted)?.state === 2) this.tracks.delete(this.wanted); this.requestTrack(this.wanted); }
    stop() {
      this.wanted = ''; this.status('');
      for (const item of this.queue.splice(0)) { this.inflight.delete(item.entry.sha256); const error = new Error('Music request superseded'); error.superseded = true; item.reject(error); }
    }
    status(message, retry = false) {
      const text = root.document.getElementById('music_status'), button = root.document.getElementById('retry_music');
      if (text && text.textContent !== message) text.textContent = message;
      if (button && button.hidden !== !retry) button.hidden = !retry;
    }
  }

  async function boot(Module) {
    try {
      const config = root.SUPERTUX_DEPLOY_CONFIG;
      if (!config || !/^[a-f0-9]{64}$/.test(config.manifestSha256 || '')) throw new Error('Missing asset build identity');
      const response = await root.fetch(config.manifestUrl, {cache: 'no-store'});
      if (!response.ok) throw new Error('The game download list could not load');
      const buffer = await response.arrayBuffer();
      if (await hash(buffer) !== config.manifestSha256) throw new Error('The game was updated. Reload this page to continue.');
      const manifest = JSON.parse(new TextDecoder().decode(buffer));
      if (manifest.schema !== 1 || !/^[a-f0-9]{64}$/.test(manifest.inventorySha256) || !manifest.runtimeRoot.startsWith('/')) throw new Error('Invalid game download list');
      const loader = new Loader(Module, manifest); Module.supertuxAssets = loader;
      for (const [name, entry] of Object.entries(manifest.packages)) {
        if (!Number.isSafeInteger(entry.bytes) || entry.bytes <= 0 || !/^[a-f0-9]{64}$/.test(entry.sha256)) throw new Error('Invalid game package');
        entry.kind = name;
      }
      let [startup, wasm, javascript] = await Promise.all([
        loader.obtain(manifest.packages.startup, (loaded, total) => Module.setStatus(`Downloading game… (${loaded}/${total})`)),
        loader.obtain(manifest.packages.wasm), loader.obtain(manifest.packages.javascript)
      ]);
      Module.wasmBinary = wasm;
      Module.getPreloadedPackage = (_, bytes) => {
        if (!startup || startup.byteLength !== bytes) throw new Error('Incompatible startup package');
        const result = startup; startup = null; return result;
      };
      Module.setStatus('Starting SuperTux…');
      const script = root.document.createElement('script');
      const url = root.URL.createObjectURL(new Blob([javascript], {type: 'text/javascript'}));
      script.src = url;
      script.onload = () => root.URL.revokeObjectURL(url);
      script.onerror = () => { root.URL.revokeObjectURL(url); root.supertux_boot_failed('The game could not load. Reload to retry.'); };
      root.document.body.appendChild(script);
      const retry = root.document.getElementById('retry_music');
      if (retry) retry.addEventListener('click', () => loader.retry());
      const clear = root.document.getElementById('clear_assets_cache');
      if (clear) clear.addEventListener('click', async () => {
        clear.disabled = true;
        const cleared = await loader.cache.clear();
        loader.status(cleared ? 'Downloaded assets cleared. Saves and settings are kept. Reload to download again.' : 'Asset storage is unavailable. Saves and settings are kept.');
        clear.disabled = false;
      });
    } catch (error) { root.supertux_boot_failed(error.message + ' Reload to retry.'); }
  }
  const api = {Cache, Loader, hash, valid, boot, DB_NAME, LIMIT};
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.SupertuxAssets = api;
})(typeof window !== 'undefined' ? window : globalThis);
