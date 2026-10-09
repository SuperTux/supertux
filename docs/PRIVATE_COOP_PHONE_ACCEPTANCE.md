# Private co-op: HTTPS staging and phone acceptance

Phase 7B is merged through [PR #14](https://github.com/jbbejena/supertux/pull/14),
at `c385d97aba1476a984e497a33c1bd24183c55293` on `mobile-web-audit`.
Its [supported-level handoff](PRIVATE_COOP_SUPPORTED_LEVEL.md) covers the original
Welcome to Antarctica level. **The published URL below still serves Phase 7A**;
merging code alone does not update that separately published staging Worker.
On 2026-10-09, GitHub rejected workflow dispatch with HTTP 422,
“Actions has been disabled for this repository.” CI and a new HTTPS publication
require Actions to be re-enabled. No production deployment is part of Phase 7B.

Phase 8A retried dispatch after that merge and received the same HTTP 422.
The existing preview therefore remains Phase 7A. Local performance measurements
and the next staged implementation steps are in
[PRIVATE_COOP_PHASE8.md](PRIVATE_COOP_PHASE8.md). Workflow listings reporting
`active` do not establish repository-wide Actions availability; the integration
cannot inspect those permissions (HTTP 403).

For the new runtime, first run the WebAssembly workflow on the reviewed branch,
then publish only its exact successful, non-PR validation artifact:

```sh
gh workflow run wasm.yml --repo jbbejena/supertux --ref mobile-web-audit
# Wait for both Release and Debug, then use their exact source and run identity:
gh workflow run mobile-web-preview.yml --repo jbbejena/supertux \
  --ref mobile-web-audit \
  -f ref=FULL_TESTED_RUNTIME_SHA -f validation_run_id=SUCCESSFUL_RUN_ID \
  -f publish_coop=true
```

For Phase 7B phone acceptance, choose **Play Welcome to Antarctica** in step 3.
Additionally verify coin pickup, enemy contact, growth, information panels,
brick breaking, fireballs/ice melting, secret-area tile fading, checkpoint
restart and actual level completion. Repeat with both host and guest roles.

## Previously published Phase 7A evidence

Phase 7A is merged into `mobile-web-audit` at
`e5cfbe08ea4bb4959ad8b84cd3763366dd8eeba1`. The shared display supports the
controlled static scene, not a campaign level. The existing production Worker
still has no `COOP_ROOMS` binding.

The [HTTPS host preview](https://supertux-private-input-proof.jshbjnr.workers.dev/index.html?coop=1)
is published. [Integration validation](https://github.com/jbbejena/supertux/actions/runs/37843161404)
passed Release/Debug WASM and representative Linux for that exact runtime.
[Hosted staging validation](https://github.com/jbbejena/supertux/actions/runs/37847833680)
verified 116 frontend files and 79 immutable payloads, reused all 79 R2 objects
without uploading replacements, and passed all nine display checks in each of
Chromium 151 and Linux WebKit 26.5. Both reports contain no errors. These are
emulated browsers, not physical phones or separate-network acceptance.

## Publish a separate tested preview

The existing **Mobile Web Preview** workflow now has an explicit `publish_coop`
option, defaulting to false. Ordinary preview builds still publish downloadable
artifacts. With staging selected, supply a complete runtime commit SHA and an
explicit successful validation run for that exact source. The staging path
rejects PR artifacts, foreign repositories, mismatched source/toolchain/build
identities, incomplete assets and missing artifacts. It never selects “latest.”

The workflow runs from the staging tooling branch while checking out the runtime
at the requested tested SHA. Its deployment and browser validation tools are
checked out separately;
the Worker, browser code and complete game artifact use the tested runtime. This
allows publication tooling to be reviewed without rebuilding or changing that
runtime. The build job contains no deployment secrets.

For the validated merged Phase 7A runtime:

```sh
gh workflow run mobile-web-preview.yml --repo jbbejena/supertux \
  --ref codex/private-coop-staging \
  -f ref=e5cfbe08ea4bb4959ad8b84cd3763366dd8eeba1 \
  -f validation_run_id=37843161404 -f publish_coop=true
```

The separate publication job uses the existing `tux` environment's
`CLOUDFLARE_ACCOUNT_ID`, `CLOUDFLARE_API_TOKEN`, `R2_ACCESS_KEY_ID` and
`R2_SECRET_ACCESS_KEY`. Credentials remain in GitHub environment secrets; neither
the checkout nor the cloud setup stores their values. The recent successful
production deployment establishes that route; secret-list inspection is not
available to this integration. Missing or denied credentials fail publication
with the relevant variable/API error.

Publication is limited to Worker `supertux-private-input-proof`, using
`wrangler.coop.toml`, its SQLite `COOP_ROOMS` binding and the existing private R2
bucket. The configuration check rejects a different Worker name or attached
routes/environments/services. Existing content-addressed R2 objects are checked
and reused; incompatible immutable objects are never overwritten. The production
Worker/configuration/workflow is not invoked.

Staging runs serialize and do not cancel an active publication. Ordinary preview
validation keeps cancellation in a separate concurrency group. Production's
existing serialization is unchanged.

The readiness check requires the exact frontend/manifest/build identity, hashes
every presentation asset, verifies MIME and immutable artwork caching, checks
every startup/deferred R2 payload with HEAD, and exercises origin-bound room
creation without logging invitation credentials. It retries rollout within one
five-minute budget. Large game files are not downloaded or buffered by this
probe. Chromium and Linux WebKit then run the actual compiled-host/two-browser
display proof through the hosted Worker. Reports and screenshots are retained
in `private-coop-staging-evidence`; the successful run summary contains the host
HTTPS URL and runtime SHA.

HTTP probes identify themselves as `SuperTux-Coop-Validation`. The hosted edge
rejects Python's default `Python-urllib` user-agent with HTTP 403; validation
uses an explicit descriptive user-agent rather than disabling edge protection.

Touch regression automation observes a native input update after dispatching
each menu touch edge. Sampling before dispatch can mistake an earlier frame
for consumption of the new event. Setup resets use a direct touch on Restart
Level, independent of menu hover/selection; the required neutral controls and
real spawn position remain asserted. A failed reset captures a screenshot.

## Two physical phones

Use the host HTTPS URL from the successful run summary, including `?coop=1`.
Staging is a separate origin, with its own browser saves/settings and asset
cache. Production saves are not imported. The entire room currently expires after
15 minutes, including active play; create a new room for a longer testing session.

1. Phone A: use Start, expand **Private co-op**, then **Create room**.
2. Share **Guest shared view** with Phone B. That invitation contains a temporary
   guest credential; keep it private. Wait for Player 2 to join.
3. On the newly published Phase 7B runtime, Phone A chooses **Play Welcome to
   Antarctica**. Both screens should show the original level and both players.
   Phone A controls Player 1; Phone B controls Player 2. The older Phase 7A URL
   only offers **Start shared view proof**, a static diagnostic arena.
4. Try movement, Jump and independent finger releases. Move apart and confirm
   both living players remain visible through the native shared camera.
5. Pause/resume on the host, rotate each phone, and inspect safe areas and control
   visibility. Release fingers before continuing; old held inputs must not replay.
6. Background/lock each phone, lose and restore Wi-Fi, and try again. Recovery is
   explicit: return the host to the title screen, rejoin, then restart the scene.
7. Where death occurs, wait for **Tap Action** before ordinary respawn; all-player
   death must restart with fresh display state.
8. Reload the host and confirm existing campaign saves/settings survive. The
   guest must not create campaign progression.

Repeat on separate networks and swap host/guest roles, including an iPhone Safari
host and guest. Record device model, OS/browser, network, orientation, observed
delay, any disconnect, and screenshots for failures. Hosted Linux browser tests
are emulation, not physical Safari or a two-network acceptance result.

The Phase 7B handoff records the subsequent supported-level implementation and
local evidence. Phase 8 still requires physical-phone and separate-network
acceptance before expanding content or describing mobile co-op as supported.
