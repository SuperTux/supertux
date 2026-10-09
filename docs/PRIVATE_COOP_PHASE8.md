# Phase 8: private co-op on phones

Phase 7B is merged into `mobile-web-audit` at
`c385d97aba1476a984e497a33c1bd24183c55293`. Its supported shared-view level is
the original **Welcome to Antarctica**. The host owns simulation, audio,
menus and saves; the guest renders authoritative state and controls Player 2.
The existing shared camera keeps both living players visible.

## Order of work

| Stage | Deliverable | Acceptance |
| --- | --- | --- |
| 8A | Exact staging runtime and repeatable measurements | Verified build identity, browser evidence and before-change response/loading/traffic measurements |
| 8B | Smoother guest presentation | Safe camera/object interpolation improves measured presentation without stale objects or growing queues |
| 8C | Phone Host/Join flow | Share/copy invitation, meaningful loading progress, readiness and touch entry work without diagnostic instructions |
| 8D | Predictable interruptions and replay | Shared pause/recovery, neutral input, clear completion and bounded room lifetime |
| 8E | Physical-phone acceptance | Both roles complete the supported level on iPhone Safari and Android across separate networks |
| 8F | One additional supported level | Inventory, new interactions, coordinated transitions and real-device completion pass before advertising support |

8B–8D can proceed locally while CI/device access is unavailable. Physical
acceptance remains the gate for claiming phone co-op support and expanding
content. Keep production publication separate.

## Phase 8A implementation

`tests/wasm/coop_benchmark.py` exercises two separate browser processes against
the real local Worker/private R2/Durable Object. Each run creates fresh temporary
persistent profiles, measures a cold visit, then opens a warm page in the same
profile. Profiles are removed only after their browser processes close.
The guest's HTTP cache stays enabled. Host developer HTML routing disables its
HTTP cache; its warm visit therefore measures reuse from the separate persistent
asset store. Guest and host startup durations are recorded separately.

After both browsers join, the host selects the original supported level. A
host-only console fixture places both players on safe terrain near spawn and
enables safety. Repeated alternating LEFT/RIGHT presses use the ordinary guest
input relay. Each sample starts at the guest keyboard event and ends when its
Canvas2D render callback has updated the authoritative drawn state by at least
one world pixel. This avoids confusing input acknowledgment or travel to 30
pixels with first visible response. It observes JavaScript/drawn state, not a
physical display's photon timing.

The report retains all samples, median and nearest-rank p95, selection-to-ready,
frame arrival/draw intervals, local receive-to-draw age, draw callback cost,
host animation-callback and native input-update intervals, UTF-8 JSON bytes,
largest snapshot and peak
client WebSocket buffered bytes. Rates use the total measured window rather
than averaging per-run rates. Host animation callbacks are diagnostic browser
scheduling observations, not native simulation FPS. The movement route measures
the spawn-area presentation, not worst-case whole-level bandwidth.

Resource Timing records subresource requests, encoded/decoded HTTP body sizes
and browser-reported transfer sizes. The local HTTP proxy separately counts
actual artwork/startup requests and streamed response-body bytes, making cache
reuse observable even when cached Resource Timing sizes are omitted or misleading.
These body counters exclude HTTP headers. Resource Timing excludes document
navigation, WebSocket traffic and TLS/IP overhead. Zero values alone do not prove a cache hit in an
engine without size reporting. The resource buffer is enlarged so the 1,049
presentation PNGs are retained. Reports record the exact runtime configuration
and the hashes of the measurement scripts; invitation credentials are excluded.

`tools/web/coop/relay_proxy.mjs` adds an ordered byte-stream profile after the
local relay starts. The constrained profile uses 150 ms nominal RTT, 30 ms
jitter range and 5 Mb/s per direction. It preserves ordering, streams ordinary
HTTP without shaping and bounds queued bytes with backpressure. This is synthetic
local WebSocket transport testing, without cellular/WAN/TLS emulation. It does
not drop arbitrary gameplay messages or introduce reordering that TCP would
prevent. The proxy refuses remote upstreams and is never deployed.

No simulation, renderer, guest protocol or original assets change in Phase 8A.
Staging's phone instructions now select the actual campaign level rather than
the static display test. GNU/Linux manual validation gains an optional
`full_matrix` input: true remains the default; false requests its existing one
representative Release build. Required gate names, automatic selection and full
weekly/tag coverage remain unchanged.

Live repository ruleset inspection returned an empty list and the integration
branch reports `protected: false`. Classic branch-protection inspection is
unavailable to the integration (HTTP 403), so that rule verification remains
incomplete. Repository rules/settings and required check names are unchanged.

### Reproduce the baseline

Use the pinned toolchain and complete assembly instructions in
[PRIVATE_COOP_SUPPORTED_LEVEL.md](PRIVATE_COOP_SUPPORTED_LEVEL.md). The artifact
must pass complete source/configuration/manifest verification before the runner
starts. Run browser suites separately from compilation, especially on limited
CPU hosts. Install the locked local relay dependencies with
`npm ci --prefix tools/web/coop --ignore-scripts`.

```sh
python tests/wasm/coop_benchmark.py build/upload --output evidence/chromium
python tests/wasm/coop_benchmark.py build/upload --profile constrained --output evidence/chromium-constrained
python tests/wasm/coop_benchmark.py build/upload --browser webkit --output evidence/webkit
python tests/wasm/coop_benchmark.py build/upload --browser webkit --profile constrained --output evidence/webkit-constrained
```

Defaults are three runs and ten movement samples per run. Output must be new or
empty, preventing accidental mixing of old and new evidence. Supply
`--webkit-executable` where a local dependency launcher is needed. A hosted
loopback-profile run supports `--url HTTPS_STAGING_ORIGIN`, but fails if the
remote runtime identity differs from the supplied artifact. The constrained
profile only supports a local relay. Debug requires the existing explicit
`--record-known-ub` option; newly introduced sanitizer sites still fail.
Reports written before a failed command exits are partial evidence. Check the
exit status, `failure.txt`, and the trial/sample counts before comparing results.
The report also records requested run/sample counts and a `complete` flag.

`--ephemeral` retains diagnostic private-context coverage. A controlled HTTP
probe found that Linux WebKit's ephemeral context fetched the same immutable
response on every attempt, while a persistent profile reused it across same-tab
navigation and a new tab. Initial ephemeral-context timing results are retained as
diagnostics rather than representing ordinary Safari warm-cache behavior.
This probe used a simple local HTTP endpoint with a counted response body;
physical iPhone Safari cache behavior still needs acceptance testing.

Do not use these timing observations as a flaky CI performance threshold. The
summary/transport tests are included in existing focused tests; the benchmark
is explicit evidence collection. Existing browser/audio/touch/save/co-op checks
remain the correctness gates. Significant runtime changes in 8B–8D also require
Debug WASM and representative native coverage.

### Measured baseline, 2026-10-09

The primary baseline is the merged Phase 7B runtime at `c385d97aba1476a984e497a33c1bd24183c55293`,
Release, Emscripten 6.0.11, with manifest inventory
`60acd956e19c043bb01f777dffe6448cafa6097822037ff6f71695b30a6e15c5`.
Each row contains three independent regular-profile trials and 30 movement
samples. Chromium 151.0.7922.34 used SwiftShader; Linux WebKit reported 26.5.
Playwright 1.62 ran on a Linux cloud host with a four-CPU quota, two separate
browser processes, 844 × 390 touch viewports and device scale factor 1.
Builds, correctness suites and benchmark profiles ran sequentially.

| Browser / WebSocket profile | First draw median / p95 / maximum (ms) | Received updates/s | View JSON bytes/s | Draw callback median / p95 (ms) |
| --- | --- | ---: | ---: | --- |
| Chromium / loopback | 228.1 / 410.2 / 781.6 | 7.53 | 201,876 | 1.8 / 9.2 |
| Chromium / constrained | 494.4 / 1,495.1 / 2,539.0 | 7.30 | 195,776 | 1.7 / 5.6 |
| WebKit / loopback | 223.5 / 252.0 / 256.0 | 8.19 | 219,294 | 2.0 / 7.0 |
| WebKit / constrained | 482.5 / 594.0 / 636.0 | 8.08 | 216,392 | 2.0 / 8.0 |

Median receive-to-draw age was approximately 100 ms in every row, consistent
with the existing 90 ms presentation delay plus render scheduling. Campaign
frames remain discrete at approximately eight updates/s. This supports safe
presentation interpolation as the first 8B work; it does not establish that
reducing the buffer alone improves correctness or response. The largest measured
spawn-area frame was 27,276 UTF-8 bytes. Host native input-update intervals had
medians of 15–16 ms and p95 of 24–32.7 ms, but Chromium retained stalls up to
2,359.3 ms. Those stalls and all response outliers remain in the evidence.
Browser animation-callback intervals differ from native input-update intervals;
neither metric establishes physical-device simulation performance.

Loading medians below use three observations per row. Host readiness ends at
`Module.supertuxReady`, before the trusted Start/menu interaction. Guest readiness
includes verified artwork and a connected room; level selection happens later.
The last column starts with the host's campaign button and ends at the guest's
playable native baseline. These durations are separate steps, not one combined
time to gameplay.

| Browser / profile | Host cold / warm (s) | Guest cold / warm (s) | Select level → guest playable (s) |
| --- | --- | --- | ---: |
| Chromium / loopback | 4.20 / 9.21 | 8.68 / 6.23 | 1.00 |
| Chromium / constrained | 3.69 / 9.97 | 9.16 / 6.97 | 1.23 |
| WebKit / loopback | 7.15 / 3.34 | 14.34 / 11.09 | 0.89 |
| WebKit / constrained | 7.25 / 3.23 | 21.04 / 16.50 | 1.04 |

Every one of the twelve trials produced these actual local upstream counts:

| Payload | Cold requests / response-body bytes | Warm requests / response-body bytes |
| --- | ---: | ---: |
| Host DATA + WASM delivery | 2 / 137,499,108 | 0 / 0 |
| Guest presentation PNGs | 1,049 / 18,637,169 | 0 / 0 |

These counters exclude HTML, JS, manifests, headers and transport overhead.
HTTP is unshaped loopback even in the constrained profile. Warm visits reuse the
same live browser profile across navigation/new tabs; closing and reopening the
browser process is not measured here. Chromium's host warm duration was slower
despite zero package requests. Persistent-store processing/browser scheduling
needs separate profiling before claiming a startup speed improvement. WebKit's
cached Resource Timing still reported some transfer bytes despite zero actual
art requests; the backend counters avoid treating those values as downloads.

[The machine-readable summary](measurements/private-coop-phase8a-2026-10-09.json)
retains runtime/tool hashes, conditions, every per-run loading duration, HTTP
counts and distributions. Raw reports, console logs and screenshots are retained
in this cloud environment at `/workspace/supertux-env/phase8a-evidence/`.
An additional 120 private-context samples remain diagnostic evidence, not the
regular-profile baseline. There is no before/after gameplay improvement in 8A:
the runtime and assets are unchanged. No WAN, physical iPhone/Android,
uninterrupted level completion or whole-level worst-case traffic result is
claimed.

Local correctness validation passed Release and Debug WASM builds and complete
artifact verification, 20 co-op scenarios each in Release Chromium, Release
WebKit and Debug Chromium, all four native CTest cases, 66 JavaScript tests,
16 web-tool tests, six CI-selection/gate tests, actionlint and Cppcheck 2.22.
Debug recorded the same five documented upstream sanitizer sites and no new
sites. Fixtures cover native supported-level interactions and completion; they
do not substitute for ordinary physical-phone playthroughs. Existing dedicated
audio/touch/save browser gates remain configured; their earlier evidence is
retained rather than described as newly rerun by this measurement-only change.

### Hosted validation and staging

Phase 8A dispatch of WebAssembly at the merged integration branch was rejected
with HTTP 422: **“Actions has been disabled for this repository.”** Workflow
listings say active, but the integration cannot read repository-wide Actions
permissions (HTTP 403). No setting was changed. The public private-co-op staging
manifest still identifies the old Phase 7A runtime; merging Phase 7B did not
publish it. Physical phones and separate networks remain unverified.

Once the repository owner enables Actions, validate an exact runtime ref:

```sh
gh workflow run wasm.yml --repo jbbejena/supertux --ref mobile-web-audit
# After this tooling PR is merged, select one native build:
gh workflow run gnulinux.yml --repo jbbejena/supertux --ref mobile-web-audit -f full_matrix=false
# Wait for success, then supply the exact tested runtime and WASM run:
gh workflow run mobile-web-preview.yml --repo jbbejena/supertux \
  --ref mobile-web-audit -f ref=FULL_TESTED_RUNTIME_SHA \
  -f validation_run_id=SUCCESSFUL_WASM_RUN_ID -f publish_coop=true
```

Before dispatch, verify which source the ref currently selects. Both build types
and the native check must succeed for the intended source. Staging reuses only
the explicit successful non-PR artifact and verifies source, configuration,
toolchain, manifest and every payload; an old/unverified latest artifact is not
accepted. Publication remains serialized on the separate Worker. Existing `tux`
credentials stay in GitHub secrets. Production is not part of this procedure.

## Follow-on implementation boundaries

Phase 8B implementation and validation are recorded in
[PRIVATE_COOP_SMOOTHING.md](PRIVATE_COOP_SMOOTHING.md). The 8A tables above remain
the historical before-change baseline.

For 8B, establish the native draw coordinate contract before camera smoothing:
commands already contain camera-transformed positions. Handle parallax, clip
rectangles and screen-space panels without applying a second camera transform.
Match stable objects/commands explicitly; one UID may own several changing
commands. Interpolate safe movement, while animation art, pickups, removals,
tile changes and lifecycle transitions stay discrete. Snap on teleport,
death/respawn, restart, viewport changes and ambiguous correspondence. Keep
complete baselines, bounded history, no extrapolated gameplay and host authority.
Profile before introducing asset-ID compaction or deltas; deltas would need
explicit baseline/resynchronization and hibernation/slow-receiver recovery.

For 8C, provide an obvious co-op entry only where the server supports rooms.
Host game → share/copy guest-view invitation → Player 2 ready → Start together.
Join game accepts the existing private invitation, verifies art and shows
progress/retry before acknowledging the scene. Use trusted Start/Resume for host
audio. Keep proof/controller-only scenes, hashes and packet diagnostics outside
normal play. Respect safe areas, portrait/landscape, multitouch and cancellation.

For 8D, explicitly coordinate waiting/loading/playing/paused/interrupted/complete.
Host backgrounding pauses simulation; guest loss neutralizes input and pauses
the prototype at a safe simulation boundary. Resume starts a fresh neutral
generation and preserves trusted Safari audio activation. Recovery can remain
title screen → fresh/rejoined room → restart; seamless mid-level reconnect is
deferred. Completion offers Replay/Return to lobby with a new acknowledged epoch.
The current **entire room expires after 15 minutes**, including active play.
Separate the invitation window from bounded active-session/idle limits, chosen
from measured level duration. Warn before a hard expiry, pause safely and offer
a fresh-room path. Heartbeats must not create an indefinite room lifetime.

For 8E, use real iPhone Safari and Android Chrome, swap roles, and test Wi-Fi
versus cellular. Complete the level without placement/bonus fixtures. Verify
shared camera, touch releases, pickups, enemy contact, checkpoint restart,
completion, rotation, lock/app switching, lost network, recovery and host save
persistence. Supplement hard-to-reach fire/ice cases with labelled diagnostic
coverage. Record devices, browser/OS, network, runtime identity, cold/warm state
and failures. Linux WebKit/mobile emulation cannot satisfy this gate.

For 8F, inventory one additional level's new objects, scripts, drawing/lighting
and transitions. Fetch its required art before acknowledged start and reuse
unchanged content hashes, without making all worlds mandatory guest downloads.
Maintain a level/interaction support matrix and validate ordinary completion
on two devices before exposing the level as supported.

Use separate reviewable PRs for presentation, entry flow, lifecycle policy and
new content. Public matchmaking/accounts, more players, host migration, guest
physics/prediction, save merging, new camera rules, a WebRTC rewrite and full
campaign support remain outside these increments.
