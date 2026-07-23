#!/usr/bin/env python3
"""Generate deterministic checksums, release metadata and a CycloneDX component manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import uuid
from datetime import datetime, timezone
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True)
    parser.add_argument("--components", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, default=Path("."))
    parser.add_argument("assets", nargs="+", type=Path)
    arguments = parser.parse_args()

    assets = []
    for asset_path in sorted((path.resolve() for path in arguments.assets), key=lambda item: item.name):
        if not asset_path.is_file():
            raise FileNotFoundError(asset_path)
        assets.append(
            {"name": asset_path.name, "size": asset_path.stat().st_size, "sha256": sha256(asset_path)}
        )

    components = json.loads(arguments.components.read_text(encoding="utf-8"))
    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    output_dir = arguments.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    checksums = "".join(f"{asset['sha256']}  {asset['name']}\n" for asset in assets)
    (output_dir / "SHA256SUMS.txt").write_text(checksums, encoding="utf-8", newline="\n")

    metadata = {
        "schemaVersion": 1,
        "product": "OpenIPC Dashboard",
        "version": arguments.version,
        "generatedAt": generated_at,
        "assets": assets,
        "components": components,
    }
    metadata_name = f"OpenIPC-Dashboard-{arguments.version}-release-metadata.json"
    (output_dir / metadata_name).write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n"
    )

    cdx_components = []
    for component in components:
        cdx_components.append(
            {
                "type": component["type"],
                "name": component["name"],
                "version": component["version"],
                "scope": component.get("scope", "required"),
                "licenses": [{"license": {"name": component["license"]}}],
            }
        )
    bom = {
        "bomFormat": "CycloneDX",
        "specVersion": "1.5",
        "serialNumber": f"urn:uuid:{uuid.uuid5(uuid.NAMESPACE_URL, f'https://openipc.org/dashboard/{arguments.version}')}",
        "version": 1,
        "metadata": {
            "timestamp": generated_at,
            "component": {"type": "application", "name": "OpenIPC Dashboard", "version": arguments.version},
        },
        "components": cdx_components,
    }
    bom_name = f"OpenIPC-Dashboard-{arguments.version}.cdx.json"
    (output_dir / bom_name).write_text(
        json.dumps(bom, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n"
    )
    print(json.dumps({"checksums": "SHA256SUMS.txt", "metadata": metadata_name, "sbom": bom_name}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
