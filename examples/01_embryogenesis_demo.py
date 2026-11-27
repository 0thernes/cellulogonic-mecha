#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Runnable Example 01: Zero-Blueprint 5-Fate 3D Clonal Embryogenesis Verification (v1.7.0)."""

from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_binary() -> Path:
    env_bin = os.environ.get("CELLULOGONIC_BIN")
    if env_bin:
        p = Path(env_bin)
        if p.exists():
            return p
    if sys.platform == "win32":
        cands = [ROOT / "bin" / "cellulogonic.exe", ROOT / "bin" / "cellulogonic_linux"]
    else:
        cands = [ROOT / "bin" / "cellulogonic_linux", ROOT / "bin" / "cellulogonic.exe"]
    for p in cands:
        if p.exists():
            return p
    raise FileNotFoundError(
        "Compiled binary not found. Run .\\build.ps1 (Win64) or ./build_linux.sh (Linux/macOS) first."
    )


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
    m_fates = re.findall(r"Fates\[P=\s*(\d+)\s+Ec=\s*(\d+)\s+N=\s*(\d+)\s+M=\s*(\d+)\s+G=\s*(\d+)\s+Gm=\s*(\d+)\]", res.stdout)
    assert m_fates, "Missing final Fates telemetry row!"
    _, ec, ne, mu, gu, gm = map(int, m_fates[-1])
    assert all(x > 0 for x in (ec, ne, mu, gu, gm)), f"Expected all 5 differentiated fates > 0, got Ec={ec} N={ne} M={mu} G={gu} Gm={gm}"
    m_sha = re.search(r"SHA=([0-9a-f]{64})", res.stdout)
    assert m_sha, "Missing 64-hex state SHA-256!"

    # Verify cross-kernel bit-exact determinism (SIMD vs --scalar)
    res_sca = subprocess.run(
        [str(exe), "embryo", "--epochs", "200", "--size", "32", "--seed", "5000", "--scalar"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    m_sha_sca = re.search(r"SHA=([0-9a-f]{64})", res_sca.stdout)
    assert m_sha_sca and m_sha_sca.group(1) == m_sha.group(1), "SIMD vs Scalar SHA-256 parity mismatch!"
    print(
        f"\nPASS: Exact 5-fate 3D clonal embryogenesis verified "
        f"(Ec={ec} N={ne} M={mu} G={gu} Gm={gm}, 0 drift, SIMD==Scalar SHA={m_sha.group(1)[:16]}...)"
    )


if __name__ == "__main__":
    main()
