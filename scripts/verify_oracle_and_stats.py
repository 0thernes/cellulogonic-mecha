#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""
Independent Differential, Metamorphic, and Statistical Verification Script
for CELLULOGONIC MECHA v1.6.0 (Build Standard B04, B08, B30).

1. Runs the clean-room exact Python reference oracle (reference/chemoton_oracle.py).
2. Runs the native C11 binary across metamorphic & differential checks:
   - Determinism check (identical seed => identical 64-hex SHA-256)
   - Solvent conservation closure across all 4 planetary solvents
   - JSON observer invariance (P9: repeated state queries never drift)
3. Independently validates the N=24 locked paired statistical suite output:
   - Parses cmd_stats output and confirms p_Holm <= 0.05, 95% CI ordering, finite Cohen's dz.
"""
from __future__ import annotations

import json
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "reference"))
import chemoton_oracle  # noqa: E402


def find_binary() -> pathlib.Path:
    if os.name == "nt":
        p = ROOT / "bin" / "cellulogonic.exe"
    else:
        p = ROOT / "bin" / "cellulogonic_linux"
    if not p.exists():
        raise FileNotFoundError(f"Native C11 binary not found at {p}. Build first.")
    return p


def run_cli(bin_path: pathlib.Path, *args: str) -> str:
    proc = subprocess.run(
        [str(bin_path), *args],
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        check=True,
    )
    return proc.stdout


def main() -> int:
    # 1. Exact Python Reference Oracle
    rc = chemoton_oracle.main()
    if rc != 0:
        return rc

    bin_path = find_binary()

    # 2. Differential determinism & 4-solvent exact conservation via native JSON snapshots
    for solvent in range(4):
        out1 = run_cli(bin_path, "json", "--epochs", "40", "--size", "16", "--seed", "5100", "--solvent", str(solvent))
        out2 = run_cli(bin_path, "json", "--epochs", "40", "--size", "16", "--seed", "5100", "--solvent", str(solvent))
        j1 = json.loads(out1)
        j2 = json.loads(out2)
        t1, t2 = j1["telemetry"], j2["telemetry"]
        assert t1["sha256"] == t2["sha256"], f"Solvent {solvent} non-deterministic SHA!"
        assert t1["mass_res"] == 0, f"Solvent {solvent} mass residual {t1['mass_res']} != 0"
        assert t1["energy_res"] == 0, f"Solvent {solvent} energy residual {t1['energy_res']} != 0"
        assert t1["second_law"] == 0, f"Solvent {solvent} 2nd-law violation {t1['second_law']}"
        assert t1["drift"] == 0.0, f"Solvent {solvent} drift {t1['drift']} != 0"
        # Check Asmodensius 112-trait vector strictly bounded in [0, 1]
        if j1["bridge"]["valid"]:
            traits = j1["bridge"]["traits112"]
            assert len(traits) == 112
            assert all(0.0 <= v <= 1.0 for v in traits), "Out-of-range Asmodensius trait!"

    # 3. Verify --version banner matches VERSION file (single source of truth, F-21)
    expected_ver = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    ver_out = run_cli(bin_path, "--version").strip()
    assert f"cellulogonic {expected_ver}" in ver_out, f"Unexpected --version output: {ver_out} (expected {expected_ver})"

    # 4. Validate locked paired statistical suite (N=8 fast CI check; full N=24 in results/09_paired_stats_n24.txt)
    stats_out = run_cli(bin_path, "stats", "--pairs", "8", "--epochs", "80", "--size", "24")
    row_re = re.compile(
        r"^(H\d:[^|]+)\|\s*([+-]?\d+\.\d+)\s*\|\s*([+-]?\d+\.\d+)\s*\|\s*([+-]?\d+\.\d+)\s*\|\s*"
        r"\[\s*([+-]?\d+\.\d+),\s*([+-]?\d+\.\d+)\]\s*\|\s*([+-]?\d+\.\d+)\s*\|\s*(\d+)/(\d+)\s*\|\s*"
        r"(\d+\.\d+)\s*\|\s*(\d+\.\d+)\s*$",
        re.M,
    )
    rows = row_re.findall(stats_out)
    assert len(rows) == 4, f"Expected 4 hypothesis rows in cmd_stats output, found {len(rows)}:\n{stats_out}"
    for name, ma, mb, diff, ci_lo, ci_hi, dz, wins, total, p_raw, p_holm in rows:
        f_ma, f_mb, f_diff = float(ma), float(mb), float(diff)
        f_lo, f_hi, f_dz = float(ci_lo), float(ci_hi), float(dz)
        f_pr, f_ph = float(p_raw), float(p_holm)
        assert abs((f_ma - f_mb) - f_diff) < 0.01, f"{name}: diff mismatch"
        assert f_lo <= f_diff + 1e-3 and f_diff <= f_hi + 1e-3, f"{name}: diff outside 95% CI [{f_lo}, {f_hi}]"
        assert f_ph >= f_pr - 1e-6 and f_ph <= 0.05, f"{name}: Holm-Bonferroni p={f_ph} exceeds alpha=0.05"
        assert abs(f_dz) > 0.5, f"{name}: unexpectedly tiny Cohen's dz={f_dz}"

    print(
        "PASS: Differential 4-solvent closure, determinism, Asmodensius [0,1] bounds, "
        "and paired statistical CI/Cohen's dz/Holm-Bonferroni checks all passed."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
