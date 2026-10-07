export default {
  async fetch(request, env) {
    const url = new URL(request.url);

    if (url.pathname.startsWith("/game-assets/")) {
      if (request.method !== "GET" && request.method !== "HEAD") {
        return new Response("Method not allowed", {
          status: 405,
          headers: { Allow: "GET, HEAD" },
        });
      }

      const key = url.pathname.slice("/game-assets/".length);
      const allowed = /^(data|wasm)\/[a-f0-9]{64}\/supertux2\.(data|wasm)$/;
      if (!allowed.test(key)) return new Response("Not found", { status: 404 });

      const object = request.method === "HEAD"
        ? await env.GAME_ASSETS.head(key)
        : await env.GAME_ASSETS.get(key);

      if (!object) return new Response("Not found", { status: 404 });

      const headers = new Headers();
      if (object.writeHttpMetadata) object.writeHttpMetadata(headers);
      headers.set("ETag", object.httpEtag);
      headers.set("Cache-Control", "public, max-age=31536000, immutable");
      headers.set(
        "Content-Type",
        key.endsWith(".wasm") ? "application/wasm" : "application/octet-stream",
      );

      return new Response(request.method === "HEAD" ? null : object.body, {
        headers,
      });
    }

    if (url.pathname === "/") {
      const indexUrl = new URL("/index.html", url);
      return env.ASSETS.fetch(new Request(indexUrl, request));
    }

    return env.ASSETS.fetch(request);
  },
};
