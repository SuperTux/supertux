# Mobile Web Phase 4A — Cloudflare deployment

## Goal

Phase 4A externalizes the large browser runtime files and makes a repeatable Cloudflare deployment without changing SuperTux gameplay or asset contents.

The production path is:

```text
GitHub Actions
  -> Release Emscripten build
  -> SHA-256 content addressing
  -> R2: supertux2.data + supertux2.wasm
  -> Cloudflare Worker: HTML/JS/static shell
  -> same-origin /game-assets/* gateway backed by private R2
```

The R2 bucket remains private. The Worker streams only hashed SuperTux assets from the `GAME_ASSETS` binding, so the browser does not need a public R2 URL or cross-origin CORS configuration.

## Required GitHub Actions secrets

The repository must define:

- `CLOUDFLARE_ACCOUNT_ID`
- `CLOUDFLARE_API_TOKEN`
- `R2_ACCESS_KEY_ID`
- `R2_SECRET_ACCESS_KEY`

The expected R2 bucket is `supertux-assets`.

Do not commit credentials, R2 secret keys, or API tokens.

## Runtime routing

`mk/emscripten/template.html.in` now has an optional `Module.locateFile()` override.

Normal/local builds still use the generated files beside the HTML page.

The deploy workflow injects:

```js
window.SUPERTUX_DEPLOY_CONFIG = {
  dataUrl: "/game-assets/data/<sha256>/supertux2.data",
  wasmUrl: "/game-assets/wasm/<sha256>/supertux2.wasm"
};
```

The same generated SuperTux code then requests the immutable, content-addressed files through the Worker.

## Why both DATA and WASM are in R2

The 311 MiB DATA package is too large for a normal Worker static-asset bundle.

WASM is also kept in R2 so the frontend bundle stays small and never depends on the static platform's per-file limit. Each gets its own SHA-256 path, so changing code does not force the 311 MiB DATA package to be uploaded again when its bytes are unchanged.

## Worker

`worker/index.js` has two responsibilities:

1. Serve the small generated frontend from the Worker static-asset binding.
2. Stream exact hashed `data/.../supertux2.data` and `wasm/.../supertux2.wasm` objects from private R2.

The R2 responses are immutable-cacheable because their URL contains the complete content hash.

The Worker deliberately does not buffer the large object into Worker memory.

## Deployment workflow

The workflow is:

`.github/workflows/mobile-web-deploy.yml`

It is available manually as **Mobile Web Deploy** on the fork's default branch.

For a manual Phase 4A test, run it with:

`mobile-web-phase4a-cloudflare-deploy`

The workflow:

1. checks out the selected ref and recursive submodules;
2. installs the pinned Emscripten/vcpkg toolchain;
3. builds Release WASM;
4. hashes DATA and WASM;
5. checks R2 for each exact object;
6. uploads only missing objects through the S3-compatible R2 endpoint;
7. generates a small frontend containing exact remote URLs;
8. deploys `supertux-mobile` through Wrangler;
9. performs HTTP smoke checks against the Worker and its R2 gateway;
10. prints the workers.dev URL in the GitHub Actions job summary.

After the Phase 4A branch is merged into `mobile-web-audit`, pushes to `mobile-web-audit` also deploy automatically.

The HTTP smoke checks allow up to two minutes of retries per endpoint because a
new `workers.dev` deployment can briefly return HTTP 404 after Wrangler succeeds.
Persistent HTTP failures, missing frontend configuration, and incorrect R2 MIME
types still fail the job. Each probe logs which endpoint it is checking.

## First deployment

First register a `workers.dev` subdomain for the Cloudflare account in the
Cloudflare dashboard under **Workers & Pages**. This configuration enables
`workers_dev` and does not define custom routes. Wrangler cannot complete a
noninteractive first deployment until the account has its subdomain.

Open GitHub:

**jbbejena/supertux -> Actions -> Mobile Web Deploy -> Run workflow**

Use:

`mobile-web-phase4a-cloudflare-deploy`

The first deployment can take considerably longer because it must compile dependencies/game code and upload the full DATA package.

Later deployments skip the 311 MiB upload whenever `supertux2.data` has the same SHA-256.

## Expected URL

Wrangler should create/update the Worker named:

`supertux-mobile`

Its `workers.dev` URL is printed by the deployment job. A custom domain can be added later without changing the game/runtime architecture.

## Device acceptance

Once the deployment succeeds, open the printed HTTPS URL on a physical iPhone Safari installation.

Validate at minimum:

- cold load from a fresh cache;
- Start -> menu -> world map -> real level;
- touch-only left/right/jump/action;
- simultaneous movement + jump + action;
- Pause/Continue;
- portrait/landscape transitions;
- background/Resume;
- audio activation/recovery;
- whether Safari reloads or evicts the page while handling the full DATA package.

The full DATA package remains unchanged in Phase 4A. If a real phone cannot reliably load it, Phase 4B should address package reduction/splitting based on that device evidence.
