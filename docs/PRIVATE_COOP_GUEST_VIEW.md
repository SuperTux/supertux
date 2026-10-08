# Private browser co-op: Phase 7A guest display

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

Results and measured relay payload/latency observations are recorded after the
compiled browser validation completes. No physical iPhone Safari or hosted
separate-network play has been verified in this environment.

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
