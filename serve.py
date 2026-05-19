#!/usr/bin/env python3
"""
Simple file server with live-reload via Server-Sent Events.
The watch.sh script signals a reload by hitting GET /trigger-reload.
"""

import os
import sys
import threading
from http.server import HTTPServer, SimpleHTTPRequestHandler

BUILD_DIR = sys.argv[1] if len(sys.argv) > 1 else "build-web"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 8000

# All connected SSE clients
clients: list = []
clients_lock = threading.Lock()

RELOAD_SNIPPET = b"""
<script>
(function() {
  var es = new EventSource('/__reload');
  es.onmessage = function() { location.reload(); };
  es.onerror = function() {
    // Reconnect after server restart
    setTimeout(function() { location.reload(); }, 500);
  };
})();
</script>
"""

class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=BUILD_DIR, **kwargs)

    def log_message(self, format, *args):
        # Suppress per-request noise; keep errors
        if args and str(args[1]) not in ('200', '304'):
            super().log_message(format, *args)

    def do_GET(self):
        if self.path == '/__reload':
            self._sse_handler()
        elif self.path == '/__trigger-reload':
            self._trigger_reload()
        else:
            # Check if this is an HTML file we should inject into
            fs_path = self.translate_path(self.path)
            if os.path.isdir(fs_path):
                fs_path = os.path.join(fs_path, 'index.html')
            if fs_path.endswith('.html') and os.path.isfile(fs_path):
                self._serve_html_injected(fs_path)
            else:
                super().do_GET()

    def _serve_html_injected(self, fs_path):
        with open(fs_path, 'rb') as f:
            content = f.read()
        injected = content.replace(b'</body>', RELOAD_SNIPPET + b'</body>')
        if b'</body>' not in content:
            injected = content + RELOAD_SNIPPET
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_header('Content-Length', str(len(injected)))
        self.send_header('Cache-Control', 'no-cache')
        self.end_headers()
        self.wfile.write(injected)

    def _sse_handler(self):
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.send_header('Cache-Control', 'no-cache')
        self.send_header('Connection', 'keep-alive')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        q = threading.Event()
        with clients_lock:
            clients.append(q)
        try:
            # Keep connection alive; unblock when reload is triggered
            while True:
                fired = q.wait(timeout=15)
                if fired:
                    self.wfile.write(b"data: reload\n\n")
                    self.wfile.flush()
                    q.clear()
                else:
                    # Heartbeat to keep connection alive
                    self.wfile.write(b": ping\n\n")
                    self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            with clients_lock:
                clients.remove(q)

    def _trigger_reload(self):
        with clients_lock:
            for q in clients:
                q.set()
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"ok")



class ThreadedHTTPServer(HTTPServer):
    """Handle each request in its own thread (needed for concurrent SSE + file serving)."""
    def process_request(self, request, client_address):
        t = threading.Thread(target=self._handle, args=(request, client_address))
        t.daemon = True
        t.start()

    def _handle(self, request, client_address):
        try:
            self.finish_request(request, client_address)
        except (ConnectionResetError, BrokenPipeError):
            pass  # Browser closed connection mid-reload; harmless
        except Exception:
            self.handle_error(request, client_address)
        finally:
            self.shutdown_request(request)


if __name__ == '__main__':
    os.chdir('.')  # serve relative to cwd
    server = ThreadedHTTPServer(('', PORT), Handler)
    print(f"[server] Serving {BUILD_DIR}/ on http://localhost:{PORT}")
    server.serve_forever()