// Controlled Safari lifecycle/audio races against the production shell.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const {test} = require('node:test');
const source = fs.readFileSync(path.join(__dirname, '../../mk/emscripten/browser.js'), 'utf8');

class Target {
  constructor() { this.listeners = {}; this.style = {}; this.hidden = true; }
  addEventListener(type, callback) { (this.listeners[type] ||= []).push(callback); }
  emit(type, event = {}) { for (const callback of this.listeners[type] || []) callback(event); }
  closest() { return null; }
  focus() {}
}

function runtime(visual = true) {
  const ids = Object.fromEntries(['game_shell', 'game_area', 'overlay', 'start_button',
    'play_muted', 'status', 'canvas', 'spinner', 'progress', 'progress_desc',
    'fullscreen_button'].map(id => [id, new Target()]));
  const document = new Target(), window = new Target();
  document.getElementById = id => ids[id];
  document.hidden = false;
  window.innerWidth = 960; window.innerHeight = 600;
  const view = new Target();
  Object.assign(view, {width: 390.8, height: 844.2, offsetLeft: 0, offsetTop: 3});
  if (visual) window.visualViewport = view;
  let observe;
  window.ResizeObserver = class {
    constructor(callback) { observe = callback; }
    observe(target) { assert.equal(target, ids.game_area); }
  };
  let safeX = 0, safeY = 0;
  ids.game_area.getBoundingClientRect = () => ({
    width: parseFloat(ids.game_shell.style.width) - safeX,
    height: parseFloat(ids.game_shell.style.height) - safeY
  });
  const calls = [], frames = new Map(), timers = new Map();
  let serial = 0;
  window.requestAnimationFrame = callback => { frames.set(++serial, callback); return serial; };
  window.supertux_saveFiles = () => { calls.push(['flush']); return Promise.resolve(true); };
  const Module = {canvas: ids.canvas, ccall: (name, _, types, args) => calls.push([name, ...args])};
  vm.runInNewContext(source, {Module, window, document, console: {warn() {}, info() {}},
    setTimeout: callback => { timers.set(++serial, callback); return serial; },
    clearTimeout: id => timers.delete(id)});
  const frame = () => { const batch = [...frames.values()]; frames.clear(); batch.forEach(callback => callback()); };
  return {ids, document, window, view, calls, frames, timers, frame, shell: Module.supertuxShell,
    safe: (x, y) => { safeX = x; safeY = y; }, observedResize: () => observe(),
    start: () => ids.start_button.emit('click'), muted: () => ids.play_muted.emit('click')};
}

class Audio extends Target {
  constructor() { super(); this.state = 'suspended'; this.resumeCalls = 0; }
  suspend() { this.state = 'suspended'; return Promise.resolve(); }
  resume() { ++this.resumeCalls; this.state = 'running'; return Promise.resolve(); }
}
const settle = async () => { await Promise.resolve(); await Promise.resolve(); };

test('SDK/custom HTML without the template controls keeps the existing runtime', () => {
  const Module = {canvas: new Target()};
  vm.runInNewContext(source, {Module, document: {getElementById: () => null}});
  assert.equal(Module.supertuxShell, undefined);
});

test('early lifecycle never calls C++; ready/start are separate; unavailable audio remains playable', () => {
  const r = runtime(); r.window.emit('blur'); r.window.emit('pagehide'); r.frame();
  assert.deepEqual(r.calls, []);
  r.shell.ready(); assert.equal(r.shell.active, false);
  r.start(); assert.equal(r.shell.active, true);
  assert.equal(r.ids.overlay.style.display, 'none');
});

test('visible viewport minus safe area uses integer CSS pixels; bursts/no-op/zero sizes do not reallocate', () => {
  const r = runtime(); r.safe(32, 24); r.shell.ready(); r.frame();
  assert.deepEqual(r.calls.filter(x => x[0] === 'set_resolution'), [['set_resolution', 358, 820]]);
  r.view.height = 640.9;
  for (let i = 0; i < 30; ++i) r.view.emit('resize');
  assert.equal(r.frames.size, 1); r.frame();
  assert.deepEqual(r.calls.filter(x => x[0] === 'set_resolution').at(-1), ['set_resolution', 358, 616]);
  const count = r.calls.length; r.view.emit('scroll'); r.frame(); assert.equal(r.calls.length, count);
  r.view.height = 0; r.view.emit('resize'); r.frame(); assert.equal(r.calls.length, count);
  r.view.height = 640.9; r.safe(40, 30); r.observedResize(); r.frame();
  assert.deepEqual(r.calls.filter(x => x[0] === 'set_resolution').at(-1), ['set_resolution', 350, 610]);
  const fallback = runtime(false); fallback.shell.ready();
  assert.deepEqual(fallback.calls.at(-1), ['set_resolution', 960, 600]);
});

test('real engine resume is invoked synchronously; background/return cannot auto-resume', async () => {
  const r = runtime(), audio = new Audio(); r.shell.setAudioContext(audio); r.shell.ready();
  r.start(); assert.equal(audio.resumeCalls, 1); assert.equal(r.shell.active, false);
  await settle(); assert.equal(r.shell.active, true);
  r.document.hidden = true; r.document.emit('visibilitychange'); r.window.emit('pagehide');
  assert.equal(r.shell.active, false); assert.equal(audio.state, 'suspended');
  assert.ok(r.calls.some(x => x[0] === 'save_config')); assert.ok(r.calls.some(x => x[0] === 'flush'));
  r.document.hidden = false; r.document.emit('visibilitychange'); r.window.emit('pageshow');
  assert.equal(r.shell.active, false); assert.equal(audio.resumeCalls, 1);
  r.start(); await settle(); assert.equal(r.shell.active, true);
});

test('reject/throw/interrupted resume has visible recovery and muted fallback', async () => {
  for (const mode of ['reject', 'throw', 'interrupted']) {
    const r = runtime(), audio = new Audio(); r.shell.setAudioContext(audio); r.shell.ready();
    audio.resume = () => {
      if (mode === 'throw') throw Error('denied');
      if (mode === 'reject') return Promise.reject(Error('denied'));
      audio.state = 'interrupted'; return Promise.resolve();
    };
    r.start(); await settle();
    assert.equal(r.shell.active, false); assert.equal(r.ids.play_muted.hidden, false);
    r.muted(); assert.equal(r.shell.active, true); assert.equal(audio.state, 'suspended');
  }
});

test('late audio resume cannot bypass a timeout or lifecycle pause', async () => {
  for (const cancel of ['timeout', 'blur']) {
    const r = runtime(), audio = new Audio(); let resolve;
    audio.resume = () => new Promise(done => { resolve = done; });
    r.shell.setAudioContext(audio); r.shell.ready(); r.start();
    if (cancel === 'timeout') [...r.timers.values()][0](); else r.window.emit('blur');
    audio.state = 'running'; audio.emit('statechange'); resolve(); await settle();
    assert.equal(r.shell.active, false); assert.equal(audio.state, 'suspended');
    assert.equal(r.ids.overlay.style.display, 'flex');
  }
});

test('audio interruption pauses active game; SDK unlock cannot unmute covered or muted play', async () => {
  const r = runtime(), audio = new Audio(); r.shell.setAudioContext(audio); r.shell.ready();
  r.start(); await settle(); audio.state = 'interrupted'; audio.emit('statechange');
  assert.equal(r.shell.active, false);
  audio.state = 'running'; audio.emit('statechange'); assert.equal(audio.state, 'suspended');
  r.muted(); audio.state = 'running'; audio.emit('statechange');
  assert.equal(r.shell.active, true); assert.equal(audio.state, 'suspended');
});

test('canceled input clears actions; game gesture defaults suppressed; fullscreen rejection remains playable', async () => {
  const r = runtime(); r.shell.ready(); r.start();
  const before = r.calls.length;
  r.ids.canvas.emit('touchcancel'); r.ids.canvas.emit('pointercancel');
  assert.deepEqual(r.calls.slice(before), [['reset_browser_input'], ['reset_browser_input']]);
  let prevented = false;
  r.ids.canvas.emit('touchmove', {preventDefault: () => { prevented = true; }});
  assert.equal(prevented, true);
  r.document.fullscreenEnabled = true;
  r.ids.game_shell.requestFullscreen = () => Promise.reject(Error('Safari rejected'));
  r.ids.fullscreen_button.emit('click'); await settle(); assert.equal(r.shell.active, true);
});

test('context loss freezes game, saves best effort, and cannot be resumed', () => {
  const r = runtime(); r.shell.ready(); r.start();
  r.ids.canvas.emit('webglcontextlost', {preventDefault() {}});
  assert.equal(r.shell.active, false); assert.equal(r.ids.start_button.hidden, true);
  r.start(); assert.equal(r.shell.active, false);
});
