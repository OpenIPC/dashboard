#!/usr/bin/env python3
"""
Simple HTTP server with CORS support for serving archive video files
"""
import http.server
import socketserver
import sys
import os
from urllib.parse import unquote

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Cache-Control', 'no-cache')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def guess_type(self, path):
        # Ensure video files are served with correct MIME type
        if path.endswith('.mp4'):
            return 'video/mp4'
        elif path.endswith('.avi'):
            return 'video/x-msvideo'
        elif path.endswith('.mkv'):
            return 'video/x-matroska'
        elif path.endswith('.mov'):
            return 'video/quicktime'
        return super().guess_type(path)

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8081
    directory = sys.argv[2] if len(sys.argv) > 2 else os.getcwd()
    
    # Change to the specified directory
    os.chdir(directory)
    
    with socketserver.TCPServer(("127.0.0.1", port), CORSHTTPRequestHandler) as httpd:
        print(f"Archive HTTP Server running on http://127.0.0.1:{port}")
        print(f"Serving files from: {directory}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")