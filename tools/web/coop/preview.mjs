// Actual local Cloudflare runtime, assets and private R2; no credentials needed.
import {Miniflare, convertV4MiniflareOptions} from 'miniflare';
import {build as bundle} from 'esbuild';
import {readFile, readdir, mkdtemp, rm, stat} from 'node:fs/promises';
import {createReadStream} from 'node:fs';
import {Readable} from 'node:stream';
import {tmpdir} from 'node:os';
import {createHash} from 'node:crypto';
import {resolve, dirname, join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawnSync} from 'node:child_process';

export async function startPreview(directory, port = 8787) {
  directory = resolve(directory);
  const root = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
  const manifest = JSON.parse(await readFile(join(directory, 'asset-manifest.json'), 'utf8'));
  // Verify the complete exact artifact before the Worker can serve it.
  const check = spawnSync('python3', [join(root, 'tools/web/verify_artifact.py'), directory, '--source-commit', manifest.sourceCommit, '--configuration', manifest.configuration], {encoding: 'utf8'});
  if (check.status) throw Error(check.stderr || check.stdout);
  const frontend = await mkdtemp(join(tmpdir(), 'supertux-input-proof-'));
  const assembled = spawnSync('python3', [join(root, 'tools/web/package_assets.py'), 'assemble', '--build', directory, '--output', frontend, '--cloudflare'], {encoding: 'utf8'});
  if (assembled.status) {await rm(frontend, {recursive:true}); throw Error(assembled.stderr);}
  let mf;
  try {
  const result = await bundle({entryPoints: [join(root, 'worker/index.js')], bundle: true, write: false, format: 'esm', platform: 'browser', target: 'es2022'});
  const r2Buckets = {GAME_ASSETS: 'supertux-input-proof-assets'};
  mf = new Miniflare(convertV4MiniflareOptions({host: '127.0.0.1', port, workers: [{
    name: 'supertux-input-proof', modules: true,
    script: result.outputFiles[0].text, compatibilityDate: '2026-10-06',
    durableObjects: {COOP_ROOMS: {className: 'CoopRoom', useSQLite: true}},
    r2Buckets,
    assets: {directory: frontend, binding: 'ASSETS', assetConfig: {html_handling: 'none'}, routerConfig: {has_user_worker: true, invoke_user_worker_ahead_of_assets: true}},
  }, {
    // Only available through Miniflare#getWorker, never the public listener.
    name: 'supertux-input-proof-seed', modules: true, r2Buckets,
    compatibilityDate: '2026-10-06',
    script: `export default {async fetch(request, env) {
      if (request.method !== 'PUT') return new Response(null, {status: 405});
      const object = await env.GAME_ASSETS.put(decodeURIComponent(new URL(request.url).pathname.slice(1)), request.body || new Uint8Array());
      return Response.json({size: object.size}, {status: 201});
    }};`,
  }]}));
  const seeder = await mf.getWorker('supertux-input-proof-seed');
  // Buffer arguments to getR2Bucket().put() are serialized as large JSON RPC
  // messages. Stream the actual files over HTTP with a known length instead.
  async function uploadFile(key, path) {
    const size = (await stat(path)).size, file = createReadStream(path);
    try {
      const response = await seeder.fetch(`http://seed.invalid/${encodeURIComponent(key)}`, {
        method: 'PUT', headers: {'Content-Length': String(size)},
        body: size === 0 ? null : Readable.toWeb(file), duplex: 'half',
      });
      if (response.status !== 201 || (await response.json()).size !== size)
        throw Error(`Local R2 upload did not complete: ${key}`);
    } finally {file.destroy();}
  }
  async function upload(dir, prefix = '') {
    for (const entry of await readdir(dir, {withFileTypes: true})) {
      const key = prefix + entry.name, path = join(dir, entry.name);
      if (entry.isDirectory()) await upload(path, key + '/');
      else await uploadFile(key, path);
    }
  }
  await upload(join(directory, 'game-assets'));
  for (const [category, suffix] of [['startup','data'], ['wasm','wasm']])
    await uploadFile(`${suffix}/${manifest.packages[category].sha256}/supertux2.${suffix}`, join(directory, `supertux2.${suffix}`));
  const published = await readFile(join(frontend, 'asset-manifest.json'));
  await uploadFile(`manifest/${createHash('sha256').update(published).digest('hex')}/asset-manifest.json`, join(frontend, 'asset-manifest.json'));
  const url = String(await mf.ready);
  return {mf, url, manifest, async dispose() {await mf.dispose(); await rm(frontend,{recursive:true,force:true});}};
  } catch (error) {
    await mf?.dispose(); await rm(frontend,{recursive:true,force:true}); throw error;
  }
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const directory = process.argv[2];
  if (!directory) throw Error('Usage: node tools/web/coop/preview.mjs COMPLETE_PREVIEW [PORT]');
  const starting = startPreview(directory, Number(process.argv[3] || 8787));
  for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, async () => {await (await starting).dispose(); process.exit(0);});
  const preview = await starting;
  console.log(`INPUT_PROOF_READY ${preview.url}index.html?coop=1`);
}
