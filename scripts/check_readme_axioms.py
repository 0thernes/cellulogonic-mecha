#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Verify that README.md reproduces the exact verbatim Lean 4.35.0 `#print axioms` output
from `results/lean4-audit.txt` with zero drift, and that zero forbidden axioms exist."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
AXIOMS_PATH = ROOT / "results" / "lean4-audit.txt"
README_PATH = ROOT / "README.md"
ALLOWED = {"propext", "Classical.choice", "Quot.sound"}
FORBIDDEN = ("sorryAx", "Lean.ofReduceBool", "Lean.trustCompiler", "_native", "declaration uses 'sorry'")
EXPECTED_THEOREMS = 78


def main() -> None:
    if not AXIOMS_PATH.exists():
        print(f"FAIL: missing {AXIOMS_PATH}", file=sys.stderr)
        sys.exit(1)

    axioms_txt = AXIOMS_PATH.read_text(encoding="utf-8").replace("\r\n", "\n").strip()
    for tok in FORBIDDEN:
        if tok in axioms_txt:
            print(f"FAIL: forbidden axiom token {tok!r} found in {AXIOMS_PATH}", file=sys.stderr)
            sys.exit(1)

    matches = re.findall(
        r"'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])",
        axioms_txt,
        flags=re.S,
    )
    if len(matches) != EXPECTED_THEOREMS:
        print(f"FAIL: expected {EXPECTED_THEOREMS} audited theorems in {AXIOMS_PATH}, got {len(matches)}", file=sys.stderr)
        sys.exit(1)

    for name, _, axs in matches:
        for ax in [a.strip() for a in axs.split(",") if a.strip()]:
            if ax not in ALLOWED:
                print(f"FAIL: unpermitted axiom {ax!r} detected in {name}", file=sys.stderr)
                sys.exit(1)

    readme = README_PATH.read_text(encoding="utf-8").replace("\r\n", "\n")
    marker_start = "<!-- LEAN_AXIOMS_START -->"
    marker_end = "<!-- LEAN_AXIOMS_END -->"
    if marker_start not in readme or marker_end not in readme:
        print("FAIL: README.md missing LEAN_AXIOMS_START / LEAN_AXIOMS_END markers", file=sys.stderr)
        sys.exit(1)

    block = readme.split(marker_start, 1)[1].split(marker_end, 1)[0]
    m = re.search(r"```text\n(.*?)\n```", block, flags=re.S)
    if not m:
        print("FAIL: README.md axioms block missing ```text fence", file=sys.stderr)
        sys.exit(1)

    readme_axioms = m.group(1).strip()
    if readme_axioms != axioms_txt:
        print("FAIL: README.md verbatim axioms block drifted from results/lean4-audit.txt!", file=sys.stderr)
        sys.exit(1)

    print(
        f"PASS: All {len(matches)}/{EXPECTED_THEOREMS} Lean 4.35.0 theorem axioms verified "
        f"(strictly within [propext, Classical.choice, Quot.sound], 0 sorryAx) and README.md matches 100% verbatim!"
    )


if __name__ == "__main__":
    main()
