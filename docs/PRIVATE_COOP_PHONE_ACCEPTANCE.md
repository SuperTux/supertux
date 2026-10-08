# Private co-op: HTTPS staging and phone acceptance

Phase 7A is merged into `mobile-web-audit` at
`e5cfbe08ea4bb4959ad8b84cd3763366dd8eeba1`. The shared display supports the
controlled static scene, not a campaign level. The existing production Worker
still has no `COOP_ROOMS` binding.

## Publish a separate tested preview

The existing **Mobile Web Preview** workflow now has an explicit `publish_coop`
option, defaulting to false. Ordinary preview builds still publish downloadable
artifacts. With staging selected, supply a complete runtime commit SHA and an
explicit successful validation run for that exact source. The staging path
rejects PR artifacts, foreign repositories, mismatched source/toolchain/build
identities, incomplete assets and missing artifacts. It never selects “latest.”

The workflow runs from the staging tooling branch while checking out the runtime
at the requested tested SHA. Its deployment and browser validation tools are checked out separately;
the Worker, browser code and complete game artifact use the tested runtime. This
allows publication tooling to be reviewed without rebuilding or changing that
runtime. The build job contains no deployment secrets.

For the merged Phase 7A runtime, once integration validation succeeds:

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

## Two physical phones

Use the host HTTPS URL from the successful run summary, including `?coop=1`.

1. Phone A: use Start, expand **Private co-op**, then **Create room**.
2. Share **Guest shared view** with Phone B. That invitation contains a temporary
   guest credential; keep it private. Wait for Player 2 to join.
3. Phone A: choose **Start shared view proof**. Both screens should show the arena
   and both numbered players. Phone A controls Player 1; Phone B controls Player 2.
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

After this acceptance, Phase 7B should inventory one existing campaign level and
replicate its enemies, pickups, spawning/removal, changed tiles/objects,
checkpoints and completion. The host continues owning simulation and saves.
