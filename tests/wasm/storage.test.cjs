// Focused failure/race checks for the production pre-main storage initializer.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { test } = require('node:test');
const source = fs.readFileSync(path.join(__dirname, '../../mk/emscripten/storage.js'), 'utf8');

function runtime(options = {}) {
  const files = new Map();
  const callbacks = [];
  const calls = [];
  let dependencies = 0;
  let now = 1000;
  const warning = { hidden: true };
  const context = {
    Module: { preRun: [], print: () => {}, printErr: message => calls.push(['warning', message]) },
    window: {},
    document: { getElementById: () => warning },
    localStorage: {
      getItem: () => { if (options.localDenied) throw Error('denied'); return options.backup ?? null; },
      setItem: (_, value) => calls.push(['backup', value])
    },
    IDBFS: {}, Date: { now: () => now },
    FS: {
      mkdirTree: () => {},
      mount: () => { calls.push(['mount']); if (options.mountThrow) throw Error('mount failed'); },
      unmount: () => { files.clear(); calls.push(['unmount']); },
      analyzePath: filename => ({ exists: files.has(filename) }),
      writeFile: (filename, data) => files.set(filename, data),
      readFile: filename => files.get(filename),
      syncfs: (populate, callback) => {
        calls.push(['sync', populate]);
        if (options.syncThrow) throw Error('sync failed');
        callbacks.push(callback);
      }
    },
    addRunDependency: () => ++dependencies,
    removeRunDependency: () => --dependencies,
    abort: message => { throw Error(message); }
  };
  vm.runInNewContext(source, context);
  return { ...context, files, calls, callbacks, warning,
    start: () => context.Module.preRun[0](),
    dependencies: () => dependencies,
    advance: ms => { now += ms; },
    storage: context.Module.supertuxStorage };
}

test('hydration holds main and outward flush until completion; mounted once', async () => {
  const r = runtime(); r.start(); r.start();
  assert.equal(r.dependencies(), 1);
  assert.equal(await r.storage.flush(), false);
  r.window.supertux2_syncfs();
  assert.deepEqual(r.calls, [['mount'], ['sync', true]]);
  r.callbacks.shift()(null);
  assert.equal(r.dependencies(), 0);
  assert.equal(r.storage.state, 'indexeddb');
});

test('hydrated config takes precedence over stale localStorage, including later C++ load hook', () => {
  const r = runtime({ backup: 'stale' }); r.start();
  r.files.set(r.storage.root + 'config', 'hydrated');
  r.callbacks.shift()(null);
  r.window.supertux_loadFiles();
  assert.equal(r.files.get(r.storage.root + 'config'), 'hydrated');
});

test('legacy localStorage config imports only when no persisted config exists', () => {
  const r = runtime({ backup: 'legacy' }); r.start(); r.callbacks.shift()(null);
  assert.equal(r.files.get(r.storage.root + 'config'), 'legacy');
});

for (const failure of ['mountThrow', 'syncThrow', 'callbackError']) {
  test(`${failure} releases main, reports fallback, and does not flush a partial mount`, async () => {
    const r = runtime({ [failure]: true, backup: 'backup' }); r.start();
    if (failure === 'callbackError') {
      r.files.set(r.storage.root + 'config', 'partial');
      const callback = r.callbacks.shift(); callback(Error('IndexedDB denied')); callback(null);
    }
    assert.equal(r.dependencies(), 0);
    assert.equal(r.storage.state, 'memory');
    assert.equal(r.warning.hidden, false);
    assert.equal(r.files.get(r.storage.root + 'config'), 'backup');
    assert.equal(await r.storage.flush(), false);
  });
}

test('localStorage denial does not hold startup', () => {
  const r = runtime({ localDenied: true }); r.start(); r.callbacks.shift()(null);
  assert.equal(r.dependencies(), 0);
  assert.equal(r.storage.state, 'indexeddb');
});

test('explicit flushes serialize and include requests arriving during the first flush', async () => {
  const r = runtime(); r.start(); r.callbacks.shift()(null);
  const a = r.storage.flush(), b = r.storage.flush();
  assert.equal(r.callbacks.length, 1);
  r.callbacks.shift()(null);
  assert.equal(await a, true);
  assert.equal(r.callbacks.length, 1);
  r.callbacks.shift()(null);
  assert.equal(await b, true);
});

test('callback errors and synchronous exceptions release flush guard and allow explicit retry', async () => {
  const r = runtime(); r.start(); r.callbacks.shift()(null);
  const failed = r.storage.flush(); r.callbacks.shift()(Error('quota'));
  assert.equal(await failed, false);
  const original = r.FS.syncfs;
  r.FS.syncfs = () => { throw Error('sync threw'); };
  assert.equal(await r.storage.flush(), false);
  r.FS.syncfs = original;
  const retry = r.storage.flush(); r.callbacks.shift()(null);
  assert.equal(await retry, true);
  assert.equal(r.storage.lastError, null);
  assert.equal(r.warning.hidden, true);
});

test('per-frame requests do not overlap and are throttled after completion', () => {
  const r = runtime(); r.start(); r.callbacks.shift()(null);
  for (let i = 0; i < 10; ++i) r.window.supertux2_syncfs();
  assert.equal(r.callbacks.length, 1); r.callbacks.shift()(null);
  r.window.supertux2_syncfs(); assert.equal(r.callbacks.length, 0);
  r.advance(1000); r.window.supertux2_syncfs(); assert.equal(r.callbacks.length, 1);
});
