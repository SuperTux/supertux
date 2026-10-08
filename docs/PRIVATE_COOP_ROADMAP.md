# SuperTux private browser co-op roadmap

Updated: 2026-10-08 UTC (2026-10-07 in Denver).

Status: phases 5, 6A and the contained 6B remote-input proof are implemented.
Phase 7A adds a controlled guest display of static terrain, both players and the
host's shared camera. Phase 7B gameplay/object replication remains pending.
Evidence, supported scenarios and
device/network limits are in [PRIVATE_COOP_INPUT_PROOF.md](PRIVATE_COOP_INPUT_PROOF.md).
The separate display architecture and acceptance scope are in
[PRIVATE_COOP_GUEST_VIEW.md](PRIVATE_COOP_GUEST_VIEW.md).

## Goal and first playable scope

Two friends open SuperTux in their browsers, create/join a private room, and play together in the same world. Reuse the existing local co-op game, mobile controls, browser shell, and Cloudflare deployment.

The first playable prototype supports:

- Exactly two players: one host and one guest.
- One explicitly selected and validated level, with a documented list of supported objects and interactions.
- Joining before the level starts, with both players ready before simulation begins.
- The existing shared-camera and local co-op rules.
- Host-controlled level selection, world-map navigation and progression saves.
- Clear waiting, pause, connection-loss and unsupported-content messages.

Expand campaign coverage only after this slice works on separate devices and networks. Keep single-player and local co-op working throughout.

## Current foundation and remaining work

The input-proof baseline is `mobile-web-audit` at `c49c8a97cfc64403c3bba9c37637ba9083142de7`, including merged startup PRs #8/#9 and roadmap PR #10. Repository default `master` is not the complete browser integration baseline.

The user reports that mobile browser play now works. Browser boot, touch controls, viewport/lifecycle/audio integration, save hydration, Cloudflare Worker/R2 delivery, and startup download optimization provide the foundation. Existing automated browser evidence and device limits are recorded in [MOBILE_WEB_STARTUP_OPTIMIZATION.md](MOBILE_WEB_STARTUP_OPTIMIZATION.md); this report of playability does not certify every mobile device or a full campaign.

The engine's existing local co-op now has compiled-browser coverage for two
independent input owners, touch plus Player 2, device rejoin, death/respawn,
checkpoint restart, door/sector transition, completion and host persistence.
An explicit transient remote slot and opt-in private Worker/Durable Object
relay provide the two-browser input proof. There is no runtime world replication
for campaign objects. The controlled Phase 7A scene now has a guest world display.
Coverage is contained to the documented interactions;
it does not certify a full campaign or physical phones.

The separate, unmerged `fix/stalled-music-downloads` branch addresses stalled optional music requests. It was inspected and is not duplicated by the input proof. Refresh its merge status before subsequent integration.

## Architecture: host simulation, guest input, WebSocket relay

The host browser runs the authoritative game simulation for both players. It owns collisions, enemies, scripts, object creation/removal, pickups, deaths, checkpoints and transitions. The guest sends ordinary gameplay controls and displays state supplied by the host. The guest must not independently execute authoritative interactions or write host progression.

Use WebSockets first, with a Cloudflare Worker routing connections to a Durable Object for each room. The room handles membership and relays bounded messages between the two browsers. The game simulation stays in the host browser. Cloudflare documents this multi-client coordination model in its [Durable Objects WebSocket guidance](https://developers.cloudflare.com/durable-objects/best-practices/websockets/).

Keep transport outside Player movement and physics. Queue incoming messages at a narrow browser-to-WASM boundary and consume them at a defined simulation point. Define a small protocol with build/content identity, session/level identity, input sequence numbers, state updates and critical events. Reject incompatible builds before starting.

This WebSocket-first choice supersedes the earlier conditional WebRTC-first recommendation in [MOBILE_WEB_AUDIT.md](MOBILE_WEB_AUDIT.md). Keep transport replaceable; consider WebRTC only after measuring relay latency and demonstrating a need. Movement prediction/reconciliation also follows evidence from the playable prototype.

The largest task is the guest representation of the world. Forwarding Player 2 inputs alone does not synchronize a second browser. Existing save/editor serialization is not a complete network snapshot system, and two independently simulated worlds are not assumed deterministic.

## Implementation phases and acceptance checks

The numbering continues the original audit's phases 5–8. Each phase produces a reviewable result before the next architectural expansion.

| Phase | Deliverable | Acceptance evidence |
| --- | --- | --- |
| **5 — Browser local co-op** | Validate two local players and fix reproduced browser/controller defects. | Independent movement/jump/action; single-player and all-player death; respawn; checkpoint restart; sector transition; completion; player removal and save reload. |
| **6A — Remote controller ownership** | Add a remote-owned Player 2 input slot, initially driven by an in-process/replay source. | Correct held/pressed/released behavior; safe player creation/removal; local input/hotplug cannot steal the slot; stale/disconnected inputs become neutral. |
| **6B — Two-browser input proof** | Add a minimal private WebSocket relay and diagnostic guest controls. | A second browser controls Player 2 on the host's real level; version mismatch and a third client are rejected; disconnect clears controls; queues remain bounded. Guest presentation may still be diagnostic. |
| **7A — Guest display foundation** | Define stable session/level/entity identities and a guest presentation path. Replicate both players in a controlled scene. | Guest sees both players and their visual/death state; snapshot ordering and interpolation work; guest does not run authoritative collision/script effects. This is not yet the supported-level co-op milestone. |
| **7B — One supported level** | Replicate all state needed by the selected level and verify its interactions. | Both screens agree through enemy contact, pickups, object changes, death/respawn, checkpoint restart and completion. Unsupported content is identified explicitly. |
| **8 — Private co-op usability and expansion** | Complete Host/Join UX, coordinated loading/transitions, lifecycle recovery and incremental content support. | Two real devices on separate networks play the supported scope; host and guest mobile roles are tested; host saves remain authoritative; interruption behavior is predictable. |

### Phase 5: use the existing local game as the reference

Test two keyboard/gamepad sources on desktop browsers and, where supported, one mobile touch player plus a second local input device. Two sets of touch buttons are not required. Verify shared-camera behavior on a small viewport without redesigning the camera.

Inspect the audit's player-count, device membership and hotplug concerns against current code, and distinguish reproduced defects from source-level suspicions. Document intended existing co-op behavior before changing it.

### Phase 6: own the input slot explicitly

Likely seams are `InputManager`, `Controller`, `Level::initialize`, `GameSession::on_player_added/on_player_removed`, and the simulation input update point. A remote slot must create a real Player 2 even though it has no local keyboard/gamepad assignment. Controller and player lifetimes must remain safe, including script-controller switching.

Queue sequence-tagged inputs, preserve button edges, and clear held controls on disconnect, timeout, pause and teardown. Give the guest gameplay control only; host menu, world-map and save authority stay local. Test late/duplicate input and local hotplug while Player 2 exists.

For the relay proof, use ephemeral private room credentials, separate host/guest roles, a two-client limit, room expiry and basic message size/rate validation. Bound buffered output and queued input; a slow peer must not accumulate an unlimited backlog. This stage proves remote input, not synchronized gameplay on the guest screen.

### Phase 7: prove replication in a bounded scope

Choose the test level and inventory every required object type and interaction before claiming coverage. Representative state includes player pose/velocity/animation/bonus/death, enemy and platform state, collectibles, projectiles, blocks/tile changes, spawn/despawn, checkpoints and session state. Preserve host authority over random outcomes and script effects.

Use snapshots for current visual state and explicit delivery/acknowledgment rules for critical events and transitions. Include generation/session identity so late updates cannot affect a restarted level. Load acknowledgment must keep peers on the same scene. Recover missing entity state through a fresh authoritative baseline rather than silently inventing local state.

Measure input-to-visible-response delay, update bandwidth and queue depth under injected delay, jitter and disconnects. On reliable WebSockets, pay particular attention to delayed ordered backlogs and reconnection; transport loss injection belongs at the appropriate layer. Set the performance acceptance budget from these measurements, not from an arbitrary update rate.

### Phase 8: make interruptions understandable

Host backgrounding or loss of heartbeat should pause/freeze the guest with a clear message, not allow divergent play. Explicit host pause stops the shared session. Guest loss neutralizes remote controls and pauses the prototype while waiting for a clear host decision. Resume must retain the existing trusted browser audio/lifecycle flow.

For the first version, a lost connection may require rejoining the room and restarting the level. Seamless in-level reconnect is not required. A host exit ends the room; automatic host migration is deferred.

Initially keep joining between levels. Coordinate each level load before resuming. Expand objects, scripts, bosses and campaign levels incrementally with a supported-content matrix and regression evidence. Validate both host-on-phone and guest-on-phone roles before describing mobile co-op as supported.

## Work deliberately deferred

- Public matchmaking, server browsers, accounts and competitive anti-cheat.
- More than two players, mid-level joining and automatic host migration.
- Dedicated server-side game simulation and generalized rollback.
- WebRTC/STUN/TURN infrastructure before relay measurements justify it.
- Movement prediction before measured responsiveness requires it.
- New camera rules, gameplay balance changes, progression/save schema redesign and guest save merging.
- PWA/offline infrastructure and broad engine rewrites.

## Suggested Codex run boundaries

1. **First run:** Phase 5 validation plus Phase 6A remote-input ownership, then Phase 6B if the prerequisites pass and the transport proof remains contained. Keep a review checkpoint before world replication. If local co-op fails, resolve and document the relevant defects before continuing.
2. **Guest display run:** Phase 7A, reviewed independently because it establishes the simulation/presentation boundary.
3. **Playable-level run:** Phase 7B, possibly split by object family if needed. A single run is not assumed sufficient for full replication.
4. **Usability runs:** Phase 8 room/lifecycle polish and content expansion, each scoped to demonstrated support.

Use a new implementation branch based on the current browser integration branch, inspect current work first, and avoid duplicating concurrent fixes. Preserve existing native, single-player, local co-op and browser startup behavior. Run focused tests plus Release WASM/browser checks; include Debug and representative native validation for substantial shared input/runtime changes. Documentation-only work does not require rebuilding the full native matrix.

For each implementation PR, record the exact build tested, scenarios passed, supported content, latency/bandwidth observations where relevant, and remaining limitations. Separate automated Chromium/WebKit results from physical-phone and separate-network evidence. An implementation handoff will be prepared after this roadmap is reviewed.
