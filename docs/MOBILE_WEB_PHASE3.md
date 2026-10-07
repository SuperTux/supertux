# Mobile Web Phase 3: reusable touch controls

Implementation date: 2026-10-03. Repository: `jbbejena/supertux`.
Branch: `mobile-web-phase3-touch-controls`. No merge or upstream changes.

## Base and scope

Phase 2 PR [#2](https://github.com/jbbejena/supertux/pull/2) was merged before this phase. The implementation starts from `mobile-web-audit` merge commit `0c1c57c8a`, containing completed Phase 2 tip `133205bf29c4bceeb298b75c480a944e515ae667`. Phase 1 PR [#1](https://github.com/jbbejena/supertux/pull/1), merged at `9807fb370`, is also included. The original [audit](MOBILE_WEB_AUDIT.md) remains historical; [Phase 1](MOBILE_WEB_PHASE1.md) and [Phase 2](MOBILE_WEB_PHASE2.md) describe the repaired runtime.

This phase reuses `MobileController`, SDL3 browser input, the existing mobile PNGs, and normal `Control` actions for player 0. It adds no DOM gameplay buttons, fake key events, networking, accounts, matchmaking, physics changes, level edits, or gameplay test cheats. Strict SDL3 linkage, PhysFS, storage hydration, the Start/Resume audio gate, and the Phase 2 viewport/lifecycle handling remain in place.

**Acceptance status:** the implementation and browser artifact are ready for physical-device acceptance. Browser emulation and Linux WebKit cannot establish that an actual iPhone loads this large package, feels comfortable, or survives Safari eviction. Do not call the real-iPhone milestone certified until the device steps below pass.

## Input and layout

The path is:

`DOM Pointer Events` → pinned SDL3 Emscripten backend → `SDL_EVENT_FINGER_DOWN/MOTION/UP/CANCELED` → `ScreenManager::process_events()` → `MobileController` → browser touch mask on `Controller` → existing player/world-map/menu consumers.

The pinned SDL3 implementation already uses Pointer Events, not an Android-specific Java layer or a legacy DOM Touch Events adapter. Its `Emscripten_UpdateTouchFromEvent()` normalizes canvas coordinates by CSS width/height minus one and uses `pointerId + 1` as `SDL_FingerID`. There is no second owner of gameplay actions in JavaScript.

Changes:

- `Config::Config()` skips browser touch discovery before SDL initialization. After the game is ready, `supertuxShell.ready()` supplies `navigator.maxTouchPoints`; the first SDL finger-down also discovers touch. `browser_touch_controls` is persisted as `-1` Auto, `0` Off, or `1` On. An explicit Off survives subsequent touches.
- `MobileController::update_browser_layout()` calculates the same hit/draw rectangles before input and simulation, including when graphics are hidden. It uses the **rendered viewport**, not the entire canvas. `browser_finger_pos()` converts SDL-normalized positions through the physical window size, viewport offset, and scale. Mouse fallback uses `Viewport::to_logical()`.
- Browser auto magnification can scale below one despite enabled mobile controls. Android/native magnification behavior remains unchanged. This preserves minimum logical menu dimensions on CSS-pixel phone canvases.
- Controls stay within the viewport and consequently within the shell's existing safe-area content box. Direction pad sits bottom left; Jump and Action bottom right; Item and Pause sit near the top center, leaving the item HUD and coin display accessible. Existing PNGs provide button/pressed/direction/pause imagery. Item has a small text label.
- Buttons scale with visible height and the saved control scale, with bounds to avoid portrait overlap. Action/Jump/Item/Pause targets are at least 44 CSS pixels in the supported layouts; the pad is at least 132 pixels so its vertical thirds are at least 44 pixels. At default 844×390, Action/Jump targets are 78 pixels. Portrait remains available; landscape is advised in the Start overlay.
- The existing Controls menu now exposes Auto/On/Off, Show Touch Controls, and scale on web. The scale list includes the existing 130% default instead of incorrectly displaying 50% for that value. Native choices remain unchanged.

## Multitouch, release, and cancellation

A control owns a finger only when its initial down lands in a widget. Owned fingers can slide between widgets, slide out to neutral, and return. Motion without a corresponding owned down cannot re-create a canceled gesture. Multiple fingers are unioned each simulation step, so releasing one finger does not release a button still held by another.

Completed short touches are retained per finger until one simulation step consumes the press. A DOWN/UP pair between steps therefore still activates Jump/select. A normal SDL cancellation following pointer-up must not erase that completed tap; an actual cancellation while the contact is owned discards it.

For browser controllers, `Controller::set_touch_controls()` stores a separate touch bitset. `hold()/pressed()/released()/update()` combine it with the existing local source state. A touch release cannot overwrite a held keyboard key. The browser touch mask clears when controls are disabled; disabling also clears the touchscreen mode flag. Native controller storage and native MobileController action delivery retain their existing behavior.

`ScreenManager::cancel_browser_touch()` cancels just one finger, including its UI ownership. The exported `cancel_browser_touch(pointer_id)` **queues** an SDL canceled event after any already queued down/motion events. It does not reset the keyboard or other fingers.

The shell tracks pointer IDs only for contact lifetime and stale-event suppression:

- Ordinary pointer-up removes that ID before implicit capture loss/leave; it does not reset remaining contacts.
- Unexpected capture loss, pointer cancellation, or active-contact leave cancels that ID.
- Reset/background/resize quarantines held IDs until their release or a new down. Stale move/up cannot cause SDL's motion path to synthesize a new down and resurrect input.
- Phase 2 full resets on background, orientation and canvas-size changes remain. Returning still requires the native Resume button and the real audio context's activation gate.
- Screen/menu changes discard owned touch gestures to prevent a held select action from activating the next screen after its controller reset.

The pointer-ID conversion depends on the pinned SDL3 backend. Re-audit it before changing SDL versions; do not silently retain this assumption with another provider.

## Direct menu navigation

Touch outside the widgets goes through one owned UI finger. `ScreenManager::process_events()` sends a properly initialized SDL mouse motion before its button event so the touched row is selected before HIT. Other gameplay fingers cannot create clicks or mouse-up events for that UI contact.

`Menu::event()` uses SDL3's **button coordinates** for button events, rather than reading the motion member of the union. Touch motion clears the desktop wheel dead zone, so a previous wheel operation cannot prevent a direct tap from selecting its row. Existing menu behavior handles the resulting actions; there is no special web menu.

The normal route is Start → Start Game → story intro (Pause skips) → DOWN on the world map → Jump to enter Welcome to Antarctica → Jump to dismiss its intro. Pause opens the existing game menu; Jump activates its selected Continue row. Item, Up and Down use the existing input actions.

## File and symbol map

| File | Relevant symbols/change |
|---|---|
| `mk/emscripten/browser.js` | `ready`, `resetInput`, `forgetTouches`, per-pointer lifecycle handlers; capability discovery and quarantine |
| `mk/emscripten/template.html.in` | Start overlay touch guidance; no gameplay overlay buttons |
| `CMakeLists.txt` | Export `set_browser_touch_available`, `cancel_browser_touch` |
| `src/port/emscripten.hpp` | Availability bridge and ordered SDL cancellation event |
| `src/supertux/gameconfig.cpp`, `.hpp` | Browser preference load/save/defaults; runtime availability |
| `src/supertux/menu/options_menu.cpp`, `.hpp` | Mode, visibility and scale UI; 130% choice |
| `src/video/viewport.cpp` | Browser exception to native mobile minimum magnification |
| `src/control/mobile_controller.cpp`, `.hpp` | Viewport layout/conversion, owned normalized fingers, completed taps, reset/cancel, existing images |
| `src/control/controller.cpp`, `.hpp` | Browser touch mask separate from local input; combined edges |
| `src/supertux/screen_manager.cpp`, `.hpp` | UI-finger ownership, finger cancellation, touch-to-menu events, transition resets |
| `src/gui/menu.cpp` | SDL3 button union fields and direct-touch row selection |
| `tests/wasm/controller.test.cpp` | Native input baseline and compiled WASM source/edge tests |
| `tests/wasm/browser.test.cjs` | Production shell cancellation, capture, quarantine and lifecycle checks |
| `tests/wasm/touch_smoke.py` | Actual compiled normal-level touch flow, read-only observations and settings persistence |
| `.github/workflows/wasm.yml` | Phase 3 push trigger, controller tests, Release/Debug Chromium touch checks, Release WebKit checks and evidence; explicit Bash makes `tee` pipelines fail on the original command |

## Build and validation

The existing pinned toolchain was reused: Emscripten **6.0.11**; vcpkg **c748cb44f2a435fcf015c35225c9d5545fe0021c**; SDL3 **3.4.16#1**, SDL3_image **3.4.4#1**, SDL3_ttf **3.2.2#1**, PhysFS **3.2.0#1**. Release and Debug use strict unresolved-symbol failure and the same coherent SDL3 libraries as Phase 1. No dependency/provider or threading change is made here.

Validation results are recorded in the accompanying `report.json` files and logs. The local suites run against complete matched HTML/JS/WASM/data outputs, served over HTTP, not `file://`.

| Check | Result |
|---|---|
| Strict Emscripten Release and Debug linking | Passed; no unresolved symbols. Packager warns about the 311 MiB data bundle. |
| Storage / production shell Node tests | 10/10 and 11/11 passed. |
| Compiled Controller source ownership / action edges | Passed in actual WASM under Node and native host C++. |
| Release Chromium 151.0.7922.173 normal touch route | All 10 check groups passed, including real movement/jump, three contacts and saved preference reload. |
| Release Linux WebKit 26.5 touch routing | All 10 check groups passed with the injection limitations below. |
| Final Release Chromium / Release WebKit existing browser regression | All 12 check groups passed in each engine. |
| Final Debug Chromium existing browser regression | All 12 check groups passed with 31 recorded diagnostics from the unchanged exact Phase 1 allowlist. |
| Debug Chromium new normal touch route | All 9 interaction groups completed; final sanitizer gate **failed** for the additional autotile parser site below, reproduced in two completed runs. Failure logs/screenshots are preserved. |
| Physical iPhone/iPad/Android, touch-only full level clear | Not performed; acceptance remains open. |

The Chromium touch samples measure initial `x=0`, Right `x=18.7125`, then moving Jump `x=35.8275, y=644.246` versus ground `y=673.196`. At the triple-contact sample Right, Jump and Action are all held. Releasing Jump retains Right/Action; releasing Right retains only Action. These are observations from the actual player's existing getters, not an input-model simulation. Runtime execution used a CSS 844×390 touch context at DPR 1; the separate browser regression suite covers DPR 3. Local browser processes ran sequentially to avoid software-GPU contention.

Hosted GitHub Actions is not claimed as passing. The workflow now includes these focused checks and rejects the additional Debug diagnostic. Local execution is the validation evidence for this handoff.

The touch suite starts with fresh storage and the normal title screen; the only boot argument is verbose logging. It does not inject a level filename. After touch-only entry, it enables the existing developer console solely to install a retained read-only observer calling `Player::get_x()/get_y()/get_input_held()`. Keyboard events are used for this instrumentation, never for movement or level/menu entry. When the normal pause menu stops the script scheduler, the same getters are queried once through that console. No invincibility, position setters, scripted finish, movement setters, or production test exports are used.

Chromium uses trusted CDP touch events for multiple contacts. Partial `touchEnd` lists the fingers being released; a small native-event probe confirmed the corresponding pointer-up and remaining touch count. WebKit uses a native touchscreen tap for Start and DOM Pointer Events through SDL3's real registered handlers for the remaining multi-contact checks, because Playwright does not expose trusted multifinger WebKit injection. WebKit results therefore validate engine/event-handler integration, not physical iOS dispatch.

Checks include real Right movement and moving jumps, Right+Jump+Action together, independent releases, two fingers on one button, pad slides through neutral, cancellation plus stale motion/up, a completed tap in one browser turn, Item, Pause/Continue, portrait/landscape/toolbar-height changes, safe-area padding, background/Resume, and actual Controls-menu Off→On with IDBFS reload. Fresh matching samples and movement are awaited with a bounded deadline, rather than assuming simulation time matches wall-clock waits. Ordinary touch restarts reset enemies between unrelated action groups; long layout/settings checks pause the ordinary level to prevent idle enemy collisions invalidating the observations. This smoke is **not a complete level-clear or comfort test**.

The Controls category is entered by tapping its tile directly. Pointer motion can
change the selected category when the Options menu opens, so relative navigation
from an assumed Video selection can open Extras instead. Menu changes through
direct event dispatch now discard UI-finger ownership, as controller actions do.
The opening finger is deliberately moved and released over a different submenu
row; it must not change the selection before Off/On changes and IDBFS persistence
are verified through the actual game configuration.

The existing browser suite also checks actual fonts/images/assets, PhysFS boot, normal keyboard play, the real OpenAL context, repeated lifecycle/audio recovery, DPR 3 sizing and letterboxing, persisted setting/progression fixtures, and explicit storage failure/retry/fallback.

Debug is **not sanitizer-clean upstream**. `--record-known-ub` records only the exact pre-existing player/SimpleSquirrel/obstack sites documented in Phase 1; any additional site fails. The allowance has not been expanded. The normal touch route additionally exposed `src/supertux/autotile_parser.cpp:227:79`: an invalid/uninitialized `bool` while loading the world map. `AutotileParser::parse_autotile()` declares `bool solid;` at line 154, permits a missing `solid` for corner tiles, then passes that uninitialized value to `Autotile`. Packaged corner entries in `data/images/autotiles_ice_world.satc` omit `solid`, as intended by the parser. The parser is byte-for-byte unchanged from Phase 2 tip `133205bf29c4bceeb298b75c480a944e515ae667`; the last file change predates this work. This confirms the source defect predates Phase 3; no complete baseline rebuild was performed to reproduce its nondeterministic stack value. The new check remains failing for this additional site rather than classifying Debug as clean. Resolve this narrowly in a separate reviewed parser repair before requiring a green Debug matrix; do not broaden the allowlist. Gameplay/parser/library changes remain outside this touch phase.

A full unrelated desktop build is not run. The native Controller test compiles and executes with the host compiler, native platform branches remain in place, and the cross-platform menu change is confined to correct SDL3 fields and touch row selection.

## Artifact and reproduction

Generated review files (outside Git, in the task workspace):

- `/workspace/artifacts/supertux-phase3-release.zip`: complete Release site with original `supertux2.html`, matching JS/WASM/data, icons/background, `index.html` alias, README and this handoff.
- `/workspace/artifacts/supertux-phase3-validation.zip`: six suite evidence directories, an additional preserved first Debug failure, console/test/build logs, screenshots, Release/Debug compile commands, and the manifest.
- `/workspace/artifacts/supertux-phase3-release/manifest.json`: published source commit/tree, pinned dependencies, hashes of compiled production changes and every site file, individual suite results, and explicit device/full-level-clear limitations.

The outputs were built with uncommitted implementation sources before the final documentation commit. Their embedded Git description therefore names the merged base, not the final Phase 3 commit. The manifest verifies that all 16 changed production/CMake/shell files match the committed sources; it identifies the actual review artifact without implying it was rebuilt after publication. Documentation/tests/CI edits after linking do not change those compiled files. Both archive CRCs and the HTML alias/file SHA-256 values are checked when packaging.

The full `.data` package remains **326,676,452 bytes (311.54 MiB)**. Decoded resources, temporary downloads, WASM memory and GPU allocations add overhead. This is a material **real-phone loading/eviction gate**, not resolved by touch input. Package splitting or mobile content reduction belongs to measured follow-up work if the device cannot boot reliably.

Build using the pinned Phase 1/2 CMake/vcpkg path and recursive submodules; `.github/workflows/wasm.yml` contains reproducible commands. Copy `template.html` over generated `supertux2.html` after linking. Focused checks:

```sh
node tests/wasm/storage.test.cjs
node tests/wasm/browser.test.cjs
em++ -std=c++17 -I build -I src tests/wasm/controller.test.cpp src/control/controller.cpp -o build/controller-test.js
node build/controller-test.js
g++ -std=c++17 -I build -I src tests/wasm/controller.test.cpp src/control/controller.cpp -o build/controller-native-test
build/controller-native-test
python tests/wasm/browser_smoke.py build --output build/browser-evidence
python tests/wasm/touch_smoke.py build --output build/touch-evidence
python tests/wasm/touch_smoke.py build --browser webkit --output build/webkit-touch-evidence
```

For Debug only, add `--record-known-ub` to both browser commands. Install Playwright 1.62.0, Pillow 11.3.0, and Playwright browsers/dependencies as the workflow does. An installed Chromium can be selected with `--chromium /path/to/chromium`; `--webkit-executable` is an optional local launcher. CI uses normal Playwright installation. Linux WebKit needed extracted runtime libraries in this workspace; this is not an iPhone emulator.

For a desktop review, serve the complete artifact directory with an ordinary HTTP server. For phones, place **all** files together on the intended HTTPS preview host, retaining exact names and relative paths. Serve WASM as `application/wasm`; keep the large data response intact. Test a fresh cache. There is no pthread/SharedArrayBuffer requirement and no new isolation-header requirement. No deployment or service worker is added by this phase.

## Real-device acceptance and handoff

Use the packaged Release output identified by its manifest. Record model, iOS/browser version, orientation, asset loading time, failures/reloads, and whether the device becomes hot or frame pacing degrades. Do not substitute screenshots from Linux for these checks.

1. On an actual iPhone Safari tab, load the normal HTTPS URL from a fresh cache and wait for Start. Confirm no blank screen, OOM reload loop, fatal storage error, missing images, or unresolved symbol. Tap Start and confirm audible sound or the explicit muted recovery choice.
2. Use only touch: Start Game, skip the story with Pause, move DOWN on the map, Jump into Welcome to Antarctica, then Jump to dismiss its intro. Do not use a level CLI shortcut or developer controls.
3. Hold Right, add Jump, then Action. Release each finger in different orders. Repeat with two fingers on Jump or Action, quick taps, pad sliding left/right/up/down, movement out of a widget and back, and an extra accidental finger outside controls. Check comfort, finger occlusion and missed presses.
4. Use Pause/Continue, Item when inventory is available, duck/climb/doors where relevant, ordinary deaths/respawn and a checkpoint. **Play through a complete representative level**; automated checks currently establish entry and normal actions, not a full clear.
5. Rotate both ways; expand/collapse Safari toolbars; background/lock while fingers are held; return and press Resume. Confirm no drift, repeated actions, double audio, or inadvertent pause-menu activation. Check notch/home-indicator and Safari edge gestures.
6. In Options → Controls, change scale/visibility; switch Auto→Off, then directly tap the mode row to restore On. Reload and verify the preference and progression survive. Test private browsing/storage denial separately after the primary normal-tab flow.
7. Repeat the core simultaneous-finger and lifecycle checks on iPad Safari and Android Chrome. Test hybrid/desktop Auto and explicit Off with ordinary keyboard input. Fullscreen remains optional as in Phase 2.

If a real device fails to load before Start, capture its browser/network/memory evidence before changing input code; investigate the asset/memory gate first. If native multitouch differs from the SDL/WebKit handler checks, capture pointer IDs/types and SDL event order before adding a new adapter. If buttons are uncomfortable, adjust the single C++ geometry owner rather than introducing parallel HTML actions.

The next run should start from this branch tip/merged audit state, verify the packaged manifest, and perform device acceptance. Also investigate the precisely identified Debug autotile diagnostic above; CI must retain its failure until the defect is repaired. Networking remains deferred. Do not claim Phase 3's physical-device milestone complete or proceed on that assumption while those gates remain open.
