#!/usr/bin/env python3
"""Download the latest go2rtc binaries for all supported platforms."""

from __future__ import annotations

import fnmatch
import io
import os
import sys
import tarfile
import zipfile
from pathlib import Path
from typing import Dict, Iterable

import requests

API_URL = "https://api.github.com/repos/AlexxIT/go2rtc/releases/latest"

TARGETS: Dict[str, Dict[str, Iterable[str]]] = {
    "windows": {
        "patterns": ["go2rtc-windows-amd64.zip", "go2rtc-windows-x64.zip"],
        "binary": "go2rtc.exe",
    },
    "linux": {
        "patterns": ["go2rtc-linux-amd64.tar.gz", "go2rtc-linux-x64.tar.gz"],
        "binary": "go2rtc",
    },
    "macos": {
        "patterns": ["go2rtc-darwin-amd64.tar.gz", "go2rtc-macos-amd64.tar.gz"],
        "binary": "go2rtc",
    },
}

BINARY_ROOT = Path("src-tauri") / "binaries"


def ensure_destination(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def find_asset(asset_name: str, patterns: Iterable[str]) -> bool:
    return any(fnmatch.fnmatch(asset_name, pattern) for pattern in patterns)


def download_asset(url: str) -> bytes:
    response = requests.get(url, timeout=30)
    response.raise_for_status()
    return response.content


def extract_from_zip(data: bytes, filename: str) -> bytes:
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        target = next((name for name in archive.namelist() if name.endswith(filename)), None)
        if target is None:
            raise FileNotFoundError(f"{filename} not found in archive")
        with archive.open(target) as file_obj:
            return file_obj.read()


def extract_from_tar(data: bytes, filename: str) -> bytes:
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        member = next((entry for entry in archive.getmembers() if entry.name.endswith(filename)), None)
        if member is None:
            raise FileNotFoundError(f"{filename} not found in archive")
        extracted = archive.extractfile(member)
        if extracted is None:
            raise FileNotFoundError(f"{filename} not found in archive")
        return extracted.read()


def main() -> int:
    print("[INFO] Fetching go2rtc release metadata...")
    response = requests.get(API_URL, timeout=30)
    response.raise_for_status()
    release = response.json()
    assets = release.get("assets", [])

    if not assets:
        print("[ERROR] No assets found in the latest go2rtc release", file=sys.stderr)
        return 1

    processed = 0
    for platform, info in TARGETS.items():
        patterns = info["patterns"]
        binary_name = info["binary"]

        asset = next(
            (item for item in assets if find_asset(item.get("name", ""), patterns)),
            None,
        )

        if asset is None:
            print(f"[WARN] No matching asset for {platform}, skipped")
            continue

        download_url = asset.get("browser_download_url")
        if not download_url:
            print(f"[WARN] Asset for {platform} lacks download URL, skipped")
            continue

        print(f"[INFO] Downloading {asset['name']} for {platform}...")
        payload = download_asset(download_url)

        if asset["name"].endswith(".tar.gz"):
            binary_data = extract_from_tar(payload, binary_name)
        elif asset["name"].endswith(".zip"):
            binary_data = extract_from_zip(payload, binary_name)
        else:
            print(f"[WARN] Unsupported archive format for {asset['name']}, skipped")
            continue

        destination = BINARY_ROOT / platform / binary_name
        ensure_destination(destination)
        destination.write_bytes(binary_data)
        if platform != "windows":
            os.chmod(destination, 0o755)

        print(f"[INFO] Saved {destination}")
        processed += 1

    if processed == 0:
        print("[ERROR] No go2rtc binaries downloaded", file=sys.stderr)
        return 1

    print("[INFO] go2rtc binaries are ready")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
