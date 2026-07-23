#!/usr/bin/env python3
"""Package/runtime smoke for startup, readiness, port conflict and recovery."""

from __future__ import annotations

import json
import os
import socket
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.request
from pathlib import Path


STARTUP_BUDGET_SECONDS = 20.0
SHUTDOWN_BUDGET_SECONDS = 5.0


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.bind(("127.0.0.1", 0))
        return int(listener.getsockname()[1])


def runtime_environment(data_root: Path, http_port: int, websocket_port: int) -> dict[str, str]:
    environment = os.environ.copy()
    environment.update(
        {
            "QT_QPA_PLATFORM": "offscreen",
            "OPENIPC_WEB_DEPLOYMENT_PROFILE": "localhost",
            "OPENIPC_WEB_BIND_ADDRESS": "127.0.0.1",
            "OPENIPC_WEB_PORT": str(http_port),
            "OPENIPC_WEBSOCKET_PORT": str(websocket_port),
            "OPENIPC_DATA_ROOT": str(data_root),
            "XDG_DATA_HOME": str(data_root / "data"),
            "XDG_CONFIG_HOME": str(data_root / "config"),
            "HOME": str(data_root),
            "APPDATA": str(data_root / "appdata"),
            "LOCALAPPDATA": str(data_root / "localappdata"),
        }
    )
    return environment


def start(executable: Path, data_root: Path, http_port: int, websocket_port: int, log_path: Path,
          extra_arguments: list[str] | None = None, password_file: Path | None = None):
    log_file = log_path.open("wb")
    environment = runtime_environment(data_root, http_port, websocket_port)
    if password_file is not None:
        environment["OPENIPC_INITIAL_ADMIN_PASSWORD_FILE"] = str(password_file)
    process = subprocess.Popen(
        [str(executable), "--server-only", *(extra_arguments or [])],
        cwd=str(executable.parent),
        env=environment,
        stdout=log_file,
        stderr=subprocess.STDOUT,
    )
    return process, log_file


def wait_for_readiness(process: subprocess.Popen, port: int, log_path: Path | None = None) -> tuple[dict, float]:
    started = time.monotonic()
    url = f"http://127.0.0.1:{port}/api/v1/health/ready"
    last_error = "no response"
    while time.monotonic() - started < STARTUP_BUDGET_SECONDS:
        if process.poll() is not None:
            details = ""
            if log_path is not None and log_path.exists():
                details = log_path.read_text(encoding="utf-8", errors="replace")[-4000:].strip()
            suffix = f"\n{details}" if details else ""
            raise RuntimeError(
                f"server exited before readiness with code {process.returncode}{suffix}"
            )
        try:
            with urllib.request.urlopen(url, timeout=1.0) as response:
                payload = json.loads(response.read().decode("utf-8"))
                data = payload.get("data", {})
                if response.status == 200 and payload.get("ok") and data.get("ready"):
                    return data, time.monotonic() - started
        except (OSError, ValueError, urllib.error.URLError) as error:
            last_error = str(error)
        time.sleep(0.1)
    raise RuntimeError(f"readiness budget exceeded: {last_error}")


def stop(process: subprocess.Popen) -> float:
    started = time.monotonic()
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=SHUTDOWN_BUDGET_SECONDS)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=2.0)
            raise RuntimeError("shutdown budget exceeded")
    return time.monotonic() - started


def login(port: int, username: str, password: str) -> None:
    request = urllib.request.Request(
        f"http://127.0.0.1:{port}/api/v1/auth/login",
        data=json.dumps({"username": username, "password": password}).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(request, timeout=2.0) as response:
        payload = json.loads(response.read().decode("utf-8"))
        if response.status != 200 or not payload.get("ok"):
            raise RuntimeError("headless administrator could not authenticate")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: server_only_smoke.py <dashboard-executable>", file=sys.stderr)
        return 2
    executable = Path(sys.argv[1]).resolve()
    if not executable.is_file():
        raise FileNotFoundError(executable)

    http_port = free_port()
    websocket_port = free_port()
    while websocket_port == http_port:
        websocket_port = free_port()

    with tempfile.TemporaryDirectory(prefix="openipc-server-smoke-") as temporary:
        root = Path(temporary)
        runtime_root = root / "runtime"
        first, first_log = start(executable, runtime_root, http_port, websocket_port, root / "first.log")
        try:
            health, startup_seconds = wait_for_readiness(first, http_port, root / "first.log")
            required = {"version", "profile", "timeToReadyMs", "tlsRuntimeAvailable", "webRtcAvailable"}
            missing = required.difference(health)
            if missing:
                raise RuntimeError(f"readiness payload is missing: {sorted(missing)}")

            conflict, conflict_log = start(
                executable, root / "conflict", http_port, websocket_port, root / "conflict.log"
            )
            try:
                conflict.wait(timeout=STARTUP_BUDGET_SECONDS)
                if conflict.returncode == 0:
                    raise RuntimeError("second server unexpectedly acquired the occupied port")
            finally:
                if conflict.poll() is None:
                    stop(conflict)
                conflict_log.close()

            shutdown_seconds = stop(first)
            first_log.close()

            recovered, recovered_log = start(
                executable, runtime_root, http_port, websocket_port, root / "recovered.log"
            )
            try:
                _, recovery_seconds = wait_for_readiness(
                    recovered, http_port, root / "recovered.log"
                )
            finally:
                stop(recovered)
                recovered_log.close()

            password = "OpenIPC-P11-Smoke!"
            password_file = root / "initial-admin-password.txt"
            password_file.write_text(password + "\n", encoding="utf-8")
            password_file.chmod(0o600)
            initialized, initialized_log = start(
                executable, runtime_root, http_port, websocket_port, root / "initialized.log",
                ["--initialize-admin", "admin"], password_file
            )
            try:
                _, bootstrap_seconds = wait_for_readiness(
                    initialized, http_port, root / "initialized.log"
                )
                login(http_port, "admin", password)
            finally:
                stop(initialized)
                initialized_log.close()

            persisted, persisted_log = start(
                executable, runtime_root, http_port, websocket_port, root / "persisted.log"
            )
            try:
                wait_for_readiness(persisted, http_port, root / "persisted.log")
                login(http_port, "admin", password)
            finally:
                stop(persisted)
                persisted_log.close()

            print(
                json.dumps(
                    {
                        "startupSeconds": round(startup_seconds, 3),
                        "shutdownSeconds": round(shutdown_seconds, 3),
                        "recoverySeconds": round(recovery_seconds, 3),
                        "bootstrapSeconds": round(bootstrap_seconds, 3),
                        "headlessAdminPersisted": True,
                        "profile": health["profile"],
                        "tlsRuntimeAvailable": health["tlsRuntimeAvailable"],
                        "webRtcAvailable": health["webRtcAvailable"],
                    },
                    sort_keys=True,
                )
            )
        finally:
            if first.poll() is None:
                stop(first)
            first_log.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
