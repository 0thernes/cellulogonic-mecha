#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Runnable Example 01: Zero-Blueprint 5-Fate 3D Clonal Embryogenesis Verification (v1.5.0)."""

from __future__ import annotations

import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXPECTED_SHA = "9526ca6a245c2f5887eb7f55d3a45e6ac0e667ec6dd88fd41f1ceb49bc3bbb89"


def find_binary() -> Path:
    candidates = [
        ROOT / "bin" / "cellulogonic.exe",
        ROOT / "bin" / "cellulogonic_linux",
    ]
    for p in candidates:
        if p.exists():
            return p
    raise FileNotFoundError("Compiled binary not found. Run .\\build.ps1 (Win64) or ./build_linux.sh (Linux) first.")


def main() -> None:
    exe = find_binary()
    print(f"Running 3D Clonal Embryogenesis via {exe.relative_to(ROOT)} ...")
    res = subprocess.run(
        [str(exe), "embryo", "--epochs", "200", "--size", "32", "--seed", "5000"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    print(res.stdout.strip())
    assert "mass_res=0 amu | energy_res=0 uq | 2nd_law_viol=0" in res.stdout, "Conservation drift detected!"
    assert "Fates[P= 4 Ec= 8 N=18 M= 9 G= 6 Gm= 5]" in res.stdout, "Expected all 5 differentiated fates!"
    assert EXPECTED_SHA in res.stdout, f"SHA-256 mismatch (expected {EXPECTED_SHA})!"
    print("\nPASS: Exact 5-fate 3D clonal embryogenesis verified with 0 drift and exact SHA-256!")


if __name__ == "__main__":
    main()
