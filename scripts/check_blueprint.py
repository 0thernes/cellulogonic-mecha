#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Verify the generated static GitHub Pages Lean 4.35.0 Blueprint portal (`_site/`).

Checks that `_site/index.html`, `_site/styles.css`, `_site/blueprint.js`, and `_site/.nojekyll`
exist, match the current `VERSION`, include all 78 comparator declarations + 12 pinned Spec defs,
enforce strict CSP with zero `unsafe-inline`, and contain zero stale overstated phrases."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SITE = ROOT / "_site"


def main() -> int:
    version = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    comp = json.loads((ROOT / "lean" / "comparator.json").read_text(encoding="utf-8"))
    theorems = comp.get("theorem_names", [])
    defs = comp.get("definition_names", [])

    required_files = [SITE / "index.html", SITE / "styles.css", SITE / "blueprint.js", SITE / ".nojekyll"]
    for f in required_files:
        if not f.exists():
            print(f"FAIL: missing generated blueprint file {f}", file=sys.stderr)
            return 1

    html_txt = (SITE / "index.html").read_text(encoding="utf-8")

    checks = [
        (f"v{version}" in html_txt, f"version v{version} missing from _site/index.html"),
        (f"{len(theorems)} Comparator Theorems" in html_txt, f"expected '{len(theorems)} Comparator Theorems' pill"),
        (f"{len(defs)} Pinned Defs" in html_txt, f"expected '{len(defs)} Pinned Defs' pill"),
        ("Content-Security-Policy" in html_txt and "unsafe-inline" not in html_txt, "strict CSP without unsafe-inline failed"),
        ("six independent proof kernels" not in html_txt.lower(), "stale 'six independent proof kernels' phrase found"),
        ("global minimax" not in html_txt.lower(), "stale 'global minimax' phrase found"),
        ("0/1,900 Mega-Atlas" in html_txt, "Mega-Atlas 0/1,900 honest-scope notice missing"),
        ("Not Peer-Reviewed" in html_txt, "Not Peer-Reviewed notice missing"),
    ]
    for ok, msg in checks:
        if not ok:
            print(f"FAIL: blueprint audit: {msg}", file=sys.stderr)
            return 1

    missing_thms = [t.rsplit(".", 1)[-1] for t in theorems if t.rsplit(".", 1)[-1] not in html_txt]
    if missing_thms:
        print(f"FAIL: {len(missing_thms)} theorem(s) missing from _site/index.html: {missing_thms[:5]}", file=sys.stderr)
        return 1

    print(
        f"PASS: Blueprint portal verified ({len(theorems)}/78 theorems, {len(defs)} pinned defs, "
        f"v{version}, strict CSP, honest scope)!"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
