<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Submission Readiness Matrix — Defensible Independent AI Research Playbook v1.3

**Repository:** `0thernes/cellulogonic-mecha` (`v1.7.0`)  
**Author:** Alexander Stephen Donahue — Independent Researcher, Spokane, WA, USA · ORCID [`0009-0006-8168-9648`](https://orcid.org/0009-0006-8168-9648)  
**Assessment Date:** 2026-11-26  
**Scope Statement:** Independent research preprint and executable simulation + formal-proof artifact. **Not peer-reviewed.** Zero (`0 / 1,900`) overarching [Mega-Atlas v2.6.0](docs/MEGA_ATLAS_CROSSCHECK.md) open problems are claimed solved or retired. **Software Heritage SWHID:** `swh:1:snp:8c1696436104a888897ce3b97867eda983d56160`.

---

## 43-Item Gold Standard Checklist (Playbook v1.3)

| # | Playbook v1.3 Requirement | Status | Concrete Evidence / Artifact |
|---:|---|---|---|
| 1 | Dedicated public repository with clean git history on `main` | **DONE** | `https://github.com/0thernes/cellulogonic-mecha`, branch `main` |
| 2 | Dual open license (code + docs/preprint) & REUSE/SPDX | **DONE** | `LICENSE` (Apache-2.0), `LICENSE-DOCS` (CC-BY-4.0), `LICENSES/`, `REUSE.toml`, SPDX headers |
| 3 | Persistent author identity & ORCID | **DONE** | Alexander Stephen Donahue, Independent Researcher, ORCID `0009-0006-8168-9648` |
| 4 | Machine-readable citation & FAIR metadata + Gebru Datasheet | **DONE** | `CITATION.cff` (`v1.7.0`), `CITATION.bib`, `.zenodo.json`, `codemeta.json`, `croissant.json`, `ro-crate-metadata.json`, `docs/DATASHEET.md` |
| 5 | Transparent AI-assistance disclosure, CRediT & COI (code, proofs, prose) | **DONE** | `README.md`, `docs/AI_DISCLOSURE.md`, `paper/main.tex` §7, `paper/WHITE_PAPER.md`, `.zenodo.json` |
| 6 | Honest claim-boundary & real-vs-metaphor limitations | **DONE** | `LIMITATIONS.md`, `CLAIMS.md` (C1–C8), `docs/statement-fidelity.md`, `ERRATA.md` |
| 7 | Explicit falsification criteria ("what would prove us wrong") | **DONE** | `CLAIMS.md` falsification column + 8 planted mutants `M1`–`M8` |
| 8 | Zero-sorry, zero-custom-axiom machine-checked proofs (`Spec.lean`) | **DONE** | `lean/` — **78/78** Pure Lean 4.35.0 declarations (10 proof modules + `Spec.lean`, 15 `.lean` files; 5 annotated bookkeeping helpers) strictly within `{propext, Classical.choice, Quot.sound}` |
| 9 | Pinned toolchain & manifest (`lean-toolchain`, `lake-manifest.json`) | **DONE** | `lean/lean-toolchain` (`v4.35.0`), `lean/lake-manifest.json`, zero Mathlib |
| 10 | Statement-only Challenge vs Solution separation (Palomar) | **DONE** | `lean/Challenge.lean` (imports ONLY `Spec.lean`) vs `lean/Solution.lean` |
| 11 | Multi-kernel checks (Lean default/paranoid + 4 independent checkers) | **DONE** | `.github/workflows/ci.yml` (`lean4-six-kernel`: Lean default & paranoid + `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`) + `results/lean4-multi-kernel.txt` |
| 12 | Adversarial false-variant refutations (`¬P`) inside formal proofs | **DONE** | `lean/CellulogonicProofs/AdversarialTests.lean` (`AT01`–`AT08` across Groups I–VIII) |
| 13 | Non-vacuity witnesses for every quantified theorem group | **DONE** | `lean/CellulogonicProofs/NonVacuity.lean` (`NV01`–`NV08`) |
| 14 | Live negative elaboration & kernel tamper rejection tests | **DONE** | `scripts/test_lean_negative.py` (`6/6` elaboration + `Spec` `sorry`-tamper + axiom checks) + CI tampered-`Solution` `lake comparator --paranoid` failure gate |
| 15 | Statement-fidelity matrix mapping formal defs <-> C11 <-> paper prose | **DONE** | `docs/statement-fidelity.md` (with explicit bookkeeping helper table §2) + `lean/formalization.yaml` (v0.4 schema-validated) |
| 16 | Verbatim `#print axioms` output reproduced in `README.md` & CI-diffed | **DONE** | `results/lean4-audit.txt`, `scripts/check_readme_axioms.py`, `diff -u` in CI |
| 17 | Independent exact-arithmetic reference oracle (different language) | **DONE** | `reference/chemoton_oracle.py` (6,270 exact `fractions.Fraction` checks) |
| 18 | Deterministic property-based testing across randomized inputs | **DONE** | `scripts/prop_check.py` (2,720 trials incl. randomized native C11 binary runs) |
| 19 | Mutation testing (deliberately broken mutants caught) | **DONE** | `test/test_falsification.c` — 8/8 planted mutants (`M1`–`M8`, incl. simulation-level `M3`) caught |
| 20 | Metamorphic & conservation invariants verified at runtime | **DONE** | `G05`–`G09`, `G23` Z-translation invariance, `G08` P9 observer independence |
| 21 | Cross-platform & cross-ISA bit-exact reproducibility | **DONE** | 3-OS / 2-ISA CI (`ubuntu-latest` x86-64, `windows-latest` x86-64, `macos-latest` ARM64) |
| 22 | Dual-kernel SIMD vs scalar 0-bit parity | **DONE** | `G07`, `G09` (`SIMD_AVX2 == SCALAR_REF`, portable `-march=x86-64` + isolated AVX2 target) |
| 23 | Sanitizers (`ASan`/`UBSan`) & fuzzing harness on binary parser | **DONE** | `test/fuzz_cmbin_abi.c` (1,000-case standalone + `libFuzzer` CI smoke + ASan/UBSan) |
| 24 | Zero post-warmup heap allocation audit (hot loop) | **DONE** | `G10` + 3-module `nm -u` audit (`chemoton_physics.o`, `cellular_world.o`, `metrology_covi.o`) |
| 25 | Pre-registered quantitative hypotheses & ablation controls | **DONE** | `docs/preregistration.md` (H1–H4) + `results/09_paired_stats_n24.txt` |
| 26 | Non-parametric permutation p-values, 95% CIs, effect sizes, FWER | **DONE** | 10,000 sign-flip permutations, 95% bootstrap CIs, Cohen's `dz`, Holm-Bonferroni (`p_Holm <= 0.05` asserted) |
| 27 | Live Crossref citation reachability, title/author & retraction audit | **DONE** | `scripts/check_citations.py` (`v2`) + `results/citation-audit.json` (`36/36` DOIs live-verified, `36/36` title & author aligned, `0` retractions) |
| 28 | Publication-quality AMS-LaTeX preprint PDF (0 overfull, 0 undef refs) | **DONE** | `paper/main.tex` -> `dist/Donahue_2026_cellulogonic_mecha.pdf` via Tectonic (`0` Overfull `\hbox`/`\vbox`) |
| 29 | Clean-room deterministic arXiv source tarball (`mtime=0`, USTAR) | **DONE** | `dist/arxiv_submission.tar.gz` verified by clean-room Tectonic rebuild |
| 30 | Interactive static proof blueprint / research portal (GitHub Pages) | **DONE** | `scripts/build_blueprint.py` + `scripts/check_blueprint.py` + `.github/workflows/pages.yml` (`_site/`) |
| 31 | Cryptographic SHA-256 manifests for all deliverables & data | **DONE** | `CHECKSUMS.sha256` (LF-normalized), `data/MANIFEST.sha256`, CI `SHA256SUMS` |
| 32 | Dual Independent Timestamps: DigiCert RFC 3161 + Bitcoin OTS | **DONE** | `scripts/stamp_timestamps.py`, `data/timestamps/CHECKSUMS.sha256.tsr` (DigiCert TSA) + `data/timestamps/CHECKSUMS.sha256.ots` (3-calendar Bitcoin OTS), verified in CI |
| 33 | SLSA build provenance attestation (`attest-build-provenance@v4`) | **DONE** | `.github/workflows/ci.yml` & `.github/workflows/release.yml` (Sigstore Rekor) |
| 34 | Offsite archival registration (`0thernes-research-archive` + SWHID) | **DONE** | Registered as `0TH-BUILD-001` in `0thernes/0thernes-research-archive`; SWHID `swh:1:snp:8c1696436104a888897ce3b97867eda983d56160` |
| 35 | Clean-clone replication protocol & dated replication log | **DONE** | `REPLICATION.md` (`just reproduce`, dated Win64/WSL2/3-OS CI log; external human replication invited) |
| 36 | Red-team adversarial audit dossier & multi-cycle hardening log | **DONE** | `docs/reviews/2026-11-25-10-pass-red-team-audit.md` + `HARDENING_LOG.md` |
| 37 | Security threat model & SHA-256-verified secret/vuln scanners | **DONE** | `docs/threat-model.md`, `SECURITY.md`, CI `gitleaks 8.30.1` + `osv-scanner 2.7.1` + `cppcheck` |
| 38 | Loopback web bridge CSRF, DNS-rebinding, CSP & accessibility hardening | **DONE** | `web/server.mjs`, `web/index.html` (WCAG 2.2 AA), `test/test_web_server.mjs` (13/13 PASS) |
| 39 | Version single-source-of-truth check across all manifests | **DONE** | `VERSION` + `scripts/check_versions.py` (17 tracked targets) |
| 40 | Open-problem crosswalk with explicit anti-hype scope discipline | **DONE** | `docs/MEGA_ATLAS_CROSSCHECK.md` (13 `v2.6.0` IDs verified verbatim; 0/1,900 retired) |
| 41 | Runnable minimal examples (C11 ABI + Python) | **DONE** | `examples/quickstart_c_abi.c`, `examples/01_embryogenesis_demo.py` (both run in 3-OS CI) |
| 42 | External submission gates (arXiv endorsement / Zenodo / JOSS) | **READY (MANUAL)** | arXiv tarball ready pending endorser; Zenodo deposit ready via `.zenodo.json` (human click required; never fabricated) |
| 43 | External independent human replication | **OPEN (INVITED)** | Clean-clone protocol & SHA-256 anchors published; third-party replication openly invited via `REPLICATION.md` |

---

## Venue Readiness Notes

- **arXiv (`q-bio.NC` / `cs.LO` / `physics.bio-ph`):** Clean-room `dist/arxiv_submission.tar.gz` compiles cleanly with 0 overfull hboxes/vboxes and includes `main.tex`, `refs.bib`, `main.bbl`, and vector PDF figures. Requires author manual submission / endorsement.
- **Zenodo Archive:** `.zenodo.json` metadata is valid, dual-licensed, and discloses AI assistance per Zenodo depositor guidelines. No DOI is fabricated prior to deposit.
- **Software Heritage (SWH):** Direct browser archival link: `https://archive.softwareheritage.org/save/origin/https://github.com/0thernes/cellulogonic-mecha/visit/git/` (interactive browser Anubis PoW gate respected; never bypassed).
