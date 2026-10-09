# Private browser co-op: Phase 7A guest display

This document records the original static-arena milestone. The subsequent
[Phase 7B handoff](PRIVATE_COOP_SUPPORTED_LEVEL.md) covers the supported
Welcome to Antarctica campaign level and its native object/drawing baselines.

Based on merged input-proof PR #11, browser integration commit
`f7c436e3bcb12363c5f193bd50b28f4d2ee691f7`. The implementation branch is
`codex/private-coop-guest-view`; the integration branch remains `mobile-web-audit`.

This is the guest display foundation, limited to one controlled static scene.
The host simulates both players using normal SuperTux physics and local co-op.
The guest sends Player 2 controls and draws both players, original sprite poses,
death state and the host's existing shared camera. It runs no game simulation,
collision, scripts, random outcomes, music engine or save/settings storage.
This does not establish Phase 7B campaign/object synchronization.

## Supported scene and presentation

`tools/web/coop/scene.json` defines a fixed arena: solid snow ground, four solid
static platforms, a uniform sky and a spawn point. Packaging derives a native
`levels/web/coop-view.stl`, the identical guest tile grid, sprite-action metadata
and content-addressed PNG files outside the source data tree. The originals,
campaigns and native/Android asset packaging are preserved. There are no enemies,
pickups, moving platforms, hazards, projectiles, blocks, doors, checkpoints,
cutscenes or completion triggers in this scene. Those need Phase 7B coverage.

The proof starts only from the host title screen after a view-capable guest has
joined. It uses an owned in-memory Savegame for ordinary player initialization;
it does not select or save a campaign world. Host controls Player 1; guest
controls Player 2. Ordinary co-op Action respawn and all-player death/restart are
host simulation events. The guest presents their resulting state.

The Canvas2D guest is deliberately a small presentation client. It receives exact
host-selected small-Tux sprite action/frame, offsets, mirroring, rotation, alpha,
visibility and death state; it does not advance animations independently. Both
players have numbered labels. HUD, sound, above-screen/dead-player arrows and
native menus are not replicated. Other levels and bonus forms show an explicit
unsupported-content message and neutralize guest input.

The shared camera is exported after the native sector draw. Its translation,
scale and logical viewport are used by the guest, with aspect-preserving
letterboxing on a different display. It follows the native group of living
players, not exclusively the host or guest. There is no camera redesign.

## Protocol and lifetime

Room protocol and WebSocket subprotocol are version 2 (`supertux-coop-v2`). The
manifest hash validates code, configuration, startup/deferred assets, presentation
scene and every frontend file before joining. An older invitation/build is
rejected; request a new room link after updating both pages.

A host-only `view` message is a complete visual baseline with:

- A monotonically allocated host GameSession ID and restart epoch.
- Snapshot sequence, host monotonic time and current input generation.
- A fixed supported scene ID or `unsupported`, and the shared camera transform.
- Exactly two stable player IDs (native slot + 1), pose and visual/death fields.

The synchronous C++/JavaScript bridge copies values and UTF-8 action names during
the draw pass. JavaScript never retains native object pointers. Snapshots are
limited to approximately 15 Hz, at most 2,048 UTF-8 bytes, and stop during host
pause/menus. Input/auth/other messages retain the 512-byte bound and existing
command-rate/heartbeat/lifetime rules. Guests cannot publish snapshots.

The Durable Object validates finite values, known fields, entity count/IDs,
camera bounds, supported scene/pose names, generation and order. It runs no game
simulation. The output window remains 32 messages. A slow viewer retains one
latest complete snapshot, plus at most three existing latest status values;
input edges are never coalesced as snapshots. There is no accumulated world
history or unbounded server queue.

The guest keeps at most eight snapshots and renders 90 ms behind host time.
Positions/camera interpolate only within the same session/epoch; animation and
death remain discrete. It never extrapolates or predicts. Old, duplicate and
out-of-order snapshots cannot replace newer state. Pause/input-generation
changes clear interpolation history. A 1.2-second visual watchdog freezes the
display and releases controls; late updates cannot replay held buttons.
Disconnect/backgrounding clears display history and remote controls. Recovery
still requires host title screen → rejoin → restart; no seamless reconnect.

## Delivery and local reproduction

Build Release with the pinned Emscripten 6.0.11/vcpkg configuration documented in
[PRIVATE_COOP_INPUT_PROOF.md](PRIVATE_COOP_INPUT_PROOF.md). The packaging option
`--presentation` is enabled by Emscripten.cmake, including full-preload builds.
`asset-manifest.json` includes `coop-view.html`, `coop-view.js`, `coop-scene.json`
and all `coop-art/<sha256>.png` frontend hashes and byte sizes. Artifact
verification checks completeness, paths, image identities and scene references.
The guest validates the manifest, scene and every PNG before use. PNGs are served
through the frontend binding with immutable cache headers; large game payloads
retain streaming private R2 delivery. No new production binding is enabled.

From this checkout, use a complete Release preview/artifact:

```sh
python3 tools/web/package_assets.py assemble --build build --output build/upload
python3 tools/web/verify_artifact.py build/upload --source-commit "$(git rev-parse HEAD)" --configuration Release
npm ci --prefix tools/web/coop --ignore-scripts
node tools/web/coop/preview.mjs build/upload 8787
```

Open `index.html?coop=1` on that preview, use Start, then open the **Private co-op**
panel. Create a room and open its **Guest shared view** invitation in another
browser. Wait for Player 2 to join, then click **Start shared view proof** on the
host. The older controller invitation and Antarctica/forest buttons retain the
Phase 6 diagnostic proof; those campaigns have no synchronized guest view.

An ordinary HTTP server can deliver the full artifact, but room creation needs
the Worker/Durable Object relay supplied by the preview command. Hosted staging
uses the existing `wrangler.coop.toml`, complete verified frontend and game assets;
see the input-proof staging instructions. This run does not merge or deploy to
production. Production `wrangler.toml` still has no `COOP_ROOMS` binding.

## Validation and remaining acceptance

The focused CI adds `coop_view_smoke.py` to Release Chromium/WebKit and Debug
Chromium, while preserving existing boot/audio/touch/save/cache/local/remote
proof checks. Shared C++ changes continue selecting Linux and lint. New unit
coverage checks schema/authority, generations, stale snapshots, epoch isolation,
interpolation without prediction, discrete death, bounded history/backpressure,
artifact completeness and presentation delivery. Full native matrix remains
available through the existing workflow policy.

The committed runtime `fdc11aa6334ba3e98d00c2a64e8d1acc57665bbe` passes all nine
two-browser display scenarios in Release Chromium 151.0.7922.34 and Linux WebKit
26.5. Each test uses separate actual browser processes, the compiled host and
real local workerd/Durable Object, with 844×390 touch-capable viewports. No CPU or
network throttling was applied; Chromium uses software SwiftShader rendering.
Fresh browser contexts and a local same-machine relay are used. These are not
physical-phone, WAN or Safari device results.

| Measured observation | Release Chromium | Release Linux WebKit |
| --- | ---: | ---: |
| Input to visible 30-pixel guest movement | 334 ms | 326 ms |
| Maximum observed visual snapshot | 482 bytes | 470 bytes |
| Observed snapshot rate | 11.8/s | 11.0/s |
| Visual snapshot payload throughput | 5,405 bytes/s | 4,832 bytes/s |

The movement measurement includes the time for ordinary physics to travel 30
pixels and the 90 ms presentation delay. It is not an isolated round-trip latency
measurement. Throughput/rate averages span active play and test menu pauses;
WebSocket framing, input/status messages and initial asset downloads are excluded.
Injected 20/180 ms delay is at the guest presentation-delivery boundary after the
real relay, not simulated WAN packet loss. History stays within eight complete
snapshots and rendered order never reverses.

Presentation files total 1,691,673 bytes for the viewer's code/HTML, scene and
PNG inventory, plus the 853,561-byte complete build manifest and host-index
compatibility check. No game WASM/DATA/music is fetched by the guest, and it
creates no IndexedDB save/settings database. The mandatory host DATA gains only
4,943 bytes for the generated fixture; deferred soundtrack bytes are unchanged.

The browser scenarios verify ordinary movement and trusted touch Jump, isolated
input owners, native group camera, pause/fresh neutral Resume, dying/dead/Action
respawn, all-player restart with a fresh epoch, jitter/order/bounds, absence of
guest simulation/downloads/storage and background/disconnect cleanup. Death
labels use the native surviving-target presentation anchor; they do not invent
guest respawn behavior. Native HUD/sound/arrow art is still unreplicated.

Focused JavaScript tests pass 55/55; packaging/upload tests 6/6; CI-selection
tests 6/6; actionlint and repository Cppcheck pass. Release and Debug WASM builds
and complete artifact verification pass. Debug Chromium passes all nine display
scenarios on runtime `43874d348266c456d7a9c16300abed75c4fd017f`, with no new
sanitizer sites. The complete Debug WASM job and representative Linux Release
build pass in CI run `37834580301`. Retained boot/audio/viewport, asset/cache, later-area, touch and
local/remote co-op checks also pass in that run; final PR validation includes
the frame-synchronized death fixture described below.

Debug exposed an existing uninitialized `Player::m_reset_action` flag in the
idle-animation path. It is now initialized to false, and the old Player-bool
sanitizer exemption is removed. The remaining exact pre-existing libc++/obstack
diagnostics continue to be recorded with instrumentation enabled; any new site
fails validation.

The existing required gate and check names are preserved. This change selects
Release/Debug WASM, representative Linux and lint/focused tests, without the
unaffected native matrices. GitHub's ruleset listing is empty; classic branch
protection inspection returns HTTP 403 (`Resource not accessible by integration`),
so its required-check configuration could not be independently verified. No
repository rules/settings were changed.

The post-merge Release CI failure in run `37824389939` occurred in the touch
test's restart-menu sequence. The regression test now observes the existing
native per-frame status callback between touch down/release/next gestures.
Local reproduction also exposed its read-only observer stopping while restart
temporarily removed sector bindings, and slow screenshots extending held RUN
into a hazard. The observer skips only those exact missing-binding errors, and
screenshots follow the independent release assertions. Restart still requires
neutral controls at the packaged spawn lane, x=96, after landing on its flat
ground; the assertion accepts the native 672–674 pixel collision-contact range.
No control, restart or gameplay check was removed.

The new display fixture initially failed Release CI after closing the scripting
console: timed Escape gestures left the host paused. Its keyboard edges now wait
for native input updates and verify pause/resume state before proceeding. This
changes automation timing, not production input or lifecycle handling.

No physical iPhone Safari or hosted separate-network play has been verified in
this environment. The separate HTTPS publication workflow and physical-device
checklist are in [PRIVATE_COOP_PHONE_ACCEPTANCE.md](PRIVATE_COOP_PHONE_ACCEPTANCE.md).

For device acceptance, use isolated HTTPS staging and two physical phones on
different networks. Try both host-on-phone and guest-on-phone roles; verify
two-player movement/jump, shared camera, Action respawn, all-player restart,
portrait/landscape and safe areas, host Pause/trusted Resume, guest backgrounding
and explicit rejoin. Confirm the host's campaign saves/settings survive and the
guest never writes progression. Record input-to-visible delay and bandwidth on
those networks before choosing a performance budget or prediction work.

The next implementation boundary is Phase 7B: choose an existing level, inventory
every object/interaction and replicate all state needed for enemies, pickups,
spawn/despawn, tile/object changes, checkpoints and completion. Expand scope only
after both screens agree through those interactions.
