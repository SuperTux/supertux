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
textures, gradients, rounded rectangles, lines and text. Arbitrary scenes,
lighting/displacement shaders and other drawing primitives are rejected rather
than inferred by the guest. The chosen level has white ambient lighting.

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
- Host WebSocket buffered output is bounded to 128 KiB; the guest input bound
  remains 16 KiB. The existing 32-message receive-credit window remains.
- A slow receiver coalesces one latest complete visual baseline. Cloudflare's
  2 KiB WebSocket attachment limit is respected: attachment metadata stores
  only ordering markers, and the single large pending baseline lives in memory.
  If hibernation drops it, the next complete host frame recovers the picture.
- Completion is a host-only critical status, retained separately from visual
  coalescing and scoped to the exact session/epoch. Guest controls are cleared.

All assets use the existing same-origin, content-addressed frontend routes and
manifest verification. No Worker/R2 credentials are needed for a local preview.
The guest checks hashes before decoding. Downloaded original art can use the
existing immutable HTTP cache. This does not touch saves or origin-wide storage.

## Building and trying the local preview

Use pinned Emscripten 6.0.11 and the repository's web CMake/vcpkg configuration,
as documented in the startup optimization handoff. Release and Debug validation
also run the supported-level scenarios through `coop_view_smoke.py`, retaining
the earlier arena, input, pause, restart and lifecycle scenarios.

```sh
cmake --build build --parallel 4
python3 tools/web/package_assets.py assemble --build build --output build/upload
python3 tools/web/verify_artifact.py build/upload --source-commit "$(git rev-parse HEAD)" --configuration Release
npm ci --prefix tools/web/coop --ignore-scripts
node tools/web/coop/preview.mjs build/upload 8787
```

For internal local checks, open the server's `index.html?coop=1` on the same
machine. A plain HTTP file server delivers the complete artifact but does not
provide the room relay. Physical phones require the separate HTTPS staging
workflow and an exact successful non-PR validation artifact; PR artifacts are
never fed into privileged deployment. Do not merge or publish production as a
part of testing this branch.

## Validation and device status

Validation results and measured presentation sizes will be recorded after the
compiled browser checks finish. Physical iPhone Safari, real Android browsers
and two separate networks are **unverified**. Use the existing
[phone acceptance checklist](PRIVATE_COOP_PHONE_ACCEPTANCE.md), replacing the
arena selection with Play Welcome to Antarctica and additionally checking coin
collection, powerups, enemy contact, checkpoint restart and level completion.

Phase 8 remains responsible for physical-device acceptance, smoother guest
presentation, coordinated usability improvements and incremental content
expansion. This phase does not claim support for all worlds, scripts or bosses.
