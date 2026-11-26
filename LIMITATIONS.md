# Limitations, Threats to Validity & Real-vs-Metaphor Boundary (`LIMITATIONS.md`)

This document states clearly what **CELLULOGONIC MECHA `v1.5.0`** does **not** prove, where its abstractions diverge from physical wet-lab biology, and how its terminology maps to literal computation (Build Standard `B41`).

## 1. Literal-vs-Metaphor Boundary Table

| Term Used in Code / Paper | Literal Computational Reality Inside `cellulogonic-mecha` | What It Is **NOT** |
|---|---|---|
| **Chemoton (`S, X, M, E, L, P, T, W`)** | Exact `int64_t` counts of 8 coarse-grained molecular classes obeying 19 integer stoichiometric reactions (`R1`–`R15c`) with `0 amu` / `0 uq` residual and `q_r > 0`. | Not atomistic molecular dynamics, quantum chemistry, or wet-lab reaction kinetics. |
| **Canham–Helfrich Bilayer (`Q = 100 V^2 / L^3`)** | Cleared-denominator integer squared reduced-volume inequality governing symmetric fission, asymmetric budding, and osmotic lysis. | Not a continuous Helfrich curvature-elasticity PDE mesh solver. |
| **Endosymbiosis / Organelles (`org[8]`)** | Up to 8 nested internal `CellOrganelle` structs carrying independent integer chemistry, PMF, and miniature operon copies. | Not a full genomic reconstruction of mitochondrial/chloroplast endosymbiosis. |
| **Levin Bioelectricity (`V_m`)** | Integer/fixed-range compartment transmembrane potential `[-90, +40] mV` coupled across cadherin neighbors by conservative discrete gap-junction relaxation. | Not a Hodgkin–Huxley ion-channel electrophysiology model or proof of biological anatomical memory. |
| **Fröhlich–Pokorný GHz Microtubules** | Classical deterministic integer relaxation ODE (`frohlichStep`, Lean `T48`) gated on metabolic ATP pumping (`sPump > sCrit`) with inverse-length frequency scaling (`T49`). | **Strictly NOT a quantum-consciousness, Orch-OR, or Bose–Einstein condensate claim.** |
| **3D Embryogenesis (`Wnt/BMP/Shh`, GRN, Axons, CPG)** | Local 3D paracrine neighbor diffusion fields, 8-channel `uint8_t` TF state, 4 cadherin classes, 2 weighted axon pointers/cell, Hippo TF7 leaky integrator, and ATP-billed (`R8`) phase oscillators. | Not an experimental vertebrate/invertebrate embryology model or proof of real organismal consciousness/sentience. |
| **Tardigrade TDP/CAHS Bioglass** | Locus-gated vitrification flag that suppresses desiccation/shock decay by `D_glass >= 10` while conserving mass and dissipating organelle PMF to heat. | Not atomistic modeling of intrinsically disordered protein vitrification kinetics. |
| **4 Planetary Solvents (`Terran`, `Titan`, `Vent`, `Venus`)** | Four parameterized rows in `SOLVENT_TABLE` varying `e_pmf`, `r2_pmf_yield`, and lipid-class stability while enforcing strict `q_r > 0`. | Not wet-lab proof of habitability in Titan's hydrocarbon lakes or Venus's sulfuric-acid clouds. |

## 2. Threats to Validity

1. **Formal-vs-C11 Implementation Boundary:** Lean 4.35.0 (`lean/`) proves 70 exact mathematical theorems over the discrete specification (`Spec.lean`), **not** a machine-checked compiler verification of `src/*.c`. Runtime C11 safety and bit-exactness rest on `-Wall -Wextra -Wshadow -Wstrict-prototypes -Wconversion -Werror`, static zero-heap hot-loop symbol audits, ASan/UBSan sanitizers in CI, `test/fuzz_cmbin_abi.c` (1,000 cases) + `test/fuzz_cmbin.c`, 26/26 falsification gates, 8/8 caught mutants, and 2,700 property checks.
2. **Spatial & Population Scale Ceiling:** The canonical benchmark chamber is `32×32×32 μm` (`1024` spatial columns, up to `4096` cell slots). Scaling to macroscopic tissues (>100,000 cells) or geological epochs is outside the current single-workstation scope.
3. **Observed Empirical Tradeoffs:**
   - Endosymbiosis concentrates biomass into larger megacells (`6.66×` max volume), reducing headcount (`43` vs. `542`) and single-seed final `COVI` (`77.32` vs. `80.00`) relative to high-headcount prokaryotic controls.
   - Titan cryogenic `n_H >= 4` gearing lowers carrying capacity (`120` vs. `313–358`) and adaptation percentage (`73.3%` vs. `89.9%–97.5%`).
   - Host CPU frequency and background load vary wall-clock throughput (`19.56`–`33.50 M rxns/s`), though deterministic SHA-256 digests remain 0-bit invariant.
4. **Peer Review & Mega-Atlas Status:** This work is an independent research preprint (not peer-reviewed) and retires **`0 / 1,900`** overarching open problems in `0thernes/unsolved-problems-mega-atlas` ([`docs/MEGA_ATLAS_CROSSCHECK.md`](docs/MEGA_ATLAS_CROSSCHECK.md)).
