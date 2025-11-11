#!/usr/bin/env python3
"""
Simple HTTP server for OTA app distribution during development.

Usage:
    python3 simple_ota_server.py [port]

Place your manifest.json and app binaries in the 'ota_files' directory:
    ota_files/
    ├── manifest.json
    └── apps/
        ├── app1.bin
        └── app2.bin
"""

import http.server
import socketserver
import os
import sys
from pathlib import Path

DEFAULT_PORT = 8080
OTA_DIR = "ota_files"

class OTAHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Custom handler that serves files from OTA_DIR and logs requests."""
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=OTA_DIR, **kwargs)
    
    def log_message(self, format, *args):
        """Override to provide colored output."""
        print(f"[OTA Server] {self.address_string()} - {format % args}")
    
    def end_headers(self):
        """Add CORS headers for easier testing."""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()


def setup_example_files():
    """Create example directory structure if it doesn't exist."""
    ota_path = Path(OTA_DIR)
    apps_path = ota_path / "apps"
    
    if not ota_path.exists():
        print(f"Creating {OTA_DIR} directory structure...")
        ota_path.mkdir()
        apps_path.mkdir()
        
        # Create example manifest
        manifest = ota_path / "manifest.json"
        manifest.write_text("""{
  "apps": [
    {
      "name": "Example App 1",
      "version": "1.0.0",
      "url": "http://localhost:8080/apps/app1.bin"
    },
    {
      "name": "Example App 2",
      "version": "2.0.0",
      "url": "http://localhost:8080/apps/app2.bin"
    }
  ]
}
""")
        print(f"✓ Created example manifest.json")
        print(f"  Place your app .bin files in {apps_path}/")
        print()


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
    
    setup_example_files()
    
    # Get local IP addresses
    import socket
    hostname = socket.gethostname()
    try:
        local_ip = socket.gethostbyname(hostname)
    except:
        local_ip = "127.0.0.1"
    
    print("=" * 60)
    print("ESP32 OTA Development Server")
    print("=" * 60)
    print(f"Serving OTA files from: {os.path.abspath(OTA_DIR)}")
    print(f"Port: {port}")
    print()
    print("Access URLs:")
    print(f"  Local:   http://127.0.0.1:{port}/manifest.json")
    print(f"  Network: http://{local_ip}:{port}/manifest.json")
    print()
    print("Update your ESP32 menuconfig with:")
    print(f"  MANIFEST_URL = http://{local_ip}:{port}/manifest.json")
    print()
    print("Press Ctrl+C to stop")
    print("=" * 60)
    print()
    
    try:
        with socketserver.TCPServer(("", port), OTAHTTPRequestHandler) as httpd:
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\nServer stopped.")
        sys.exit(0)


if __name__ == "__main__":
    main()
