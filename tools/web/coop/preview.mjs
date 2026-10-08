// Actual local Cloudflare runtime, assets and private R2; no credentials needed.
import {Miniflare, convertV4MiniflareOptions} from 'miniflare';
import {build as bundle} from 'esbuild';
import {readFile, readdir, mkdtemp, rm} from 'node:fs/promises';
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
  const result = await bundle({entryPoints: [join(root, 'worker/index.js')], bundle: true, write: false, format: 'esm', platform: 'browser', target: 'es2022'});
  const mf = new Miniflare(convertV4MiniflareOptions({
    host: '127.0.0.1', port, name: 'supertux-input-proof', modules: true,
    script: result.outputFiles[0].text, compatibilityDate: '2026-10-06',
    durableObjects: {COOP_ROOMS: {className: 'CoopRoom', useSQLite: true}},
    r2Buckets: ['GAME_ASSETS'],
    assets: {directory: frontend, binding: 'ASSETS', assetConfig: {html_handling: 'none'}, routerConfig: {has_user_worker: true, invoke_user_worker_ahead_of_assets: true}},
  }));
  const bucket = await mf.getR2Bucket('GAME_ASSETS');
  // Seed local R2 one file at a time (the setup process needs at most the
  // largest file). Serving remains a streaming R2 response in the Worker.
  async function upload(dir, prefix = '') {
    for (const entry of await readdir(dir, {withFileTypes: true})) {
      const key = prefix + entry.name, path = join(dir, entry.name);
      if (entry.isDirectory()) await upload(path, key + '/');
      else await bucket.put(key, await readFile(path));
    }
  }
  await upload(join(directory, 'game-assets'));
  for (const [category, suffix] of [['startup','data'], ['wasm','wasm']])
    await bucket.put(`${suffix}/${manifest.packages[category].sha256}/supertux2.${suffix}`, await readFile(join(directory, `supertux2.${suffix}`)));
  const published = await readFile(join(frontend, 'asset-manifest.json'));
  await bucket.put(`manifest/${createHash('sha256').update(published).digest('hex')}/asset-manifest.json`, published);
  const url = String(await mf.ready);
  return {mf, url, manifest, async dispose() {await mf.dispose(); await rm(frontend,{recursive:true,force:true});}};
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const directory = process.argv[2];
  if (!directory) throw Error('Usage: node tools/web/coop/preview.mjs COMPLETE_PREVIEW [PORT]');
  const preview = await startPreview(directory, Number(process.argv[3] || 8787));
  console.log(`INPUT_PROOF_READY ${preview.url}index.html?coop=1`);
  for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, async () => {await preview.dispose(); process.exit(0);});
}
