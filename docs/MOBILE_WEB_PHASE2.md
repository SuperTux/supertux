# Mobile web Phase 2: Safari shell and lifecycle handoff

> Current browser packaging and CI are described in [MOBILE_WEB_STARTUP_OPTIMIZATION.md](MOBILE_WEB_STARTUP_OPTIMIZATION.md). The soundtrack is now downloaded track by track; only small music descriptors remain mandatory. Use its complete preview assembly, cache-clearing, deployment and focused-CI instructions. The full-package sizes and workflow behavior below describe the historical revision of this handoff.

## Base and scope

Phase 1 [PR #1](https://github.com/jbbejena/supertux/pull/1) was merged into `mobile-web-audit`. This phase starts from merge commit `9807fb370`, containing completed Phase 1 tip `8c973fe0a180ef1c1215aee8f034c3fcfdfe288f`, on `mobile-web-phase2-safari-shell`. Read [MOBILE_WEB_AUDIT.md](MOBILE_WEB_AUDIT.md) and [MOBILE_WEB_PHASE1.md](MOBILE_WEB_PHASE1.md) for the foundation and existing limitations.

The implementation provides a browser shell, viewport sizing, explicit Start/Resume, audio recovery, background pause, and input neutralization. It does **not** enable/tune on-screen gameplay controls, add networking, change level/player/enemy mechanics, or merge into another branch. Native/Android rendering and lifecycle branches retain their existing behavior.

**Status:** local Release and Debug compilation and browser checks are recorded below. This is a real-device-ready diagnostic build, not a claim that iPhone/iPad Safari acceptance is complete. No physical iPhone, iPad, Android phone, or macOS Safari was available.

## Browser behavior

The game downloads and initializes its existing assets/storage automatically. Once C++ announces game readiness, an HTML **Start** button covers the frozen canvas. Pressing it resumes the existing audio engine from that gesture before gameplay is released. Returning after blur, background, pagehide/pageshow, or audio interruption requires **Resume**; becoming visible/focused never starts the simulation automatically.

If audio resume rejects, throws, fails to reach `running`, or takes more than three seconds, the prompt offers retry or **Play without sound**. A late promise cannot bypass this prompt or a background pause. Missing/closed audio still permits play. Muted fallback is session behavior, not a change to saved music/sound preferences; a subsequent Resume can retry audio.

Fullscreen is optional and capability checked. The shell catches rejection and continues normal tab operation. It does not require fullscreen, orientation lock, a PWA, a service worker, pthreads, or SharedArrayBuffer.

## Implementation map

| File / symbol | Responsibility |
| --- | --- |
| [mk/emscripten/template.html.in](../mk/emscripten/template.html.in) | `meta viewport` with `viewport-fit=cover`; fixed safe-area shell; inner canvas area; native HTML Start/Resume/muted/fullscreen buttons; visible status/error/storage messages |
| [mk/emscripten/browser.js](../mk/emscripten/browser.js), `Module.supertuxShell` | Browser viewport, audio activation, lifecycle, input cancellation, and context-loss handling; linked once through `--pre-js` |
| `supertuxShell.ready`, `activate`, `pause`, `save`, `fail` | Separate initialized and active states; guard asynchronous resume with an epoch; freeze before suspension; request settings save plus Phase 1 serialized storage flush |
| `resize`, `resizeNow` | Coalesce events through rAF; measure visible viewport and CSS safe-area content box; ignore invalid/no-op integer sizes; reset input before dimensions change |
| [src/port/emscripten.hpp](../src/port/emscripten.hpp), `set_resolution` | Validate dimensions; resize video and notify the entire screen stack through `ScreenManager::on_window_resize`, rather than only menus |
| `set_browser_suspended`, `reset_browser_input` | Narrow exports bridging browser lifecycle/cancellation into the existing C++ managers |
| [src/supertux/screen_manager.cpp](../src/supertux/screen_manager.cpp), `set_browser_suspended`, `loop_iter`, `process_events` | Freeze simulation, scripting, rendering, and sound updates; pump window/device events while paused; discard covered-canvas inputs; reset elapsed time on return; avoid conflicting browser SDL focus-pause/resize ownership |
| `ScreenManager::reset_browser_input` | Reset all player Controllers and SDL keyboard state; flush queued keyboard/text/mouse/finger/gamepad/joystick action events; clear console text-lock and MobileController fingers/actions |
| [src/control/mobile_controller.cpp](../src/control/mobile_controller.cpp), `reset` | WASM-only cancellation/reset; ignore a retained SDL mouse hold until release; no activation, layout, artwork, or hit-target tuning |
| [src/control/keyboard_manager.hpp](../src/control/keyboard_manager.hpp), `reset_text_input` | Clear a console-key text lock when its key-up was lost |
| [src/audio/sound_manager.cpp](../src/audio/sound_manager.cpp), `SoundManager::SoundManager` | Pass the actual SDK OpenAL `AL.currentCtx.audioCtx` to the shell; no second AudioContext or replacement audio engine |
| [src/video/sdl/sdl_screen_renderer.cpp](../src/video/sdl/sdl_screen_renderer.cpp), `start_draw` | WASM-only SDL3 viewport correction: express viewport in final render coordinates so changing scale does not scale/clip its physical rectangle twice |
| [CMakeLists.txt](../CMakeLists.txt), [Emscripten.cmake](../mk/cmake/SuperTux/Emscripten.cmake) | Export two narrow lifecycle hooks; include browser pre-JS in link dependencies; preserve strict linkage, coherent SDL3 dependencies, and Phase 1 storage |
| [tests/wasm/browser.test.cjs](../tests/wasm/browser.test.cjs) | Controlled lifecycle/audio races, viewport coalescing, safe-area changes, cancellation, fullscreen rejection, and context loss |
| [tests/wasm/browser_smoke.py](../tests/wasm/browser_smoke.py) | Full compiled game, real IndexedDB/OpenAL/renderer, level keyboard play, pause/reset, failure recovery, and mobile viewport rendering in Chromium/WebKit |
| [.github/workflows/wasm.yml](../.github/workflows/wasm.yml) | Phase 2 branch trigger; shell unit checks; Chromium Release/Debug smoke; additional WebKit Release smoke; archived evidence |

## Viewport and rendering decisions

`#game_shell` tracks `visualViewport.width/height/offsetLeft/offsetTop`, with `innerWidth/innerHeight` fallback. CSS applies `env(safe-area-inset-*)` once; `#game_area.getBoundingClientRect()` supplies the inner dimensions to C++. The CSS fallback includes `100dvh`, but JavaScript owns the final visible dimensions. A ResizeObserver handles safe-area/content-box changes that settle after orientation events.

Canvas CSS fills the area. Its backing store and SDL window use **integer CSS pixels**, without multiplying by devicePixelRatio. This intentionally trades Retina sharpness for bounded mobile GPU/memory cost. The game retains its existing Viewport scaling/letterboxing and virtual screen limits; there is no forced aspect ratio or portrait ban.

Window, visualViewport resize/scroll, orientation, fullscreen, pageshow, and content-box events are coalesced. Equal integer sizes do not reapply video configuration/recreate the lightmap. The browser's exported resize owns the new size and screen notifications; old intermediate SDL window-resize notifications are ignored on WASM. Native SDL event processing is preserved. SDK/custom HTML without the template controls keeps the legacy SDL loop/focus/resize behavior; shell ownership starts only when its readiness hook suspends the game.

Image inspection found a separate SDL3 scaling defect: setting a physical viewport at scale 1 and then changing render scale also changed its rectangle. Portrait menus were clipped even though DOM/canvas dimensions matched. The WASM renderer now sets final scale and converts the physical viewport rectangle into that coordinate system. The browser test checks substantial rendered pixels on both horizontal sides at every tested mobile size, in addition to dimensions, and records screenshots. Native rendering remains unchanged.

Canvas `touch-action: none`, non-passive touchmove/gesture prevention, selection/callout suppression, and page overflow/overscroll policy protect game input. HTML controls retain ordinary button/keyboard behavior. Pointer/touch cancellation, lost capture, SDL finger cancellation, resizing, and suspension neutralize actions. This does not promise control over reserved Safari/iOS edge/system gestures.

## Lifecycle and audio ownership

The shell uses the SDK's existing OpenAL AudioContext. SDK 6.0.11 has one-time key/mouse/touch unlock listeners; the shell adds repeated explicit activation and checks the actual context state. Covered-canvas events are filtered before reaching SDL/document unlock handlers. Unexpected SDK unlocks while covered or muted are suspended again. Revisit the `AL.currentCtx.audioCtx` bridge when upgrading Emscripten: it is a pinned SDK integration, not a general Web Audio API field.

The C++ pause is independent of GameSession's pause menu and `pause_on_focusloss`. It preserves an existing menu/pause state rather than toggling it. While frozen, it pumps window/device events but runs no simulation/scripting/drawing/audio update. Resetting `last_time` and `elapsed_time` prevents background catch-up. Native focus-loss behavior is retained.

Blur/hidden/pagehide request config serialization and the Phase 1 save/flush path. There is no unload handler. These asynchronous writes remain **best effort** at suspension/eviction; successful explicit/periodic flush is the meaningful commit point. This adds no full in-level save state. Context loss freezes, requests saving, and shows a reload message; it does not attempt GPU restoration.

## Validation

The SDK/dependency strategy is unchanged: Emscripten **6.0.11**, vcpkg **c748cb44f2a435fcf015c35225c9d5545fe0021c**, SDL3 **3.4.16#1**, SDL3_image **3.4.4#1**, SDL3_ttf **3.2.2#1**, PhysFS **3.2.0#1**. Both Release and Debug link with `ERROR_ON_UNDEFINED_SYMBOLS=1`.

Local checks use Chromium **151.0.7922.173**, Playwright **1.62.0**, and Linux Playwright WebKit **26.5**. Chromium uses SwiftShader; WebKit is the Linux headless engine, **not iOS/macOS Safari**. No audible speaker/headphone output or hardware GPU performance is verified by these tests.

- Ten Phase 1 storage failure/race checks and nine browser-shell failure/race checks pass.
- Full Release/Debug Chromium smoke and Release WebKit smoke exercise menu/assets/fonts, a real Welcome to Antarctica level, keyboard movement/jump, config consumption/progression-fixture reload, denied IndexedDB fallback, and flush failures/retry.
- Browser audio instrumentation observes the real engine, exactly one AudioContext, and actual buffer-source starts. It creates no extra context. Tests verify pre-gesture suspension, repeated resume, foreground audio suspension, a forced real-context resume rejection, visible muted recovery, and later restored audio.
- A held-key background sequence verifies identical uncovered canvas frames during suspension, explicit Resume after pageshow/focus, and stationary player position after a missing key-up. The check occurs near spawn, and jump is measured before moving left along the starting floor to avoid an enemy/respawn contaminating the measurement. Console access remains read-only; no compiled gameplay test exports were added.
- Mobile emulation at DPR 3 exercises **390×844 → 844×390 → 844×320 → 390×700 → 390×844**, then a nonzero CSS safe-area fixture. Both canvas dimensions and rendered horizontal coverage are checked. These are controlled sizes/insets, not measurements of real Safari toolbar/notch behavior.
- Debug keeps UBSan, SAFE_HEAP, and assertions. Only the exact pre-existing sites documented in Phase 1 are recorded with `--record-known-ub`; novel sites still fail. Debug is **not sanitizer-clean** and this phase does not repair those unrelated player/library defects.

CI configuration has been updated, but no hosted Actions success is claimed. The connected fork reported zero runs for `mobile-web-audit` during this work. Owner-side workflow registration/enabling may still be needed. The existing master-only production upload condition is preserved; this phase does not deploy a public site.

## Build, artifacts, and repeatable commands

Use Phase 1's pinned build commands. The new pre-JS is an incremental link dependency; no dependency reinstall, SDL-provider change, or new asset generation strategy is needed. Copy the configured `template.html` over the generated `supertux2.html` after building, as CI does.

```sh
node tests/wasm/storage.test.cjs
node tests/wasm/browser.test.cjs
python -m pip install playwright==1.62.0 Pillow==11.3.0
python -m playwright install --with-deps chromium webkit
python tests/wasm/browser_smoke.py build-release --output evidence-release
python tests/wasm/browser_smoke.py build-release --browser webkit --output evidence-webkit
python tests/wasm/browser_smoke.py build-debug --record-known-ub --output evidence-debug
```

`--chromium /path/to/chromium` selects an installed Chromium. `--webkit-executable` is an optional local launcher. For a copied artifact without `config.h`, supply `--data-dir` with the compiled virtual data directory from the manifest. Never validate files while rebuilding them.

Workspace outputs:

- `/workspace/supertux-phase2-artifact/`: complete Release files, `index.html` alias, and source/artifact checksum manifest.
- `/workspace/supertux-phase2-release.zip`: server-ready Release archive.
- `/workspace/supertux-phase2-validation.zip`: browser reports, console/build logs, and screenshots.
- `/workspace/supertux-phase2-evidence-release/`, `...-debug/`, `...-webkit/`: separate evidence directories.

The embedded build version was generated from the merged Phase 1 base while Phase 2 source edits were present. The artifact manifest records the final source tree and production source checksums to identify those edits precisely; the package is not an untouched audit/Phase 1 build.

The asset package remains **326,676,452 bytes (311.54 MiB)**, with 128 MiB initial / 512 MiB maximum WASM memory, plus JS/preload/decoded assets/GPU overhead. No mobile memory or download optimization was made. If a real phone cannot load this artifact, collect console/memory/network evidence before expanding scope or raising memory limits.

Serve the complete directory through HTTP/HTTPS with correct WASM MIME and consistent cache/versioning. For local desktop validation:

```sh
python -m http.server 8000 --directory /workspace/supertux-phase2-artifact
# Open http://localhost:8000/ (index.html) or /supertux2.html.
```

An actual phone needs a reachable HTTPS URL, not the workstation's localhost. No deployment is performed here. No COOP/COEP headers are required by this unthreaded build.

## Real-device acceptance gates

Use the Release artifact in a standalone tab on a real iPhone and iPad, then Android Chrome. Record OS/browser versions, device model, artifact manifest checksums, actual renderer, console errors, and whether audio is audible. Safari Web Inspector is useful when available.

1. Cold-load a fresh origin; observe loading, Start, main menu, and no PhysFS/storage/link errors. Repeat with existing saves and private mode. Confirm explicit memory-only warnings when persistence fails.
2. Press Start once: hear menu music. Enter a level using existing menus/an external keyboard for this phase; hear effects. Verify muted recovery/retry if sound is blocked. On-screen gameplay control activation remains Phase 3.
3. Rotate portrait↔landscape repeatedly before Start, in menu, in level, and while paused. Check all menu items/HUD fit, letterboxing is stable, and content/buttons remain clear of notch/home indicator.
4. Expand/collapse browser toolbars; open/close any keyboard/file picker used by existing UI. Check visualViewport offsets, safe areas, canvas backing dimensions, and absence of resize/allocation storms. Pinch/double-tap/long-press the game surface; reserved system gestures may still cancel input.
5. Hold an input, switch apps, lock/unlock, switch tabs, and navigate away/back where bfcache is available. On return, require Resume; no unexpected movement/audio or stuck key/finger/mouse/gamepad action. Repeat while already paused in the game's own menu.
6. Interrupt audio with another app/call/output-route change. Resume from another gesture. Check sound/music continuity, no duplicate audio engine, no frozen Resume spinner, and muted fallback.
7. Change a setting and commit normal progression; background/return and reload. Force tab eviction separately. Record what survives a completed flush; do not expect restoration of the current in-level simulation.
8. Exercise unsupported/rejected fullscreen and GPU context loss where practical. Tab play must remain usable; context loss must present reload guidance and preserve previously committed saves.
9. Measure cold download, peak memory, sustained frame rate, heat, and WebGL failures on an older/low-memory iPhone. The 311 MiB package is still a material release gate.

## Phase 3 implementation handoff

Base the next phase on this completed branch or its merged tip. Preserve Phase 1 dependency/link/storage fixes and this shell's Start/Resume, CSS-pixel resolution, pause/reset exports, and OpenAL ownership. First run the real-device gates above: Linux WebKit/mobile emulation is not Safari certification.

Then review post-SDL touch discovery and reuse generic `MobileController` through `Control` actions. This phase added reset/cancellation support only; it did not change activation defaults, hit geometry, coordinate conversion, scale, artwork, or touch-menu UX. Add activation and viewport-aligned geometry in small changes, accounting for `Viewport` letterbox offsets and the shell's safe-area content box. Test simultaneous direction+jump/action and independent finger release on actual iPhone/iPad/Android devices. Do not add an independent HTML gameplay-button owner unless a reproduced SDL backend gap requires it. Networking and multiplayer changes remain outside that phase.
