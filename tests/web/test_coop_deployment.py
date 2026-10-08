import hashlib
import http.server
import json
from pathlib import Path
import sys
import tempfile
import threading
import time
import unittest
import urllib.error
sys.path.insert(0, str(Path(__file__).parents[2] / 'tools/web'))
from verify_coop_deployment import check_config, deployment_url, origin, verify_once


class DeploymentTests(unittest.TestCase):
    def test_config_cannot_publish_to_production_or_attach_routes(self):
        original = (Path(__file__).parents[2] / 'wrangler.coop.toml').read_text()
        with tempfile.TemporaryDirectory() as directory:
            config = Path(directory) / 'wrangler.toml'
            config.write_text(original); check_config(config)
            for invalid in [original.replace('supertux-private-input-proof', 'supertux-mobile'),
                            'routes = []\n' + original,
                            original.replace('COOP_ROOMS', 'OTHER_ROOMS'),
                            original.replace('"/coop-art/*"', '"/other/*"')]:
                config.write_text(invalid)
                with self.assertRaises(ValueError): check_config(config)

    def test_only_the_separate_workers_dev_origin_is_accepted(self):
        url = 'https://supertux-private-input-proof.example.workers.dev'
        self.assertEqual(deployment_url('Published ' + url), url)
        self.assertEqual(origin(url + '/'), url)
        with self.assertRaises(ValueError): deployment_url('https://supertux-mobile.example.workers.dev')
        for invalid in ['http://' + url[8:], url + '/other', url + '?ref=other', url + '#token', url.replace('example.', 'user@example.')]:
            with self.assertRaises(ValueError): origin(invalid)

    def fixture(self, directory, tamper=None):
        root = Path(directory); frontend = root / 'frontend'; frontend.mkdir()
        manifest = {'sourceCommit': 'a' * 40, 'frontend': {}, 'packages': {}}
        contents = {'assets.js': b'bootstrap', 'supertux2.js': b'javascript',
                    'coop-view.js': b'view', 'coop-art/' + 'b' * 64 + '.png': b'png'}
        for name, data in contents.items():
            entry = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
            if name == 'assets.js': manifest['bootstrapSha256'] = entry['sha256']
            elif name == 'supertux2.js': manifest['packages']['javascript'] = entry
            else: manifest['frontend'][name] = entry
            if '/' not in name: (root / name).write_bytes(data)
        encoded = json.dumps(manifest).encode()
        config = {'manifestUrl': '/game-assets/manifest/' + hashlib.sha256(encoded).hexdigest() + '/asset-manifest.json',
                  'manifestSha256': hashlib.sha256(encoded).hexdigest()}
        page = '<script>window.SUPERTUX_DEPLOY_CONFIG = ' + json.dumps(config) + ';</script>'
        (frontend / 'index.html').write_text(page)
        payload = root / 'data'; payload.write_bytes(b'payload')
        key = 'data/' + 'c' * 64 + '/supertux2.data'
        requests = []
        class Handler(http.server.BaseHTTPRequestHandler):
            def log_message(self, *args): pass
            def respond(self, data, mime, size=None):
                self.send_response(200)
                self.send_header('Content-Type', mime)
                self.send_header('Cache-Control', 'public,max-age=31536000,immutable')
                self.send_header('Content-Length', str(len(data) if size is None else size))
                self.end_headers()
                if self.command != 'HEAD': self.wfile.write(data)
            def do_GET(self):
                requests.append(('GET', self.path))
                if not self.headers.get('User-Agent', '').startswith('SuperTux-Coop-Validation/'):
                    self.send_error(403); return
                if self.path == '/asset-manifest.json':
                    remote = dict(manifest, sourceCommit='d' * 40) if tamper == 'manifest' else manifest
                    self.respond(json.dumps(remote).encode(), 'application/json')
                elif self.path == config['manifestUrl']: self.respond(encoded, 'application/json')
                elif self.path == '/': self.respond(page.encode(), 'text/html')
                elif self.path[1:] in contents:
                    name = self.path[1:]; body = contents[name]
                    if tamper == 'frontend' and name == 'coop-view.js': body = b'bad!'
                    mime = 'image/png' if name.endswith('.png') else 'text/javascript'
                    if tamper == 'mime' and name.endswith('.png'): mime = 'text/html'
                    self.respond(body, mime)
                else: self.send_error(404)
            def do_HEAD(self):
                requests.append(('HEAD', self.path))
                if self.path == '/game-assets/' + key:
                    self.respond(b'', 'application/octet-stream', 6 if tamper == 'payload' else 7)
                else: self.send_error(404)
            def do_POST(self):
                requests.append(('POST', self.path))
                value = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
                if tamper == 'rooms' or self.headers['Origin'] != base or value != {'protocol': 2, 'build': config['manifestSha256']}:
                    self.send_error(403); return
                data = json.dumps({'protocol': 2, 'build': value['build'], 'room': 'e' * 32,
                                   'host': 'f' * 64, 'guest': '1' * 64}).encode()
                self.send_response(201); self.send_header('Content-Length', str(len(data)))
                self.end_headers(); self.wfile.write(data)
        server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        base = f'http://127.0.0.1:{server.server_port}'
        threading.Thread(target=server.serve_forever, daemon=True).start()
        self.addCleanup(server.server_close); self.addCleanup(server.shutdown)
        return base, root, frontend, manifest, {key: (payload, 'application/octet-stream')}, time.monotonic() + 10, requests

    def test_exact_frontend_payload_headers_and_room_origin_pass_without_data_download(self):
        with tempfile.TemporaryDirectory() as directory:
            args = self.fixture(directory); result = verify_once(*args[:-1])
            self.assertTrue(result['roomCreation'])
            self.assertEqual(result['sourceCommit'], 'a' * 40)
            self.assertNotIn('host', result); self.assertNotIn('guest', result)
            self.assertTrue(any(method == 'HEAD' for method, _ in args[-1]))
            self.assertFalse(any(method == 'GET' and path.endswith('.data') for method, path in args[-1]))

    def test_stale_http_200_manifest_is_rejected_before_creating_a_room(self):
        with tempfile.TemporaryDirectory() as directory:
            args = self.fixture(directory, 'manifest')
            with self.assertRaisesRegex(ValueError, 'exact tested build'): verify_once(*args[:-1])
            self.assertFalse(any(method == 'POST' for method, _ in args[-1]))

    def test_corrupt_presentation_and_wrong_image_mime_are_rejected(self):
        for tamper in ['frontend', 'mime']:
            with self.subTest(tamper=tamper), tempfile.TemporaryDirectory() as directory:
                args = self.fixture(directory, tamper)
                with self.assertRaises(ValueError): verify_once(*args[:-1])

    def test_incomplete_r2_payload_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            args = self.fixture(directory, 'payload')
            with self.assertRaisesRegex(ValueError, 'Incomplete hosted payload'): verify_once(*args[:-1])

    def test_missing_room_binding_is_a_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            args = self.fixture(directory, 'rooms')
            with self.assertRaises(urllib.error.HTTPError): verify_once(*args[:-1])
