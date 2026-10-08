import {coopFetch} from './coop.js';
export {CoopRoom} from './coop.js';

export default {
  async fetch(request, env) {
    const url = new URL(request.url);

    if (url.pathname.startsWith('/coop/')) return coopFetch(request, env);

    if (url.pathname.startsWith("/game-assets/")) {
      if (request.method !== "GET" && request.method !== "HEAD") {
        return new Response("Method not allowed", {
          status: 405,
          headers: { Allow: "GET, HEAD" },
        });
      }

      const key = url.pathname.slice("/game-assets/".length);
      const allowed = /^(?:data\/[a-f0-9]{64}\/supertux2\.data|wasm\/[a-f0-9]{64}\/supertux2\.wasm|data-gzip\/[a-f0-9]{64}\/supertux2\.data\.gz|wasm-gzip\/[a-f0-9]{64}\/supertux2\.wasm\.gz|js-gzip\/[a-f0-9]{64}\/supertux2\.js\.gz|music\/[a-f0-9]{64}\/track\.(?:ogg|wav)|manifest\/[a-f0-9]{64}\/asset-manifest\.json)$/;
      if (!allowed.test(key)) return new Response("Not found", { status: 404 });

      const object = request.method === "HEAD"
        ? await env.GAME_ASSETS.head(key)
        : await env.GAME_ASSETS.get(key);

      if (!object) return new Response("Not found", { status: 404 });

      const headers = new Headers();
      if (object.writeHttpMetadata) object.writeHttpMetadata(headers);
      headers.set("ETag", object.httpEtag);
      headers.set("Content-Length", String(object.size));
      headers.set("Cache-Control", "public, max-age=31536000, immutable");
      headers.set(
        "Content-Type",
        key.endsWith(".gz") ? "application/gzip" : key.endsWith(".wasm") ? "application/wasm" : key.endsWith(".ogg") ? "audio/ogg" : key.endsWith(".wav") ? "audio/wav" : key.endsWith(".json") ? "application/json" : "application/octet-stream",
      );

      if (request.headers.get("If-None-Match") === object.httpEtag)
        return new Response(null, {status: 304, headers});

      return new Response(request.method === "HEAD" ? null : object.body, {
        headers,
      });
    }

    if (url.pathname === "/") {
      const indexUrl = new URL("/index.html", url);
      return env.ASSETS.fetch(new Request(indexUrl, request));
    }

    if (/^\/coop-art\/[a-f0-9]{64}\.png$/.test(url.pathname)) {
      const response = await env.ASSETS.fetch(request);
      if (!response.ok && response.status !== 304) return response;
      const headers = new Headers(response.headers);
      headers.set('Content-Type','image/png');
      headers.set('Cache-Control','public, max-age=31536000, immutable');
      return new Response(response.body,{status:response.status,headers});
    }

    return env.ASSETS.fetch(request);
  },
};
