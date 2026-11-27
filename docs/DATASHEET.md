<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Datasheet for CELLULOGONIC MECHA Empirical Artifacts (`v1.7.0`)

> Structured according to Gebru et al. (2021), *"Datasheets for Datasets"*, Communications of the ACM 64(12).

---

## 1. Motivation

- **For what purpose was this dataset / benchmark archive created?** To provide deterministic, bit-reproducible empirical outputs, multi-arm paired statistical logs (`N = 24`, seeds `5000..5023`), 13-vector restraint ablation tables (`results/08_castration_ablation_13vec.csv`), cross-OS SHA-256 parity benchmarks, Lean 4.35.0 `#print axioms` and multi-kernel audit transcripts, and a live Crossref bibliographic audit (`results/citation-audit.json`) supporting **CELLULOGONIC MECHA `v1.7.0`** — an independent computational artificial-life laboratory (not peer-reviewed).
- **Who created and funded the dataset?** Independent researcher **Alexander Stephen Donahue** (`Independent Researcher, Spokane, WA, USA`; ORCID [`0009-0006-8168-9648`](https://orcid.org/0009-0006-8168-9648); `0_0@0thernes.art`), self-funded with zero external grants and zero commercial conflicts of interest, utilizing disclosed AI engineering assistance (Google DeepMind / Antigravity coding & formal-verification agents under human direction, detailed in [`docs/AI_DISCLOSURE.md`](AI_DISCLOSURE.md)).

## 2. Composition

- **What do the archived files represent?** Every file in [`results/`](../results/) is a deterministic, machine-generated output of the clean-room C11 simulation engine (`bin/cellulogonic`), the Lean 4.35.0 formal verification pipeline (`lean/`), or the live Crossref citation audit (`scripts/check_citations.py`):
  - **Tabular Ablation Dataset (`results/08_castration_ablation_13vec.csv`):** 29 experimental conditions × 15 typed columns (`condition`, `restraint_pct`, `alive`, `eukaryotes`, `multicellular`, `covi`, `stage`, `met`, `kill_del`, `dorm_spores`, `hgt_smug`, `met_bypass`, `mass_res`, `energy_res`, `second_law_viol`) measuring baseline (`0%`), intermediate (`50%`), and maximal (`100%`) perturbations across all 13 Omega restraint vectors on locked seed `5000`.
  - **Paired Statistical Suite (`results/09_paired_stats_n24.txt`):** Locked paired runs across 24 seeds (`5000..5023`, 180 epochs) for four pre-registered hypotheses (`H1`–`H4`, [`docs/preregistration.md`](preregistration.md)), reporting paired differences, 95% bootstrap CIs, Cohen's $d_z$, win counts, and 10,000-permutation step-down Holm–Bonferroni $p$-values.
  - **Deterministic Trajectory & Parity Logs (`results/01..07`, `10..12`):** Hardware probe, Win64/Linux AVX2-vs-Scalar 300-epoch parity (`83317026…532c9bbe`), zero-seed abiogenesis, endosymbiosis ablation, 4-solvent de-novo adaptation & Terran→Venus transplant, bioelectric cancer policing, tardigrade TDP anhydrobiosis, 4-build ABI bridge, 26-gate/8-mutant falsification logs, and 5-fate clonal 3D embryogenesis.
  - **Formal & Bibliographic Audit Artifacts (`results/lean4-audit.txt`, `results/lean4-multi-kernel.txt`, `results/citation-audit.json`):** Verbatim `#print axioms` output for all 78 `Spec.lean` theorems, 6-kernel-check CI transcript, and JSON Crossref audit (`36/36` DOIs, `title_aligned=36/36`, `author_aligned=36/36`, `0` retractions).
- **Does the dataset contain confidential, personally identifiable, or sensitive human data?** **No.** All simulation records are synthetic outputs of a deterministic integer-quantum physics engine; citation records reference only public published scientific literature.
- **Are there missing values?** Zero missing or NaN values in any archived CSV, JSON, or simulation log (enforced by `G01`–`G26`, `scripts/verify_oracle_and_stats.py`, and `scripts/check_citations.py`).

## 3. Collection & Generation Process

- **How were the data generated?** Exclusively by compiling and executing the portable zero-dependency C11 source code in [`src/`](../src/) and [`include/`](../include/) using deterministic counter-based `Philox4x32-10` PRNG streams (`seed = 5000` for single-seed logs; `seeds = 5000..5023` for `N = 24` paired statistics), and by running `lake env lean VerifyAxioms.lean` (`leanprover/lean4:v4.35.0`) and `scripts/check_citations.py` (`api.crossref.org`).
- **Pre-registration & Anti-Cherry-Picking:** The four primary hypotheses (`H1`–`H4`), sample size (`N = 24`), seed range (`5000..5023`), and Holm–Bonferroni $\alpha = 0.05$ threshold are documented in [`docs/preregistration.md`](preregistration.md).

## 4. Preprocessing, Integrity & Verification

- **Cryptographic Integrity:** Every archived file in `results/` is hashed in [`data/MANIFEST.sha256`](../data/MANIFEST.sha256); top-level deliverables (`dist/`, `lean/`, `paper/`, `results/lean4-audit.txt`, `results/citation-audit.json`, `VERSION`) are hashed in LF-normalized [`CHECKSUMS.sha256`](../CHECKSUMS.sha256) and anchored by dual independent timestamps: DigiCert RFC 3161 ([`data/timestamps/CHECKSUMS.sha256.tsr`](../data/timestamps/CHECKSUMS.sha256.tsr)) and 3-calendar Bitcoin OpenTimestamps ([`data/timestamps/CHECKSUMS.sha256.ots`](../data/timestamps/CHECKSUMS.sha256.ots)), verified on every CI run via `scripts/stamp_timestamps.py --verify`.
- **Cross-Platform Determinism:** Native C11 execution achieves 0-bit SHA-256 parity across Windows x86-64 (`clang 23.1.0`), Ubuntu Linux x86-64 (`gcc 13.3.0`), AVX2 vs. Scalar modes, and macOS Apple Silicon ARM64 CI.
- **Independent External Audit Note:** The 2026-11-26 external statement audit rebuilt the Lean library only (`ERRATA.md`); it did not independently re-run the C11 benchmarks or statistical scripts. Third-party clean-clone replication is openly invited ([`REPLICATION.md`](../REPLICATION.md)).

## 5. Uses & Explicit Out-of-Scope Warnings

- **Recommended Uses:** Reproducible ALife benchmarking, studying conservation-exact discrete cellular models, formal-verification workflow benchmarking (Lean 4.35.0 Palomar `Spec`/`Challenge`/`Solution`), deterministic cross-ISA simulation testing, and educational exploration.
- **Explicit Out-of-Scope / Prohibited Misuses:**
  - Do **NOT** interpret coarse-grained integer simulation logs as wet-lab biochemical proof of abiogenesis, astrobiological habitability (Titan/Venus), tardigrade radioprotection, or biological consciousness.
  - Do **NOT** present the 78 Lean 4.35.0 theorems about `Spec.lean` as a formal verification of the C11 source files (`src/*.c`) or as a solution to any open problem in `0thernes/unsolved-problems-mega-atlas` (`0 / 1,900` retired; [`docs/MEGA_ATLAS_CROSSCHECK.md`](MEGA_ATLAS_CROSSCHECK.md)).
  - Do **NOT** represent this preprint or dataset as externally peer-reviewed.

## 6. Distribution & Maintenance

- **Distribution:** GitHub (`https://github.com/0thernes/cellulogonic-mecha`), GitHub Pages Blueprint (`https://0thernes.github.io/cellulogonic-mecha/`), Software Heritage (`swh:1:snp:8c1696436104a888897ce3b97867eda983d56160`), Master Research Archive (`0TH-BUILD-001` in `0thernes/0thernes-research-archive`), FAIR manifests (`croissant.json`, `ro-crate-metadata.json`, `codemeta.json`), and Zenodo-ready `.zenodo.json`.
- **Licensing:** Dual-licensed — Code, Lean 4 proofs, scripts, and CI under **Apache-2.0** ([`LICENSE`](../LICENSE)); papers, documentation, figures, and archived empirical datasets/logs under **CC-BY-4.0** ([`LICENSE-DOCS`](../LICENSE-DOCS), [`REUSE.toml`](../REUSE.toml)).
- **Maintenance:** Maintained by Alexander Stephen Donahue (`0_0@0thernes.art`) with semantic versioning ([`CHANGELOG.md`](../CHANGELOG.md)), errata logging ([`ERRATA.md`](../ERRATA.md)), and structured 10-class GitHub Issue templates.
