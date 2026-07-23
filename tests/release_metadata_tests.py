#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ReleaseMetadataTests(unittest.TestCase):
    def test_generates_checksums_metadata_and_valid_cyclonedx_identity(self) -> None:
        with tempfile.TemporaryDirectory(prefix="openipc-release-metadata-") as directory:
            root = Path(directory)
            first = root / "Dashboard.exe"
            second = root / "Dashboard.AppImage"
            first.write_bytes(b"windows-package")
            second.write_bytes(b"linux-package")
            subprocess.run(
                [sys.executable, str(ROOT / "tools" / "generate_release_metadata.py"),
                 "--version", "v0.2.8", "--components",
                 str(ROOT / "packaging" / "third-party-components.json"),
                 "--output-dir", str(root), str(first), str(second)],
                check=True, capture_output=True, text=True,
            )

            checksums = (root / "SHA256SUMS.txt").read_text(encoding="utf-8")
            self.assertIn(hashlib.sha256(first.read_bytes()).hexdigest(), checksums)
            self.assertIn(hashlib.sha256(second.read_bytes()).hexdigest(), checksums)

            metadata = json.loads(
                (root / "OpenIPC-Dashboard-v0.2.8-release-metadata.json").read_text(encoding="utf-8"))
            self.assertEqual(metadata["version"], "v0.2.8")
            self.assertEqual({asset["name"] for asset in metadata["assets"]},
                             {first.name, second.name})

            bom = json.loads((root / "OpenIPC-Dashboard-v0.2.8.cdx.json").read_text(encoding="utf-8"))
            self.assertEqual(bom["bomFormat"], "CycloneDX")
            self.assertEqual(bom["specVersion"], "1.5")
            uuid.UUID(bom["serialNumber"].removeprefix("urn:uuid:"))
            self.assertGreaterEqual(len(bom["components"]), 6)


if __name__ == "__main__":
    unittest.main()
