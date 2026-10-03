// Browser-owned viewport, lifecycle, and activation. Linked inside the runtime
// with --pre-js; native/Android builds keep their existing SDL behavior.
(function () {
  'use strict';
  const element = id => document.getElementById(id);
  const shell = element('game_shell'), area = element('game_area');
  const overlay = element('overlay'), button = element('start_button');
  const mutedButton = element('play_muted'), status = element('status');
  const canvas = Module.canvas;
  // CMake's intermediate SDK HTML/custom embeddings may not use our template.
  // Keep their existing SDL loop instead of crashing on absent shell controls.
  if (!shell || !area || !overlay || !button || !mutedButton || !status || !canvas) return;
  let ready = false, active = false, failed = false, muted = false;
  let audio = null, pending = false, epoch = 0, resizeFrame = 0;
  let lastWidth = 0, lastHeight = 0;
  let audioTimer = 0;

  function call(name, types = [], args = []) {
    return Module.ccall(name, null, types, args);
  }

  function resetInput() {
    if (ready && !failed) call('reset_browser_input');
  }

  function suspendAudio() {
    if (!audio || audio.state === 'closed') return;
    try {
      Promise.resolve(audio.suspend()).catch(error => console.warn('Audio suspend failed:', error));
    } catch (error) {
      console.warn('Audio suspend failed:', error);
    }
  }

  function save() {
    if (!ready || failed) return;
    try {
      call('save_config');
      // Phase 1 serializes requests and reports storage errors. This is best
      // effort on pagehide; mobile browsers need not finish asynchronous writes.
      window.supertux_saveFiles();
    } catch (error) {
      console.warn('Could not save browser settings:', error);
    }
  }

  function prompt(message) {
    overlay.style.display = 'flex';
    status.textContent = message;
    button.hidden = false;
    button.disabled = false;
    button.textContent = Module.supertuxStarted ? 'Resume' : 'Start';
    element('spinner').style.display = 'none';
    element('progress').hidden = true;
    element('progress_desc').textContent = '';
  }

  function pause(message = 'Paused. Press Resume to continue.') {
    const wasActive = active;
    ++epoch;
    active = false;
    pending = false;
    clearTimeout(audioTimer);
    if (ready && !failed) {
      call('set_browser_suspended', ['number'], [1]);
      prompt(message);
    }
    suspendAudio();
    if (wasActive) save();
  }

  function resizeNow() {
    resizeFrame = 0;
    // visualViewport reflects Safari's toolbar/keyboard. Safe-area padding is
    // applied once by CSS; use the inner area's measured CSS pixels for SDL.
    const view = window.visualViewport;
    const width = view ? view.width : window.innerWidth;
    const height = view ? view.height : window.innerHeight;
    if (!(width > 0 && height > 0)) return;
    shell.style.left = (view ? view.offsetLeft : 0) + 'px';
    shell.style.top = (view ? view.offsetTop : 0) + 'px';
    shell.style.width = width + 'px';
    shell.style.height = height + 'px';
    if (!ready || failed) return;
    const rect = area.getBoundingClientRect();
    const w = Math.floor(rect.width), h = Math.floor(rect.height);
    if (w < 1 || h < 1 || (w === lastWidth && h === lastHeight)) return;
    resetInput();
    call('set_resolution', ['number', 'number'], [w, h]);
    lastWidth = w;
    lastHeight = h;
  }

  function resize() {
    if (!resizeFrame) resizeFrame = window.requestAnimationFrame(resizeNow);
  }

  function activate(withoutAudio) {
    if (!ready || failed || pending || document.hidden) return;
    const attempt = ++epoch;
    pending = true;
    muted = withoutAudio || !audio || audio.state === 'closed';
    button.disabled = true;
    // Focus happens before resume so focus/blur cannot undo a completed start.
    canvas.focus({preventScroll: true});
    function complete() {
      if (attempt !== epoch || document.hidden || failed) return;
      clearTimeout(audioTimer);
      pending = false;
      active = true;
      Module.supertuxStarted = true;
      call('set_browser_suspended', ['number'], [0]);
      overlay.style.display = 'none';
      mutedButton.hidden = true;
      resize();
    }
    function blocked(error) {
      if (attempt !== epoch || !pending) return;
      ++epoch; // A late resume promise must not dismiss the recovery prompt.
      pending = false;
      clearTimeout(audioTimer);
      suspendAudio();
      console.warn('Audio activation needs another gesture:', error);
      prompt('Audio could not start. Try again or play without sound.');
      mutedButton.hidden = false;
    }
    if (muted) {
      suspendAudio();
      complete();
      return;
    }
    // Invoke resume in this trusted click handler, before any await. The context
    // is the real OpenAL engine, including Safari's interrupted state.
    audioTimer = setTimeout(() => blocked('Timed out waiting for audio'), 3000);
    try {
      Promise.resolve(audio.resume()).then(() => {
        if (audio.state === 'running') complete();
        else blocked('Audio state: ' + audio.state);
      }, blocked);
    } catch (error) { blocked(error); }
  }

  async function fullscreen() {
    try {
      if (document.fullscreenElement) await document.exitFullscreen();
      else if (document.fullscreenEnabled && shell.requestFullscreen) await shell.requestFullscreen();
    } catch (error) {
      console.info('Fullscreen is unavailable; continue playing in this tab.', error);
    }
    resize();
  }

  Module.supertuxShell = {
    resize,
    resetInput,
    get active() { return active; },
    get audioState() { return audio ? audio.state : 'unavailable'; },
    setAudioContext(context) {
      audio = context;
      suspendAudio();
      audio.addEventListener('statechange', () => {
        if (active && !muted && audio.state !== 'running') pause('Audio was interrupted. Press Resume to continue.');
        // SDK one-time unlock listeners can still fire. Keep covered/muted game
        // audio suspended without replacing or monkey-patching that engine.
        else if (!pending && (!active || muted) && audio.state === 'running') suspendAudio();
      });
    },
    ready() {
      ready = true;
      call('set_browser_suspended', ['number'], [1]);
      resizeNow();
      prompt(audio ? 'Ready. Press Start to play.' : 'Ready. Audio is unavailable on this browser.');
    },
    fail(message) {
      pause(message);
      overlay.style.display = 'flex';
      status.textContent = message;
      failed = true;
      button.hidden = true;
      mutedButton.hidden = true;
    }
  };

  button.addEventListener('click', () => activate(false));
  mutedButton.addEventListener('click', () => activate(true));
  const fullscreenButton = element('fullscreen_button');
  fullscreenButton.hidden = !(document.fullscreenEnabled && shell.requestFullscreen);
  fullscreenButton.addEventListener('click', fullscreen);
  window.addEventListener('keydown', event => {
    if (active && (event.key === 'F11' || (event.key === 'Enter' && event.altKey))) {
      event.preventDefault();
      event.stopImmediatePropagation();
      fullscreen();
    }
  }, true);
  // Covered-canvas gestures must not reach SDL or the SDK's document unlock
  // listeners. Real buttons/links remain operable, including keyboard access.
  for (const type of ['keydown', 'keyup', 'mousedown', 'mouseup', 'touchstart', 'touchend']) {
    window.addEventListener(type, event => {
      if (!active && !event.target.closest('button, a, input')) {
        event.preventDefault();
        event.stopImmediatePropagation();
      }
    }, {capture: true, passive: false});
  }
  for (const type of ['touchmove', 'gesturestart', 'gesturechange', 'contextmenu']) {
    canvas.addEventListener(type, event => event.preventDefault(), {passive: false});
  }
  for (const type of ['pointercancel', 'touchcancel', 'lostpointercapture']) {
    canvas.addEventListener(type, resetInput);
  }
  canvas.addEventListener('click', () => canvas.focus({preventScroll: true}));
  canvas.addEventListener('webglcontextlost', event => {
    event.preventDefault();
    Module.supertuxShell.fail('Graphics context lost. Reload this page to continue. Recent committed progress is kept.');
  });
  window.addEventListener('resize', resize);
  window.addEventListener('orientationchange', () => { resetInput(); resize(); });
  document.addEventListener('fullscreenchange', resize);
  if (window.visualViewport) {
    window.visualViewport.addEventListener('resize', resize);
    window.visualViewport.addEventListener('scroll', resize);
  }
  // Safe-area CSS can settle after the orientation event without a new window
  // size. Observe the inner area too; integer-size dedup prevents resize loops.
  if (window.ResizeObserver) new window.ResizeObserver(resize).observe(area);
  window.addEventListener('blur', () => pause());
  document.addEventListener('visibilitychange', () => {
    if (document.hidden) { pause(); save(); }
    else resize(); // Returning never automatically resumes gameplay or audio.
  });
  window.addEventListener('pagehide', () => { pause(); save(); });
  window.addEventListener('pageshow', () => { if (ready) pause(); resize(); });
  resize();
})();
