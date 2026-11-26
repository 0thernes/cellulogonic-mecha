# CELLULOGONIC MECHA v1.5.0

Clean-room 3D off-lattice developmental embryogenesis, morphogenetic GRN, chemiosmotic, cytoskeletal, endosymbiotic, and xenocellular artificial life engine in zero-dependency C11 + dual AVX2/Scalar kernels, paired with 70 six-kernel Pure Lean 4.35.0 formal proofs and a 60 FPS WebGL2 3D Observatory.

[![Version](https://img.shields.io/badge/version-1.5.0-0284C7)](VERSION)
[![Status](https://img.shields.io/badge/status-active-059669)](ROADMAP.md)
[![Code License](https://img.shields.io/badge/code-Apache--2.0-blue)](LICENSE)
[![Docs License](https://img.shields.io/badge/docs%20%26%20paper-CC--BY--4.0-blueviolet)](LICENSE-DOCS)
[![Lean 4.35.0](https://img.shields.io/badge/Lean%204.35.0-70%2F70%20theorems%20%C2%B7%206--kernel-7C3AED)](results/lean4-multi-kernel.txt)
[![Falsification](https://img.shields.io/badge/falsification-26%2F26%20gates%20%C2%B7%208%2F8%20mutants-059669)](results/11_falsification_win64.txt)
[![Mega-Atlas](https://img.shields.io/badge/Mega--Atlas%20retired-0%2F1%2C900-475569)](docs/MEGA_ATLAS_CROSSCHECK.md)

`status: active` · **Author:** Alexander Stephen Donahue (Independent Researcher, Spokane, WA, USA · ORCID [`0009-0006-8168-9648`](https://orcid.org/0009-0006-8168-9648)) · **Independent research preprint — not peer-reviewed.**

![CELLULOGONIC MECHA v1.5.0 Social Preview](docs/media/social-preview.svg)

---

## Position in the 4-Build Artificial Life Ecosystem

| Build | Repository / Pillar | Biological Layer | Bridge Interface |
|---|---|---|---|
| **Build 1** | `AUTOPHAGE` / `Zeta-Omega ALife` | **DNA & Genetic Engine** (128B/256B polymorphic x86-64 Galois-field genomes, $\mathbb{F}_2$ ribosomes, polynomial sex) | `cellulogonic_ingest_autophage_128b()` & `cellulogonic_ingest_autophage_genome()` smoothly decode 128B/256B DNA into cellular operons with Lipschitz locality |
| **Build 2** | **`CELLULOGONIC MECHA` (this repo)** | **CELLULAR Layer** (Chemoton stoichiometry, Canham–Helfrich bilayers, Mitchell chemiosmosis, nested endosymbiosis, Levin bioelectric tissues, Tardigrade TDP bioglass, 4 xenocellular solvents, 3D embryogenesis GRNs) | Flat C11 ABI (`include/cellulogonic_abi.h`, `ABI 0x00010500`), Win64 DLL / `.a` & Linux `.so` / `.a` |
| **Build 3** | `Asmodensius` | **Creatures, Beings & Entities** (embodied sensorimotor organisms & eutelic organ budgets) | `cellulogonic_project_asmodensius_112()` projects cellular, cytoskeletal, GHz microtubule & morphogenetic phenotypes strictly into `[0, 1]^112` |
| **Build 4** | `Asmodensius-Native` | **World Model** (thermodynamic/spatial world substrate & Clean-Slate `P1`–`P10` verification invariants) | Exact `P2` (First/Second Law), `P4` (`Philox4x32-10` + `"CMB5"` `.cmbin` + SHA-256), `P9` (Observer Independence), `P10` (`0` post-warmup allocs) |

---

## Verified Proof & Benchmark Highlights (`v1.5.0`, Intel Core Ultra 9 275HX)

| Verification Gate | Verified Result | Primary Evidence |
|---|---|---|
| **Lean 4.35.0 Six-Kernel Palomar Suite** | **70 / 70 Audited Theorems** (`62` core `T01`–`T58`, `C59`–`C62` + `8` `NonVacuity` witnesses `NV01`–`NV08`; `0 sorry` in `Solution`, `0` custom axioms, strictly `[propext, Classical.choice, Quot.sound]`; local 5-kernel PASS + 6/6 CI) | [`results/lean4-audit.txt`](results/lean4-audit.txt) · [`results/lean4-multi-kernel.txt`](results/lean4-multi-kernel.txt) |
| **Win64 PE64 Falsification (`clang 23.1.0`, `-Werror`)** | **26 / 26 PASSED · 8 / 8 Planted Mutants Caught (`0.866s`) · 1,000/1,000 Fuzz Cases PASS** | [`results/11_falsification_win64.txt`](results/11_falsification_win64.txt) |
| **Linux ELF64 Falsification (`gcc 13.3.0`)** | **26 / 26 PASSED · 8 / 8 Planted Mutants Caught (`0.902s`)** | [`results/11_falsification_linux.txt`](results/11_falsification_linux.txt) |
| **Randomized Property Suite (`B10`)** | **2,700 / 2,700 `prop_check` trials PASSED (`0` counterexamples)** | [`scripts/prop_check.py`](scripts/prop_check.py) |
| **4-Way Cross-OS / Cross-Kernel SHA-256 Parity** | **0-Bit Diff** (`681a9747…519eceff`) · `22.64 M rxns/s` Win64 / `15.75 M rxns/s` Linux (`1,222,070` rxns) | [`docs/benchmarks.md`](docs/benchmarks.md) |
| **Exact Mass / 1st-Law / 2nd-Law Closure (`P2`)** | **`0 amu`, `0 uq`, `0.00e+00` drift, `0` violations across all 4 solvents, `0` post-warmup allocs** | Every run in [`results/`](results/) |
| **3D Embryogenesis (8 Identical-DNA Blastomeres)** | **`50` alive (`maxT 47`), ALL 5 fates (`Gm=5`), `80.0%` peak sort, `328` wired axons, `725` CPG pulses, exact Z-invariance** | [`results/12_embryogenesis_grn_3d.txt`](results/12_embryogenesis_grn_3d.txt) |
| **Locked Paired Stats (`N = 24`, Seeds `5000..5023`)** | **4 / 4 Pre-Registered Hypotheses UNANIMOUS `24/24` WINS (`96/96` total; $p_{\text{Holm}} = 0.00040$; $d_z \in [1.47, 4.21]$)** | [`results/09_paired_stats_n24.txt`](results/09_paired_stats_n24.txt) |
| **AMS-LaTeX White Paper & Live DOI Audit** | **Zero Overfull `\hbox`, Zero undefined refs, clean-room `arxiv_submission.tar.gz` PASS, `36/36` DOIs verified live** | [`dist/Donahue_2026_cellulogonic_mecha.pdf`](dist/Donahue_2026_cellulogonic_mecha.pdf) |

---

## Quickstart

### 1. Build & Verify Everything on Windows x64 (`clang 23.1.0` + LLD + 26 Gates + 70 Pure Lean 4.35.0 Proofs)
```powershell
.\build.ps1
```

### 2. Build & Verify on Linux / WSL2 ELF64 (`gcc 13.3.0` + 26 Gates + Lean 4.35.0 Audit)
```bash
chmod +x build_linux.sh
./build_linux.sh
```

### 3. Run Python Oracle, Property Checks, Web Server Security Suite, Axiom Audit, Citations & Paper Build
```bash
python3 scripts/verify_oracle_and_stats.py
python3 scripts/prop_check.py
python3 scripts/check_versions.py
node test/test_web_server.mjs
python3 scripts/check_readme_axioms.py
python3 scripts/check_citations.py
python3 scripts/build_paper.py
```

---

## Usage

### Standalone Clean-Room 3D Developmental & Cellular Lab Experiments
```powershell
.\bin\cellulogonic.exe embryo --epochs 200 --size 32 --seed 5000
.\bin\cellulogonic.exe probe
.\bin\cellulogonic.exe bench --epochs 300 --size 32 --seed 5000
.\bin\cellulogonic.exe abiogenesis --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe endosymbiosis --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe xenocell --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe bioelectric --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe anhydrobiosis --epochs 120 --size 32 --seed 5000
.\bin\cellulogonic.exe ablation --epochs 180 --size 32 --seed 5000 --out results\08_castration_ablation_13vec.csv
.\bin\cellulogonic.exe stats --pairs 24 --epochs 180 --size 32
.\bin\cellulogonic.exe bridge --seed 5000
```

### Launch the CSRF-Hardened 60 FPS WebGL2 3D Embryogenesis & Cellular Microscopy Observatory
```powershell
node web\server.mjs
# Open http://127.0.0.1:8788 (all mutations via POST /api/cmd with Content-Type: application/json)
```

### Direct Drop-In C11 ABI (`include/cellulogonic_abi.h`, `ABI 0x00010500`)
```c
#include "cellulogonic_abi.h"

CellWorldConfig cfg;
cellulogonic_default_config(&cfg);
cfg.regime = CELL_REGIME_ABIOGENESIS_ZERO_SEED;
cfg.seed   = 5000ULL;

CellWorld* world = cellulogonic_create(&cfg);
cellulogonic_step_epochs(world, 240);

uint8_t dna128[128] = {0};
cellulogonic_ingest_autophage_128b(world, dna128, 0);

CellSpecimen apex;
cellulogonic_inspect_apex(world, &apex);

float asmodensius_traits[112];
cellulogonic_project_asmodensius_112(&apex, asmodensius_traits);

char sha256_hex[65];
cellulogonic_state_sha256(world, NULL, sha256_hex);
cellulogonic_destroy(world);
```

---

## Verbatim Lean 4.35.0 `#print axioms` Audit (`70 / 70` Theorems)

Synchronized automatically against [`results/lean4-audit.txt`](results/lean4-audit.txt) via `python3 scripts/check_readme_axioms.py`. Every theorem depends strictly on a subset of `[propext, Classical.choice, Quot.sound]` (`0 sorryAx`, `0 native_decide`, `0` custom axioms):

<!-- LEAN_AXIOMS_START -->
```text
'Cellulogonic.StoichiometryThermo.T01_all_reactions_conserve_mass' depends on axioms: [propext]
'Cellulogonic.StoichiometryThermo.T02_all_reactions_first_law_exact' depends on axioms: [propext, Quot.sound]
'Cellulogonic.StoichiometryThermo.T03_all_reactions_strict_second_law' depends on axioms: [propext, Quot.sound]
'Cellulogonic.StoichiometryThermo.T04_endergonic_biosynthesis_requires_atp' does not depend on any axioms
'Cellulogonic.StoichiometryThermo.T05_respiration_quadruples_fermentation_yield' depends on axioms: [propext]
'Cellulogonic.StoichiometryThermo.T06_closed_box_cycle_dissipation_strict' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.StoichiometryThermo.T07_step_preserves_total_mass' depends on axioms: [propext, Quot.sound]
'Cellulogonic.StoichiometryThermo.T08_step_preserves_first_law_ledger' depends on axioms: [propext, Quot.sound]
'Cellulogonic.StoichiometryThermo.T09_heat_sink_monotone' depends on axioms: [propext, Quot.sound]
'Cellulogonic.StoichiometryThermo.T10_fission_and_engulfment_exact_conservation' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.MembraneGeometry.T11_symmetric_fission_doubles_Q' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.MembraneGeometry.T12_symmetric_fission_iff_half_threshold' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.MembraneGeometry.T13_high_Q_blocks_symmetric_fission' does not depend on any axioms
'Cellulogonic.MembraneGeometry.T14_equal_ratio_asymmetric_split_suboptimal' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.MembraneGeometry.T15_exosome_asymmetric_budding_escapes_half_barrier' does not depend on any axioms
'Cellulogonic.MembraneGeometry.T16_optimal_area_partition_strictly_improves_budding' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.MembraneGeometry.T17_osmotic_lysis_monotone_in_volume' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MembraneGeometry.T18_lipid_synthesis_relieves_membrane_stress' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MembraneGeometry.T19_double_membrane_vesicle_preservation' depends on axioms: [propext]
'Cellulogonic.BioenergeticsEndosymbiosis.T20_prokaryotic_bioenergetic_ceiling' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T21_prokaryotic_power_density_strictly_decreasing' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T22_endosymbiotic_scaling_liberation' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T23_endosymbiont_surface_amplification' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T24_secondary_endosymbiosis_additive_power' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T25_mitotic_heteroplasmy_conservation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T26_asymmetric_bottleneck_purifying_selection' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.BioenergeticsEndosymbiosis.T27_egt_reduces_organelle_maintenance' depends on axioms: [propext, Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T28_cring_terran_threshold' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T29_cring_acidophile_shift' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T30_cring_cryogenic_shift' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T31_gap_junction_pairwise_dissipation_nonneg' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T32_gap_junction_laplacian_energy_decrease' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T33_gap_junction_homogenization_bound' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.ChemiosmosisAutopoiesis.T34_inert_crystal_covi_zero' depends on axioms: [propext]
'Cellulogonic.ChemiosmosisAutopoiesis.T35_white_noise_entropy_gate_zero' depends on axioms: [propext]
'Cellulogonic.ChemiosmosisAutopoiesis.T36_markov_blanket_boundary_separation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T37_3d_tube_geometry_lumen_lower_bound' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T38_minimum_viable_organ_budget_bound' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T39_syncytial_compression_factor' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T40_hydrostatic_skeleton_incompressibility' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T41_eutely_hypertrophy_cancels_hyperplasia' does not depend on any axioms
'Cellulogonic.EutelicOrganogenesis.T42_anhydrobiosis_bioglass_vitrification' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T43_flames_excretory_osmoregulation_balance' depends on axioms: [propext, Quot.sound]
'Cellulogonic.EutelicOrganogenesis.T44_dna_to_organ_hierarchical_conservation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T45_stokes_overdamped_dissipation_nonneg' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T46_soft_body_pairwise_momentum_conservation' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T47_cytokinetic_prolate_elongation_area_excess' depends on axioms: [propext,
 Classical.choice,
 Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T48_frohlich_mt_coherence_pumping_threshold' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T49_mt_gigahertz_standing_wave_frequency_scaling' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.CytoskeletonNanomechanics.T50_actomyosin_pulsatile_work_bounded_by_atp' depends on axioms: [propext,
 Quot.sound]
'Cellulogonic.MorphogeneticGRN.T51_wolpert_french_flag_partition' depends on axioms: [propext]
'Cellulogonic.MorphogeneticGRN.T52_steinberg_differential_adhesion_sorting' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MorphogeneticGRN.T53_apical_constriction_wedge_curvature' depends on axioms: [propext]
'Cellulogonic.MorphogeneticGRN.T54_eutelic_lineage_countdown_bound' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MorphogeneticGRN.T55_hippo_contact_inhibition_growth_stop' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MorphogeneticGRN.T56_hebbian_axon_pruning_and_potentiation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MorphogeneticGRN.T57_cpg_pacemaker_sensorimotor_bound' depends on axioms: [propext, Quot.sound]
'Cellulogonic.MorphogeneticGRN.T58_morphogenetic_signaling_zero_drift' depends on axioms: [propext, Quot.sound]
'Cellulogonic.ContinuumBiomechanics.C59_xpbd_dissipation_monotone_nonneg' depends on axioms: [propext, Quot.sound]
'Cellulogonic.ContinuumBiomechanics.C60_carcass_detritus_100pct_conservation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.ContinuumBiomechanics.C61_yolk_cleavage_mass_conservation' depends on axioms: [propext, Quot.sound]
'Cellulogonic.ContinuumBiomechanics.C62_biotensegrity_elastic_nonneg' depends on axioms: [propext, Quot.sound]
'Cellulogonic.NonVacuity.NV01_thermo_titan_boundary_witness' depends on axioms: [propext]
'Cellulogonic.NonVacuity.NV02_membrane_sextic_nonvacuous_witness' does not depend on any axioms
'Cellulogonic.NonVacuity.NV03_endosymbiosis_liberation_nonvacuous_witness' does not depend on any axioms
'Cellulogonic.NonVacuity.NV04_chemiosmosis_bioelectric_nonvacuous_witness' does not depend on any axioms
'Cellulogonic.NonVacuity.NV05_eutelic_bioglass_nonvacuous_witness' does not depend on any axioms
'Cellulogonic.NonVacuity.NV06_cytoskeleton_frohlich_nonvacuous_witness' does not depend on any axioms
'Cellulogonic.NonVacuity.NV07_embryogenesis_trace_nonvacuous_witness' depends on axioms: [propext]
'Cellulogonic.NonVacuity.NV08_continuum_biomechanics_nonvacuous_witness' does not depend on any axioms
```
<!-- LEAN_AXIOMS_END -->

---

## Honest Verification Boundaries & Mega-Atlas Non-Retirement

- **Formal vs. Runtime Boundary:** Lean 4.35.0 verifies the 70 discrete mathematical, thermodynamic, geometric, bioenergetic, developmental, and continuum invariants inside `lean/` (`docs/statement-fidelity.md`). Runtime C11 memory safety, AVX2/scalar parity, `"CMB5"` `.cmbin` validation, stochastic emergence, web bridge security, and `N = 24` statistical tests are verified outside the Lean kernel by `test/test_falsification.c` (26 gates + 8 mutants), `test/fuzz_cmbin_abi.c` (dual-mode standalone + `libFuzzer`), `test/test_web_server.mjs` (12 tests), `reference/chemoton_oracle.py`, `scripts/prop_check.py`, and cross-platform CI.
- **Unsolved Problems Mega-Atlas (`0 / 1,900` Retired):** As documented in [`docs/MEGA_ATLAS_CROSSCHECK.md`](docs/MEGA_ATLAS_CROSSCHECK.md), **`0 / 1,900`** overarching open problems in `0thernes/unsolved-problems-mega-atlas` are claimed solved or retired. Scoped invariants and computational probes touch `BIO-001`, `BIO-015`, `BIO-022`, `SUB-001`, `SUB-047`, `SYS-009`, and `XSPEC-009`, while every parent problem remains **OPEN**.
- **Limitations & Real-vs-Metaphor:** See [`LIMITATIONS.md`](LIMITATIONS.md) and [`CLAIMS.md`](CLAIMS.md) (including explicit "What Would Prove Us Wrong" falsifiers).

---

## Documentation, Manuscripts & Governance

- **Compiled AMS-LaTeX White Paper (`v1.5.0`, 36 Live-Verified DOIs):** [`dist/Donahue_2026_cellulogonic_mecha.pdf`](dist/Donahue_2026_cellulogonic_mecha.pdf) · [`paper/main.tex`](paper/main.tex) · [`paper/refs.bib`](paper/refs.bib) · [`dist/arxiv_submission.tar.gz`](dist/arxiv_submission.tar.gz)
- **Epistemic Ledgers:** [`CLAIMS.md`](CLAIMS.md) · [`LIMITATIONS.md`](LIMITATIONS.md) · [`REPLICATION.md`](REPLICATION.md) · [`EVIDENCE.md`](EVIDENCE.md) · [`HARDENING_LOG.md`](HARDENING_LOG.md) · [`DEVIATIONS.md`](DEVIATIONS.md) · [`ERRATA.md`](ERRATA.md)
- **Audit & Reference Docs:** [`docs/statement-fidelity.md`](docs/statement-fidelity.md) · [`docs/MEGA_ATLAS_CROSSCHECK.md`](docs/MEGA_ATLAS_CROSSCHECK.md) · [`docs/AI_DISCLOSURE.md`](docs/AI_DISCLOSURE.md) · [`docs/benchmarks.md`](docs/benchmarks.md) · [`docs/preregistration.md`](docs/preregistration.md) · [`docs/reviews/2026-11-25-10-pass-red-team-audit.md`](docs/reviews/2026-11-25-10-pass-red-team-audit.md)
- **Deep-Dive & Architecture:** [`INDEX.md`](INDEX.md) · [`SUMMARY.md`](SUMMARY.md) · [`ARCHITECTURE.md`](ARCHITECTURE.md) · [`VERIFICATION.md`](VERIFICATION.md) · [`docs/MASTER_REPORT.md`](docs/MASTER_REPORT.md)

---

## Dual Open License

Copyright © 2026 **Alexander Stephen Donahue** (`Independent Researcher, Spokane, WA, USA` · `0_0@0thernes.art` · ORCID [`0009-0006-8168-9648`](https://orcid.org/0009-0006-8168-9648)).
- **Code, C11 engine, Lean 4.35.0 formal proofs, scripts, and CI:** Licensed under the **Apache License, Version 2.0** ([`LICENSE`](LICENSE)).
- **Research papers, PDFs, documentation, figures, and empirical result logs:** Licensed under **Creative Commons Attribution 4.0 International (CC BY 4.0)** ([`LICENSE-DOCS`](LICENSE-DOCS)).
