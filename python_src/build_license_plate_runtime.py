#!/usr/bin/env python3
"""Builds a self-contained Python runtime archive for the license-plate module.

The script creates a virtual environment, installs the requested dependencies,
copies the required module sources/models, and produces a compressed archive
along with checksum/size information suitable for runtime-manifest.json.

Run it on the target operating system so that the bundled interpreter matches
(platform-specific build artefacts cannot be cross-compiled by this script).
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import shutil
import subprocess
import sys
import tarfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
PY_SRC_ROOT = REPO_ROOT / "python_src"
DEFAULT_OUTPUT_ROOT = PY_SRC_ROOT / "build" / "license_plate_runtime"
DEFAULT_ARCHIVE_ROOT = DEFAULT_OUTPUT_ROOT / "dist"

REQUIREMENTS_DIR = PY_SRC_ROOT / "requirements"
REQUIREMENTS_CPU = REQUIREMENTS_DIR / "requirements_cpu.txt"
REQUIREMENTS_DML = REQUIREMENTS_DIR / "requirements_dml.txt"


def detect_platform_tag() -> str:
    system = sys.platform
    machine = platform.machine().lower()

    if system.startswith("win"):
        if machine in {"amd64", "x86_64"}:
            return "win32-x64"
        if machine in {"arm64", "aarch64"}:
            return "win32-arm64"
        raise RuntimeError(f"Unsupported Windows architecture: {machine}")

    if system.startswith("linux"):
        if machine in {"x86_64", "amd64"}:
            return "linux-x64"
        raise RuntimeError(f"Unsupported Linux architecture: {machine}")

    if system == "darwin":
        if machine in {"arm64", "aarch64"}:
            return "darwin-arm64"
        if machine in {"x86_64", "amd64"}:
            return "darwin-x64"
        raise RuntimeError(f"Unsupported macOS architecture: {machine}")

    raise RuntimeError(f"Unsupported platform: {system}")


def get_default_archive_ext(platform_tag: str) -> str:
    if platform_tag.startswith("win32"):
        return "zip"
    return "tar.xz"


def run(cmd: list[str], cwd: Path | None = None, env: dict[str, str] | None = None) -> None:
    print(f"[build] $ {' '.join(str(c) for c in cmd)}")
    subprocess.run(cmd, check=True, cwd=cwd, env=env)


def create_venv(python: str, target: Path) -> None:
    run([python, "-m", "venv", str(target)])


def get_venv_python(venv_path: Path) -> Path:
    if platform.system().lower().startswith("win"):
        return venv_path / "Scripts" / "python.exe"
    return venv_path / "bin" / "python3"


def pip_install(python_bin: Path, requirements: Path, upgrade_pip: bool) -> None:
    if upgrade_pip:
        run([str(python_bin), "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"])
    run([str(python_bin), "-m", "pip", "install", "--no-cache-dir", "-r", str(requirements)])


def clean_pycache(root: Path) -> None:
    for path in list(root.rglob("__pycache__")):
        shutil.rmtree(path, ignore_errors=True)
    for pyc in list(root.rglob("*.pyc")):
        try:
            pyc.unlink()
        except OSError:
            pass


def copy_python_sources(target_dir: Path) -> None:
    ignore_patterns = shutil.ignore_patterns(
        "__pycache__",
        "*.pyc",
        "*.pyo",
        "*.log",
        "build",
        "dist",
        ".venv",
        "venv",
        "localpycs",
        "*.spec",
        "*.toc",
        "build_license_plate_runtime.py"
    )
    shutil.copytree(PY_SRC_ROOT, target_dir, dirs_exist_ok=True, ignore=ignore_patterns)
    clean_pycache(target_dir)


def strip_unneeded_files(venv_path: Path) -> None:
    system = platform.system().lower()

    if system.startswith("win"):
        patterns = ("*.pdb", "*.a", "*.lib")
        for pattern in patterns:
            for file in venv_path.glob(f"**/{pattern}"):
                try:
                    file.unlink()
                except OSError:
                    pass
        return

    if system.startswith("linux"):
        for pattern in ("*.a", "*.la", "*.o"):
            for file in venv_path.glob(f"**/{pattern}"):
                try:
                    file.unlink()
                except OSError:
                    pass

        strip_bin = shutil.which("strip")
        if strip_bin:
            shared_objects = list(venv_path.rglob("*.so")) + list(venv_path.rglob("*.so.*"))
            for so_path in shared_objects:
                try:
                    print(f"[strip] Stripping {so_path}")
                    subprocess.run([strip_bin, "--strip-unneeded", str(so_path)], check=True)
                except subprocess.CalledProcessError:
                    print(f"[strip] Warning: failed to strip {so_path}")


def make_archive(staging_dir: Path, archive_dir: Path, archive_name: str, fmt: str) -> Path:
    archive_dir.mkdir(parents=True, exist_ok=True)
    if fmt == "zip":
        base = archive_dir / archive_name
        shutil.make_archive(str(base), "zip", root_dir=staging_dir, base_dir=".")
        return base.with_suffix(".zip")

    if fmt in {"tar.gz", "tgz"}:
        archive_path = archive_dir / f"{archive_name}.tar.gz"
        with tarfile.open(archive_path, "w:gz") as tar_obj:
            tar_obj.add(staging_dir, arcname=".")
        return archive_path

    if fmt in {"tar.xz", "txz"}:
        archive_path = archive_dir / f"{archive_name}.tar.xz"
        with tarfile.open(archive_path, "w:xz") as tar_obj:
            tar_obj.add(staging_dir, arcname=".")
        return archive_path

    raise ValueError(f"Unsupported archive format: {fmt}")


def sha256sum(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def build_runtime(args: argparse.Namespace) -> None:
    platform_tag = args.platform or detect_platform_tag()
    archive_ext = args.archive or get_default_archive_ext(platform_tag)
    output_root = Path(args.output_dir or DEFAULT_OUTPUT_ROOT)
    staging_dir = output_root / platform_tag
    venv_path = staging_dir / "python"
    python_src_target = staging_dir / "python_src"

    if staging_dir.exists() and not args.keep_existing:
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True, exist_ok=True)

    create_venv(args.python, venv_path)
    venv_python = get_venv_python(venv_path)

    requirements_file = Path(args.requirements) if args.requirements else (
        REQUIREMENTS_DML if args.provider == "dml" else REQUIREMENTS_CPU
    )
    pip_install(venv_python, requirements_file, upgrade_pip=not args.skip_pip_upgrade)

    strip_unneeded_files(venv_path)

    if python_src_target.exists() and not args.keep_existing:
        shutil.rmtree(python_src_target)
    copy_python_sources(python_src_target)

    clean_pycache(staging_dir)

    archive_dir = Path(args.archive_dir or DEFAULT_ARCHIVE_ROOT)
    archive_name = f"license-plate-runtime-{platform_tag}"
    archive_path = make_archive(staging_dir, archive_dir, archive_name, archive_ext)

    size_bytes = archive_path.stat().st_size
    checksum = sha256sum(archive_path)

    print("\n=== Build complete ===")
    print(f"Archive: {archive_path}")
    print(f"Size: {size_bytes:,} bytes")
    print(f"SHA256: {checksum}")

    manifest_entry = {
        "url": f"<upload-url>/{archive_path.name}",
        "sha256": checksum,
        "size": size_bytes,
        "archiveType": archive_ext,
        "pythonExecutable": "python/Scripts/python.exe" if platform_tag.startswith("win32") else "python/bin/python3",
        "scriptRoot": "python_src",
        "version": args.runtime_version,
        "provider": args.provider
    }

    print("\nManifest snippet (replace <upload-url> and adjust fields as needed):")
    print(json.dumps(manifest_entry, indent=2))


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build license-plate runtime archive")
    parser.add_argument("--platform", help="Platform tag (e.g. win32-x64). Default: auto-detect")
    parser.add_argument("--provider", choices=["cpu", "dml"], default="cpu", help="Dependency profile to install")
    parser.add_argument("--requirements", help="Custom requirements file path")
    parser.add_argument("--python", default=sys.executable, help="Python interpreter used to create the virtual environment")
    parser.add_argument("--archive", choices=["zip", "tar.gz", "tgz"], help="Archive format. Default derived from platform")
    parser.add_argument("--output-dir", help="Staging directory for assembled runtime")
    parser.add_argument("--archive-dir", help="Directory where final archives are written")
    parser.add_argument("--runtime-version", default="1.0.0", help="Version label written into manifest snippet")
    parser.add_argument("--skip-pip-upgrade", action="store_true", help="Do not upgrade pip/setuptools before installing requirements")
    parser.add_argument("--keep-existing", action="store_true", help="Reuse existing staging directory contents if present")
    parser.add_argument("--verbose", action="store_true", help="Currently unused placeholder for future logging control")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> None:
    args = parse_args(argv or sys.argv[1:])
    build_runtime(args)


if __name__ == "__main__":
    main()
