# Evidence Index (`EVIDENCE.md`)

Central directory linking every claim in [`CLAIMS.md`](CLAIMS.md) to its concrete machine-generated proof transcript, empirical log, or verification artifact in `CELLULOGONIC MECHA v1.5.0`.

## 1. Formal Verification Evidence (`lean/` & `results/`)
- [`results/lean4-audit.txt`](results/lean4-audit.txt) — Verbatim Lean 4.35.0 `#print axioms` output for all `70 / 70` audited theorems (`0 sorryAx`, `0 native_decide`, strictly `[propext, Classical.choice, Quot.sound]`).
- [`results/lean4-multi-kernel.txt`](results/lean4-multi-kernel.txt) — Independent local multi-kernel verification transcript (`lean`, `leanchecker --fresh`, `leanchecker-paranoid --fresh`, `con-leche --verified` 3,326 decls, `con-ron --verified` 3,326 decls, `nanoda_bin` 3,735 decls with `unpermitted_axiom_hard_error: true`).
- [`lean/comparator.json`](lean/comparator.json) & [`lean/formalization.yaml`](lean/formalization.yaml) — Palomar Challenge/Solution comparator contract and `v0.4` metadata.
- [`docs/statement-fidelity.md`](docs/statement-fidelity.md) — Row-by-row Lean 4 ↔ C11 fidelity and divergence audit.

## 2. Native C11 Falsification, Parity, Property & Empirical Evidence (`results/`)
- [`results/01_silicon_probe.txt`](results/01_silicon_probe.txt) — CPUID ISA probe (`Intel Core Ultra 9 275HX`, AVX2/BMI1/BMI2/POPCNT/RDSEED/RDTSCP).
- [`results/02_bench_win64.txt`](results/02_bench_win64.txt) & [`results/02_bench_linux.txt`](results/02_bench_linux.txt) — 4-way cross-OS / cross-ISA 0-bit SHA-256 parity (`681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff`, `51,488` steps, `1,222,070` reactions).
- [`results/03_protocell_abiogenesis.txt`](results/03_protocell_abiogenesis.txt) — Zero-seed spontaneous protocell abiogenesis (`0` -> `1,737` vesicles, peak `598` alive, peak `COVI 89.49`).
- [`results/04_endosymbiosis_eukaryogenesis.txt`](results/04_endosymbiosis_eukaryogenesis.txt) — Prokaryotic `1/R` ceiling vs. serial endosymbiosis (`7.63×` max volume `1305` vs `171`, `262` secondary chimeras, `119` EGT).
- [`results/05_xenocellular_transplants.txt`](results/05_xenocellular_transplants.txt) — De-novo 4-solvent adaptation (`90.8% / 66.7% / 94.5% / 87.1%`) & Terran→Venus transplant (`59.7%`).
- [`results/06_bioelectric_multicellular.txt`](results/06_bioelectric_multicellular.txt) — Levin bioelectric polarization & Track A vs. Track B anti-cancer policing (`0` vs. `105`).
- [`results/07_tardigrade_anhydrobiosis.txt`](results/07_tardigrade_anhydrobiosis.txt) — Tardigrade TDP bioglass vitrification (`232` vs. `2` post-shock survivors, `116×`).
- [`results/08_castration_ablation_13vec.txt`](results/08_castration_ablation_13vec.txt) & [`results/08_castration_ablation_13vec.csv`](results/08_castration_ablation_13vec.csv) — 13-vector restraint sweep & 50% Escape-Breeding Paradox.
- [`results/09_paired_stats_n24.txt`](results/09_paired_stats_n24.txt) — Locked `N = 24` paired permutation tests with 95% bootstrap CIs & Cohen's $d_z$ (`96/96` wins, `p_Holm = 0.00040`).
- [`results/10_ecosystem_4build_bridge.txt`](results/10_ecosystem_4build_bridge.txt) — 4-Build `AUTOPHAGE` 128B/256B -> Cellulogonic -> `Asmodensius` `[0,1]^112` bridge (`ABI 0x00010500`).
- [`results/11_falsification_win64.txt`](results/11_falsification_win64.txt) & [`results/11_falsification_linux.txt`](results/11_falsification_linux.txt) — 26/26 gates + 8/8 mutants caught on Win64 & Linux (plus `1,000/1,000` `fuzz_cmbin_abi` cases & static zero-heap hot-loop audit).
- [`results/12_embryogenesis_grn_3d.txt`](results/12_embryogenesis_grn_3d.txt) — 5-fate clonal 3D embryogenesis (`SHA 9526ca6a...bc3bbb89`).

## 3. Manuscript, Citation & Checksum Integrity
- [`dist/Donahue_2026_cellulogonic_mecha.pdf`](dist/Donahue_2026_cellulogonic_mecha.pdf) & [`dist/arxiv_submission.tar.gz`](dist/arxiv_submission.tar.gz) — Frozen AMS-LaTeX preprint (`0` Overfull `\hbox`, `0` undefined refs, clean-room verified).
- [`paper/refs.bib`](paper/refs.bib) — `36 / 36` primary academic DOIs verified live via `scripts/check_citations.py`.
- [`CHECKSUMS.sha256`](CHECKSUMS.sha256) & [`data/MANIFEST.sha256`](data/MANIFEST.sha256) — Cryptographic SHA-256 manifests for frozen outputs and empirical data logs.
