# Phase 8B: smoother private co-op presentation

Implementation base: `mobile-web-audit` at
`316292b337860a0545f56033ef2a1969d80f47e7` (merged Phase 8A PR #15).
Frozen runtime tested locally: `5303342794e2fa7d844842d60d347abfb0a2ee6c`.
Later commits in this PR only add measurement failure diagnostics and documentation.
The supported campaign scope remains the original **Welcome to Antarctica**,
one host and one guest, using the existing shared camera. The host owns all
simulation, collisions, audio, lifecycle, completion and progression saves.

## Presentation contract

The host still sends complete bounded snapshots at no more than ten updates/s.
The guest never predicts gameplay, extrapolates movement, runs a simulation or
requests the game WASM/DATA. It renders positions between two received native
snapshots with the existing 90 ms presentation delay. It holds the newest
available baseline if there is no future endpoint.

Native texture destinations already contain translation, viewport and scale.
The browser must not apply the camera again. Each captured draw command now
optionally ends with `[translationX, translationY, scale]`, from the actual
drawing context used by that request. Text capture carries the same metadata.
This distinguishes world geometry, parallax and screen-space overlays without
guessing one global camera transform. `world.playerUids` maps Player 1/2 to their
native object UIDs, independently of draw order or object enumeration.

`SnapshotBuffer` recovers local geometry anchors solely for correspondence. It
indexes individual texture quads by owner, kind, layer, clipping, shape and art
identity. Static tiles retain exact artwork; moving owners can match across
sprite animation/facing changes. Each candidate must match uniquely in both
directions. Tile batches can reorder, grow or shrink without pairing by array
index. A UID can own several requests. Ambiguous, missing, changed-size or
distant candidates remain discrete. Spatial matching is conservatively capped
at a quarter of the smaller quad dimension and at most 32 local pixels.

Only matched destination rectangles and moving labels are interpolated, directly
between native screen coordinates. Camera/player diagnostic positions follow
the same interval; a player's position only interpolates when its owner has
matched texture geometry. Original snapshots are immutable. Art frames, facing,
colors, fades, angles, clipping, new/removed objects, tile changes, coins,
checkpoint and authoritative phase remain from one complete discrete baseline.
Nonmoving panels/text/gradients/lines remain discrete. A changed tile cannot
slide into a removed neighbour, and the next exact baseline discards removals.

The sampled host time is monotonic, so delivery jitter cannot rewind the picture.
History remains at most eight complete snapshots and one cached matching plan
for the current pair. Pausing, backgrounding, restart, room reset and disconnect
clear that plan. New sessions/epochs/scenes/generations, viewport changes,
death/visibility/size-category changes, player/camera jumps over 128 pixels,
large zoom changes, phase transitions and host-time gaps over 500 ms snap to
a fresh complete baseline. The existing 1,200 ms stale-update freeze releases
guest controls. Completion still stops drawing and neutralizes controls.

Worker validation accepts the older eight-field commands without interpolation
and validates new transform bounds and player UID membership. Exact manifest
identity still keeps room participants on one build. Limits remain 256 requests,
2,048 quads, 512 entity descriptions and 64 KiB per encoded message, with native
capture failing closed before those limits. No new route, bucket, binding,
privileged artifact path, dependency or production deployment is introduced.

## Build, preview and rollback

Use the pinned build/assembly instructions in
[PRIVATE_COOP_SUPPORTED_LEVEL.md](PRIVATE_COOP_SUPPORTED_LEVEL.md). The complete
preview must contain both the host artifact and hashed guest presentation art.
The local `tools/web/coop/preview.mjs` runs the real private relay without
Cloudflare credentials. Start → Private co-op → Create room → share Guest shared
view → Play Welcome to Antarctica after Player 2 joins.

In the prepared cloud workspace, the exact tested Release preview is
`/workspace/supertux-web-release/preview-phase8b`:

```sh
cd /workspace/supertux
source /workspace/supertux-env/env.sh
node tools/web/coop/preview.mjs /workspace/supertux-web-release/preview-phase8b 0
```

Open the host URL printed after `INPUT_PROOF_READY` in a browser that can reach
that local server, then follow the Host/Guest steps above with a second browser.
Port `0` chooses a free port and preserves the existing server on port 8787.
This local loopback preview is not a public phone/share link. For another
checkout, assemble the complete artifact first using the linked build guide.

Publish private HTTPS staging only through the exact successful non-PR artifact
procedure in [PRIVATE_COOP_PHASE8.md](PRIVATE_COOP_PHASE8.md). Production remains
separate. Reverting this runtime change restores discrete campaign frames; old
eight-field snapshots also retain a discrete fallback in the new guest.

## Measurement and acceptance

Use `tests/wasm/coop_benchmark.py` sequentially with regular persistent temporary
profiles. It now separates received updates from actual Canvas2D draws and
reports the fraction of draws using interpolation. Receive-to-draw age uses the
newest required endpoint for an interpolated draw. Callback cost includes
correspondence planning and painting. Draw frequency includes repaints during
stationary intervals; the compiled co-op suite separately asserts changing
intermediate native player quads within the same snapshot pair.

Cold/warm state, viewport, browsers, tool hashes, runtime identity, network
conditions and all outliers must accompany comparisons. HTTP remains unshaped
loopback; the constrained profile shapes ordered WebSocket bytes only.
Movement fixtures near spawn do not establish uninterrupted completion or
whole-level worst-case traffic. Physical iPhone Safari/Android and separate
networks remain separate acceptance gates.

Phase 8C will improve Host/Join entry. Phase 8D will address interruption/replay
and the current entire-room 15-minute expiry. This change does not claim those
later stages are complete.

### Equivalent before/after results

The before artifact is the preserved Release runtime
`c385d97aba1476a984e497a33c1bd24183c55293`. Its game/renderer/Worker sources
are unchanged through the merged Phase 8A base; only measurements, workflows
and documentation changed there. After is the frozen Phase 8B runtime above.
Both sides used the same current compatible local Worker and proxy, and the
same updated measurement tools at `9da2714bee979e15ab313420e4bf64c2339bf6a2`;
their hashes and frozen copies are retained. Later failure-capture hardening only
handles browser-launch failures and keeps the joining guest available for diagnostics;
it does not change successful-trial timing or the game runtime.
This reruns the baseline rather than mixing the older 8A timing definitions.

Conditions: Linux cloud host with a four-CPU quota; Playwright 1.62; Chromium
151.0.7922.34 with SwiftShader and Linux WebKit 26.5; separate host/guest browser
processes, 844 × 390 touch viewport, DPR 1. Runs/builds were sequential. Each side
requested three fresh-profile trials with ten alternating guest movement samples.
Warm pages reused the same live regular browser profiles. No browser-process
restart, physical phone, cellular network or photon timing was measured.
Constrained means nominal 150 ms RTT, 30 ms jitter range and 5 Mb/s each direction
for ordered WebSocket traffic; HTTP stayed unshaped on loopback.

**16 of 24 primary trials completed.** Statistics below condition on those
completed trials, ten response samples each. Failed trials retain partial samples,
states and screenshots separately; their sets exit nonzero and `complete` stays
false. They are not zeros or successful results. There were no retries within the
primary sets. All completed-trial outliers remain included; failure counts prevent
these selected timings being mistaken for reliable session acceptance.

| Profile | Completed trials, before → after | First draw median / p95, ms, before → after | Canvas draws/s, before → after | View JSON kB/s, before → after |
| --- | --- | --- | --- | --- |
| Chromium, loopback | 3/3 → 2/3 | 225.6 / 455.1 → 171.4 / 394.3 | 6.4 → 27.1 | 170.5 → 140.8 |
| Chromium, constrained | 1/3 → 0/3 | 552.2 / 2489.2 → Unavailable | 6.5 → — | 175.3 → — |
| WebKit, loopback | 2/3 → 3/3 | 232.5 / 253.0 → 150.0 / 188.0 | 8.3 → 28.6 | 222.4 → 218.4 |
| WebKit, constrained | 2/3 → 3/3 | 464.5 / 557.0 → 431.0 / 542.0 | 8.2 → 28.6 | 222.0 → 218.8 |

The completed loopback trials show more frequent presentation and a lower median
response. The new WebKit constrained set also completed all three trials. The new
Chromium constrained set completed none, so it establishes no response or traffic
result. Draw rates count stationary repaints; the compiled suite independently
proves changing intermediate geometry. Interpolation is not native simulation FPS.
Animation art still updates discretely and no extrapolated state is introduced.

| Profile | Canvas callback median / p95, ms, before → after |
| --- | --- |
| Chromium, loopback | 2.0 / 8.4 → 2.0 / 17.1 |
| Chromium, constrained | 1.7 / 9.5 → Unavailable |
| WebKit, loopback | 2.0 / 6.0 → 2.0 / 15.0 |
| WebKit, constrained | 2.0 / 5.0 → 2.0 / 14.0 |

The drawing-cost tail increased. View JSON throughput depends on the observed
update rate and is not a compression result. For example, WebKit loopback received
8.30 → 7.85 updates/s and its largest spawn-area message grew 27,174 → 28,220 bytes.
The transform/identity metadata has a real payload cost. Across accepted compiled
co-op suites the largest complete campaign message was 51,503 bytes, within the
unchanged limits. These spawn measurements do not cover whole-level worst-case
traffic or battery use.

Startup assets were not changed in this stage. The raw startup DATA remains
176,669,514 bytes; guest art is 1,049 original hashed PNGs, 18,637,169 bytes. The
backend counted cold host DATA/WASM bodies of 137,499,108 → 137,499,095 bytes
(an incidental WASM change, not a packaging optimization). Every completed trial
counted zero warm host package and zero warm guest-art network requests/body bytes.
That demonstrates reuse in live profiles, not persistence across browser restarts.
All resource timing/body counters and loading distributions remain in raw reports.

| Loopback role/browser | Cold ready median, seconds, before → after | Warm ready median, seconds, before → after |
| --- | --- | --- |
| Host, Chromium | 5.64 → 4.77 | 13.79 → 9.14 |
| Guest, Chromium | 12.30 → 10.44 | 7.30 → 7.42 |
| Host, WebKit | 9.73 → 11.57 | 4.65 → 4.08 |
| Guest, WebKit | 19.72 → 18.36 | 18.07 → 14.17 |

Host ready is before trusted Start/menu; guest ready is before campaign selection.
These small, completion-conditioned samples are not a startup speed claim. Warm
reuse can still spend seconds mounting/decoding assets; Chrome's warm host was
slower than its cold host in this environment. Original startup-byte reductions
remain documented separately in MOBILE_WEB_STARTUP_OPTIMIZATION.md.

### Failures and acceptance limits

Primary failures: one new Chromium loopback trial, two old Chromium constrained
trials and all three new Chromium constrained trials failed during movement;
one old WebKit trial in each profile failed during warm guest join. Host captures
include native-update gaps of 2.79–5.43 seconds and animation-callback gaps up to
25.60 seconds. Several hosts explicitly reported `Connection timed out` after
those gaps; the active-host relay idle limit is 2.5 seconds. A guest disconnected
in another trial while the captured host still appeared connected. Warm-join
captures showed the host disconnected before the guest was ready. No schema or
console exception identified interpolation as the cause. The cloud shell also
briefly lost its transport connection during measurements. The exact source of
host stalls and every disconnect is not isolated; these observations are not a
claim about physical-phone behavior or a completed root-cause fix.

No timeout was relaxed and no failed assertion was removed. The benchmark now
captures failure stage and available host/guest state, runs the remaining
independent fresh-profile trials, then fails the set if any trial failed. The two
pre-primary baseline attempts (initial settling failure, then sample-9 host-left
failure) and the initial compiled-suite failure are also retained, not added to
primary successes. Lifecycle/keepalive/recovery investigation is a priority for
Phase 8D before phone acceptance; more frequent drawing does not fix it.

Compact evidence: [private-coop-phase8b-2026-10-09.json](measurements/private-coop-phase8b-2026-10-09.json).
Raw reports, samples, failed states, screenshots, exact measurement scripts and
build/test logs remain in `/workspace/supertux-env/phase8b-evidence/`.

## Local correctness results

Release and Debug WASM were rebuilt using Emscripten 6.0.11 and the pinned
vcpkg revision. Both complete artifacts passed exact source/configuration/
manifest/payload verification at the frozen runtime above. The GCC 14 native
Debug build succeeded and all four CTest cases passed.

| Suite | Browser | Checks | Result |
| --- | --- | ---: | --- |
| Compiled co-op, Release | Chromium 151 | 21 | Pass on rerun; first attempt described below |
| Compiled co-op, Release | Linux WebKit 26.5 | 21 | Pass |
| Compiled co-op, Debug | Chromium 151 | 21 | Pass; five known upstream sanitizer sites retained |
| Single-player/audio/save/viewport, Release | Chromium / WebKit | 12 each | Pass |
| Native touch and cancellation, Release | Chromium / WebKit | 10 each | Pass |

The 107 browser scenarios include actual changing intermediate native player
quads, matching player identities, terrain/parallax, information panels, tile
removal, enemy contact, growth, fire/ice, checkpoint restart, completion, touch
releases, host pause, generation reset, jitter and background/disconnect input
neutralization. Placement/bonus fixtures are labelled and do not count as an
ordinary complete phone playthrough. No new console errors were accepted.

Focused validation passed: 72 JavaScript tests, 16 web-tool tests, six CI tests,
workflow actionlint and Cppcheck 2.22 with repository workflow flags. An intentional
missing-browser CLI fault retained all three launch failures, recorded zero
successful trials and `complete=false`, and returned exit code 1. The new
regressions cover immutable geometry, discrete art/facing, reordered/culling
batches, ambiguity/legacy fallback, teleports/death/growth/viewport/generation/
phase/gap barriers, monotonic jitter handling and clear/reset ordering.

One initial full Release Chromium attempt timed out while moving from the arena
to the campaign baseline. Its guest reported relay timeout; no schema or console
exception was observed. A fresh campaign pilot and the unchanged full rerun,
WebKit and Debug suites passed. The cause was not fully isolated and no timeout
or assertion was weakened. The failed report/logs remain in the raw evidence;
interruption reliability still needs Phase 8D and real-device acceptance.

## CI, staging and remaining gates

This PR changes shared C++ presentation code, browser code, the narrow Worker
schema and measurement tooling. The existing selector requests focused tests,
lint, Release WASM, Debug WASM and one representative Linux build. Other native
platforms remain available through their manual/full, weekly and tag coverage;
no workflow, required check name, gate, permission or concurrency setting changes.
Documentation-only revisions remain compilation-free under the existing policy.

On 2026-10-09 the integration branch remained at the base above and the newest
Actions runs were still dated 2026-10-08. The earlier dispatch rejection was
HTTP 422, "Actions has been disabled for this repository." No repository setting
was changed. Rulesets were empty and the integration branch reported unprotected;
classic protection inspection returned HTTP 403 and remains unverified.

The private staging manifest still identifies Phase 7A runtime
`e5cfbe08ea4bb4959ad8b84cd3763366dd8eeba1`. This PR has no new hosted validation or
publication. After Actions is enabled, run the existing Release/Debug WASM and
representative Linux workflows on the reviewed exact source, then publish only
its explicit successful non-PR validation artifact using the Phase 8A procedure.
Do not merge or deploy production as part of this PR.

Physical iPhone Safari, Android Chrome, separate networks, app switching/lock,
rotation during play, ordinary supported-level completion and role swapping
remain unverified. Follow [the phone acceptance checklist](PRIVATE_COOP_PHONE_ACCEPTANCE.md)
after private HTTPS staging is current. Host/Join polish is Phase 8C; coordinated
interruption/replay and safe room expiry are Phase 8D. The current entire-room
15-minute limit remains unchanged. More content is gated on those acceptance
results; a full guest game/simulation or seamless reconnect is outside this stage.
