// Runs inside Emscripten's generated runtime (--pre-js), before main/config/save reads.
// Keep this path in agreement with PhysfsSubsystem::find_mount_userdir().
(function() {
  'use strict';
  const root = '/home/web_user/.local/share/supertux2/';
  const dependency = 'supertux-user-storage';
  const flushWarning = 'Could not save progress to browser storage; retry before closing this page.';
  let mounted = false;
  let saving = false;
  let requests = [];
  let nextSync = 0;
  let started = false;

  const storage = Module['supertuxStorage'] = {
    root: root,
    state: 'initializing',
    lastError: null,
    flush: flush
  };

  function warn(message, error) {
    storage.lastError = message;
    Module['printErr'](message + (error ? ': ' + error : ''));
    const element = document.getElementById('storage_warning');
    if (element) {
      element.textContent = message;
      element.hidden = false;
    }
  }

  function importConfig() {
    // A hydrated IndexedDB config wins. localStorage is only a legacy/memory fallback.
    try {
      if (!FS.analyzePath(root + 'config').exists) {
        const config = localStorage.getItem('supertux2_config');
        if (config !== null) FS.writeFile(root + 'config', config);
      }
    } catch (error) {
      warn('Could not read the browser settings backup.', error);
    }
  }

  function backupConfig() {
    try {
      if (FS.analyzePath(root + 'config').exists) {
        localStorage.setItem('supertux2_config', FS.readFile(root + 'config', { encoding: 'utf8' }));
      }
    } catch (error) {
      warn('Could not back up browser settings.', error);
    }
  }

  function pump() {
    if (saving || requests.length === 0) return;
    saving = true;
    const batch = requests;
    requests = [];
    function complete(error) {
      saving = false;
      nextSync = Date.now() + (error ? 30000 : 1000);
      if (error) {
        warn(flushWarning, error);
      } else if (storage.lastError === flushWarning) {
        storage.lastError = null;
        const element = document.getElementById('storage_warning');
        if (element) element.hidden = true;
      }
      batch.forEach(resolve => resolve(!error));
      // Explicit requests made during an active flush must get their own later flush.
      pump();
    }
    try {
      FS.syncfs(false, complete);
    } catch (error) {
      complete(error);
    }
  }

  function flush() {
    if (storage.state !== 'indexeddb') return Promise.resolve(false);
    return new Promise(resolve => {
      requests.push(resolve);
      pump();
    });
  }

  window.supertux_loadFiles = importConfig;
  window.supertux_saveFiles = function() {
    backupConfig();
    return flush();
  };
  window.supertux2_syncfs = function() {
    // The existing game calls this once per frame. Avoid overlapping transactions
    // or continuously queuing work; explicit save requests bypass this throttle.
    if (storage.state === 'indexeddb' && !saving && Date.now() >= nextSync) {
      return flush();
    }
  };

  Module['preRun'] = Module['preRun'] || [];
  Module['preRun'].push(function() {
    if (started) return;
    started = true;
    addRunDependency(dependency);
    let finished = false;
    function ready(error) {
      if (finished) return;
      finished = true;
      try {
        if (error) {
          // Discard any partially populated mount. Keep a usable MEMFS directory.
          if (mounted) FS.unmount(root);
          FS.mkdirTree(root);
          storage.state = 'memory';
          warn('Browser storage is unavailable. Progress will last only for this session; settings use a local backup when available.', error);
        } else {
          storage.state = 'indexeddb';
          Module['print']('Browser storage hydrated before main: ' + root);
        }
        importConfig();
      } catch (fatal) {
        abort('Could not initialize the browser user directory: ' + fatal);
      } finally {
        removeRunDependency(dependency);
      }
    }
    try {
      FS.mkdirTree(root);
      FS.mount(IDBFS, {}, root);
      mounted = true;
      FS.syncfs(true, ready);
    } catch (error) {
      ready(error);
    }
  });
})();
