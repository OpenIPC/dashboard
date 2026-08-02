#!/usr/bin/env python3
"""Static contract gate for the embedded, dependency-free Web client."""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WEB = ROOT / "src" / "web"
EXPECTED_SCRIPTS = ["core.js", "monitor.js", "devices.js", "admin.js", "app.js"]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    html = (WEB / "index.html").read_text(encoding="utf-8")
    scripts = re.findall(r'<script\s+src="/([^"?]+)(?:\?[^\"]*)?"\s+defer></script>', html)
    require(scripts == EXPECTED_SCRIPTS,
            f"Web modules must load in dependency order: {EXPECTED_SCRIPTS}; got {scripts}")

    contents: dict[str, str] = {}
    declarations: dict[str, str] = {}
    for name in EXPECTED_SCRIPTS:
        path = WEB / name
        require(path.is_file(), f"Missing embedded Web module: {name}")
        contents[name] = path.read_text(encoding="utf-8")
        require(path.stat().st_size < 40 * 1024, f"Web module grew beyond 40 KiB: {name}")
        for function_name in re.findall(r"^(?:async\s+)?function\s+([A-Za-z0-9_$]+)\s*\(",
                                        contents[name], re.MULTILINE):
            previous = declarations.get(function_name)
            require(previous is None,
                    f"Global function {function_name} is declared in {previous} and {name}")
            declarations[function_name] = name

    require("const state =" in contents["core.js"] and "async function api(" in contents["core.js"],
            "core.js must own shared state and the HTTP API client")
    require("function renderMonitorGrid(" in contents["monitor.js"],
            "monitor.js must own the live monitor")
    require("function bindDigitalZoom(" in contents["monitor.js"]
            and "function startPushToTalk(" in contents["monitor.js"],
            "monitor.js must expose digital zoom and browser push-to-talk")
    require("function renderDeviceList(" in contents["devices.js"]
            and "function openDiscovery(" in contents["devices.js"],
            "devices.js must own camera management and discovery")
    require("function renderUsers(" in contents["admin.js"]
            and "function loadDiagnostics(" in contents["admin.js"],
            "admin.js must own users, logs and diagnostics")
    require("const state =" not in contents["app.js"]
            and "function renderMonitorGrid(" not in contents["app.js"],
            "app.js must remain a small coordinator, not regain feature ownership")
    require("function workspacePageCount(" in contents["app.js"]
            and "function toggleKiosk(" in contents["app.js"],
            "app.js must coordinate paged layouts and kiosk mode")

    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    routes = (ROOT / "src" / "backend" / "web" / "DashboardWebApi.cpp").read_text(encoding="utf-8")
    for name in EXPECTED_SCRIPTS:
        require(f"src/web/{name}" in cmake, f"{name} is missing from Qt resources")
        require(f'"/{name}"' in routes, f"{name} is missing from the static route allowlist")

    print(f"Web asset contract passed: {len(EXPECTED_SCRIPTS)} modules, "
          f"{len(declarations)} unique functions")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"web asset contract failed: {error}", file=sys.stderr)
        raise SystemExit(1)
