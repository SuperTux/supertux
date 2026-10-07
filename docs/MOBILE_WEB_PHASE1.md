# Mobile web Phase 1: modern WASM boot and runtime reliability

Implementation/validation date: 2026-10-03. Repository: `jbbejena/supertux`. Branch: `mobile-web-phase1-wasm-reliability`, based on audit commit `a3703205d7f8bead1335da887c95f62c545b14f8`. The [architecture audit](MOBILE_WEB_AUDIT.md) remains the planning baseline.

The current game now links strictly with real SDL3 image/font libraries, hydrates browser saves before C++ startup, boots its menu, and plays Welcome to Antarctica with keyboard input in desktop Chromium. A persisted music-volume setting was read by C++ after reload and serialized again as 37. A save-format progression fixture also survived reload byte-for-byte.

**Limits:** this is desktop browser evidence, not Safari/iPhone validation or a full campaign test. Debug runs expose existing upstream undefined behavior in player/library code. No gameplay, touch, multiplayer, networking, memory-limit, exception-strategy, or upstream changes were made. Nothing was merged.

## Dependency repair and reproduction

Pinned tools/dependencies:

| Component | Validated version/revision |
| --- | --- |
| Emscripten | 6.0.11, compiler revision `a0014542110d6078c3a1a7941fa1ddb3a2281f16` |
| Local build image | `emscripten/emsdk:6.0.11`, digest `sha256:cdefec943f04fd4b2b2fe23b0a1a346be9fc560ef5784a83faa27dd351381372` |
| vcpkg | `c748cb44f2a435fcf015c35225c9d5545fe0021c` |
| SDL3 | 3.4.16#1 |
| SDL3_image | 3.4.4#1, PNG/JPEG features |
| SDL3_ttf | 3.2.2#1, HarfBuzz feature |
| PhysFS | 3.2.0#1 |
| Local CMake / Ninja | 3.28.3 / 1.11.1 |
| Browser | Chromium 151.0.7922.173, headless, SwiftShader |
| Browser test tools | Playwright 1.62.0; CI pins Pillow 11.3.0 |

The original CMake configuration was run with the pinned tools. A strict minimal link using the old `USE_SDL=3`, `USE_SDL_IMAGE=3`, and `USE_SDL_TTF=3` flags reproduced `wasm-ld: ... undefined symbol: IMG_Load_IO`.

SDK 6.0.11 has SDL3 3.4.2 and SDL3_ttf ports, but **no SDL3_image port selected by `USE_SDL_IMAGE=3`**. Its SDL2 image port activates only for value 2. The former setup combined SDK SDL/font dependencies with installed vcpkg headers/libraries and did not link the required image implementation. This is more precise than assuming all three flags were equivalent SDL3 providers.

[CMakeLists.txt](../CMakeLists.txt) now resolves and links SDL3, SDL3_image, SDL3_ttf, Ogg, Vorbis, and VorbisFile through ordinary imported CMake targets on WASM as well as native builds. The Emscripten SDL/Vorbis port flags were removed. The pinned `tinygettext` submodule injects `-sUSE_SDL=3`; the parent removes that exact compile option and links its SDL iconv support to the same imported SDL3 target. The submodule itself is unchanged.

Actual Ninja dependency records for `src/video/sdl_surface.cpp.o` point to the vcpkg installation's `include/SDL3/` and `include/SDL3_image/` headers. The final link includes that installation's `libSDL3.a`, `libSDL3_image.a`, and `libSDL3_ttf.a`. `llvm-nm` confirms real definitions of `IMG_Load_IO` and `TTF_OpenFont*` in the image/font archives. Menu and level images/fonts render through the existing PhysFS-backed SDL3 IO adapter; neither `IMG_Load_IO` nor image loading was stubbed or replaced.

[Emscripten.cmake](../mk/cmake/SuperTux/Emscripten.cmake) now enables `ERROR_ON_UNDEFINED_SYMBOLS=1`. Both full Release and Debug game builds completed successfully. Browser preload plugins were removed: the real SDL3_image decoder reads file bytes, so duplicating browser image/audio predecoding is unnecessary. The complete asset set is retained.

The first browser run revealed a further defect: the old CMake code appended the linker flags twice. Adding `--pre-js` to that list consequently evaluated the storage initializer twice and caused concurrent mounts. Link flags now appear once; the generated Release JavaScript contains one storage initializer. The initializer also guards repeated execution of its own pre-run callback.

## Filesystem and runtime startup

Startup sequence:

1. The SDK loads the packaged `.data` files under the compiled `BUILD_CONFIG_DATA_DIR` path.
2. [storage.js](../mk/emscripten/storage.js), included with `--pre-js`, registers the `supertux-user-storage` run dependency.
3. It creates `/home/web_user/.local/share/supertux2/`, mounts IDBFS once, and runs `FS.syncfs(true, callback)`.
4. After hydration, an existing config wins over the legacy `localStorage.supertux2_config` backup. The backup imports only if no config exists.
5. The callback releases the dependency, allowing main/config/profile/save consumers to run.
6. [PhysfsSubsystem](../src/supertux/main.cpp) retains `PHYSFS_init(argv[0])`, mounts the packaged data, checks essential data directories/files, and mounts the already initialized user directory with the existing write-directory/search precedence.

The broken C++ mount referencing a JavaScript `m_userdir` identifier was removed. Browser startup bypasses native pref-directory discovery/migration; native and Android code remains in the other preprocessor branch. There is no Asyncify addition or busy wait.

Mount/hydration errors release startup through an explicit memory-only fallback. A partially populated IDBFS mount is unmounted before falling back. A fixed browser status banner remains inside the visible viewport when the canvas fills the window. Outward IDBFS flushes are disabled in memory mode; localStorage config backup is still attempted. An unrecoverable failure to create the fallback directory aborts startup with an error instead of silently continuing.

`Module.supertuxStorage.flush()` returns a promise indicating commit success. Explicit requests serialize, including another request arriving during an active flush. Synchronous throws and callback errors release the guard and permit retry. The existing per-frame hook is throttled to at most one new request per second after completion, or a 30-second automatic retry delay after failure; explicit requests bypass that delay. A successful retry clears the flush-error banner. This is not a general dirty-save implementation or a guarantee against tab eviction/unload loss.

[template.html.in](../mk/emscripten/template.html.in) now declares both `set_resolution` parameters and uses `null` for void cwrap/ccall return types. Runtime initialization binds exports; the [ScreenManager::run](../src/supertux/screen_manager.cpp) signal announces initialized game screens/video. Resize waits for that signal. PhysFS/boot failures remain visible instead of appearing as completed downloads. Optional shell callbacks are guarded for other embeddings.

## Validation and its boundaries

[tests/wasm/storage.test.cjs](../tests/wasm/storage.test.cjs) executes the production initializer against controlled filesystem callbacks. Ten checks cover delayed hydration, single mounting, stale backup precedence, legacy import, mount failure, synchronous populate failure, populate callback failure, localStorage denial, serialized flushes, guard recovery, and per-frame throttling.

[tests/wasm/browser_smoke.py](../tests/wasm/browser_smoke.py) serves the **real full game artifact** over HTTP and uses actual Chromium/IndexedDB. Both Release and Debug were compiled and exercised. The test:

- boots a new origin and captures a rendered main menu with images/fonts;
- verifies one hydration message before main;
- saves a valid config fixture with music volume 37 and a valid-format `profile1/phase1-fixture.stsg`;
- seeds a conflicting localStorage volume 12, reloads, calls C++ `save_config`, and verifies that 37 survives C++ parsing/serialization;
- verifies identical progression-fixture bytes after reload;
- forces synchronous and callback flush failures inside the real runtime, restores the filesystem function, and successfully commits again;
- loads `levels/world1/welcome_antarctica.stl` using the game's existing CLI path, dismisses its intro, and sends ordinary keyboard movement/jump/pause events;
- measures player positions through existing read-only scripting methods in the developer console, without adding compiled gameplay exports;
- denies IndexedDB in a new context and verifies successful memory-mode menu boot with an on-screen warning;
- fails HTTP errors, uncaught JavaScript, unresolved symbols, fatal game errors, and new sanitizer diagnostics.

The fixture-seeding page deliberately disarms unload writes so its running default C++ config cannot replace the seed. Successful explicit flush is the commit point. The progression fixture validates filesystem persistence, **not** completion of a campaign level or restoration of an in-level simulation. The music-volume fixture verifies actual C++ consumption. Keyboard tests use real key events and a read-only position/input observer prepared through the existing console while the ordinary pause menu stops enemies. Fresh observer samples measure jump height, landing, and more than 50 pixels of movement while the corresponding key is held. Each observation has a ten-second deadline, and held keys are released even on failure. This avoids assuming a wall-clock delay advances enough Debug simulation or repeatedly typing queries during live play.

Both browsers logged `SDL_Renderer: opengles2`. This is software GPU validation through SwiftShader, not a hardware GPU benchmark or an audible audio test. No real Safari, iPhone, iPad, Android browser, native, or Android application build was run. No full desktop target was rebuilt.

The data package remains **326,676,452 bytes (311.54 MiB)**. Release WASM is approximately 8.76 MiB; instrumented Debug WASM is approximately 149.62 MiB. Initial/maximum WASM memory remain 128/512 MiB. Package, decoded texture, JavaScript, GPU, and browser overhead remain a real-device acceptance risk.

## Existing Debug sanitizer findings

Debug completed the functional browser checks but is **not sanitizer-clean**. Its diagnostics concern source files unchanged from the audit branch:

| Observed diagnostic | Repository evidence / follow-up |
| --- | --- |
| Invalid bool in libc++ `__utility/swap.h:43` during scripting startup | `external/simplesquirrel/source/object.cpp`: the move constructor initializes `vm` but not `weak` before swapping. This is a likely origin; no symbolized sanitizer call stack was obtained. |
| Null `Climbable` reference at `src/object/player.cpp:325` | `Player::move_to_sector()` calls `stop_climbing(*m_climbing)` while the constructor initializes `m_climbing` to null. |
| Invalid bool at `src/object/player.cpp:2264` | `m_reset_action` is declared and read but absent from the Player constructor initializer list. |
| Null-pointer arithmetic in `external/obstack/obstack.c:143/236/263` and `src/util/obstackpp.hpp:24` | The existing `__PTR_ALIGN` macro aligns through arithmetic with a null base on this target. |

These are recorded defects, not proof that Release avoids them. They deserve a separate narrowly scoped source/library repair before treating the whole game as runtime-clean. Player code, obstack, and the SimpleSquirrel submodule were deliberately left unchanged to honor the gameplay boundary.

The browser test defaults to failing **every** sanitizer diagnostic. For Debug only, CI passes `--record-known-ub`: an exact site/message allowlist records these observations in `report.json` and emits visible GitHub warning annotations. Unexpected sites/messages still fail. Undefined-symbol checks and UBSan/SAFE_HEAP/ASSERTIONS remain enabled. A successful Debug job means boot/input/storage checks pass with the listed existing defects; it does not mean undefined behavior is fixed.

One Debug run emitted the SDK's generic requestAnimationFrame advice around shutdown. Inspection of SDK 6.0.11 `libeventloop.js` confirms the existing non-positive main-loop FPS selects rAF. The game-loop timing and simulation were not changed.

## Build and test commands

Check out the implementation branch and recursive submodules. Install/activate Emscripten 6.0.11 and bootstrap vcpkg at the revision above. Use a fresh output directory and the existing core manifest dependencies:

```sh
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
  -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON
rsync -a data/ build-wasm/data/
cmake --build build-wasm --parallel 4
cp build-wasm/template.html build-wasm/supertux2.html
node tests/wasm/storage.test.cjs
python -m pip install playwright==1.62.0 Pillow==11.3.0
python -m playwright install chromium
python tests/wasm/browser_smoke.py build-wasm --output build-wasm/browser-evidence
```

Repeat in a separate Debug directory with `-DCMAKE_BUILD_TYPE=Debug`. For a diagnostic functional run, append `--record-known-ub` to the browser command; omit it to enforce a completely clean sanitizer gate. An installed desktop Chromium can be selected with `--chromium /path/to/chromium`. For an artifact-only directory without `config.h`, pass `--data-dir` with the compiled virtual data path.

For manual play, serve the complete directory, for example `python -m http.server 8000 --directory build-wasm`, then open `http://localhost:8000/supertux2.html`. Keep HTML, JS, WASM, data, icon, and shell PNGs from one build together. Do not validate while rebuilding files being served. Deploy with correct WASM MIME and consistent cache/versioning; this phase did not deploy a public site.

## Focused CI and artifacts

[.github/workflows/wasm.yml](../.github/workflows/wasm.yml) pins SDK/vcpkg revisions, explicitly disables native manifest defaults for WASM, keys dependencies on SDK/revision/manifest/workflow, uses bounded parallelism, checks required artifacts, and runs storage plus real browser tests for both configurations. Missing artifacts now fail. Validation logs, compile commands, JSON reports, and screenshots are archived separately. The implementation branch has a push trigger. The existing master-only production upload condition is preserved; this phase does not publish or merge.

At local validation time, the fork's Actions API returned no workflow/run records. Consequently **no hosted GitHub Actions success is claimed**. The equivalent full local build/browser checks ran. If hosted runs remain absent after the draft PR, the repository owner must enable/register the fork workflows in GitHub Actions and dispatch WebAssembly on `mobile-web-phase1-wasm-reliability`. Available connected tools do not expose workflow enable/dispatch; a lack of API records alone does not establish the precise GitHub setting responsible.

Local delivery locations in this session:

- `/workspace/supertux-phase1-artifact/`: matched Release HTML/JS/WASM/data/shell images plus SHA-256 manifest.
- `/workspace/supertux-phase1-release.zip`: complete portable Release artifact.
- `/workspace/supertux-evidence-release/` and `/workspace/supertux-evidence-debug/`: browser reports, console logs, and screenshots.
- `/workspace/supertux-build-release/` and `/workspace/supertux-build-debug/`: full build outputs.
- `/workspace/supertux-toolchain/`: configure/build logs, strict original-flag image probe, and pinned vcpkg installation.

The core repair was published as `6d57a647bfa8f7cb5fe2bfab65d16b904957bdbe`, followed by test refinement `2cd939c5ded3081372917bcd141b4e8b51646f7f` and the final status-banner/handoff update on this branch. The local Git transport lacked write credentials, so the connected GitHub API published matching Git trees, verified against the local trees. Artifacts were compiled and tested from these implementation files before the final documentation commit. Their embedded revision label can therefore identify the preceding local/configuration revision; the manifest identifies the delivered source and validation revision explicitly. Rebuilding the branch updates that display metadata.

## Next-phase handoff

1. Review this branch and the matched artifact/evidence; do not merge automatically.
2. Establish hosted CI runs and retain their artifacts. Native CMake now uses the same pre-existing native dependency targets, but native/Android compilation still needs its existing CI coverage.
3. Investigate the precise Debug UB findings above in a separately authorized player/library repair. Do not suppress instrumentation or widen the known-site allowlist to hide a new failure.
4. Test this Release artifact on real Safari/iPhone before changing mobile UI. Record cold load, asset/texture memory, first menu/level, storage hydrate/reload, audio gesture behavior, and tab background/eviction. No Safari success was verified here.
5. Begin the audit's Phase 2 viewport/lifecycle/audio work only after accepting those boot/storage gates. Keep touch controls and multiplayer out of that implementation scope.

Retain the user-data directory and save formats, coherent dependency provider, strict undefined-symbol policy, and pre-main hydration barrier. General save dirty tracking, asset reduction, mobile DPR/safe areas, touch input, co-op/networking, and gameplay changes remain later work.
