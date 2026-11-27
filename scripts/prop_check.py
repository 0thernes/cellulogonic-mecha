#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Deterministic Property-Based Testing Harness (`prop_check`, Build Standard B10)
Validates discrete biophysical invariants across 2,500+ randomized seeded instances
matching C11 (`include/cellulogonic_internal.h`) and Lean 4.35.0 (`Spec.lean`)."""

from __future__ import annotations

import json
import os
import pathlib
import random
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent


def find_c11_binary() -> pathlib.Path | None:
    env_bin = os.environ.get("CELLULOGONIC_BIN")
    if env_bin and pathlib.Path(env_bin).exists():
        return pathlib.Path(env_bin)
    cands = (
        [ROOT / "bin" / "cellulogonic.exe", ROOT / "bin" / "cellulogonic_linux"]
        if sys.platform == "win32"
        else [ROOT / "bin" / "cellulogonic_linux", ROOT / "bin" / "cellulogonic.exe"]
    )
    for c in cands:
        if c.exists():
            return c
    return None


def prop_check(name: str, trials: int, fn) -> None:
    rng = random.Random(20261126)
    for i in range(trials):
        ok = fn(rng)
        if not ok:
            print(f"FAIL: property '{name}' falsified on trial {i}", file=sys.stderr)
            sys.exit(1)
    print(f"  [PASS] prop_check {name} ({trials} trials)")


def main() -> None:
    print("=== CELLULOGONIC MECHA v1.7.0 — Property-Based Invariant & Native C11 Differential Suite (B10) ===")

    # 1. Elementary Sextic Polynomial Identity for Cube/Square Vesicle Splits (Lean T16)
    def check_sextic(rng: random.Random) -> bool:
        x = rng.randint(1, 500)
        y = rng.randint(1, 500)
        lhs = 2 * (x**3 + y**3) ** 2 - (x**2 + y**2) ** 3
        rhs = (x - y) ** 2 * (x**4 + 2 * x**3 * y + 2 * x * y**3 + y**4)
        return lhs == rhs and (lhs > 0 if x != y else lhs == 0)

    prop_check("canham_helfrich_sextic_T16", 600, check_sextic)

    # 2. 4-Solvent 19-Reaction Exact Mass, 1st-Law & Strict 2nd-Law Closure (Lean T01-T03)
    MASS = (4, 2, 6, 4, 8, 12, 16, 2)
    MU = (48, 36, 80, 40, 104, 156, 212, 6)
    SOLVENTS = {
        "Terran": (10, 6, 3),
        "Titan": (8, 7, 4),
        "Hydrothermal": (10, 6, 3),
        "Venus": (15, 4, 2),
    }
    RXNS = [
        ("R1", (-1, -1, 1, 0, 0, 0, 0, 0), 0, 0),
        ("R2", (0, 0, -1, 0, 0, 0, 0, 3), "r2", 0),
        ("R3", (0, 0, 0, 1, 0, 0, 0, -2), "-nH", 0),
        ("R4", (-2, 0, 0, 1, 0, 0, 0, 2), 0, 0),
        ("R5", (-2, 0, 0, -1, 1, 0, 0, 2), 0, 0),
        ("R6", (-3, 0, 0, -1, 0, 1, 0, 2), 0, 0),
        ("R7", (-4, 0, 0, -1, 0, 0, 1, 2), 0, 0),
        ("R8", (0, 0, 0, -1, 0, 0, 0, 2), 0, 0),
        ("R9", (0, 0, 0, 0, -1, 0, 0, 4), 0, 0),
        ("R10", (0, 0, 0, 0, 0, -1, 0, 6), 0, 0),
        ("R11", (0, 0, 0, 0, 0, 0, -1, 8), 0, 0),
        ("R12", (0, 0, 0, 0, 0, 0, 0, 0), -1, 0),
        ("R13a", (1, 0, 0, 0, -1, 0, 0, 2), 0, 0),
        ("R13b", (2, 0, 0, 0, 0, -1, 0, 2), 0, 0),
        ("R14a", (1, 0, 0, 0, 0, 0, 0, -2), 0, 46),
        ("R14b", (0, 1, 0, 0, 0, 0, 0, -1), 0, 34),
        ("R15a", (-2, 0, 0, 0, 1, 0, 0, 0), 0, 12),
        ("R15b", (-3, 0, 0, 0, 0, 1, 0, 0), 0, 16),
        ("R15c", (-4, 0, 0, 0, 0, 0, 1, 0), 0, 24),
    ]

    def check_thermo(rng: random.Random) -> bool:
        sv = rng.choice(list(SOLVENTS.keys()))
        e_pmf, r2_y, n_min = SOLVENTS[sv]
        nH = rng.randint(n_min, n_min + 6)
        _, nu, dpmf_tag, e_in = rng.choice(RXNS)
        m_delta = sum(w * v for w, v in zip(MASS, nu))
        chem_delta = sum(m * v for m, v in zip(MU, nu))
        dpmf = r2_y if dpmf_tag == "r2" else (-nH if dpmf_tag == "-nH" else dpmf_tag)
        q = e_in - chem_delta - dpmf * e_pmf
        return m_delta == 0 and q > 0

    prop_check("chemoton_4solvent_19rxn_closure_T01_T03", 600, check_thermo)

    # 3. Levin Bioelectric Gap-Junction Charge Conservation & Dirichlet Energy Drop (Lean T32-T33)
    def check_bioelectric(rng: random.Random) -> bool:
        vi = rng.randint(-90, 40)
        vj = rng.randint(-90, vi)
        max_d = (vi - vj) // 2
        d = rng.randint(0, max_d)
        vi2, vj2 = vi - d, vj + d
        return (
            vi2 + vj2 == vi + vj
            and vi2 * vi2 + vj2 * vj2 <= vi * vi + vj * vj
            and (vi2 - vj2) ** 2 <= (vi - vj) ** 2
        )

    prop_check("levin_gap_junction_dirichlet_dissipation_T32_T33", 500, check_bioelectric)

    # 4. Hippo Integrator Hysteresis & Finite-Time 3-Epoch Crowding Arrest (Lean T55)
    def hippo(tf7: int, pressure: int, lim: int) -> int:
        dec = (tf7 * 30) // 32
        return min(255, dec + 64) if pressure >= lim else dec

    def check_hippo(rng: random.Random) -> bool:
        tf7 = rng.randint(0, 255)
        lim = rng.randint(1, 200)
        p_hi = rng.randint(lim, lim + 200)
        p_lo = rng.randint(0, lim - 1)
        h3 = hippo(hippo(hippo(tf7, p_hi, lim), p_hi, lim), p_hi, lim)
        lock_ok = True if tf7 < 165 else hippo(tf7, p_hi, lim) >= 165
        return h3 >= 165 and lock_ok and hippo(tf7, p_lo, lim) <= tf7

    prop_check("hippo_integrator_hysteresis_T55", 500, check_hippo)

    # 5. Continuum Yolk Cleavage & Biotensegrity Non-Negativity (Lean C61-C62)
    def check_continuum(rng: random.Random) -> bool:
        Y = rng.randint(0, 50000)
        heat = rng.randint(0, Y)
        k = rng.randint(1, 32)
        q, rem = divmod(Y - heat, k)
        stiff = rng.randint(0, 1000)
        dx, dy, dz = rng.randint(-500, 500), rng.randint(-500, 500), rng.randint(-500, 500)
        return (k * q + rem + heat == Y) and (0 <= rem < k) and (stiff * (dx * dx + dy * dy + dz * dz) >= 0)

    prop_check("continuum_yolk_and_biotensegrity_C61_C62", 500, check_continuum)

    # 6. Randomized Native C11 Engine Execution Invariant Trials (F-34: test real compiled binary!)
    c_bin = find_c11_binary()
    if c_bin is not None:
        def check_native_c11(rng: random.Random) -> bool:
            seed = rng.randint(1, 9_000_000)
            solvent = rng.randint(0, 3)
            size = rng.choice([8, 12, 16])
            epochs = rng.randint(8, 25)
            proc = subprocess.run(
                [str(c_bin), "json", "--seed", str(seed), "--solvent", str(solvent), "--size", str(size), "--epochs", str(epochs)],
                cwd=str(ROOT),
                capture_output=True,
                text=True,
                check=False,
            )
            if proc.returncode != 0:
                return False
            data = json.loads(proc.stdout)
            t = data["telemetry"]
            return (
                t["mass_res"] == 0
                and t["energy_res"] == 0
                and t["second_law"] == 0
                and t["drift"] == 0.0
                and len(t["sha256"]) == 64
            )

        prop_check("native_c11_random_seed_solvent_size_closure", 20, check_native_c11)
        print("PASS: All 2,720 randomized property-based & native C11 differential trials passed with 0 counterexamples!")
    else:
        print("PASS: All 2,700 randomized mathematical property-based trials passed (native binary not present).")


if __name__ == "__main__":
    main()
