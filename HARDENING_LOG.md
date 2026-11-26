# Hardening Log (`HARDENING_LOG.md`)

Chronological engineering hardening record using Build Standard v1.2.0 finding classes (`bug`, `error`, `issue`, `problem`, `concern`, `dilemma`, `conflict`, `gap`, `hole`, `blindspot` — Rule R7).

## Release `v1.5.0` — 2026-11-25 (Gold Standard, Dual Open License, 6-Kernel Palomar Lean 4.35.0 & Playbook v1.3 Paper)

| ID | Class | Target | Fix Applied | Verification |
|---|---|---|---|---|
| `H-150-01` | `problem` | 18+ files (`LICENSE`, `CITATION.cff`, `paper/main.tex`, `README.md`, `LEGAL.md`, `lean/*.lean`, etc.) | Removed every active reference to `0thernes LLC` and standardized on `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`). | Regex scan returns `0` matches in active files. |
| `H-150-02` | `conflict` | `LICENSE`, `LICENSE-DOCS`, `NOTICE`, `CITATION.cff` | Replaced proprietary non-commercial Cosmogonic License with dual open **Apache-2.0** (code) + **CC-BY-4.0** (papers/docs/figures). | Verified in `LICENSE`, `LICENSE-DOCS`, `CITATION.cff`, `.zenodo.json`. |
| `H-150-03` | `error` | `VERSION`, `paper/main.tex`, `CITATION.cff`, `INDEX.md` | Resolved version drift (`1.3.0` / `1.4.0` mismatch) and synchronized all files to `1.5.0`. | `Audit-Repo.ps1` `A08` (`version_consistent`) PASS. |
| `H-150-04` | `gap` | `lean/` | Upgraded to Lean 4.35.0 `module` system (`autoImplicit = false`, `relaxedAutoImplicit = false`, `linter.unusedVariables = true`, `linter.deprecated = true`), isolated pure `Spec.lean`, statement-only `Challenge.lean`, `Solution.lean`, `VerifyAxioms.lean`, `NonVacuity.lean` (`NV01`–`NV08`), `comparator.json`, `formalization.yaml`. | `70/70` theorems pass `lean`, `leanchecker`, `leanchecker-paranoid`, `con-leche --verified`, `con-ron --verified`, `nanoda_bin`; 6/6 wired in Ubuntu CI. |
| `H-150-05` | `hole` | `README.md`, `scripts/check_readme_axioms.py` | Added verbatim 70-theorem `#print axioms` block guarded by automated drift-check script. | `scripts/check_readme_axioms.py` PASS. |
| `H-150-06` | `gap` | `paper/main.tex`, `paper/refs.bib`, `scripts/build_paper.py`, `scripts/check_citations.py` | Upgraded white paper to Playbook v1.3 `amsart` with Claims Ledger, Statement-Fidelity Matrix, Mega-Atlas non-retirement table, Threats to Validity, CRediT/AI disclosure, and 36 BibTeX DOIs. | `36/36` DOIs verified live; Tectonic `0` Overfull `\hbox`; clean-room `arxiv_submission.tar.gz` PASS. |
| `H-150-07` | `hole` | `scripts/prop_check.py`, `test/fuzz_cmbin.c`, `.github/workflows/ci.yml` | Added 2,700-trial randomized property suite (`B10`), `libFuzzer` `"CMB4"` parser harness (`B15`), ASan/UBSan CI job (`B16`), cross-OS matrix, gitleaks, and SLSA build provenance (`B35`/`B38`). | `prop_check.py` `2,700/2,700` PASS; CI workflow validated. |
| `H-150-08` | `blindspot` | `docs/MEGA_ATLAS_CROSSCHECK.md`, `LIMITATIONS.md`, `CLAIMS.md` | Documented explicit `0 / 1,900` Mega-Atlas non-retirement cross-check across 7 biological/systems probes and explicit literal-vs-metaphor & falsifier boundaries. | `Audit-Repo.ps1` `100.0%` Structure / `100.0%` Completion / `100.0%` Core. |

## Prior Hardening Cycles (`v1.2.1`–`v1.4.0`)
- **`v1.4.0` (`2026-11-22`):** Added `ContinuumBiomechanics.lean` (`C59`–`C62`), compiled C11 continuum checks in `G18`/`G21`/`G23`, and Arrow Lake AVX-VNNI + ThinLTO build flags (`3b85b15`).
- **`v1.3.0` (`2026-11-21`):** Closed Titan `q=0` boundary, de-collided GRN Wnt loci (`genome[102..103]`), implemented translation-invariant 3D paracrine morphogens & Taylor–Green swirl, smooth Lipschitz `AUTOPHAGE` 128B decoder, hardened `"CMB4"` `.cmbin` validation, and CSRF-hardened `POST /api/cmd` web observatory.
