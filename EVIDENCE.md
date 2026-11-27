<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Evidence Index (`EVIDENCE.md`)

Central directory linking every claim in [`CLAIMS.md`](CLAIMS.md) to its concrete machine-generated proof transcript, empirical log, or verification artifact in `CELLULOGONIC MECHA v1.7.0`.

## 1. Formal Verification Evidence (`lean/` & `results/`)
- [`results/lean4-audit.txt`](results/lean4-audit.txt) — Verbatim Lean 4.35.0 `#print axioms` output for all **`78 / 78`** audited theorems about `Spec.lean` in **10 proof modules + `Spec.lean` (15 `.lean` files)** (`0 sorryAx` in `Solution`, `0 native_decide`, strictly `[propext, Classical.choice, Quot.sound]`; 5 annotated bookkeeping helpers `T25, T27, T40, T41, T44`).
- [`results/lean4-multi-kernel.txt`](results/lean4-multi-kernel.txt) — 6-kernel-check CI verification transcript (Lean default & paranoid modes + 4 independent checkers: `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`, plus `lake comparator --paranoid`).
- [`lean/comparator.json`](lean/comparator.json) & [`lean/formalization.yaml`](lean/formalization.yaml) — Palomar Challenge/Solution comparator contract (`78` theorems + `12` pinned `Spec` definitions) and `v0.4` schema-validated metadata.
- [`scripts/test_lean_negative.py`](scripts/test_lean_negative.py) — Live negative Lean 4.35.0 elaboration & `sorry`-tamper rejection test (`6/6`: false claims, live `Spec` tamper, `sorry`, custom `axiom`, `native_decide` all actively caught).
- [`docs/statement-fidelity.md`](docs/statement-fidelity.md) — Row-by-row Lean 4 (`Spec.lean`) ↔ exact C11 symbol fidelity, bookkeeping-helper annotation, and divergence audit.

## 2. Native C11 Falsification, Parity, Property & Empirical Evidence (`results/`)
- [`results/01_silicon_probe.txt`](results/01_silicon_probe.txt) — CPUID ISA probe (`Intel Core Ultra 9 275HX`, AVX2/BMI1/BMI2/POPCNT/RDSEED/RDTSCP; archived from identical `v1.6.0` physics kernel, re-verified under `v1.7.0` `ABI 0x00010700`).
- [`results/02_bench_win64.txt`](results/02_bench_win64.txt) & [`results/02_bench_linux.txt`](results/02_bench_linux.txt) — Cross-OS / cross-ISA 0-bit SHA-256 parity (`83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe`, `50,243` steps, `1,224,310` reactions).
- [`results/03_protocell_abiogenesis.txt`](results/03_protocell_abiogenesis.txt) — Zero-seed spontaneous protocell abiogenesis (`0` -> `1,707` vesicles, peak `603` alive, `107` orgs, peak `COVI 88.92`).
- [`results/04_endosymbiosis_eukaryogenesis.txt`](results/04_endosymbiosis_eukaryogenesis.txt) — Prokaryotic `1/R` ceiling vs. serial endosymbiosis (`5.64×` max volume `1122` vs `199`, `249` secondary chimeras, `121` EGT).
- [`results/05_xenocellular_transplants.txt`](results/05_xenocellular_transplants.txt) — De-novo 4-solvent adaptation (`84.2% / 77.0% / 87.1% / 77.5%`, Titan `c-ring 4.17 >= 4`) & Terran→Venus sweep (`12.4%` -> `31.6%`).
- [`results/06_bioelectric_multicellular.txt`](results/06_bioelectric_multicellular.txt) — Levin bioelectric polarization (`15.60 / 16.67 mV` vs. `0.00 mV` control) & Track A vs. Track B anti-cancer policing (`0` vs. `62`).
- [`results/07_tardigrade_anhydrobiosis.txt`](results/07_tardigrade_anhydrobiosis.txt) — Tardigrade TDP bioglass tun (`93` vs. `1` post-shock survivors, `93×`, `92` revived -> `332` rebounded).
- [`results/08_castration_ablation_13vec.txt`](results/08_castration_ablation_13vec.txt) & [`results/08_castration_ablation_13vec.csv`](results/08_castration_ablation_13vec.csv) — 13-vector restraint sweep & 50% Escape-Breeding Paradox.
- [`results/09_paired_stats_n24.txt`](results/09_paired_stats_n24.txt) — Locked `N = 24` paired permutation tests with 95% bootstrap CIs & Cohen's $d_z$ (`96/96` wins, `p_Holm = 0.00040`, `d_z ∈ [1.468, 3.560]`).
- [`results/10_ecosystem_4build_bridge.txt`](results/10_ecosystem_4build_bridge.txt) — 4-Build `AUTOPHAGE` 128B/256B -> Cellulogonic -> `Asmodensius` `[0,1]^112` bridge (physics-identical across `v1.6.0`–`v1.7.0`, `ABI 0x00010700`).
- [`results/11_falsification_win64.txt`](results/11_falsification_win64.txt) & [`results/11_falsification_linux.txt`](results/11_falsification_linux.txt) — 26/26 gates + 8/8 mutants caught on Win64 & Linux + ASan/UBSan + `1,000/1,000` `fuzz_cmbin_abi` + 3-module zero-heap audit.
- [`results/12_embryogenesis_grn_3d.txt`](results/12_embryogenesis_grn_3d.txt) — 5-fate clonal 3D embryogenesis (`47` alive, `Gm=6`, `81.1%` peak sort, `SHA cafc83d0...15bad443`).

## 3. Manuscript, Crossref Audit, FAIR Metadata, Dual Timestamps & Checksum Integrity
- [`dist/Donahue_2026_cellulogonic_mecha.pdf`](dist/Donahue_2026_cellulogonic_mecha.pdf) & [`dist/arxiv_submission.tar.gz`](dist/arxiv_submission.tar.gz) — Frozen AMS-LaTeX preprint (`v1.7.0`, `0` Overfull `\hbox`/`\vbox`, `0` undefined refs, clean-room verified).
- [`paper/refs.bib`](paper/refs.bib) & [`results/citation-audit.json`](results/citation-audit.json) — **`36 / 36`** primary academic DOIs verified live via Crossref REST API (`36/36` title-aligned, `36/36` author-aligned, `0` retractions, `0` year mismatches).
- [`croissant.json`](croissant.json), [`ro-crate-metadata.json`](ro-crate-metadata.json), [`codemeta.json`](codemeta.json) & [`docs/DATASHEET.md`](docs/DATASHEET.md) — FAIR machine-readable dataset/software metadata (MLCommons Croissant 1.0, W3C RO-Crate 1.1, CodeMeta 2.0) and Gebru et al. (2021) Datasheet.
- [`CHECKSUMS.sha256`](CHECKSUMS.sha256), [`data/MANIFEST.sha256`](data/MANIFEST.sha256), [`data/timestamps/CHECKSUMS.sha256.tsr`](data/timestamps/CHECKSUMS.sha256.tsr) & [`data/timestamps/CHECKSUMS.sha256.ots`](data/timestamps/CHECKSUMS.sha256.ots) — Cryptographic SHA-256 manifests anchored by BOTH an independent DigiCert RFC 3161 TSA timestamp (`.tsr`) and 3-calendar Bitcoin OpenTimestamps proof (`.ots`), plus Software Heritage snapshot `swh:1:snp:8c1696436104a888897ce3b97867eda983d56160` and SLSA v4 / Sigstore Rekor build provenance.
