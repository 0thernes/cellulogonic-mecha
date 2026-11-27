#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Single-source-of-truth version consistency checker across ALL tracked manifests,
headers, proofs, Web UI, tests, and paper sources (Build Standard A08 / F-34)."""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent


def read_text(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def main() -> int:
    ver_file = ROOT / "VERSION"
    if not ver_file.exists():
        print("FAIL: VERSION file missing at repository root.", file=sys.stderr)
        return 1
    version = ver_file.read_text(encoding="utf-8").strip()
    m = re.match(r"^(\d+)\.(\d+)\.(\d+)$", version)
    if not m:
        print(f"FAIL: VERSION '{version}' is not SemVer X.Y.Z", file=sys.stderr)
        return 1
    maj, min_, pat = map(int, m.groups())
    abi_hex = f"0x{maj:04X}{min_:02X}{pat:02X}u"

    checks: list[tuple[str, str]] = [
        ("include/cellulogonic_abi.h", abi_hex),
        ("src/main_cli.c", f"cellulogonic {version}"),
        ("web/server.mjs", f'VERSION = "{version}"'),
        ("web/index.html", f"v{version}"),
        ("web/app.js", f"v{version}"),
        ("web/renderer3d.js", f"v{version}"),
        ("web/styles.css", f"v{version}"),
        ("test/test_web_server.mjs", f'"{version}"'),
        ("lean/lakefile.toml", f'version = "{version}"'),
        ("lean/formalization.yaml", f"v{version}"),
        ("CITATION.cff", f'version: "{version}"'),
        ("CITATION.bib", f"v{version}"),
        (".zenodo.json", f'"version": "{version}"'),
        ("codemeta.json", f'"version": "{version}"'),
        ("croissant.json", f'"version": "{version}"'),
        ("ro-crate-metadata.json", f'"version": "{version}"'),
        ("repo.yaml", f"v{version}"),
        ("CHANGELOG.md", f"## [{version}]"),
        ("README.md", f"v{version}"),
        ("paper/main.tex", f"v{version}"),
    ]

    failures: list[str] = []
    for rel, needle in checks:
        p = ROOT / rel
        if not p.exists():
            failures.append(f"MISSING {rel}")
            continue
        txt = read_text(rel)
        if needle.lower() not in txt.lower():
            failures.append(f"MISMATCH in {rel}: expected '{needle}'")

    if failures:
        print(f"FAIL: Version consistency check failed for VERSION={version}:", file=sys.stderr)
        for f in failures:
            print(f"  - {f}", file=sys.stderr)
        return 1

    print(f"PASS: All {len(checks)} versioned manifests, headers, proofs, UI & paper files match VERSION={version} (ABI {abi_hex}).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
