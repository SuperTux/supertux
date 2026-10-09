# Private browser co-op: Phase 7B supported level

Implementation baseline: `mobile-web-audit` at
`314e62e20b3f5703bb00683ad59f5ff47beb5661` (merged PR #13).

The explicit shared-view campaign scope is **Welcome to Antarctica**,
`levels/world1/welcome_antarctica.stl`, for one host and one guest. Original
levels, images, scripts, physics, audio and single-player/native packaging are
preserved. Other campaign levels remain controller-only diagnostics and are
not advertised as synchronized guest scenes.

## Playing

Use a complete preview served by the local co-op Worker, or a separately
published HTTPS co-op staging build. The ordinary production Worker has no
private-room binding; this PR does not enable multiplayer production delivery.

1. Host opens `index.html?coop=1`, taps Start, then expands Private co-op.
2. Host creates a room and shares **Guest shared view**. Guest artwork is
   verified and loaded before the guest joins. The controller-only invitation
   does not provide a second game screen.
3. After Player 2 joins, host chooses **Play Welcome to Antarctica**.
4. Both screens use the native shared camera. Host owns Player 1, menus,
   pausing, world-map progression and saves; guest owns Player 2 input.
5. Action respawns a downed Player 2 under the existing local co-op rules.
   All-player death restarts at the activated checkpoint, when applicable.
6. Completion releases guest controls and reports the outcome. To change rooms
   or recover a disconnect, host returns to the title screen before rejoining.
   Room invitations retain the existing 15-minute lifetime.

## Architecture and content inventory

The guest remains a Canvas2D presentation client. It downloads no game WASM,
DATA or soundtrack, runs no collisions/scripts and creates no save database.
The host is the sole authoritative simulation. There is no deterministic
lockstep, video streaming, Asyncify or guest filesystem implementation.

For the supported level, the host copies the visible native color-canvas draw
requests into a complete snapshot. Native object UIDs identify requests and
moving-sprite state. Texture source regions include the texture manager's crop
origin; layering, clipping, horizontal/vertical flips, rotations, tint, opacity,
background parallax and animated tile/sprite frames come from the actual engine.
Tile-created coins, blocks, newly spawned powerups, projectiles and debris are
therefore presented through the same path as explicitly declared objects.
Removal is the absence of that UID/quad in the next complete baseline, not an
unreliable guest-side collision event. Coins, checkpoint position and level phase
are explicit presentation state. Text uses the host's wrapped strings and
geometry with a browser font; glyph appearance differs from the native font.

`coop-scene.json` extends the existing presentation inventory with a campaign
ID, canonical level path, original level hash, direct-object counts, tile IDs
and runtime-image-to-content-hashed-PNG mapping. Packaging checks the real
comment-prefixed tileset, used tile atlas images and runtime defaults (including
HUD icons and the default explanation decal). Original artwork is copied
unchanged outside the source data tree. Host startup assets are unchanged.

The level contains six tilemaps, four backgrounds, nine decals, three info
blocks, one checkpoint bell, a secret area, two sequence triggers, seven
snowballs, three smartballs, an ice block, a bomb, a jumpy, a stalactite and
24 weak blocks. The runtime also creates coins, bonus/wooden blocks, powerups,
projectiles and effect sprites. The supported color-canvas primitives are
textures, gradients, rounded rectangles, lines and text. The browser's SDL
renderer ignores attached displacement maps, including the one on a used
bonus block; the guest uses the same diffuse image. GL displacement effects,
arbitrary scenes and other drawing primitives remain unsupported. Rotation is
normalized modulo one turn, preserving rolling egg animations without exceeding
relay bounds. The chosen level has white ambient lighting.

Debug validation also exposed a native fading-coin defect: predicted rendering
could run just past the coin's lifetime and send negative opacity to SDL and
the relay. The coin now clamps its fade to the valid opacity range before
drawing. This preserves the animation and avoids an invalid presentation frame.
The guest also matches SDL's primitive opacity rules: textures/text apply
transform opacity separately, while filled panels and lines already contain it
in their colors. A canvas-renderer regression verifies that fades are applied
once rather than twice.

A new session/restart epoch waits for an exact matching guest baseline
acknowledgment before campaign physics advances. Pause and background lifecycle
continue using the existing input generations. Only the matching guest can
acknowledge a published baseline; only the host can publish completion. Loading
has a bounded timeout with a title-screen/rejoin recovery message.

## Bounds, ordering and delivery

- At most two players, 512 moving-sprite descriptions, 256 draw requests and
  2,048 visible texture quads per snapshot; maximum encoded message is 64 KiB.
- At most ten host presentation updates per second. The selected level uses
  complete discrete native frames; it does not interpolate tile changes or
  independently move enemies. The older static arena retains player/camera
  interpolation. Smoother supported-level presentation is a follow-up.
- Guest history retains at most eight frames and rejects older session, epoch,
  sequence or host time. Restart never mixes old geometry with a new baseline.
- Host WebSocket output applies a 128 KiB backpressure threshold, plus at most
  one bounded message being sent; the guest retains its 16 KiB threshold. The
  existing 32-message receive-credit window remains.
- A slow receiver coalesces one latest complete visual baseline. Cloudflare's
  2 KiB WebSocket attachment limit is respected: attachment metadata stores
  only ordering markers, and the single large pending baseline lives in memory.
  If hibernation drops it, the next complete host frame recovers the picture.
- Completion is a host-only critical status, retained separately from visual
  coalescing and scoped to the exact session/epoch. Guest controls are cleared.
  The relay test fills the receive window and reconstructs the Durable Object
  before returning credit, verifying that pending completion survives.

All assets use the existing same-origin, content-addressed frontend routes and
manifest verification. No Worker/R2 credentials are needed for a local preview.
The guest checks hashes before decoding. Downloaded original art can use the
existing immutable HTTP cache. This does not touch saves or origin-wide storage.

## Building and trying the local preview

Use pinned Emscripten 6.0.11 and the repository's
[web CMake/vcpkg configuration](MOBILE_WEB_STARTUP_OPTIMIZATION.md#build-and-preview)
to configure `build-web`. Release and Debug validation
also run the supported-level scenarios through `coop_view_smoke.py`, retaining
the earlier arena, input, pause, restart and lifecycle scenarios.

```sh
cmake --build build-web --parallel 4
python3 tools/web/package_assets.py assemble --build build-web --output build-web/upload
python3 tools/web/verify_artifact.py build-web/upload --source-commit "$(git rev-parse HEAD)" --configuration Release
npm ci --prefix tools/web/coop --ignore-scripts
node tools/web/coop/preview.mjs build-web/upload 8787
```

For internal local checks, open the server's `index.html?coop=1` on the same
machine. A plain HTTP file server delivers the complete artifact but does not
provide the room relay. Physical phones require the separate HTTPS staging
workflow and an exact successful non-PR validation artifact; PR artifacts are
never fed into privileged deployment. Do not merge or publish production as a
part of testing this branch.

## Validation and device status

The tested runtime is `9f8143297d9046f3c19ac5e963ee38bec1b597c0`, built with
Emscripten 6.0.11 and vcpkg
`c748cb44f2a435fcf015c35225c9d5545fe0021c` on 2026-10-09. Subsequent handoff and
unit-test edits do not change that tested runtime. Both assembled Release and Debug
artifacts pass complete source/configuration/toolchain/manifest verification.

| Compiled two-browser suite | Scenarios | Errors | Campaign selection to guest ready | Input to 30 world pixels | Campaign updates/s | JSON payload bytes/s | Largest snapshot |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Release Chromium 151.0.7922.34 | 20 | 0 | 1,134 ms | 718 ms | 5.8 | 180,996 | 45,522 bytes |
| Release Linux WebKit 26.5 | 20 | 0 | 869 ms | 478 ms | 8.1 | 256,587 | 46,801 bytes |
| Debug Chromium 151.0.7922.34 | 20 | 0 new | 2,578 ms | 1,000 ms | 5.6 | 176,075 | 47,333 bytes |

These are separate desktop browser processes at an 844 × 390 touch-emulated
viewport, DPR 1, on a four-CPU managed Linux environment, using the real local
Miniflare Worker, private R2 and SQLite Durable Object. Chromium uses SwiftShader.
Profiles are fresh; the guest verifies and preloads its artwork before joining.
There is no network throttling. **Selection-to-ready excludes that artwork
download and host WASM startup.** Movement timing includes actual movement to
30 pixels and the presentation delay, rather than measuring network round-trip
latency. Payload rates count JSON bytes, excluding transport/TLS overhead. These
numbers are not WAN or phone performance estimates. Screenshots intentionally
Pause/Resume around software-renderer readback; the dim guest screenshot is the
expected paused display, and the host menu is not replicated.
Each table row is one observed run, rather than a percentile benchmark.

The retained arena checks cover independent host/guest input, trusted guest
touch release, the shared group camera, Pause/Resume, death/respawn, all-player
restart, injected 20/180 ms delivery jitter, ordering/history bounds, background
and disconnect. The campaign checks cover the acknowledged initial scene, real
coin collection, snowball stomp, growth block/egg/pickup, information panels,
brick destruction/debris, fireballs, ice melting, secret-cover fading, checkpoint
restart and the actual native end sequence/progression save. Host-only Squirrel
fixtures place players or grant the fire bonus to reach individual scenarios;
guest movement, Jump and Action traverse the ordinary input relay. This is
interaction coverage, not an uninterrupted physical-device playthrough.

Debug instrumentation remains enabled. Its report records the five already
documented diagnostics in libc++ `swap.h`, external obstack and `obstackpp.hpp`
through the existing explicit `--record-known-ub` option. No new site is accepted;
the fading-coin SDL diagnostic is fixed rather than added to that list.

The representative Linux native build and all four CTest cases pass. The
62 JavaScript shell/storage/loader/relay/renderer tests, 14 Python
packaging/deployment tests, six CI change-selection fixtures, actionlint and Cppcheck pass
locally. The existing single-player browser suite passes 12 checks per engine
and touch-only normal entry/multitouch passes ten per engine, in Chromium and
Linux WebKit. These legacy browser/touch and native checks used
`c2ed22efa65130e358471f0ab7467910053d38ce`; the subsequent renderer-only fix
leaves their C++/single-player shell unchanged. Existing compilation and
browser/audio/touch/save CI steps are retained.

The guest inventory contains 1,056 canonical image mappings to 1,049 unique,
unchanged PNGs: **18,637,169 bytes**. The scene is 167,419 bytes, the shared-view
HTML/JS and common co-op JS total 32,061 bytes, and the local complete manifest is
1,014,505 bytes. These are file sizes, not a measured compressed HTTP transfer.
The host's startup DATA (176,669,514 bytes) and deferred soundtrack
(150,011,881 bytes) retain the earlier startup optimization. The guest requests
none of those packages and creates no IndexedDB save/settings database.

GitHub Actions is disabled for this repository. Dispatch returns HTTP 422,
“Actions has been disabled for this repository”; this PR therefore has no
hosted validation results or newly published HTTPS build. Enabling Actions and
publishing the exact successful non-PR artifact are documented in the
[staging handoff](PRIVATE_COOP_PHONE_ACCEPTANCE.md). Repository settings and
production deployment were not changed.

Physical iPhone Safari, real Android browsers and two separate networks are
**unverified**. Use the existing
[phone acceptance checklist](PRIVATE_COOP_PHONE_ACCEPTANCE.md), replacing the
arena selection with Play Welcome to Antarctica and additionally checking coin
collection, powerups, enemy contact, checkpoint restart and level completion.

Phase 8 remains responsible for physical-device acceptance, smoother guest
presentation, coordinated usability improvements and incremental content
expansion. This phase does not claim support for all worlds, scripts or bosses.
