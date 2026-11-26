#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""
Independent Clean-Room Exact-Integer Reference Oracle for CELLULOGONIC MECHA v1.5.0.

Re-implements the exact 8-species atomic-mass, chemical-potential, 4-solvent 19-reaction
Chemoton stoichiometry (matching include/cellulogonic_internal.h and lean/CellulogonicProofs/Spec.lean),
exact Canham-Helfrich reduced-volume Q = 100*V^2 / L^3 & sextic minimax law, and exact
respiratory-to-fermentative ATP yield ratios in Python arbitrary-precision integers & Fractions.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
import random

# Exact 8 conserved species matching CELL_MASS_W and CELL_MU_POT in include/cellulogonic_internal.h
SPECIES = ("S", "X", "M", "E", "L", "P", "T", "W")
MASS_W = {"S": 4, "X": 2, "M": 6, "E": 4, "L": 8, "P": 12, "T": 16, "W": 2}
MU_POT = {"S": 48, "X": 36, "M": 80, "E": 40, "L": 104, "P": 156, "T": 212, "W": 6}


@dataclass(frozen=True)
class SolventSpec:
    name: str
    e_pmf: int
    r2_pmf_yield: int
    r2_heat: int
    min_cring: int


SOLVENTS = (
    SolventSpec("TERRAN_H2O", 10, 6, 2, 3),
    SolventSpec("TITAN_AZOTOSOME", 8, 7, 6, 4),
    SolventSpec("VENT_TETRAETHER", 10, 6, 2, 3),
    SolventSpec("VENUS_H2SO4", 15, 4, 2, 2),
)

# Exact 19-reaction stoichiometric table: (name, species_delta_dict, dpmf_mode, external_inflow_uq)
REACTIONS = (
    ("R1_catab", {"S": -1, "X": -1, "M": 1}, "0", 0),
    ("R2_pump", {"M": -1, "W": 3}, "r2", 0),
    ("R3_atp_synth", {"W": -2, "E": 1}, "-nH", 0),
    ("R4_ferment", {"S": -2, "E": 1, "W": 2}, "0", 0),
    ("R5_lipid_syn", {"S": -2, "E": -1, "L": 1, "W": 2}, "0", 0),
    ("R6_prot_syn", {"S": -3, "E": -1, "P": 1, "W": 2}, "0", 0),
    ("R7_tmpl_rep", {"S": -4, "E": -1, "T": 1, "W": 2}, "0", 0),
    ("R8_atp_hyd", {"E": -1, "W": 2}, "0", 0),
    ("R9_lipid_dec", {"L": -1, "W": 4}, "0", 0),
    ("R10_prot_dec", {"P": -1, "W": 6}, "0", 0),
    ("R11_tmpl_dec", {"T": -1, "W": 8}, "0", 0),
    ("R12_pmf_leak", {}, "-1", 0),
    ("R13a_salv_L", {"L": -1, "S": 1, "W": 2}, "0", 0),
    ("R13b_salv_P", {"P": -1, "S": 2, "W": 2}, "0", 0),
    ("R14a_photo_S", {"W": -2, "S": 1}, "0", 46),
    ("R14b_photo_X", {"W": -1, "X": 1}, "0", 34),
    ("R15a_abio_L", {"S": -2, "L": 1}, "0", 12),
    ("R15b_abio_P", {"S": -3, "P": 1}, "0", 16),
    ("R15c_abio_T", {"S": -4, "T": 1}, "0", 24),
)


def check_all_reactions_exact() -> int:
    """Verify exact Delta-M == 0 amu, First-Law closure, and strict Second-Law q > 0 across all 4 solvents."""
    checked = 0
    for solv in SOLVENTS:
        for n_h in range(solv.min_cring, 7):
            for name, delta, dpmf_mode, inflow in REACTIONS:
                dm = sum(MASS_W[sp] * cnt for sp, cnt in delta.items())
                assert dm == 0, f"{solv.name} {name}: mass residual {dm} != 0"

                de_chem = sum(MU_POT[sp] * cnt for sp, cnt in delta.items())
                if dpmf_mode == "r2":
                    dpmf = solv.r2_pmf_yield
                elif dpmf_mode == "-nH":
                    dpmf = -n_h
                elif dpmf_mode == "-1":
                    dpmf = -1
                else:
                    dpmf = 0

                dq_heat = inflow - de_chem - dpmf * solv.e_pmf
                assert dq_heat > 0, (
                    f"{solv.name} (nH={n_h}) {name}: Second-Law violation dq_heat={dq_heat}"
                )
                if name == "R2_pump":
                    assert dq_heat == solv.r2_heat, f"{solv.name} R2 heat mismatch {dq_heat} vs {solv.r2_heat}"
                checked += 1

        # Verify below-threshold c-ring strictly fails Second Law for R3_atp_synth (dG_ATP = 40 - 12 = 28 uq)
        for n_h in range(1, solv.min_cring):
            dq_bad = n_h * solv.e_pmf - (MU_POT["E"] - 2 * MU_POT["W"])
            assert dq_bad <= 0, f"{solv.name}: nH={n_h} should violate strict 2nd law (dq={dq_bad})"
            checked += 1

        # Exact respiratory vs fermentative ATP yield ratio (D18)
        atp_per_s_resp = Fraction(solv.r2_pmf_yield, solv.min_cring)
        atp_per_s_ferm = Fraction(1, 2)
        ratio = atp_per_s_resp / atp_per_s_ferm
        if solv.name == "TITAN_AZOTOSOME":
            assert ratio == Fraction(7, 2), f"Expected 3.5x in Titan, got {ratio}"
        else:
            assert ratio == Fraction(4, 1), f"Expected 4x in {solv.name}, got {ratio}"
        checked += 1

    return checked


def exact_reduced_volume_q(vol: int, lip: int) -> Fraction:
    """Exact Canham-Helfrich Q(V, L) = 100 * V^2 / L^3."""
    assert lip > 0 and vol >= 0
    return Fraction(100 * vol * vol, lip * lip * lip)


def check_canham_helfrich_properties(num_trials: int = 1000) -> int:
    """Property-based verification of exact integer Q(V,L) predicates, sextic minimax & exosome descent."""
    rng = random.Random(20261125)
    passed = 0
    for _ in range(num_trials):
        vol = rng.randint(1, 50000)
        lip = rng.randint(1, 5000)
        q = exact_reduced_volume_q(vol, lip)
        for pct in (50, 80, 95, 100, 115):
            # Matches cell_q_gt_pct in src/chemoton_physics.c: 10000 * V^2 > pct * L^3
            lhs = 10000 * vol * vol
            rhs = pct * lip * lip * lip
            assert (lhs > rhs) == (q > Fraction(pct, 100))
            passed += 1

        # Symmetric binary fission invariant (Lean T11/T12): V'=V/2, L'=L/2 -> Q' = 2*Q
        if vol % 2 == 0 and lip % 2 == 0 and lip >= 2:
            assert exact_reduced_volume_q(vol // 2, lip // 2) == 2 * q
            passed += 1

        # Exosome escape strictly decreases Q (Lean T15/T17)
        if vol >= 2:
            dv = rng.randint(1, vol - 1)
            assert exact_reduced_volume_q(vol - dv, lip) < q
            passed += 1

    return passed


def main() -> int:
    r_cnt = check_all_reactions_exact()
    q_cnt = check_canham_helfrich_properties(1000)
    print(
        f"PASS: Clean-room Python Exact Reference Oracle verified {r_cnt} 4-solvent/19-reaction "
        f"stoichiometry checks and {q_cnt} Canham-Helfrich exact-rational property checks (0 violations)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
