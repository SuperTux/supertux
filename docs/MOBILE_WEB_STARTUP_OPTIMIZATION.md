# Browser startup, optional music and focused validation

This implementation targets `mobile-web-audit`, reconciled at `d8e5abbf17946ada56b66aef47f033040e1d7845`, rather than `master`, which lacks the complete mobile runtime. PR #8 preserves the complete game and every world. It does not deploy or merge production. The audit and Phase 1–4A notes describe their original revisions; this document supersedes their packaging, delivery and CI instructions.

## Runtime and inventory

`tools/web/package_assets.py prepare` inventories all 5,139 source files in sorted canonical path order. Each entry records its runtime path, byte size, SHA-256 and startup/music membership. The inventory includes source commit, configuration, pinned SDK, mode and compiled PhysFS data root. Its canonical JSON hash is embedded in the generated game JS and must match the loaded manifest before the preloaded package can be mounted.

The signature audit found 73 soundtrack audio files (all Ogg/Vorbis), totaling **150,011,881 bytes**. All 72 `.music` descriptors, including 11,303 bytes of loop/author/license metadata, remain in startup. Descriptor companion references are checked for existence and directory escape. Signature-based Ogg/WAVE detection under `music/` avoids guessing by extension. Graphics, fonts, levels, all world metadata, scripts, sound effects and speech remain mandatory. The source assets and native/Android packaging remain intact; all derivatives are outside `data/`.

`WEB_FULL_PRELOAD=OFF` is the default. The remaining DATA package is **176,664,571 bytes**, down from **326,676,452 bytes**: a **45.92% reduction**. Mandatory graphics are still large; this implementation does not attempt world-specific dependency splitting.

`assemble` produces a complete ordinary-HTTP preview with both raw startup packages and explicit gzip alternatives. The manifest binds DATA, WASM, generated JS, bootstrap JS and all deferred payloads. Gzip archives have addresses based on their **encoded** SHA-256; runtime validation and persistent cache keys use the **decoded** SHA-256. Browser `DecompressionStream` support selects gzip; older browsers select raw files. This keeps ordinary HTTP and Cloudflare behavior identical without relying on server content-encoding configuration. No original images/audio are recompressed or converted.

Measured gzip level 6 reduces DATA from 176,664,571 to **134,721,216 bytes**, a further 23.74%. WASM and JS also benefit. Archives use zero timestamps and empty filenames; derived compression is reused by raw identity and compressor version. The gzip/raw alternatives increase the downloadable preview artifact size, but each browser requests only its selected form.

## Music and audio ownership

The browser loader uses a maximum of two concurrent track requests. It drops obsolete queued requests and shares downloads by content hash. It does not prefetch the soundtrack. Before opening a missing track, the engine resolves its existing `.music` descriptor/fallback path and requests its raw audio asynchronously. Completed, validated bytes are written to the existing Emscripten filesystem at the compiled data root, where the existing PhysFS-backed decoder reads them normally.

C++ polls only its current requested track on the main thread. JavaScript never owns a `SoundManager` pointer and never starts playback. A late A completion after switching to B can be cached/mounted, but cannot select A. The existing decoder, companion-file interpretation, loop points and fade handling remain authoritative. During downloads, sound effects and gameplay remain available. Optional failures expose a music retry action; incomplete bytes never reach the decoder or cache.

Music-disabled/zero-volume settings, the shell's muted choice, Pause and background state inhibit new requests/playback. A completed background download waits for trusted Resume and the existing OpenAL AudioContext. Browser/Safari activation rules and shell input cancellation remain in place. The optional download status occupies the upper left rather than the touch controls.

## Asset persistence, corruption and clearing

Downloaded assets use **`supertux-downloaded-assets-v1`**, separate from `storage.js`'s save/settings IDBFS database. Startup DATA, WASM, generated JS and requested tracks are keyed by raw SHA-256. Every read validates size and SHA-256 before use. Changed code/manifest cannot mount an incompatible startup package; unchanged DATA/music can survive a build update. Network responses must be complete HTTP 200 bodies with the expected raw size/hash before publication into the cache or filesystem.

Storage is best effort. Disabled/blocked IndexedDB, quota failure, evictions, corrupt entries and bounded storage timeouts fall back to an online download. Atomic payload/metadata transactions prevent incomplete cached entries. Quota-aware pruning reserves space for other origin data, retains one startup package, and bounds downloaded assets to at most 384 MiB with LRU eviction. The loader does not delete saved games or settings to make room.

Emscripten 6.0.11's `--use-preload-cache` was evaluated: its package UUID is already a content hash, but its database uses pathname/package-name metadata and does not cover deferred tracks, application-wide quota pruning, corrupt-byte validation or the player's save-preserving clear action. The explicit cache supplies the synchronous `Module.getPreloadedPackage` hook only after asynchronous validation, instead of maintaining two competing preload databases.

Pause or open the initial Start overlay, expand **Downloaded assets**, then select **Clear downloaded assets**. This clears only the two asset stores; saves, settings, localStorage and other databases remain. Reload to download fresh content. Cache clearing invalidates pending cache writes so an in-flight track does not immediately repopulate the cleared database. Already mounted files remain usable until reload.

## Measurements

| Measure | Baseline | Optimized |
|---|---:|---:|
| DATA, raw bytes | 326,676,452 | 176,664,571 (−45.92%) |
| WASM, raw bytes | 9,199,438 | 9,199,243 |
| Generated JS, raw bytes | 975,741 | 913,750 |
| Raw DATA + WASM + JS subtotal | 336,851,631 | 186,777,564 |
| Selected DATA / WASM / JS delivery bytes | 326,676,452 / 9,199,438 / 975,741 | 134,721,216 / 2,770,049 / 135,381 |
| Cold mandatory HTTP transfer, menu | 337,023,544 (321.41 MiB) | 138,648,194 (132.23 MiB; −58.86%) |
| Warm mandatory HTTP transfer, menu | 337,023,544 | 1,020,953 (metadata/shell/icons only) |
| Cold ready / usable menu | 73.099 / 73.535 s | 41.905 / 42.299 s |
| Warm ready / usable menu | 75.645 / 75.873 s | 5.182 / 5.427 s |
| Cold ready / controllable real level | 76.099 / 76.886 s | 43.008 / 43.760 s |
| Warm ready / controllable real level | 75.553 / 76.680 s | 5.759 / 6.536 s |
| Cold post-download processing, menu | 5.604 s | 6.777 s |
| Warm post-download processing, menu | 5.476 s | 4.793 s |

The optimized menu starts 42.5% sooner cold and 92.8% sooner warm in these samples. Both warm optimized cases issue **zero DATA/WASM/generated-JS or previously downloaded music requests**. The baseline redownloads its full package because HTTP cache is disabled and it has no explicit persistent asset cache. Remaining startup work is substantial: hashing, filesystem mounting, WASM initialization and resource/image decoding still cost seconds. Gzip delivery reduces bytes but does not remove these costs. No soundtrack request occurs before game readiness; after Start, only the selected title/level track downloads while the game remains usable.

The local server sends no HTTP Content-Encoding. The optimized loader fetches explicit gzip archives and decompresses them itself. Resource Timing therefore reports archive bytes as encoded and decoded HTTP body size; the separate manifest records the larger final decoded game-file sizes. CDP encoded transfer totals above include HTTP response overhead and exclude optional music requested after readiness. Raw metadata, HTML, bootstrap and icons explain the difference between selected package subtotal and total transfer. Full records and conditions are in [measurements/web-startup-2026-10-08.json](measurements/web-startup-2026-10-08.json).

The baseline is the exact successful pinned Release artifact from GitHub run `37693784930`, source `d8e5abbf17946ada56b66aef47f033040e1d7845`. It was reused rather than rebuilding historical source. The optimized measurements use the locally compiled pinned Release artifact at `bb916c12ab3d2308ca393d885c5a4669b7a77ca6`; later follow-ups tighten storage bounds, queue/setting handling and delivery verification. The table identifies the artifact actually measured, rather than presenting it as a different source revision. Small differences in generated JS paths/embedded revision strings are included in the reported sizes.

Conditions: Linux x86-64 in this cloud environment; Chromium 151.0.7922.34 / Playwright 1.62.0, SwiftShader, 844×390 mobile/touch emulation at DPR 1, CDP 50 Mbit/s download throughput, 40 ms latency and 2× CPU slowdown. No compilation or other browser validation runs execute concurrently with the timing runs. Cold starts use a new empty disk profile. Warm starts close and restart the browser with that same profile, with HTTP cache explicitly disabled again. They therefore exercise IndexedDB persistence rather than a warm HTTP cache. One cold/warm pair per case is an observed sample, not a percentile or physical-phone benchmark.

`tests/wasm/startup_measure.py` serves the actual HTML over HTTP, captures CDP encoded transfer bytes (including response overhead), resource body sizes and time to game readiness, trusted Start and first real-level directional input. The gameplay case uses the existing CLI to enter a real level; the separate touch smoke validates the normal menu → world map → level route. Post-download time includes hash validation, WASM/JS processing, filesystem mounting, save hydration and initial game resources; warm launches still do that processing.

## Build and preview

Use recursive submodules, Emscripten **6.0.11**, vcpkg **c748cb44f2a435fcf015c35225c9d5545fe0021c**, CMake, Ninja and Python ≥3.11. Dependency defaults for native builds remain separate.

```sh
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-web -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DWARNINGS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
  -DVCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build-web --parallel 4
python3 tools/web/package_assets.py assemble --build build-web --output preview
python3 tools/web/verify_artifact.py preview --source-commit "$(git rev-parse HEAD)"
python3 -m http.server 8000 --directory preview
```

Open `http://localhost:8000/`. Keep the complete directory, including `game-assets/`, `asset-manifest.json`, `assets.js`, DATA, WASM, JS and icons. A plain static server needs no Cloudflare credentials; `file://` is unsupported. WebCrypto requires HTTPS or a localhost origin; use HTTPS when testing from another device over the network. The CMake post-build assembly also makes `build-web/` itself a complete preview. Locally in this task, the complete preview is `/workspace/supertux-startup-preview`.

For rollback/comparison, reconfigure with `-DWEB_FULL_PRELOAD=ON` and rebuild. This retains the full soundtrack in DATA, while using the same shell and persistent cache. `-DWEB_COMPRESS_ASSETS=OFF` disables generated compressed alternatives; pass `--no-compression` to any subsequent explicit assembly too. These are independent switches. Use `-DCMAKE_BUILD_TYPE=Debug` in another directory for assertions, SAFE_HEAP and UBSan.

## Cloudflare delivery and artifact reuse

Preview and deployment workflows both use the optimized packaging. Deployment publishes raw startup DATA/WASM, gzip DATA/WASM/JS, all deferred tracks and the hashed complete manifest into the existing private R2 bucket. `upload_assets.py` verifies the complete artifact, checks object length/hash metadata (downloads legacy objects lacking hash metadata for byte verification), skips unchanged immutable objects, and refuses errors other than a confirmed missing object. Static Worker assets contain only the small frontend/JS/manifest/icons.

Worker routes narrowly whitelist matching category/hash/filename combinations. GET streams the R2 body; HEAD uses metadata. Correct MIME, length, ETag, conditional reads and immutable caching are preserved. Explicit gzip archives use `application/gzip` with no HTTP Content-Encoding. Same-origin relative URLs work locally and through the Worker. Readiness still requires the exact chosen frontend/source/raw hashes and complete manifest identity; it also hashes deployed bootstrap/fallback JS and HEAD-checks every deferred/compressed object within the existing global deadline.

Preview/deploy can accept an explicit successful validation run ID. Reuse requires the same repository, source SHA, pinned SDK, Release configuration, canonical inventory, bootstrap hash and every complete raw/deferred/encoded payload. PR builds check out the actual PR head to make run/artifact identities agree. A missing/mismatched requested artifact fails rather than silently rebuilding/selecting "latest". Production reuse rejects PR-event artifacts, including fork PRs, and never uses a privileged `workflow_run` path. Blank run ID explicitly selects a fresh build.

Automatic deployment remains limited to `mobile-web-audit`; manual deployment selects an explicit ref. Its global concurrency group has `cancel-in-progress: false`. Before automatic deployment, the run compares the selected source to the current integration tip and skips an obsolete run, preventing an older queued build from overwriting a newer one. Immutable uploads can precede that check safely. No production deployment was performed in this task.

## CI policy and reporting

| Change | Prior routine jobs | New automatic jobs that execute |
|---|---:|---:|
| Documentation only | 27 | 2: selection + reporting gate |
| Web shell/touch UI | 27 | 4: selection, focused tests, Release WASM, gate |
| Worker only | 27 | 3: selection, focused tests, gate |
| Shared C++ | 27 | 5: selection, lint/focused tests, Linux, Release WASM, gate |
| Audio/cache/packaging runtime | 27 | 5–6, including Debug WASM; Linux for shared C++ |
| Shared CMake/dependencies/external/unknown | 27 | Full affected platform coverage, Release/Debug WASM, lint/tests, gate |

The original 24 native matrix builds (Linux 8; Windows/macOS/Android/FreeBSD 4 each), both WASM configurations and lint remain accessible. Native full matrices run manually, weekly Tuesday at 05:20 UTC and on `v*` tags. Direct platform changes select the affected representative job. Unknown/shared dependencies receive full coverage. Workflow changes validate syntax and affected jobs; this PR changes each platform workflow, so its initial validation is necessarily broader than a routine web-only update. Debug is automatically selected for the significant audio/cache/packaging changes and available manually for other major runtime work.

`Focused validation` has no workflow-level PR path filter. Job-level selection reports intentionally unnecessary work as skipped. Stable `wasm (Release)`, `wasm (Debug)` and `lint_cppcheck` names remain; **Required validation** always reports and rejects failed or unexpectedly cancelled needed jobs, including an unsuccessful selector. Native reusable call names have workflow prefixes; the aggregate gate supplies one compatible reporting strategy rather than recreating 24 unnecessary matrix checks. An owner must verify any inaccessible classic protection contexts before adopting the new gate.

Minimal read permissions, safe PR events and no `pull_request_target` execution with secrets are retained. Validation concurrency is scoped to workflow and PR/ref, cancels superseded runs, and scopes reusable native workflows separately to avoid cancelling their parent. Production serialization is separate. Dependency caches use the pinned SDK, vcpkg revision and manifest identity; complete tested Release artifacts can be reused explicitly as described above.

Live GitHub branch metadata reported `master` and `mobile-web-audit` unprotected; repository/branch ruleset endpoints returned empty lists. The classic branch-protection endpoint returned **403 Resource not accessible by integration**, so that rule verification remains unavailable. No repository settings/rules were changed.

## Validation and physical-device status

Validation evidence is archived by the Release/Debug composite action and was also run locally:

- Release WASM builds and complete local preview verification pass. Chromium 151 and Linux WebKit 26.5 pass the existing boot, real audio activation/failure recovery, viewport, input cancellation, config/save hydration and denied-storage checks (12 checks each).
- Both engines pass touch-only normal menu → map → real level entry, multi-touch movement/jump, cancellation, Pause/Resume and options/save checks (10 checks each). A separate real forest level (`world2/welcome_forest.stl`) renders, moves from x=96 to x=104.512, jumps from y=1377.2 to y=1355.15 and loads its forest music, using existing read-only scripting methods.
- The compiled-engine asset suite passes in both engines: readiness before any soundtrack request; every startup file mounted; title decoding; late A after B; partial failure/retry; pause/background/muted choice/Resume; persistent startup/code/music reuse with HTTP cache disabled; quota failure during cache accounting; corrupt startup rejection/redownload; save-preserving UI clearing; native music-disabled settings with immediate SFX; fresh disabled/quota storage fallback; incompatible manifest rejection (13 checks each).
- Debug WASM builds and Chromium boot/touch checks pass with assertions, SAFE_HEAP and UBSan enabled. The unchanged exact upstream player/bool/obstack sites documented in Phase 1 remain visible in the reports; novel sites fail. This is functional Debug validation, **not a sanitizer-clean claim**.
- A native Debug build and CTest pass (3/3). The fetched-Discord/RapidJSON configuration and named unit target also pass, covering the dependency target collision seen in CI. Representative hosted Linux Release passes.
- The explicit full-preload Release build is complete and verifiable, and passes the real Chromium boot/audio/keyboard/storage/viewport smoke. Optimized mode was restored afterward.
- Focused checks pass: 34 Node tests for shell/storage/loader/Worker, 11 Python packaging/upload/change-selection/gate tests, pinned actionlint syntax validation, Cppcheck 2.22.0 and diff whitespace checks. Selection tests cover docs, shell, Worker, assets, shared C++, CMake/dependencies, platforms, workflows and unknown paths. Gate tests reject failed/cancelled/skipped needed jobs and unsuccessful selection.
- Wrangler 4.148.0 deployment dry-run passes. A real local Miniflare/workerd R2 instance verifies all **79** object routes, body streaming, lengths/MIME and frontend identity; Chromium cold/warm gameplay shell and actual title music work through that Worker. Exact readiness Python fixtures reject stale source/manifest/bootstrap, incorrect lengths and MIME. No production credentials or deployment are needed for these tests.
- Hosted run [37709229157](https://github.com/jbbejena/supertux/actions/runs/37709229157) passed Release/Debug WASM, lint and all five representative native platforms plus **Required validation** at `0b0a27de3d25c09f023654f5e16cb302fb2c09b2`. Final follow-up changes retain these gates and rerun the relevant validation before PR handoff. Superseded runs were observed cancelled, with no production run cancelled or dispatched.

The final self-review fixed duplicate WASM requests caused by the SDK incoming API, concurrent storage-denial fallback, quota exceptions in asynchronous cache callbacks, large-package timeout/transaction cleanup, obsolete queued music after Pause, incomplete reused previews, missing legacy R2 hash verification, and stale deployed bootstrap/manifest URLs. Clear-downloads is disabled until its action is installed. WebKit's initial audio gate accepts its legitimate inactive `interrupted` state as well as `suspended`, while retaining the requirement for a real running context and scheduled audio after trusted Start. Native music-disabled settings cannot resume a previously loaded source through a repeated play request.

CI's original cache path pointed outside the manifest-mode installed directory. All web validation/preview/deploy caches now target `build/vcpkg_installed` with a new pinned key. The native unit target is `supertux-unit-tests`; the legacy `tests` alias is provided when no vendor target owns that name. CI disables RapidJSON's unrelated tests/docs/examples, avoiding its target collision without changing that submodule.

Linux Playwright WebKit 26.5 and Chromium mobile emulation are not physical iPhone Safari. Audio tests observe the game's actual OpenAL context and scheduled decoder buffers, not audible speaker quality. Physical iPhone/iPad/macOS Safari, hardware GPU performance, silent switch behavior, memory/tab eviction and thermal stability remain unverified.

Before production acceptance on a physical iPhone/iPad:

- Open a new private/cleared-assets session, reach the menu and play Antarctica, then a forest/later-world level; verify images, SFX and touch movement/jump.
- Confirm music appears after its first track download without stopping play; rapidly change areas and confirm a late old track never starts.
- Mute or Pause/background during a track download, return with Resume, and verify no automatic gesture bypass or audible playback while muted. Test Safari interruptions and silent-switch behavior.
- Close/reopen the tab on the same build and confirm downloaded DATA/tracks are reused. Test denied/low-quota storage and a failed network request followed by music Retry.
- Change a setting and save progress, clear only downloaded assets through the overlay, reload and confirm both remain.
- Check portrait/landscape, safe areas, multi-touch cancellation and app switching. Record cold/warm timing, peak memory and tab-eviction behavior on the minimum supported device/OS.

Further opportunities: audit per-world graphics dependencies before splitting textures/fonts, measure mounting/decoding/GPU upload costs, and evaluate lossless generated image derivatives with decoded pixel/alpha/dimension checks. No lossy audio conversion or visual reduction is needed for this change.
