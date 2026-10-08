# Private browser input proof

This implements roadmap phases 5, 6A and the contained 6B input proof. The host
runs the complete game and shows both players. The guest is a diagnostic
keyboard/button controller: it downloads no game world, WASM, DATA or soundtrack,
does not simulate gameplay, and does not show a synchronized world. Phase 7 has
not started. The production Worker has no room binding; networking stays off
unless the separate proof configuration is used and a host explicitly creates
a room.

## Baseline and local co-op fixes

The branch starts from `mobile-web-audit` at
`c49c8a97cfc64403c3bba9c37637ba9083142de7`, including roadmap PR #10 and startup
PRs #8/#9. `master` lacks this browser baseline. The separate, unmerged
`fix/stalled-music-downloads` at `78a4302630d9eade6b33fbd1bdcfc4f84cd4090f`
was inspected and is not duplicated here.

The compiled baseline reproduced a gamepad defect: unplugging/reconnecting the
second emulated browser gamepad during a level failed to recreate Tux2. The
gamepad live-join predicate now agrees with the joystick path. The existing
automatic pause on local device unplug is retained. Local player additions are
flushed before another device event; removal flushes marked Players before their
borrowed controller is destroyed. Additional-player status now grows the actual
savegame status rather than the unrelated dummy status. The latter count and
lifetime issues were established by source review; they were not reported as
independently reproduced crashes. A null climbing reference in sector movement
was also corrected within the transition path.

## Input ownership and lifecycle

`RemoteController` owns transient slot 1 (Player 2) without a keyboard/device
assignment. Slot 0 remains local. Device mapping, rebinding, keyboard writes,
automatic slot management and player menus cannot claim or delete the remote
slot. Local/script calls to `set_control` do not write it. Existing temporary
script controllers still work for end sequences.

Browser callbacks enqueue four-word commands; the game thread copies them into
stack storage in `InputManager::update_remote`. No C++ Player/controller pointer
is retained in JavaScript. Remote previous-state capture and application happen
after local event handling and before screen simulation. State transitions are
consumed one per update, so a down/up pair arriving together retains pressed and
released edges. Identical heartbeat refreshes coalesce. Both queues cap at 32;
overflow clears input and changes generation. Inputs queued for over 250 ms and
held input without refresh for 750 ms also fail neutral.

Each state has a positive sequence and generation. Duplicate/reordered sequences
and old generations are rejected. Pause, console/menu focus, cutscenes, shell
backgrounding, level restart, sector change and disconnect clear held input.
Resume requires fresh input; guest held keys/fingers are cleared on session
changes. Guest controls are only Left, Right, Up, Down, Jump, Action and Item;
menu, pause, map, console and cheat controls are absent.

Join is accepted only at the host title screen with one local player. An
established source survives level/sector restart. In-level rejoin is refused:
return to title, explicitly rejoin, then start a new level. Closing a room ends
input but retains the inert remote slot for safe Player/controller lifetime;
reload the host to return to ordinary local slot management. Network membership
and credentials are never serialized. Saves keep their existing schema, shared
progress and the persistent local player's bonus/pocket; the transient remote
player is omitted.

## Relay protocol and limits

`worker/coop.js` exports an ephemeral `CoopRoom` Durable Object. The separate
`wrangler.coop.toml` supplies its SQLite binding/migration and preserves the
existing streaming private-R2 and same-origin asset routes. The production
`wrangler.toml` is unchanged. `/coop/rooms` returns a random room ID and distinct
256-bit role credentials. The invitation carries only the guest credential in
its URL fragment; socket credentials use WebSocket subprotocols, not URL paths.
Keep the invitation private: it grants the guest input role.

Exactly one host and one guest may connect. A `hello` must match protocol 1 and
the complete frontend/asset manifest hash before input is accepted. `session`
(generation/enabled) and `ack` are host-only; `input`
(generation/sequence/mask) is guest-only. The Worker never runs physics or
receives world snapshots. Origins must match; messages are text JSON with known
fields, at most 512 bytes and 60 client commands/second per socket. Valid `seen`
credits acknowledge existing server output and do not consume the command
budget again; unsolicited credits close the connection. Counting both commands
and their response credits reproduced a healthy WebKit guest disconnect during
checkpoint restart. A receiver has at most 32 unacknowledged outbound messages
(`seen` receive credit). When that window fills, session status, input
acknowledgment and heartbeat reply each retain only their latest value, up to
three pending notifications; credits flush them in that order. A Chromium
checkpoint restart exposed the previous status-history overflow (close 1013).
Gameplay edges, connection events and authentication never enter this
coalescer: exceeding their window still closes instead of buffering history.
The actual two-browser proof deliberately withholds 32 credits, then releases
them and checks that the connection remains usable. Browser output also caps
at 16 KiB. Input refresh runs every 125 ms; host status every 500 ms. Relay
inactivity closes at 2.5 seconds during active play and for guests. A host
whose session is already input-disabled gets a bounded 15-second loading grace
for synchronous WASM level parsing; this never enables guest input. Authentication
closes at 5 seconds; client silence closes at 4 seconds for guests and 20 seconds
for hosts. The C++ held-input watchdog remains 750 ms. Rooms expire after 15 minutes. Host exit closes the guest
and deletes the room. No seamless reconnect or host migration is provided.

## Run the complete proof locally

Use Emscripten **6.0.11** and the pinned vcpkg commit
`c748cb44f2a435fcf015c35225c9d5545fe0021c`. Build as documented in
[MOBILE_WEB_STARTUP_OPTIMIZATION.md](MOBILE_WEB_STARTUP_OPTIMIZATION.md#build-and-preview),
then:

```sh
python3 tools/web/package_assets.py assemble --build build-web --output preview
python3 tools/web/verify_artifact.py preview --source-commit "$(git rev-parse HEAD)"
npm ci --prefix tools/web/coop --ignore-scripts
node tools/web/coop/preview.mjs preview 8787
```

The helper verifies the exact complete artifact, seeds a local private R2 bucket
and runs the actual Miniflare/workerd Worker and SQLite Durable Object. It needs
no Cloudflare credentials. A private local seeding Worker streams files into R2
with known lengths and checks stored sizes; it is not exposed on the preview
listener. This avoids serializing the large DATA file through Miniflare's JSON
RPC, which reproduced `EPIPE` during Release/Debug setup. Keep all deferred music
and manifest files. A plain
`python3 -m http.server --directory preview` still serves normal single-player
and local co-op; it does not provide rooms.

1. Open `http://127.0.0.1:8787/index.html?coop=1`, click the trusted **Start**,
   and expand **Private input proof — diagnostic** while at the title screen.
2. Create a room; open its controller link in a second browser/profile.
3. Wait for Player 2 to join. Close the diagnostic panel and enter the normal
   campaign, or use a title-only **Start Antarctica proof** / **Start forest
   door proof** button. The buttons queue host commands through the game thread
   and the normal `GameManager`/`LevelsetScreen` path; guests cannot invoke them.
4. Host uses existing keyboard/touch input. Guest uses arrows/WASD, Space/J,
   Ctrl/K and Shift, or its direction/Jump/Action/Item buttons. Both Tuxes appear
   only on the host. Pause/Resume belongs to the host's existing trusted shell.
5. After disconnect/backgrounding, return the host to title, click guest
   **Rejoin**, and start a new proof level. Reload the host to leave proof mode
   and restore full local device membership.

For a Debug artifact use a separate build directory with
`-DCMAKE_BUILD_TYPE=Debug`. Native validation uses the normal CMake build with
`-DBUILD_TESTING=ON`, `supertux-unit-tests` and `ctest`.

## Automated evidence and scope

`tests/wasm/coop_smoke.py` launches independent host/guest browser processes and
the actual local relay. `--local-only` instead uses two browser-testable gamepad
sources and an ordinary HTTP server. The host viewport is 844×390 with touch
enabled. Chromium uses software WebGL and trusted CDP host touch; Linux WebKit
uses DOM Pointer Events through the existing SDL bridge. These are desktop
automation/emulation, not physical iPhone Safari evidence.

The normal campaign entry reaches `welcome_antarctica.stl`. Existing host-only
Squirrel getters observe real Players; long scenarios use the existing safety,
position, kill and end-sequence fixtures. Ordinary controller input performs
movement, jump, Action respawn and door Up. The actual checkpoint bell is at
(5360,512); both-player death restarts at that checkpoint. `tux_builder.stl`
tests its main→mountain door (9760,800) and destination near (864,2880).
Completion uses the existing end sequence and level finish path. This is
contained interaction coverage, not a manual playthrough of either campaign.
The observer exits when its sector/player binding disappears. Debug validation
reproduced the former 2.5-second host heartbeat closing a room during level load
(close 1001 while the guest was disabled); bounded neutral-host loading grace
fixes that without extending the active-play/guest watchdog. An earlier nested
console `load_level` fixture crashed Chromium; it was replaced by normal
title-screen entry, not treated as a solved arbitrary nested-session bug.

Run:

```sh
python tests/wasm/coop_smoke.py preview --local-only --output evidence/local
python tests/wasm/coop_smoke.py preview --output evidence/remote
python tests/wasm/coop_smoke.py preview --browser webkit --local-only --output evidence/webkit-local
python tests/wasm/coop_smoke.py preview --browser webkit --output evidence/webkit-remote
```

Reports include build identity, browser version, checks and measured controller
states; logs and canvas screenshots accompany them. CI retains the existing
boot/audio/viewport/touch/save/cache checks, adds local/remote Chromium to Release
and Debug, and local/remote WebKit to Release. Shared input changes select
representative Linux, lint and Debug as well as Release. Worker-only changes
remain focused; the required aggregate gate and cancellation policy are retained.

| Validation | Result | Runtime revision |
| --- | --- | --- |
| Release Chromium 151.0.7922.34: local co-op, real controller pop/recreation, exact config/save reload | 10 checks passed | `c22db06ea4c959caf0b8d966d2e2ca7fa2bd8054` |
| Release Chromium: actual two-browser Worker/DO relay and lifecycle | 17 checks passed | `c22db06ea4c959caf0b8d966d2e2ca7fa2bd8054` |
| Release Linux WebKit 26.5: local co-op and actual two-browser relay | 10 local + 17 remote checks passed | `c22db06ea4c959caf0b8d966d2e2ca7fa2bd8054` |
| Debug Chromium: local co-op, real controller pop/recreation and save reload | 10 checks passed; no new sanitizer sites | `c22db06ea4c959caf0b8d966d2e2ca7fa2bd8054` |
| Debug Chromium: relay, including synchronous loading grace and all lifecycle cases | 17 checks passed; no new sanitizer sites | `04d36983372192f5663056775a25c616de3caab7` |
| Native Linux Debug build and CTest | Build passed; 4/4 tests passed | `c22db06ea4c959caf0b8d966d2e2ca7fa2bd8054` (shared C++ unchanged afterward) |
| Remote/controller replay units | Native and Emscripten/Node passed | Pinned SDK 6.0.11 |
| Focused JS/Worker tests; packaging; CI selection; actionlint | 48/48; 6/6; 6/6; passed | Bounded latest-status delivery revision |
| Existing boot/audio/viewport/input/save, touch, asset/cache and forest checks | Release Chromium/WebKit and Debug Chromium passed in CI | [run 37730249613](https://github.com/jbbejena/supertux/actions/runs/37730249613) |
| Representative Linux Release and Cppcheck 2.22 | Passed in CI | Same run; repeated on loading-grace revision |

That first CI run correctly failed the aggregate gate because the new Debug
co-op entry fixture used fixed delays and observed the map instead of a level.
Entry now waits for fresh main-menu/story/map/level-intro/gameplay statuses.
Another final-head run passed Debug and both Chromium proofs but missed the
expected first WebKit transition. The archived failure canvas showed **Manage
Assets**: the shell Start click left the mouse over that row, so Enter opened
the wrong menu. Entry now taps **Start Game** directly through the existing
touch path. Subsequent transition keys remain down until the expected native
status is observed, then release even on timeout. Failed transitions also
archive the canvas and shell/focus state. No gameplay check was removed.
Death fixtures wait through the existing simulation-clock `wait` API before
pressing Action or observing checkpoint restart. A fixed wall-clock sleep could
send the only respawn edge before the three-second dying timer finished in a
slow Debug frame; the native respawn rule is unchanged.
Further Debug testing reproduced and fixed the relay load-timeout separately.
The existing touch check also restarts through its ordinary menu after slow
console setup, preserving its getter-only observation and all control checks.
The latest PR checks repeat the full focused coverage for the final head; use
the **Required validation** gate when assessing readiness.

Local evidence is under `/tmp/supertux-coop/`: `final-local-chromium`,
`final-remote-chromium`, `final-local-webkit`, `final-remote-webkit`,
`debug-entry-fixed-local2` and `grace-debug-remote`. Each contains `report.json`,
`console.log` and screenshots; the failed relay diagnostic additionally records
the actual close codes in `debug-relay-diagnostic/timeout-state.json`.
CI uploads the equivalent browser/co-op evidence with its complete artifacts.

The frozen, complete loading-grace previews in this environment are
`/workspace/supertux-coop-proof-release` and `/workspace/supertux-coop-proof-debug`.
Both verify as source `04d36983372192f5663056775a25c616de3caab7`, with the
pinned SDK and their respective Release/Debug configuration. Their inventory
identities are `207f0d3cdb0e2925a130583c59ef297def617af20980c3c39c689c6c467cc5b7`
and `c1f2942127685ca7fd578f502dff305e3564dc39edfb0a48ec03cbb152a542db`;
WASM hashes are `819de522875cc8955429a978e89f25113a5e970aa3db37e214a18e47329cada4`
and `c4c9d75e97a7eb45aeff4278717819411a5b5b469e34f2284a55601b33a85ad8`.
No artifact is selected by an unverified "latest" reference.

Debug retains the exact previously annotated libc++ bool, player bool and
obstack sanitizer diagnostics; they are archived, and any new site fails. The
fixed null climbing reference is no longer allowlisted. A nonfatal existing
forest custom-title path warning falls back to the standard title. An incorrect
diagnostic wait that left the Debug story running for a minute exposed an
`intro.nut` arithmetic error; this long-story behavior was not established
against the baseline and is not claimed fixed. Normal tested entry skips the
story. Arbitrary nested console level loading and a full campaign playthrough
remain outside the demonstrated scenes.

CI changes stay within the existing selector/action: six executing checks for
this shared runtime change (selection, lint/focused tests, Linux, Release WASM,
Debug WASM and the reporting gate); unrelated platform jobs intentionally skip.
The gate fails needed failures/cancellations, and validation concurrency cancels
superseded PR runs. Production deployment serialization/settings are unchanged.
GitHub returned 403 for classic branch-protection inspection and an empty
ruleset list; required-check settings could not be independently verified and
were not changed.

## Remaining device/network checks and Phase 7A boundary

No production deployment was performed. Hosted Cloudflare, real Internet delay
and physical phone checks remain. On an isolated HTTPS proof origin, test two
devices on separate networks, then swap host/guest roles. Check Safari's trusted
Start/Resume audio, orientation/shared-camera readability, multitouch release,
screen lock/background, lost Wi-Fi, expiry, explicit rejoin/new-level behavior
and host save reload. A localhost-only origin cannot be opened from another
device; use a separate HTTPS staging/tunnel setup when running that checklist.

The next independent Phase 7A change should define stable session/sector/entity
identities and a **presentation-only guest scene** for both players in one
controlled test level. Start with pose/animation/death state, ordered snapshots
and interpolation; the host alone executes physics, collisions and scripts.
Inventory the chosen level's objects before Phase 7B interaction replication.
Do not describe this input controller proof as synchronized online co-op.
