# SuperTux mobile web, Safari, and multiplayer feasibility audit

Audit date: 2026-10-02 (UTC). Repository: `jbbejena/supertux`. Branch: `mobile-web-audit`. Audited source: [00673d1dfefeedf39aaf502ac0cfb1fd005174d6](https://github.com/jbbejena/supertux/tree/00673d1dfefeedf39aaf502ac0cfb1fd005174d6). Upstream `SuperTux/supertux:master` resolved to the same commit during the audit.

This is an architecture audit and implementation plan. The only repository change is this document. No gameplay, networking, build configuration, assets, or upstream files were changed. No merge was performed.

Evidence labels used below:

- **Confirmed:** directly inspected source, history, workflow configuration, or downloaded CI text logs.
- **Narrow check:** repository JavaScript evaluated with mocked filesystem APIs in Node; this is not a browser/game test.
- **Likely:** consequence supported by source but not reproduced in the actual WASM runtime.
- **Device test:** requires a deployed artifact and real Safari/iPhone testing.

## 1. Executive summary

Mobile web is a viable direction. Current SuperTux has a substantial Emscripten build path, generic SDL touch controls, a browser main loop, browser save hooks, and independent local-player controllers. It does not yet have evidence of reliable current iPhone Safari operation.

The highest-priority boot investigation is **SDL3 image loading**, not an assumption that the old PhysFS initialization failure persists. Recent upstream Release and Debug CI jobs successfully compiled, but **both emitted `warning: undefined symbol: IMG_Load_IO`**. `SDLSurface::from_file()` calls that function to load ordinary images. `ERROR_ON_UNDEFINED_SYMBOLS=0` permits this artifact to link. This is a concrete link defect in the inspected CI artifact and a likely browser-wide boot blocker; actual runtime failure on the audited SHA has not been reproduced.

The old PhysFS issue has a relevant fix already present: `Main::run()` now passes `argv[0]` through to `PHYSFS_init()`. A maintainer reported working modern WASM after fixes in June 2026. Do not revert PhysFS or downgrade to v0.6.3 before testing the current artifact.

Other major findings:

- **Download/memory risk:** recent CI packaged approximately 311 MiB of assets. The audited tree contains 326,676,452 bytes of tracked `data/` blobs, excluding generated files and filesystem/runtime overhead. The runtime starts with 128 MiB WASM memory and permits growth to 512 MiB. Those numbers do not include all JavaScript, decoded media, or GPU memory.
- **Persistence defect:** C++ `EM_ASM` refers to `m_userdir` as if it were a JavaScript variable. It is never passed into JavaScript. A narrow check confirms the mount block silently executes no filesystem calls in that environment. Even after correcting that name, IndexedDB hydration has no startup barrier.
- **Touch verdict: A.** The existing controls are generic C++/SDL and emit ordinary `Control` actions. Reuse them; add browser integration and focused control fixes only where necessary. Android Java is an SDL launcher, not a separate touch-control implementation.
- **Touch activation risk:** `Config::Config()` queries touch devices before the game initializes SDL; only Android rechecks after SDL initialization. Browser touch controls may consequently remain disabled.
- **Safari shell gaps:** no mobile viewport meta tag, safe-area layout, touch gesture policy, page visibility/pagehide handling, explicit audio-unlock UX, or robust touch cancellation/reset path.
- **Networking verdict:** reasonably networkable at the input seam, but online co-op is a substantial replication project. Sending Player 2 inputs alone will not synchronize a second browser's enemies, objects, scripts, or progression.

Recommended sequence: strict modern WASM boot and filesystem startup first; mobile viewport/lifecycle/audio second; enable and validate existing touch controls third. The first mobile product milestone is a URL on which an actual iPhone can enter and play one level with simultaneous movement, jump, and action. Networking starts only after that milestone and browser local multiplayer validation.

**Validation limit:** no local WASM compilation or actual game/browser/Safari run was performed. Recent upstream compilation and artifacts were inspected; Node checked shell syntax and two filesystem failure cases. Details are in section 4.

## 2. Current WebAssembly architecture

### Source to browser artifact

1. [.github/workflows/wasm.yml](../.github/workflows/wasm.yml) checks out full history and recursive submodules. The workflow has Release and Debug jobs on `ubuntu-latest`.
2. It installs build utilities, clones `emsdk`, installs/activates **`latest`**, and uses the runner's vcpkg installation.
3. `emcmake cmake` configures with the vcpkg toolchain, an Emscripten chainload toolchain, `wasm32-emscripten`, `CMAKE_POLICY_VERSION_MINIMUM=3.5`, and `VCPKG_MANIFEST_FEATURES=core`.
4. [CMakeLists.txt](../CMakeLists.txt) defines `BUILD_DATA_DIR` as source `data/` and `BUILD_CONFIG_DATA_DIR` as build `data/`. Under `EMSCRIPTEN` it includes [mk/cmake/SuperTux/Emscripten.cmake](../mk/cmake/SuperTux/Emscripten.cmake).
5. The workflow copies the entire source data tree into the build data tree with `rsync -aP ../data/ data/`. This preserves generated files already there; it is not the release install-data filter.
6. `emmake make -j$(nproc)` compiles the game and links `supertux2.html`, `supertux2.js`, `supertux2.wasm`, and the preloaded `supertux2.data` package.
7. [mk/cmake/SuperTux/BuildInstall.cmake](../mk/cmake/SuperTux/BuildInstall.cmake) configures `template.html` from [mk/emscripten/template.html.in](../mk/emscripten/template.html.in) and copies the shell's logo, background, and icon.
8. CI replaces generated `supertux2.html` with `template.html` and moves `supertux2*` into an upload directory. Release uploads an artifact. A master/Release-only step renames the entry to `index.html`, zips it, and posts to the configured `UPLOAD_URL`.
9. A normal HTTP/HTTPS server must serve the complete artifact directory, keeping JS/WASM/data URLs together. `file://` is explicitly unsupported. Serve WASM with `application/wasm` and avoid mixing cache versions of JS, WASM, and data.

There is no Docker/container stage in the current WASM workflow. Android build/deployment is separate.

### Compile/link configuration

Current `Emscripten.cmake` supplies:

| Setting | Current value/purpose |
| --- | --- |
| Executable suffix | `.html` |
| Compile/common flags | `-sDISABLE_EXCEPTION_CATCHING=0 -sUSE_SDL=3 -sUSE_SDL_IMAGE=3 -sUSE_SDL_TTF=3 -sUSE_VORBIS=1 -fPIC` |
| Memory | `-sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912` |
| Undefined symbols | `-sERROR_ON_UNDEFINED_SYMBOLS=0`; a material reliability problem |
| Asset loading | `--preload-file ${BUILD_CONFIG_DATA_DIR} --use-preload-plugins` |
| Persistent filesystem library | `-lidbfs.js` |
| Optional GL | `-sFULL_ES2` plus `HAVE_OPENGL` and `USE_OPENGLES2` cache values when `ENABLE_OPENGL` is enabled |
| Debug | Undefined-behavior sanitizer, `SAFE_HEAP=1`, `ASSERTIONS=1` |
| OpenAL | Imported `OpenAL` interface, local headers under `mk/emscripten/AL/`, `-lopenal` |
| LTO | Explicitly forced off for Emscripten; comment cites wasm-ld/bitcode issue 20275 |

`CMakeLists.txt` exports `_main`, `_set_resolution`, `_save_config`, `_onDownloadProgress`, `_onDownloadFinished`, `_onDownloadError`, `_onDownloadAborted`, and `_getExceptionMessage`. It exports runtime methods `ccall` and `cwrap`.

`vcpkg.json` lists PhysFS, fmt, GLM, PNG/Zlib, Vorbis/Ogg/Freetype, and SDL3/image/ttf. Native curl/OpenAL/RAQM and GL-loader features are segregated from Emscripten. SDL dependencies have **mixed providers** in the current WASM path: emscripten SDL port flags coexist with vcpkg-installed SDL libraries, while native `add_package` handling for SDL3 and SDL3_image is skipped. `SDL3_ttf` is still in the unconditional game link list. The missing image symbol makes provider/header/library consistency a Phase 1 investigation.

**Threading:** neither the current Emscripten flags nor the WASM workflow enables `-pthread`, `USE_PTHREADS`, a pthread pool, or `PROXY_TO_PTHREAD`. The reviewed simulation/input/audio paths run from the main loop. Squirrel “threads” here are scheduled VM coroutines, not evidence of OS pthreads. Dependency internals and the final generated binary still need validation; this audit does not claim a whole-program concurrency proof.

**SharedArrayBuffer:** there is no current project-configured pthread requirement. Cross-origin isolation is therefore not an established blocker for the planned single-thread build. If later enabling pthreads, secure context, browser support, and COOP/COEP-compatible hosting/assets would become required. Do not import v0.6.3's threading flags.

### HTML and JavaScript glue

The shell creates `Module.canvas` for `#canvas`, loading/status elements, upload/download helpers, output logging, and an async script tag for `supertux2.js`.

Important hooks:

- `Module.onRuntimeInitialized` uses `cwrap` for resolution and downloader callbacks.
- `tryResize()` calls `setResolution(window.innerWidth, window.innerHeight)`. Its `cwrap` declaration lists only one numeric argument although the C++ function takes two. This is a confirmed declaration mismatch; whether the second value survives depends on Emscripten's wrapper optimization. Correct the declaration rather than relying on it.
- `window.supertux_setAutofit()` enables resize and body overflow behavior.
- `window.supertux_loadFiles()` imports `supertux2_config` from localStorage into the user directory.
- `window.supertux_saveFiles()` starts a filesystem flush and synchronously backs up `config` into localStorage.
- `window.supertux2_syncfs()` serializes flush calls with a `saving` boolean.
- `supertux_download()` and `supertux_upload()` bridge filesystem files and browser downloads/uploads.
- `supertux_xhr_file_download()` and `supertux_xhr_string_download()` connect XHR to `Downloader` callbacks. These implement add-on/update downloads, not multiplayer transport.
- `window.onerror` attempts a C++ exception message and displays failure UI.
- Canvas click restores keyboard focus. Context loss prompts reload; there is no restoration implementation.
- `resize` updates resolution; `unload` calls exported `save_config` then saves files.
- F11/Alt+Enter uses `requestFullscreen()`/`exitFullscreen()`. Failure is swallowed; no mobile fullscreen button or capability fallback is provided.

[src/port/emscripten.hpp](../src/port/emscripten.hpp) defines the exported functions. `set_resolution(int w, int h)` calls `VideoSystem::on_resize()` and `MenuManager::on_window_resize()`. This direct JS path does **not** invoke `ScreenManager::on_window_resize()`, which also notifies screens; the SDL window-resize event path does. Check whether SDL supplies that second notification rather than assuming both resize paths are equivalent.

`init_emscripten()` calls an optional `window.supertux_onready` at the beginning of `Main::run()`. It is **not** proof that PhysFS, video, assets, or a level are ready.

### Virtual filesystem and PhysFS startup

Preloading establishes asset files in Emscripten's virtual filesystem before main proceeds. The runtime mount path is tied to the compiled `BUILD_CONFIG_DATA_DIR` string and packager destination. There is no explicit `source@/stable-destination` mapping. Inspect the generated package manifest before changing this path.

The current startup order in [src/supertux/main.cpp](../src/supertux/main.cpp) is:

`Main::run()` → argument parsing → `PhysfsSubsystem(argv[0], ...)` → `ConfigSubsystem` → translations → `Main::launch_game()` → add-ons/static-data remount → `SDLSubsystem` → `InputManager` → renderer → OpenAL → scripting/resources/game/screens → main loop.

`PhysfsSubsystem::PhysfsSubsystem()`:

- Uses Android JNI/context initialization only under `__ANDROID__`; other builds use `PHYSFS_init(argv0)`.
- Enables symbolic links.
- Calls `find_mount_datadir()` then `find_mount_userdir()`.

The Emscripten data path mounts `BUILD_CONFIG_DATA_DIR` at the PhysFS virtual root. `remount_datadir_static()` remounts `images/credits`, `levels`, `locale`, `scripts`, and `shader`, then user levels when present. Preserve this precedence during startup changes.

`find_mount_userdir()` eventually selects `/home/web_user/.local/share/supertux2/` for Emscripten, creates it, tries IDBFS mount/population, sets the PhysFS write directory, and mounts the user directory ahead of base data. It also calls native-style preference/user-directory functions before the Emscripten override; null returns are not checked before conversion to strings. This is a robustness risk to test, separate from the documented `PHYSFS_init` regression.

**Confirmed defective browser mount block:**

```javascript
try {
  FS.mount(IDBFS, {}, m_userdir);
  FS.syncfs(true, (err) => { console.log(err); });
} catch(err) {}
```

`m_userdir` is a C++ member, not a defined JS variable or an `EM_ASM` argument. The repository shell defines `root`, not `m_userdir`. In the Node narrow check the block performed zero mount/sync calls and swallowed the reference error. This must be corrected, independently of whether the earlier PhysFS boot crash is fixed.

**Hydration race:** `FS.syncfs(true, callback)` is asynchronous and startup does not wait before reading config/profiles/saves. Passing a path alone fixes neither the race nor silent error handling. Move browser storage setup/population before main using a run dependency, or an equally explicit asynchronous startup boundary. Avoid blocking the browser thread.

### Saving and persistence

C++ writes through PhysFS and the Writer/file-stream abstraction:

- `Config::load()/save()` in [src/supertux/gameconfig.cpp](../src/supertux/gameconfig.cpp): settings and per-player bindings.
- `Profile::save()` in [src/supertux/profile.cpp](../src/supertux/profile.cpp): profile directory/info.
- `Savegame::load()/save()` in [src/supertux/savegame.cpp](../src/supertux/savegame.cpp): PlayerStatus and Squirrel progression state.
- `WorldMapState::save_state()` in [src/worldmap/worldmap_state.cpp](../src/worldmap/worldmap_state.cpp): map location, level completion/statistics and state; eventually calls savegame save.

`ScreenManager::loop_iter()` calls `supertux2_syncfs()` at the end of every completed loop callback, without a dirty indicator or debounce. The guard avoids overlapping calls but can still repeatedly scan/flush while idle.

The guard is set before `FS.syncfs`. A synchronous exception is caught without resetting it: the Node narrow check produced only one attempt across two calls after a synchronous failure. Async callback errors release the guard but only log an error.

`navigator.storage.persist()` requests storage durability and supplies a warning/status flag. It is not equivalent to mounting IDBFS, successful hydration, or successful save commits. LocalStorage backup covers config only. Progression is not safely protected by that backup. `unload` is especially unreliable on mobile; tab/process eviction may bypass it.

Current saves are progression/profile/settings saves, **not** a complete in-level simulation checkpoint suitable for resuming a killed Safari tab or transmitting network snapshots.

### SDL, rendering, audio, main loop

`SDLSubsystem` initializes SDL3 video, joystick, and gamepad subsystems and SDL_ttf. Audio uses OpenAL, not an SDL_mixer path.

`Main::launch_game()` explicitly forces `VideoSystem::VIDEO_SDL` for WASM. The optional GL code is present but is not the normal selected renderer. `SDLVideoSystem::create_window()` calls `SDL_CreateRenderer(window, nullptr)` and leaves backend choice to SDL. `SDLScreenRenderer` logs the actual renderer name. Do not equate this C++ SDL renderer with a verified Safari WebGL/backend configuration.

`SDLBaseVideoSystem::create_sdl_window()` creates a resizable window, sets landscape orientation hints, disables SDL's mouse↔touch synthesis, and enables shell autofit for Emscripten. It does not request a high-pixel-density window explicitly. `on_resize()` reapplies video config, viewport, and lightmap creation. `Viewport::from_size()/to_logical()` handle scaling and letterboxing; device pixel ratio and browser visual viewport are not managed explicitly by the shell.

`GLVideoSystem` has a GLES2-compatible path and `VideoSystem::create(VIDEO_OPENGL_AUTO)` has an Emscripten GL/fallback branch. This branch should not be used to infer what `Main::launch_game()` actually selects.

`SoundManager` opens an OpenAL device/context; Emscripten's `-lopenal` provides the browser implementation. Ogg/WAV decoding and `StreamSoundSource::update()` provide sounds/music. `GameSession::toggle_pause()` pauses sounds/music, and game updates resume them as menus close. There is no repository-authored AudioContext gesture-unlock or page lifecycle bridge. Emscripten may provide unlock behavior; test the generated glue and device outcome before adding a second competing audio system.

`ScreenManager::run()` registers `emscripten_set_main_loop(g_loop_iter, -1, 1)`. `loop_iter()` advances a fixed simulation step of 15 ms (`LOGICAL_FPS = 1000/15`), processes events, updates game logic, draws, updates sound, switches screens, and flushes storage. Large elapsed gaps are clamped to four steps. The early `SDL_Delay` path can still block the main thread briefly; audit mobile frame pacing before altering it.

SDL hidden/focus-lost events pause an active session when `pause_on_focusloss` is true. This is partial lifecycle handling. There are no shell `visibilitychange`, `pagehide`, `pageshow`, or explicit reset/resume handlers.

## 3. Relevant file/symbol map

Paths below exist at the audited commit. These are the primary entry points for subsequent work, not proposed new files.

| Path | Important existing symbols / responsibility |
| --- | --- |
| [CMakeLists.txt](../CMakeLists.txt) | Emscripten include, source/link targets, exports, LTO override, data-directory definitions |
| [mk/cmake/SuperTux/Emscripten.cmake](../mk/cmake/SuperTux/Emscripten.cmake) | `EM_USE_FLAGS`, `EM_LINK_FLAGS`, OpenAL interface, memory/assets |
| [mk/cmake/SuperTux/BuildInstall.cmake](../mk/cmake/SuperTux/BuildInstall.cmake) | Browser template and shell assets |
| [.github/workflows/wasm.yml](../.github/workflows/wasm.yml) | WASM configure/build/package/upload |
| [vcpkg.json](../vcpkg.json), [.gitmodules](../.gitmodules) | External dependency providers and source submodules |
| [config.h.in](../config.h.in) | Generated platform/build feature and data-path definitions |
| [mk/emscripten/template.html.in](../mk/emscripten/template.html.in) | `Module`, `tryResize`, save/load/sync helpers, XHR, fullscreen |
| [src/port/emscripten.hpp](../src/port/emscripten.hpp) | `set_resolution`, `save_config`, downloader callbacks, `init_emscripten` |
| [src/supertux/main.cpp](../src/supertux/main.cpp) | `Main::run/launch_game`, `PhysfsSubsystem`, `SDLSubsystem`, `ConfigSubsystem` |
| [src/physfs/physfs_sdl.cpp](../src/physfs/physfs_sdl.cpp) | `get_physfs_SDLRWops` / writable variant, SDL3 IO callbacks over PhysFS |
| [src/physfs/physfs_file_system.cpp](../src/physfs/physfs_file_system.cpp) | `PhysFSFileSystem` adaptation |
| [src/video/sdl_surface.cpp](../src/video/sdl_surface.cpp) | `SDLSurface::from_file` → `IMG_Load_IO` |
| [src/video/sdlbase_video_system.cpp](../src/video/sdlbase_video_system.cpp) | `create_sdl_window`, `apply_video_mode`, `on_resize` |
| [src/video/sdl/sdl_video_system.cpp](../src/video/sdl/sdl_video_system.cpp) | `SDLVideoSystem::create_window/apply_config` |
| [src/video/sdl/sdl_screen_renderer.cpp](../src/video/sdl/sdl_screen_renderer.cpp) | Backend logging, render scale/viewport/present |
| [src/video/video_system.cpp](../src/video/video_system.cpp), [src/video/viewport.cpp](../src/video/viewport.cpp) | Renderer factory; `Viewport::from_size/to_logical` |
| [src/audio/sound_manager.cpp](../src/audio/sound_manager.cpp), [src/audio/stream_sound_source.cpp](../src/audio/stream_sound_source.cpp) | OpenAL setup, music/sound pause/resume and streaming |
| [src/supertux/screen_manager.cpp](../src/supertux/screen_manager.cpp) | `run`, `loop_iter`, `process_events`, `update_gamelogic`, touch ownership |
| [src/control/mobile_controller.cpp](../src/control/mobile_controller.cpp), [src/control/mobile_controller.hpp](../src/control/mobile_controller.hpp) | `MobileController::draw/update/apply/process_finger_*`; widget hit tests |
| [src/control/controller.cpp](../src/control/controller.cpp), [src/control/controller.hpp](../src/control/controller.hpp) | `Control` enum; `Controller::set_control/update/hold/pressed/released/reset` |
| [src/control/input_manager.cpp](../src/control/input_manager.cpp) | Controller ownership; `get_controller/push_user/pop_user`; SDL event dispatch |
| [src/control/keyboard_manager.cpp](../src/control/keyboard_manager.cpp), [src/control/keyboard_config.hpp](../src/control/keyboard_config.hpp) | `process_key_event`; `KeyboardConfig::PlayerControl` mapping |
| [src/control/game_controller_manager.cpp](../src/control/game_controller_manager.cpp), [src/control/joystick_manager.cpp](../src/control/joystick_manager.cpp) | Device→player mappings; hotplug and player management |
| [src/control/codecontroller.cpp](../src/control/codecontroller.cpp) | `CodeController::press/update` for script-driven inputs |
| [src/supertux/gameconfig.cpp](../src/supertux/gameconfig.cpp) | `Config::Config/load/save`, touch discovery/settings |
| [src/supertux/menu/options_menu.cpp](../src/supertux/menu/options_menu.cpp) | `OptionsMenu::add_mobile_control_scales`; Android-only touch toggles |
| [mk/android/app/src/main/java/org/supertux/supertux2/MainActivity.java](../mk/android/app/src/main/java/org/supertux/supertux2/MainActivity.java) | SDLActivity launcher, locale, `SDL_main` library entry |
| [src/supertux/level.cpp](../src/supertux/level.cpp) | `Level::initialize/get_players`, sector ownership |
| [src/supertux/game_session.cpp](../src/supertux/game_session.cpp) | `on_player_added/removed`, `restart_level`, `check_end_conditions`, `respawn`, `set_checkpoint_pos`, `start_sequence` |
| [src/supertux/sector.cpp](../src/supertux/sector.cpp) | `Sector::activate/update`, player transfers, collision/object environment |
| [src/object/player.cpp](../src/object/player.cpp), [src/object/player.hpp](../src/object/player.hpp) | Player ID/controller, `handle_input`, `kill`, `multiplayer_prepare_spawn/respawn`, `set_controller` |
| [src/object/camera.cpp](../src/object/camera.cpp) | `Camera::update_scroll_normal_multiplayer`; shared view/zoom |
| [src/object/firefly.cpp](../src/object/firefly.cpp), [src/trigger/door.cpp](../src/trigger/door.cpp) | Checkpoint activation; sector/door transitions |
| [src/supertux/player_status.cpp](../src/supertux/player_status.cpp), [src/supertux/savegame.cpp](../src/supertux/savegame.cpp) | Per-player bonuses/pockets and shared progression saves |
| [src/worldmap/tux.cpp](../src/worldmap/tux.cpp), [src/worldmap/worldmap.cpp](../src/worldmap/worldmap.cpp), [src/worldmap/worldmap_state.cpp](../src/worldmap/worldmap_state.cpp) | Shared world map, controller 0 navigation, state persistence |
| [src/supertux/menu/multiplayer_players_menu.cpp](../src/supertux/menu/multiplayer_players_menu.cpp), [src/supertux/menu/multiplayer_player_menu.cpp](../src/supertux/menu/multiplayer_player_menu.cpp) | Local player creation, keyboard/device assignments, removal |
| [src/supertux/game_object_manager.cpp](../src/supertux/game_object_manager.cpp), [src/supertux/game_object.hpp](../src/supertux/game_object.hpp), [src/util/uid_generator.cpp](../src/util/uid_generator.cpp) | Object ownership/lifetimes, process-local IDs, editor state APIs |
| [src/squirrel/squirrel_virtual_machine.cpp](../src/squirrel/squirrel_virtual_machine.cpp), [src/math/random.cpp](../src/math/random.cpp) | VM/coroutine scheduling and random seeds; synchronization constraints |
| [src/addon/downloader.cpp](../src/addon/downloader.cpp) | Emscripten XHR dispatch/callbacks; existing networking is asset downloading |

Existing control graphics are under [data/images/engine/mobile/](../data/images/engine/mobile): direction/highlights, buttons/pressed buttons, jump, action, pause, cheats, and debug images. The misspelled `direction_hightlight_*` filenames are actual repository names.

## 4. Current browser build health

### What was actually validated

The workspace started without a repository checkout. Shell cloning failed immediately:

`fatal: unable to access ... Failed to connect to proxy port 8080 ... Could not connect to server`.

The connected GitHub API remained usable. It supplied pinned file contents, a non-truncated recursive tree, tags, commit patches, workflow metadata, and CI logs. Source excerpts and checks were staged outside repository source files. A later network-enabled Git command succeeded in obtaining a sparse documentation checkout for committing the report; the initial proxy failure was therefore not a permanent repository-access failure. No installed `emcc`, `emcmake`, or `cmake` was found. Chromium is installed, but no current game binary was available locally; the text connector did not download the 286 MB binary artifact. A full SDK/vcpkg/recursive-submodule installation and complete build was judged excessive for this audit given the inspected CI logs and absent toolchain. No desktop build was attempted.

The following **were** done:

- Inspected current workflow/configuration and historical WASM configuration.
- Checked current branch/upstream SHA and tag resolutions.
- Read recent upstream Release **and** Debug job logs.
- Checked shell JavaScript syntax with `node --check`.
- Evaluated the exact IDBFS mount body with mocked JS filesystem objects: zero filesystem calls due to swallowed undeclared `m_userdir`.
- Evaluated the shell sync helper with a synchronous storage failure: first attempt leaves `saving` true, preventing the second attempt.
- Verified referenced paths against the audited tree and checked important symbols in retrieved source.

The following **were not** done: compiling the audited SHA locally, running a WASM game artifact, reproducing an image-loader/PhysFS crash in a browser, testing real iPhone/iPad/Android, measuring game FPS or live peak memory.

### CI evidence and its limits

[Upstream run 37071977062](https://github.com/SuperTux/supertux/actions/runs/37071977062), head `43376f2e6d8416065dc7ba1cfac5569fede7a470`, is a recent upstream PR run, **not** a run on the audit branch or exact audited SHA:

- [Release job 111053279632](https://github.com/SuperTux/supertux/actions/runs/37071977062/job/111053279632): configure/build/package/artifact upload passed.
- [Debug job 111053279369](https://github.com/SuperTux/supertux/actions/runs/37071977062/job/111053279369): configure/build/package passed.
- Both resolved emsdk `latest` to `6.0.11` according to the logs.
- Release dependency logs include PhysFS `3.2.0#1`, SDL3 `3.4.16#1`, SDL3_image `3.4.4#1`, and SDL3_ttf `3.2.2#1`.
- Both logged a 311 MB asset bundle warning and **undefined `IMG_Load_IO`** before `[100%] Built target supertux2`.
- [Release artifact 11256061030](https://github.com/SuperTux/supertux/actions/runs/37071977062/artifacts/11256061030) exists: uploaded archive size **286,015,455 bytes**. Archive size is not the raw data-package size or peak runtime memory.

The recursive trees show identical blobs for current and that CI head's `CMakeLists.txt`, `Emscripten.cmake`, `wasm.yml`, `vcpkg.json`, HTML template, `sdl_surface.cpp`, mobile/input controller files, and `screen_manager.cpp`. Other files differ, including editor-related startup code; this establishes useful evidence about the same dependency/image/input paths, not an exact-SHA build result.

At inspection, the fork had zero Actions runs, and querying runs for audited upstream SHA returned none. The audited commit includes `[ci skip]`. Push triggers cover `master` and tags, not `mobile-web-audit`; `workflow_dispatch` is available to validate that branch during implementation. No workflow was triggered in this audit.

Tracked asset sizes are dominated by music (150,023,184 bytes; 143.1 MiB) and images (120,497,704 bytes; 114.9 MiB), followed by levels (19.7 MiB) and fonts (19.2 MiB). These are source-tree totals, not transfer/decoded-memory measurements. A dependency-complete one-level package could reduce unused music/world content substantially without changing gameplay, but dynamic references must be included.

### Health verdict

**Appears compilable with a recent SDK/dependency set, but not proven playable.** Do not equate successful CI linkage with a healthy browser runtime.

Specific issues and assumptions:

1. **Highest priority:** missing `IMG_Load_IO`. Trace compile include paths and the final link command, resolve a consistent SDL3/image implementation, then remove the permissive undefined-symbol setting. A dummy stub would not fix image loading.
2. **Known startup/storage defects:** undeclared JS mount path, no hydration barrier, swallowed errors and stuck flush guard.
3. **Unpinned dependencies:** `emsdk latest`, runner vcpkg revision, and no `builtin-baseline` in `vcpkg.json` make reproduction sensitive to time. Cache key includes workflow hash but not explicit SDK/manifest/baseline identity.
4. **Provider mixing:** vcpkg SDL libraries and Emscripten SDL flags need explicit ownership. PNG/JPEG loading and TTF must use compatible SDL3 IO ABIs.
5. **Memory/download:** raw copied data includes test/editor/unused content; release install exclusions do not apply to the workflow's raw `rsync`.
6. **Packaging checks are weak:** artifact upload uses `if-no-files-found: ignore`; there is no browser boot or persistence smoke step.
7. **Historical flags:** current code no longer uses old `TOTAL_MEMORY`, `USE_PTHREADS`, `DEMANGLE_SUPPORT`, or `EXTRA_EXPORTED_RUNTIME_METHODS`. No obsolete-flag build failure was demonstrated for the current set. The observed undefined symbol is stronger evidence than speculation about flag names.
8. **SDL3 IO adapter:** `src/physfs/physfs_sdl.cpp::funcClose()` calls `PHYSFS_close()` but always returns `false`, which reports failure in SDL3's boolean callback contract. `funcRead()` also does not explicitly report EOF status. These are confirmed adapter details with untested runtime impact; inspect them while verifying real image/font IO rather than assuming the missing link symbol is the only migration concern.
9. **PhysFS:** current source still needs runtime verification, but the known `nullptr` argument regression has already been corrected. Mount-path mismatch and preference-dir failures are additional hypotheses to inspect if boot fails.

## 5. v0.6.3 vs v0.7.0 vs current

Tag resolutions:

- [v0.6.3](https://github.com/SuperTux/supertux/tree/v0.6.3): `c1ddb4f28c54de77f07aa9965231a91c0ed06708`.
- [v0.7.0](https://github.com/SuperTux/supertux/tree/v0.7.0): annotated tag dereferences to `c11dfb2a2aa429be3db8de96b7fca6a6236eddc3`.
- Current audited branch: `00673d1dfefeedf39aaf502ac0cfb1fd005174d6`.

| Area | v0.6.3 | v0.7.0 | Current |
| --- | --- | --- | --- |
| CMake organization | WASM block inside root `CMakeLists.txt` | `mk/cmake/SuperTux/Emscripten.cmake` | Same separate module |
| SDL | SDL2; SDL_ttf submodule patch | SDL2/SDL2 image formats; SDL_ttf integration | SDL3/image/ttf flags and IO API migration |
| CI | WASM job in historical `.github/workflows/other.yml`; SDK `1.40.1` plus allocator/JPEG patches | Separate `wasm.yml`; SDK `latest` | Same workflow text as v0.7.0 |
| Memory | `TOTAL_MEMORY=67108864`, growth enabled | Initial 128 MiB, maximum 512 MiB | Same memory limits |
| Threading | `USE_PTHREADS=1` | No pthread flag in WASM module | No project-enabled pthread flag |
| Runtime exports | `ccall/cwrap` plus legacy extra-runtime setting | Current-style runtime export setting; exception helper added | Same general export approach |
| PhysFS provider | System library if suitable, otherwise `external/physfs` submodule fallback | Required dependency through current package system/vcpkg | Same general dependency approach |
| PhysFS init argument | `argv[0]` | `nullptr` supplied by `Main::run` | `argv[0]` restored |
| Asset mount | Preload build data; mount compiled path | Same basic design | Same basic design |
| IDBFS mount | Literal browser user-directory path | JS `m_userdir` reference | Same bad JS reference, standardized macro |
| Hydration timing | Async population without explicit startup barrier | Same architectural risk | Same architectural risk |
| Main loop | `emscripten_set_main_loop` | Same browser callback design | Same; fixed-step simulation |
| Audio | Emscripten OpenAL, Vorbis/WAV | Same broad model | Same broad model |
| Browser shell | Load/save, resize, XHR/fullscreen support; no mobile readiness proof | Larger shell; no mobile viewport/lifecycle solution | HTML template byte-identical to v0.7.0 |
| Renderer | `Main::launch_game` already forces SDL for WASM | WASM SDL path | `Main::launch_game` forces SDL renderer |

Relevant history, with concrete windows:

- [1952b2a, 2024-03-03](https://github.com/SuperTux/supertux/commit/1952b2a31a7727a8ddecc17e70250039d7a32d11) changed literal JS IDBFS mount path to `m_userdir`. This identifies the persistence mount regression introduction; present in v0.7.0/current.
- [d02dee1, 2025-12-28](https://github.com/SuperTux/supertux/commit/d02dee1d3ee99e8b1e91c36575eeb8563d97961a), “Revive Android builds,” changed non-Android `PhysfsSubsystem(argv[0], ...)` to `PhysfsSubsystem(nullptr, ...)` as well. v0.7.0 contains this call.
- [Issue 3739](https://github.com/SuperTux/supertux/issues/3739) reports v0.7.0/nightly failing with `Couldn't initialize physfs: no error`. This is historical external runtime evidence, not reproduction on current Safari.
- [0c5f269, 2026-04-30](https://github.com/SuperTux/supertux/commit/0c5f269c5b5b6559aa27f03682b6dcfb52ffb244) restores `argv[0]`. The relevant argument-change window is December 2025 through this fix. It is a plausible explanation for the report, not proof of the precise dependency-internal failure.
- [bf7b23e, 2026-06-17](https://github.com/SuperTux/supertux/commit/bf7b23eaebf07812b212239758c860a83a07e4e2) adds `-fPIC` to Emscripten flags. A June 19 comment on issue 3739 reports restored working master WASM and backports needed for 0.7. That report predates SDL3.
- [06f5f54, 2026-07-19](https://github.com/SuperTux/supertux/commit/06f5f549f00282c2759a55c31de7fca55b15cdff) migrates SDL2 to SDL3. `SDL_RWops` becomes `SDL_IOStream`, image loading becomes `IMG_Load_IO`, gamepad/event/return-value APIs change, and haptics are temporarily disabled in `MobileController`.
- Current image-loader CI evidence falls after this migration. Treat the SDL3 image/provider transition as the investigation window, not a proven first-bad runtime commit.
- OpenAL header removal was reverted in February 2026 because it did not build; local Emscripten OpenAL headers are intentionally still present.

v0.6.3 is a historical build baseline, **not** evidence that old Safari touch, audio, storage, or threaded builds were reliable. Restore useful startup invariants without importing the old SDK patches/threading configuration.

## 6. Mobile Safari compatibility findings

Categories refer to impact on the desired mobile prototype. “Already handled” describes repository mechanisms, not verified Safari behavior.

| Category | Finding and evidence | Assessment / device validation |
| --- | --- | --- |
| **Blocker** | CI unresolved `IMG_Load_IO`; `SDLSurface::from_file` uses it; permissive linker hides failure | Investigate before mobile UI work. Actual audited artifact runtime still untested |
| **Blocker for durable saves** | Invalid IDBFS JS path and no hydration barrier | Confirmed source/narrow-check defect; playable memory-only fallback possible, durable progression unavailable until fixed |
| **Likely issue; must resolve for touch milestone** | Touch query before SDL; post-init requery is Android-only | Confirm order; browser discovery outcome needs runtime test |
| **Likely issue** | Shell lacks `meta viewport`, `viewport-fit=cover`, safe-area CSS, bounded game layout | Layout may use desktop viewport and controls may sit under browser/notch UI |
| **Likely issue** | `innerWidth/innerHeight` alone; no `visualViewport` or safe-area coordination | Test orientation, browser toolbar expansion/collapse, landscape and portrait |
| **Likely issue** | Browser backing-store/DPR policy is unspecified; no explicit high-density window flag | Inspect CSS size, SDL window size, actual drawable size and renderer scale separately; cap resolution based on measurements |
| **Already handled, partial** | Resizable SDL window; `Viewport` scaling/letterboxing; JS resize hook; SDL resize notifications | Check screen/HUD notifications and redundant allocation/resize loops; correct two-argument cwrap declaration |
| **Already handled, partial** | Current normal renderer is SDL3; optional GLES2 path exists | Log actual SDL backend and WebGL version/extensions; no requirement for WebGL2 established here |
| **Likely issue** | Context loss only alerts/requires reload | Test background/lock GPU loss; ensure progress is committed before reload |
| **Polish** | Fullscreen is keyboard-driven and errors are ignored | iPhone/iPad fullscreen capabilities differ by OS/API/context. Browser-tab play must work without fullscreen or orientation lock |
| **Likely issue** | No shell `touch-action` or explicit gesture/default policy | SDL's generated backend may prevent defaults; inspect it. Test page scroll, pinch, double-tap, text selection and edge gestures |
| **Already handled, partial** | C++ maps multiple finger IDs and unions actions | Simultaneous direction+jump/action supported architecturally; browser SDL delivery untested |
| **Likely issue** | No `SDL_EVENT_FINGER_CANCELED` branch or controller-wide touch-reset hook | Focus/orientation/gesture cancellation can leave fingers/actions held; verify SDL3 event delivery |
| **Likely issue** | Touch coordinates scale to logical drawing dimensions while mouse coordinates are raw; viewport offsets are not subtracted | Test letterboxing, CSS scaling, DPR and pointer/mouse fallback hit alignment |
| **Likely issue** | Non-Android touch hit areas are half Android's height scale; geometry depends on `draw()` | Controls may be too small or use stale geometry before first draw/after hidden controls/resize |
| **Likely issue** | No explicit AudioContext user-gesture startup/resume code in repository | Test tap-to-start sound, muted first load, interrupted audio, lock/background/return. Avoid relying on autoplay |
| **Already handled, partial** | Game pause calls sound/music pause and active-session SDL focus loss pauses | This is not a complete page visibility/storage/input lifecycle bridge |
| **Likely issue** | Shell relies on `unload`; lacks visibility/pagehide/pageshow handling | Mobile app switch or eviction can lose recent settings/progression and leave input/audio state stale |
| **Already handled, partial** | Fixed-step elapsed gap clamp | Limits catch-up after suspension; does not save/reset audio/input |
| **Likely issue** | Frame-callback `SDL_Delay`, texture/lightmap recreation on resize | Profile main-thread stalls and resize storms on device before changing simulation timing |
| **Blocker if device exceeds capacity** | Approximately 311 MiB preload, 128→512 MiB WASM limits, decoded textures/audio additional | No device memory measurement. Do not simply raise maximum memory; try complete minimal asset dependencies if full data cannot boot |
| **Likely issue** | Storage persist request and async writes can fail/deny/quota; exceptions are mostly swallowed | Verify private mode, denied persistence, quota/eviction and interrupted writes; offer honest save status |
| **Polish / later** | No PWA/service worker/offline boot mechanism | Normal HTTPS URL is enough for first prototype; PWA does not solve core Safari defects |
| **Already handled in current configuration** | No project-enabled WASM pthread/SAB flag | Validate final binary is unthreaded; avoid adding isolation requirements without benefit |

**Safari-specific unknowns:** generated SDL3/Emscripten behavior is not all in this repository. Source-level absence of a custom JS handler does not prove SDL fails to handle it. Validate actual touch events, WebGL backend, audio unlock and page suspension using a pinned artifact. Embedded frames and private browsing should follow primary standalone-tab validation.

## 7. Android / touch-control architecture

### Complete input trace

`Android SDLActivity / browser SDL backend` → `SDL_EVENT_FINGER_DOWN/MOTION/UP` → `ScreenManager::process_events()` → `MobileController::process_finger_*_event()` → `m_fingers[SDL_FingerID]` → `MobileController::update()` / `activate_widget_at_pos()` → `MobileController::apply(Controller&)` → `Controller::set_control(Control, bool)` → `Player::handle_input()` and other gameplay action consumers.

For browser controls, the first arrow is supplied by the generated SDL3 Emscripten backend. There are no repository-authored DOM touch buttons, Pointer Event handlers, or key-synthesizing control overlays.

`ScreenManager` owns one `MobileController`. It applies it to `InputManager::get_controller()`, whose default player is 0. This is suitable for the first single-phone player; it is not already a two-player touch layout.

Generic reusable pieces:

- Directional pad, jump, action, pause, item hit area, and optional developer buttons.
- Finger-ID map permitting multiple active fingers; each update unions their active widgets into a `Control` bitset.
- Action-edge tracking and standard controller output; touchscreen flag lets gameplay recognize touch input.
- C++ drawing using existing mobile PNGs; no Android graphics API dependency.
- Mouse fallback permits desktop layout checks.
- Configurable control scale and visibility fields.

Platform distinctions:

- Android's `MainActivity.java` only loads the game through SDLActivity and supplies locale/library/entry information.
- Android-specific C++ concerns are JNI/assets, post-SDL touch detection, larger touch rectangles, fullscreen/mobile option defaults.
- Haptic initialization/rumble code is disabled with `#if 0` during SDL3 migration. It is not required for web controls.
- The visible-controls/haptic settings in `OptionsMenu` are in an Android-only branch. The scale helper exists, but browser UI exposure/default policy needs a focused review.

### Reuse verdict: A

The control implementation already enters the generic SuperTux input system and can mostly be reused in WASM. It is not strongly Android-specific. Browser adaptation is needed around SDL startup, event/default behavior, layout/safe areas, cancellation and audio, but **a new browser button implementation is not required by this architecture**.

Minimum touch changes to consider after boot is healthy:

1. Enable mobile controls after SDL initialization or on first genuine touch event; use browser capability detection only for initial presentation, with a way to enable/disable on hybrid devices.
2. Verify SDL3 emits distinct finger IDs and cancellation events in Safari. Use its touch path first; add a thin Pointer Events adapter only if an actual SDL backend gap is demonstrated.
3. Centralize drawing and hit geometry so dimensions update even when graphics are hidden; account for viewport offsets/safe areas.
4. Clear fingers and owned actions on cancel/background/orientation loss. A per-source action mask would avoid mobile releases overriding a keyboard/gamepad hold, but implement only the needed input-ownership scope.
5. Validate direction+jump, direction+action, and direction+action+jump, including sliding between widgets and lifting one finger while another stays held.
6. Keep touch output as `Control` actions. Avoid fake keyboard events or parallel browser/C++ control ownership.

The finger dispatch currently calls widget hit tests even when controls are disabled, and finger-up always queues a mouse-up. These deserve menu/input regression tests while changing routing.

## 8. Local multiplayer architecture

### Players and controllers

`InputManager` owns a vector of `unique_ptr<Controller>`, starts with player 0, and uses `push_user()/pop_user()` for additional users. Default maximum is four; config can remove the limit. Keyboard mappings store `(player, Control)`; gamepads/joysticks maintain separate device-to-player maps.

`Player::Player(..., player_id)` stores `m_id` and a pointer to `InputManager::get_controller(player_id)`. `Player::handle_input()` reads that controller's hold/edge state. Controllers remain owned by InputManager, not by Player. Script control may temporarily replace the pointer via `Player::set_controller()` / `use_scripting_controller()`.

**Ownership:** `GameSession` owns/retains the active level through `m_level_storage`; `Level` owns sectors; sector `GameObjectManager` containers own player and other game objects. `Level::get_players()` gathers player pointers across sectors. Controllers and game objects have distinct lifetimes. Removing a player must release its object before destroying its controller, as the local menu already warns/implements.

### Creation, membership and input separation

`Level::initialize()` adds player 0 to the first sector, then additional players for configured users with a corresponding device or `m_uses_keyboard[id]`. This is an important future seam constraint: **pushing a remote controller slot alone does not ensure Player 2 is created**, because the local-device filter can skip it.

`GameControllerManager`/`JoystickManager` auto-manage controller slots on device changes. `GameSession::on_player_added(id)` expands status, creates a named Player and starts multiplayer spawn behavior. `on_player_removed(id)` removes the matching player. Local menus manage slots and assignments.

**Confirmed live-join inconsistency:** `GameControllerManager::on_controller_added()` calls `on_player_added()` when the current savegame **is** a title-screen save, whereas `JoystickManager::on_joystick_added()` requires it **not** to be a title-screen save. Thus gamepad and joystick hotplug paths do not currently share the same in-level spawn policy. Also, `Level::initialize()` increments `s_dummy_player_status` in its additional-player loop even when Players use the savegame's `player_status`. Verify status-vector/player-count invariants during Phase 5; this is a source-level concern, not a reproduced crash. Do not claim arbitrary live join/removal already works in browsers.

Per-player gameplay input is well separated. Session/menu/world-map control is largely player 0. Preserve that policy for the first host/join product; do not accidentally give remote input authority over pause, cheats or host progression.

### Camera, spawning, death, checkpoints and transitions

- **Camera:** `Camera::update()` switches to `update_scroll_normal_multiplayer()` when more than one Player exists. It computes coverage of living players, shared translation and zoom. Current `Sector::draw()` uses one shared view; old split-screen code is disabled with `#if 0`. Mobile screens can become difficult to read when players spread apart.
- **Spawn:** `Sector::activate(position/spawnpoint)` transfers all players into the sector and places them around the common spawn location. `GameSession::on_player_added()` calls `multiplayer_prepare_spawn()`, creating a dead/target-following state rather than an arbitrary independent starting point.
- **Death:** `Player::kill()` maintains death/dying/bonus state. In multiplayer, a dead player selects another active player as a target; action triggers `multiplayer_respawn()`, left/right changes the target. `GameSession::check_end_conditions()` restarts when all players are dead.
- **Checkpoints:** `Firefly::collision()` activates the shared session checkpoint using `GameSession::set_checkpoint_pos()`. `GameSession` maintains shared `m_spawnpoints` and `m_activated_checkpoint`; `restart_level(true)` chooses the appropriate respawn location. This is session-wide, not independent checkpoint tracks for each friend.
- **Sector/door transitions:** `Door::collision()` can call `respawn_with_fade()`; `GameSession::update()` activates the destination and transfers/reactivates all players together.
- **Level completion:** session end sequence, active/winning/dead conditions, timers and `finish()` are shared. Level restart reloads level data and resets all InputManager controllers.
- **World map:** `worldmap::Tux` holds the default controller 0, shared map position, shared level entry and selection. Two local Player instances are not two independent world-map avatars.
- **Persistence:** `PlayerStatus` holds `m_num_players`, per-player `bonus` and `m_item_pockets`, shared coins/tuxdolls and world-map/progression context. Savegame serializes those and Squirrel state. Controller/user slots are separately managed and are not automatically reconstructed just because a save contains multiple statuses.

### Smallest remote-input seam

A future remote input source should write only the remote player's ordinary controller slot:

`network queue` → input source ownership for player 1 → `Controller::set_control()` → existing Player 2.

`ScreenManager::process_events()` first calls `InputManager::update()` to establish previous state, processes local events, and `update_gamelogic()` then applies touch input before simulation. Consume queued remote masks at a defined point **after previous-state capture and before each simulation step**. JS callbacks should queue messages, not mutate the world asynchronously.

Do not use `CodeController` unchanged: its `press()` writes the underlying control array directly, and `update()` clears it each step. It is built for scripts, not sustained network input ownership or retransmission. A dedicated input-source/slot API around existing Controller is smaller and safer than changing Player movement.

Future necessary input-only architecture work:

- Remote slot registration that satisfies/replaces the local-device membership filter.
- Prevent local key/gamepad/hotplug paths from claiming or removing that slot.
- Sequence/tick ordering and pressed/released transitions.
- Neutralize remote held input on timeout/disconnect/pause.
- Preserve script-controller switching and controller lifetime rules.

### State requiring authoritative synchronization

| State family | Repository evidence / replication implication |
| --- | --- |
| Players | `Player` movement/velocity, collision pose, bonus, actions, timers, death, targets, grabbed objects, invincibility, pocket state |
| Enemies | `src/badguy/badguy.cpp` / `BadGuy` state/activation, motion and death; activation depends on camera and nearest player |
| Moving platforms | `src/object/platform.hpp` / `Platform` path/movement/speed and contact state |
| Blocks / changed tiles | `src/object/block.hpp`, `bonus_block.hpp`, `weak_block.hpp`, `tilemap.cpp`; hits, contents, destruction and tile changes |
| Collectibles | `src/object/coin.hpp` and shared PlayerStatus counts; removals/events must be authoritative |
| Power-ups | `src/object/powerup.hpp` / `PowerUp`, per-player bonus and pocket changes |
| Projectiles | `src/object/bullet.hpp` / `Bullet`; motion/type/lifetime and player owner reference |
| Scripted objects | `src/object/scripted_object.hpp`, Squirrel VM/scheduler, sector environment; host executes effects, client represents results |
| Bosses | `src/badguy/boss.hpp` / `Boss` plus derived runtime state; phase/health/trigger/sequence effects |
| Checkpoints | `Firefly`, session respawn stack/active checkpoint, checkpoint coin effects |
| Sector/level transitions | Session identity, spawn/fades, end sequence, level win/restart, timers and load acknowledgments |
| World-map state | Host map location, solved/perfect flags, statistics, scripts and level selection |
| Object lifetimes/references | `GameObjectManager::add_object/flush_game_objects/move_object`, removals, player/target references and sector identity |
| Simulation context | Tick/time, random outcomes, pause/speed, relevant script state and shared camera policy |

No complete runtime snapshot serializer was found in these paths. `GameObject::save()` and `save_state()` describe object settings/editor change tracking; Savegame is progression persistence. Neither is a ready-made network snapshot system.

`UIDGenerator` uses a process-local static magic counter and each manager's local object counter. IDs are useful for local lookup but not a shared peer identity scheme by themselves. Define explicit session/sector/entity identity later. Script coroutine continuations, dynamic references, transient timers and physics state make full-world snapshots more demanding than serializing level files.

## 9. Future browser networking feasibility

### Recommended model

Use **one friend's browser as the authoritative simulator**. The other browser sends only its input and renders replicated authoritative state. Both load compatible game/assets; the host owns collisions, AI, scripts, spawning, pickups, checkpoint decisions and transitions.

This fits current separated controllers and shared GameSession. It also avoids depending on deterministic duplicate simulations. Fixed steps alone do not guarantee that: time-seeded random defaults, process-local IDs, script scheduling, object creation order, camera-sensitive enemy activation and different viewport sizes are relevant divergence sources.

Start with remote input plus interpolation of authoritative snapshots/events. Accept measurable latency for the first proof. Add local-player prediction/reconciliation only after actual latency tests show it is needed. Prediction will need movement/collision state, platform context and authoritative acknowledgments; blindly running the guest's whole world will create duplicate pickups/script effects.

Rollback is not justified by this architecture or the two-friend co-op goal.

### Transport comparison

| Option | Fit and cost |
| --- | --- |
| **WebRTC DataChannel** | Good fit for browser-host/browser-guest. Can separate reliable session/events from freshness-oriented snapshots/inputs. Requires signaling, ICE/STUN and practical TURN fallback; browser host suspension breaks availability |
| **WebSockets** | Simpler API, reliable ordered transport and easy room signaling. Browser cannot accept inbound WebSocket server connections; peer-host gameplay needs a relay server. Packet delay/head-of-line behavior must be tested |
| **WebSocket signaling + DataChannels** | Preferred initial architecture if a signaling/TURN service is available. Room code, SDP/ICE exchange over WSS; gameplay over DataChannels |
| **WebSocket gameplay relay** | Viable first transport proof/fallback when infrastructure simplicity matters more than lowest latency. Keep host authoritative; relay is not automatically a game simulator |
| Dedicated authoritative server | Larger deployment/native-headless scope; useful later for reliable background hosting but not needed for two friends initially |

Transport details should remain outside Player/physics. Queue incoming data at a narrow JS↔WASM boundary, then consume it on the game thread. Large strings/per-object `ccall` traffic would become wasteful; choose bounded batched buffers during networking implementation. This audit does not define wire formats.

Minimum service needs even with no accounts:

- Private room creation/join and ephemeral room codes.
- Same version/content checks before gameplay.
- WSS signaling or relay, room membership, disconnect/expiry behavior.
- STUN and TURN coverage for real networks if using WebRTC; signaling alone cannot guarantee connectivity.
- Host/guest readiness and transition coordination.
- Browser foreground requirement and predictable host-pause/disconnect behavior.

The room code can be the initial access credential; no public server browser/matchmaking/account system is needed. Host-only world-map navigation and progression saves simplify the first product. Guest progression copy/export can be a later explicit policy.

**Main architectural cost:** a client representation/replication layer that does not rerun authoritative object/script interactions. This is larger than the transport or controller adapter and should be proven on a bounded level before general game support.

## 10. Blockers, risks, and shortest prototype path

### Required for “URL → iPhone → playable level with touch”

1. Resolve the missing SDL3 image-loader implementation and enable strict undefined-symbol failure. Validate boot through an actual level.
2. Verify current PhysFS init/search paths and preloaded package mount; retain existing `argv[0]` fix.
3. Correct storage startup and errors, or provide an explicit playable memory-only fallback when browser storage is unavailable. Do not let hydration race default config/profiles.
4. Serve a complete, version-consistent browser artifact over HTTPS; verify WASM/data requests and loading errors.
5. Add mobile viewport/canvas layout, safe-area protection, resize/orientation handling and deliberate browser gesture policy.
6. Enable the generic MobileController in browsers, ensure geometry/hit tests agree, and clear actions on touch cancellation/backgrounding.
7. Verify simultaneous movement/jump/action and touch-only menu entry/pause/resume on a real iPhone.
8. Verify audio begins after a user gesture and recovers from interruption. A silent diagnostic build can isolate boot but does not satisfy normal playable acceptance.
9. Measure cold boot/peak memory on the target phone. If the 311 MiB package prevents usable startup, a complete minimal-level asset package becomes required before declaring the prototype playable.

These are port/integration changes. They do not require changing Player physics, enemy behavior, level design, or networking.

### Recommended before calling the mobile browser version solid

- Pinned SDK/dependency baseline and cache identity; branch CI and browser boot checks.
- Successful settings/progression flush/reload tests; dirty/debounced flush with useful error status.
- Explicit visibility/pagehide save/input/audio policy; user-visible pause after backgrounding.
- Device-measured asset pruning/loading strategy, capped drawable resolution, resize coalescing, memory/FPS budget.
- Comfortable larger touch hit areas, settings accessible on web/hybrid devices, sensible portrait guidance.
- Error/loading UX that distinguishes download, storage, graphics and runtime failure.
- Android Chrome/iPad and secondary desktop browser regression checks.
- Save export/import validation and sensible treatment of storage eviction.

### Later

- PWA/offline installation, service worker cache/version management.
- Haptics, customizable layouts/accessibility refinements, fullscreen enhancement.
- Full in-level resume after process eviction; existing saves do not provide this.
- Browser local multiplayer refinements, remote-source ownership/registration.
- Rooms/signaling/transport, state replication, prediction, reconnect and host migration.
- Dedicated hosting/account/matchmaking/public-server features are outside the initial goal.

### Most material risks

| Risk | Mitigation / evidence required |
| --- | --- |
| Green build hides runtime import failure | Strict linking and actual image/level boot smoke test |
| Full package exceeds phone resources | Measure first; complete minimal package if needed; no blind memory-limit increase |
| Save initialization corrupts or loses progress | Hydration barrier, serialized flush, denied-storage fallback and reload tests |
| SDL backend differs from source assumptions | Inspect generated SDL3 glue and real event/backend/audio behavior |
| Small local input patch accidentally alters gameplay | Keep adaptation in input/lifecycle/shell; test existing keyboard/gamepad actions |
| Online feature expands into whole-engine rewrite | Input-source proof, then bounded replication proof before broad object coverage |
| Mobile host backgrounds/locks | Explicit session pause/disconnect policy; browser host cannot promise background uptime |

## 11. Recommended architecture

For mobile web, retain the current C++ game, SDL3 renderer and generic Controller/MobileController path. Use the HTML shell for browser-owned concerns: loading, filesystem hydration, safe-area viewport, user gesture/audio, page lifecycle and browser capability/status. Use small exported integration hooks only when C++ needs a browser event. Keep one owner for touch actions.

Preserve an unthreaded baseline. Select a consistent SDL3 dependency provider and fail on missing symbols. Preserve existing save/profile formats and user-directory compatibility while moving IDBFS readiness before main. Maintain the real gameplay timestep and existing single-player/local-co-op rules.

For future online play, add transport-independent input-source ownership and a host-authoritative replication boundary around session/sector/object state. Use WebSocket room signaling plus WebRTC DataChannels if service support is available; a relay fallback remains reasonable. Host world-map and save authority are appropriate for the first private co-op release. Delay prediction until latency evidence justifies its cost.

## 12. Proposed implementation phases

Each phase should be a small reviewable change or a tightly scoped sequence of changes. Later-phase files are likely touch points, not authorization to modify them during this audit.

### Phase 1 — Modern WASM boot/runtime reliability

**Objective:** produce a reproducible, strictly linked current-source artifact that loads images, initializes PhysFS/storage safely, reaches a level in a desktop browser, and reaches the game on Safari for diagnostic verification. Touch polish is not a prerequisite for this boot milestone.

**Likely files:** `CMakeLists.txt`; `mk/cmake/SuperTux/Emscripten.cmake`; `.github/workflows/wasm.yml`; `vcpkg.json` if needed; `mk/emscripten/template.html.in`; `src/supertux/main.cpp`. Only touch `src/video/sdl_surface.cpp` or `src/physfs/physfs_sdl.cpp` if evidence requires an ABI adapter rather than a link fix.

**Boundaries:** pin/record toolchain, fix image/provider linkage, strict symbols, verify asset manifest/mount, move browser storage hydration before main, explicit storage failure fallback, fix export signatures/startup error reporting. Preserve save path/format and desktop/Android initialization.

**Acceptance:** Release and Debug link without unresolved symbols; cold browser boot loads an image, fonts and one level; no PhysFS errors; save hydration precedes consumers; persisted settings/save fixture reload or clearly reported memory-only mode; complete artifact is server-ready. Real Safari boot is recorded if a device is available; if unavailable it remains an explicit release gate.

**Risks:** mixed SDL provider/ABI, main/preRun ordering, hardcoded asset paths, dependency cache changes, full-package phone memory. Boot error diagnosis precedes unrelated improvements.

**Do not change:** gameplay, multiplayer, renderer design, controls, level contents, save schema, threading or upstream. Detailed handoff is below.

### Phase 2 — Mobile Safari shell, lifecycle and audio

**Objective:** a usable mobile canvas with predictable safe areas, resizing, gesture/audio startup and background return.

**Likely files:** HTML template; `src/port/emscripten.hpp`; `src/supertux/screen_manager.cpp`; `src/video/sdlbase_video_system.cpp`; audio integration only if generated OpenAL behavior requires it.

**Boundaries:** viewport/meta/CSS, safe-area information, resize coalescing and correct dimensions, tap-to-start/audio resume, page visibility/pause/input-reset bridge and flush request. Capability-aware fullscreen fallback; standalone tab works without fullscreen.

**Acceptance:** iPhone/iPad Safari portrait↔landscape and toolbar changes preserve canvas/control space; first tap unlocks sound; background/lock/resume neither advances unexpectedly nor leaves sound/input stuck; no resize crash or repeated uncontrolled allocations. Desktop and Android layout remain functional.

**Risks:** visualViewport differences, duplicate SDL/browser events, DPR cost, timing of audio gesture. Test on actual device.

**Do not change:** simulation timestep/physics, art/assets, game rules, online networking or PWA infrastructure.

### Phase 3 — Reuse generic touch controls; deliver mobile prototype

**Objective:** current game loads from a URL and one level can be entered and played using only touch on an iPhone.

**Likely files:** `src/supertux/main.cpp` and `src/supertux/gameconfig.cpp`; `src/control/mobile_controller.cpp` and `src/control/mobile_controller.hpp`; `src/supertux/screen_manager.cpp`; HTML template; options menu only for necessary touch enable/scale.

**Boundaries:** post-SDL/first-touch activation, safe-area/viewport-aligned geometry, cancellation/reset, comfortable hit targets, existing `Control` output. SDL touch is the first choice; browser adapter only for a reproduced backend gap.

**Acceptance:** touch-only menus and level entry; hold direction+jump/action concurrently, release one finger independently, slide between buttons, pause/resume, rotate/background without stuck actions. Real iPhone Safari is mandatory; Android Chrome and iPad smoke checks. Run a short representative level, not only a title screen.

**Risks:** pre-init touch discovery, logical/physical coordinates, touch/mouse synthesis, action ownership, phone memory. If package loading prevents device acceptance, add a separate minimal complete asset-package step before declaring this phase complete.

**Do not change:** Player movement, enemies, level design, multiplayer rules, networking; reuse existing PNGs.

### Phase 4 — Mobile performance and save durability

**Objective:** meet measured resource/loading/frame targets and preserve committed progression across routine mobile interruptions.

**Likely files:** WASM workflow/asset packaging; Emscripten flags; HTML storage helpers; `screen_manager.cpp`; limited save/config/Writer integration points.

**Boundaries:** dirty/debounced flush, error/retry/status, size reporting, dependency-complete asset grouping/pruning, measured DPR/texture/resource policy. Agree on device budgets from baseline data; do not invent success thresholds before measurement.

**Acceptance:** cold/repeat load size/time and memory measured on chosen minimum devices; stable play through a longer session; save/config reload after background/close; denied/private/quota behavior documented; no silent lost committed saves; no per-frame idle filesystem scans.

**Risks:** dynamic asset/script references, font/audio decoded memory, storage eviction. Existing save formats lack full in-level resume.

**Do not change:** gameplay content/rules, full-world save serialization, online play, speculative threading.

### Phase 5 — Browser local multiplayer validation

**Objective:** validate existing two-player gameplay in one browser before adding remote input.

**Likely files:** test/documentation around `InputManager`, local multiplayer menus, `Level`, `GameSession`, `Player` and `Camera`; fixes only for confirmed port-related problems.

**Boundaries:** two controller slots, keyboard/gamepad device mappings, join/removal, shared camera, deaths/respawn, checkpoint/door/level/world-map progression. One mobile touch player plus another supported local device is sufficient; two touch layouts are not required.

**Acceptance:** both players controlled independently; completion, single/all-player death, checkpoint restart and sector transition behave as current local co-op intends; controller removal does not leave dangling pointers; save reload verified.

**Risks:** device membership filters, status counts and shared-view zoom; Safari gamepad device support.

**Do not change:** local co-op rules, introduce remote transport or redesign camera gameplay.

### Phase 6 — Remote input-source proof

**Objective:** prove a remote-owned controller slot can drive Player 2 without changing Player physics.

**Likely files:** `src/control/input_manager.cpp` and `src/control/input_manager.hpp`; focused local-device mapping integration; `Level::initialize` membership check; `screen_manager.cpp`; narrow browser queue/binding.

**Boundaries:** input-source registration/ownership, tick/edge handling, neutralization and lifecycle. Begin with an in-process/replay input source and then a two-browser diagnostic transport. Guest display may remain a diagnostic view at this phase; input delivery is not full co-op.

**Acceptance:** host Player 2 receives only remote source; local hotplug cannot steal/remove it; disconnect clears held controls; scripting/controller lifetime remains correct.

**Risks:** pressed/released sequencing, slot creation filters, removal/lifetime ordering.

**Do not change:** world replication, matchmaking, save schema, Player movement.

### Phase 7 — Bounded authoritative replication proof

**Objective:** a host and guest display a consistent two-player state in one selected level.

**Likely files:** new replication modules near session/sector/object management; limited state accessors; HTML networking bridge; selected object representation adapters.

**Boundaries:** stable session/entity IDs, authoritative players plus representative enemy/platform/collectible/projectile/block state, spawn/despawn and reliable critical events; guest interpolation and host-only simulation/script execution.

**Acceptance:** guest sees both players and tested objects agree with host through collisions/pickups/death/checkpoint/transition cases under injected latency/loss; incompatible content rejected; input alone is not treated as synchronization.

**Risks:** missing transient state, camera-dependent activation, script side effects, ownership references, snapshot volume.

**Do not change:** generalized rollback, every enemy/boss at once, game balance, host migration or accounts.

### Phase 8 — Private rooms and expanded co-op coverage

**Objective:** two friends can host/join via a private room code and play the supported campaign scope reliably.

**Likely files:** transport/room JS and service code, co-op UI, replication coverage, connection lifecycle.

**Boundaries:** WSS signaling, WebRTC ICE/STUN/TURN or relay fallback, connection errors, host pause/exit, version checks, transitions/load acknowledgment and host saves. Expand script/boss/level state support incrementally.

**Acceptance:** separate real networks including NAT/TURN cases; iPhone host and guest roles; predictable background/disconnect behavior; no duplicate pickup/progression; documented supported content and host-save policy.

**Risks:** mobile suspension, NAT traversal, service availability, unsupported scripted state and latency.

**Do not change:** public matchmaking/server browser, accounts, rollback, dedicated authoritative hosting or automatic host migration.

## 13. Open questions requiring real-device testing

1. Can a strictly linked current artifact boot on the intended minimum iPhone/iOS version with the full package? What are cold load time, peak JS/WASM/GPU memory, tab-eviction behavior and thermal frame stability?
2. Which SDL3 renderer actually runs in Safari and Android Chrome? Does it use WebGL, and which version/extensions/texture limits matter?
3. Does current `IMG_Load_IO` fail at the first icon/image? Which SDL3/image library/header provider fixes it without duplicate SDL runtimes?
4. Does the current SDK expose a touch device only after first contact? Does corrected post-init discovery suffice, or must first-touch activation be explicit?
5. Does SDL3 deliver finger cancel and reliable independent IDs during three-finger play, pinch, edge gestures, orientation and app switching?
6. Do drawing/hit tests agree under letterboxing, visual viewport changes, safe areas, DPR and CSS scaling?
7. Does generated OpenAL unlock from a touch automatically? How does it recover from lock, interruption, backgrounding, silent mode and repeated resume?
8. Does page visibility produce SDL focus events consistently? Which explicit lifecycle signals avoid duplicate pause/resume and clear all input?
9. Does corrected hydration/flush persist config, bindings, profiles and completion in normal Safari, private mode and denied/quota conditions? What survives forced tab eviction after a successful save?
10. Does optional fullscreen work on supported iPhone/iPad versions? The prototype must remain playable without it.
11. Are local gamepads usable on intended browser/device combinations, and do player slots survive device reconnect predictably?
12. For later co-op, what RTT/jitter is tolerable without prediction, and how should both screens present the shared camera at different aspect ratios?
13. What minimum device/OS set, prototype asset scope and load/performance budgets should become release acceptance criteria? None is supplied by the repository.

## Phase 1 Implementation Handoff

The following is copy-paste-ready for a subsequent implementation task. **Do not execute it as part of this audit.**

> Implement only Phase 1: current SuperTux WASM boot/runtime reliability. Start from the committed audit on `jbbejena/supertux`; the source audited was `00673d1dfefeedf39aaf502ac0cfb1fd005174d6`. Use a separate implementation branch based on the audit branch unless the next task specifies another branch. Do not merge or modify upstream. Do not implement touch UI, gameplay changes, multiplayer, PWA support or networking.
>
> Deliver the smallest working modern browser artifact: strict linkage, images/fonts/one level loading, correct PhysFS mounts, deterministic browser storage readiness, and visible storage/boot failures. Keep the existing save directory and save format.
>
> **Known evidence to act on:** both upstream CI jobs in run `37071977062` compile but warn `undefined symbol: IMG_Load_IO`. Current `SDLSurface::from_file()` calls it. `Emscripten.cmake` disables undefined-symbol errors. CMake mixes Emscripten SDL3 ports and vcpkg SDL3 dependencies. The old `PHYSFS_init(nullptr)` bug is already fixed; retain `argv[0]`. Browser `FS.mount(IDBFS, {}, m_userdir)` uses an undeclared JS identifier, startup never waits for `syncfs(true)`, and `supertux2_syncfs` leaves its guard stuck after a synchronous throw.
>
> **Primary files:** `CMakeLists.txt`; `mk/cmake/SuperTux/Emscripten.cmake`; `.github/workflows/wasm.yml`; `vcpkg.json` if provider/baseline changes require it; `mk/emscripten/template.html.in`; `src/supertux/main.cpp`. Inspect `src/video/sdl_surface.cpp` and `src/physfs/physfs_sdl.cpp` without changing image/gameplay semantics unless the actual ABI requires an adapter.
>
> **Work sequence:**
>
> 1. Verify implementation-branch base and clean state. Check out recursive submodules. Record SDK, compiler and vcpkg revision. Use the existing Release workflow invocation as the baseline; it most recently resolved SDK `6.0.11` in the inspected logs. Pin a real available compatible SDK and dependency revision/baseline rather than assuming `latest` reproduces those results.
> 2. Build once with undefined-symbol errors enabled. Capture the actual compile include paths and final link command. Determine which SDL3 headers and image library are selected. Resolve `IMG_Load_IO` with a compatible real SDL3_image implementation, including PNG/JPEG and PhysFS-backed SDL3 IO. Prefer a single consistent SDL3 provider for SDL3/image/ttf, using the existing vcpkg dependencies if suitable. Do not paper over the failure with a stub, copied SDL2 symbol name, or continued permissive linkage. Keep changes target-scoped and preserve native builds.
> 3. Verify all final symbols resolve and the artifact contains expected JS/WASM/data/shell assets. Make missing artifacts fail CI. Include SDK/baseline/manifest identity in dependency cache strategy. Do not restore historical pthread flags, change exception strategy, raise memory limits or re-enable LTO to address unrelated failures.
> 4. Inspect generated file-packager destinations. Confirm the exact compiled `BUILD_CONFIG_DATA_DIR` mount exists and contains `credits.stxt`, fonts, images, scripts and the chosen level before PhysFS reads. Preserve current mount precedence. Add useful boot diagnostics only where needed; if a mount fails, show/log the actual path/error rather than treating download completion as success.
> 5. Establish one browser user-directory initializer before main. Create `/home/web_user/.local/share/supertux2/`, mount IDBFS once, register an Emscripten run dependency, populate with `FS.syncfs(true, callback)`, and release the dependency only after success or an explicit handled memory-only fallback. Export required FS/IDBFS/run-dependency helpers if the chosen SDK's generated scope requires them. No synchronous busy-wait or new Asyncify dependency is necessary for a pre-main JS barrier.
> 6. Remove the duplicate/broken browser-side mount in `PhysfsSubsystem::find_mount_userdir()` once the pre-main initializer owns it. C++ should set/mount the same user directory for PhysFS after readiness. Preserve native/Android behavior. Review pref-dir calls that precede the browser override and guard null results or bypass irrelevant native lookup in the browser path if required by the runtime.
> 7. Keep localStorage config compatibility, with a documented fallback precedence after IndexedDB hydration. Do not change save/profile schema or silently overwrite a hydrated newer config with stale fallback data. Ensure all hydration success/error paths release the startup dependency and boot remains possible with unavailable storage.
> 8. Fix the sync helper's synchronous-exception guard and serialize flush requests; provide useful failure reporting and retry capability. Do not launch outward sync while initial population is unfinished. A small bound/debounce is sufficient if required here; broader dirty-write optimization is Phase 4. Do not claim in-level resume or mobile `unload` durability.
> 9. Correct `cwrap('set_resolution', ...)` to declare both numeric arguments and use appropriate void return declarations. Ensure initialization and resize only call ready C++/video objects. Do not treat `init_emscripten/supertux_onready` as game-ready.
> 10. Serve the complete artifact through HTTP/HTTPS with correct MIME and consistent cache/versioning. Perform a real game boot smoke: first image/icon, font, title/menu and one level. Inspect console/network errors and actual renderer name. Use desktop browser keyboard input for Phase 1 level play; record actual Safari boot if device access is available. The full 311 MiB package is a known resource risk: measure failure before adding an asset-scoping change or attributing it to PhysFS.
> 11. Run focused storage checks: new origin, existing persisted fixture, denied/unavailable IndexedDB, sync exception/error, settings save/reload and a committed progression fixture reload. No hydration race, permanently stuck spinner/guard, unresolved imports, or silently lost committed fixture is acceptable. Test both Release and Debug linkage; desktop/Android compile-sensitive changes require appropriate configuration verification.
>
> **Baseline configure/build shape** (adjust concrete SDK/vcpkg paths, keep output outside source):
>
> ```sh
> source /path/to/emsdk/emsdk_env.sh
> emcmake cmake -S . -B build-wasm \
>   -DCMAKE_BUILD_TYPE=Release -DWARNINGS=ON \
>   -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
>   -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
>   -DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
>   -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
>   -DVCPKG_MANIFEST_FEATURES=core
> rsync -a data/ build-wasm/data/
> cmake --build build-wasm --parallel 2
> cp build-wasm/template.html build-wasm/supertux2.html
> ```
>
> The workflow's artifact-packaging step remains the authoritative packaging baseline. A plain HTTP development server is enough for the unthreaded desktop smoke test; use HTTPS for deployed device validation. Workflow-dispatch can target the implementation branch; pushing `mobile-web-audit` alone does not trigger the current master-only push filter.
>
> **Done means:** strictly linked Release/Debug artifacts; one actual browser level starts and accepts keyboard input; PhysFS reads expected data; storage is hydrated before consumers or explicit fallback is shown; persisted fixtures survive reload; exact toolchain/artifact size and browser evidence are recorded. Do not report iPhone touch play as completed in Phase 1. If Safari cannot be tested, list it as an outstanding acceptance gate. Phase 2/3 deliver the full requested mobile-touch prototype.
>
> **Final handoff back to the user:** list implementation commit(s), artifact location, pinned versions, actual browser/device tests, remaining mobile memory/Safari gates, and files changed. Stop after Phase 1.