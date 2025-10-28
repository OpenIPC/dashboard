#!/usr/bin/env python3
"""Download and stage a redistributable GStreamer runtime for Linux builds.

The script fetches the official GStreamer precompiled bundle, extracts the
runtime, and copies the required pieces into ``src-tauri/resources/gstreamer``
so the Tauri bundler can embed them into the final AppImage.
"""

import argparse
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.request
from urllib.error import HTTPError, URLError
from pathlib import Path

DEFAULT_VERSION = "1.24.7"
DEFAULT_BASE_URL = "https://gstreamer.freedesktop.org/data/pkg"


def download(url: str, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(url) as response, destination.open("wb") as fh:
        shutil.copyfileobj(response, fh)


def copy_directory(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def stage_runtime(version: str, base_url: str, target_dir: Path, force: bool) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)
    skip_marker = target_dir / ".download-skipped"

    if skip_marker.exists() and not force:
        print(
            "Skipping GStreamer download; previous attempt marked it as unavailable. "
            "Remove src-tauri/resources/gstreamer/.download-skipped or pass --force to retry."
        )
        return

    if skip_marker.exists() and force:
        skip_marker.unlink(missing_ok=True)

    scanner_path = target_dir / "libexec/gstreamer-1.0/gst-plugin-scanner"
    if scanner_path.exists() and not force:
        print("GStreamer runtime already present; skipping download.")
        return

    base = base_url.rstrip("/")
    preferred_versions = []
    seen = set()
    for candidate in [
        version,
        "1.24.8",
        "1.24.7",
        "1.24.6",
        "1.24.5",
        "1.24.4",
        "1.24.3",
        "1.24.2",
        "1.24.1",
        "1.24.0",
        "1.22.11",
        "1.22.10",
        "1.22.9",
        "1.22.8",
    ]:
        if candidate and candidate not in seen:
            preferred_versions.append(candidate)
            seen.add(candidate)

    resolved_archive = None

    with tempfile.TemporaryDirectory() as tmp_str:
        tmp_dir = Path(tmp_str)
        archive_path = tmp_dir / "gstreamer-runtime.tar.xz"

        custom_archive = os.environ.get("GSTREAMER_BUNDLE_ARCHIVE")
        if custom_archive:
            archive_path = Path(custom_archive)
            if archive_path.is_file():
                print(f"Using local archive override: {archive_path}")
                shutil.copy2(archive_path, tmp_dir / archive_path.name)
                resolved_archive = archive_path.name
                version = version or "custom"
                archive_path = tmp_dir / archive_path.name
            else:
                print(f"Provided GSTREAMER_BUNDLE_ARCHIVE does not exist: {archive_path}")

        for candidate_version in preferred_versions:
            if resolved_archive:
                break

            archive_name = f"gstreamer-1.0-x86_64-{candidate_version}.tar.xz"
            candidate_urls = []

            env_urls = os.environ.get("GSTREAMER_BUNDLE_URL", "")
            if env_urls:
                for custom_url in env_urls.split(";"):
                    stripped = custom_url.strip()
                    if stripped:
                        candidate_urls.append(stripped)

            candidate_urls.extend([
                f"{base}/unified/{candidate_version}/{archive_name}",
                f"{base}/unified/{archive_name}",
                f"{base}/unified/1.0/linux/x86_64/{archive_name}",
                f"{base}/linux/{candidate_version}/{archive_name}",
                f"{base}/1.0/{candidate_version}/linux/x86_64/{archive_name}",
                f"https://gitlab.freedesktop.org/gstreamer/gstreamer/-/releases/{candidate_version}/downloads/{archive_name}",
            ])

            for url in candidate_urls:
                try:
                    print(f"Downloading {url} ...")
                    download(url, archive_path)
                    resolved_archive = archive_name
                    version = candidate_version
                    break
                except HTTPError as exc:
                    if exc.code == 404:
                        print(f"URL not found ({url}), trying next candidate...")
                        continue
                    print(f"HTTP error while downloading {url}: {exc}")
                except URLError as exc:
                    print(f"Network error while downloading {url}: {exc}")
            if resolved_archive:
                break

        if not resolved_archive:
            message_lines = [
                "Warning: unable to locate a redistributable GStreamer bundle from known mirrors.",
                "The Linux AppImage will rely on the target system's GStreamer installation.",
                "Provide a custom bundle via GSTREAMER_BUNDLE_URL or GSTREAMER_BUNDLE_ARCHIVE,",
                "or run tools/download-gstreamer-runtime.py --force after supplying the files manually.",
            ]
            print("\n".join(message_lines))
            skip_marker.write_text("Skipped: no bundle available\n", encoding="utf-8")
            return

        print("Extracting archive ...")
        with tarfile.open(archive_path, mode="r:*") as tar:
            tar.extractall(tmp_dir)

        bundle_roots = sorted(tmp_dir.glob("gstreamer-1.0-*"))
        if not bundle_roots:
            raise RuntimeError("Unable to locate extracted GStreamer bundle root")

        bundle_root = bundle_roots[0]
        print(f"Using bundle root: {bundle_root.name}")

        layout = {
            "bin": target_dir / "bin",
            "lib": target_dir / "lib",
            "libexec/gstreamer-1.0": target_dir / "libexec/gstreamer-1.0",
            "share/gstreamer-1.0": target_dir / "share/gstreamer-1.0",
            "share/doc": target_dir / "share/doc",
            "share/licenses": target_dir / "share/licenses",
        }

        for relative, destination in layout.items():
            source = bundle_root / relative
            if source.exists():
                print(f"Staging {relative} -> {destination}")
                destination.parent.mkdir(parents=True, exist_ok=True)
                if source.is_dir():
                    copy_directory(source, destination)
                else:
                    shutil.copy2(source, destination)

        cache_dir = target_dir / "cache"
        cache_dir.mkdir(exist_ok=True)

    # Ensure executables have the correct permissions once extraction is done
    for executable in [
        target_dir / "bin/gst-launch-1.0",
        target_dir / "bin/gst-inspect-1.0",
        target_dir / "bin/gst-plugin-scanner",
        target_dir / "libexec/gstreamer-1.0/gst-plugin-scanner",
    ]:
        if executable.exists():
            executable.chmod(0o755)

    if skip_marker.exists():
        skip_marker.unlink(missing_ok=True)

    print("GStreamer runtime prepared in src-tauri/resources/gstreamer")

def main(argv=None) -> None:
    parser = argparse.ArgumentParser(description="Download GStreamer runtime for AppImage builds")
    parser.add_argument("--version", default=DEFAULT_VERSION, help="GStreamer release version to download")
    parser.add_argument("--base-url", default=DEFAULT_BASE_URL, help="Base URL for GStreamer bundles")
    parser.add_argument("--force", action="store_true", help="Force re-download even if files exist")
    args = parser.parse_args(argv)

    repository_root = Path(__file__).resolve().parents[1]
    target_directory = repository_root / "src-tauri" / "resources" / "gstreamer"

    stage_runtime(args.version, args.base_url, target_directory, args.force)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:  # pragma: no cover - surfaced to caller
        print(f"Failed to prepare GStreamer runtime: {exc}")
        sys.exit(1)
