# Transparent AI-Assistance Disclosure & CRediT Authorship Statement

**Project:** CELLULOGONIC MECHA (`v1.7.0`)  
**Author:** Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA` · ORCID: [`0009-0006-8168-9648`](https://orcid.org/0009-0006-8168-9648))  
**Date:** 2026-11-26  

## 1. Human Authorship, CRediT Taxonomy & Conflict of Interest (COI)

Alexander Stephen Donahue is the sole author and bears full scholarly, ethical, and technical accountability for every file, theorem, benchmark, and manuscript in this repository:
- **Conceptualization & Methodology:** Four-build ALife architecture, biophysical scope, thermodynamic/geometric laws, pre-registered ablation hypotheses, and epistemic verification boundaries.
- **Software, Formal Analysis & Validation:** Direction, review, and auditing of the C11 engine, 26-gate / 8-mutant falsification + 1,000-case ABI fuzz harness, 78-theorem Lean 4.35.0 `Spec.lean` suite (6 kernel checks), and cross-platform reproducibility scripts.
- **Writing, Supervision & Funding Acquisition:** Self-funded independent research with **zero external grants and zero commercial or financial conflicts of interest (COI)**; final editing and approval of all prose, tables, and manuscripts.

## 2. Disclosed AI Tool Usage

Substantive AI engineering and research assistance—specifically **Google DeepMind / Antigravity coding & formal-verification agents** operating under the author's explicit direction—was used for:
1. C11 implementation, SIMD/AVX2 optimization, and hardening fixes across `src/`, `include/`, `test/`, and `web/`.
2. Lean 4.35.0 proof engineering, Palomar `Spec`/`Challenge`/`Solution` refactoring, `NonVacuity` & `AdversarialTests` construction, and `v1.7.0` statement-audit hardening in `lean/`.
3. Verification scripts (`scripts/build_paper.py`, `scripts/check_citations.py`, `scripts/check_readme_axioms.py`, `scripts/test_lean_negative.py`, `scripts/build_blueprint.py`, `scripts/check_blueprint.py`, `scripts/stamp_timestamps.py`, `scripts/prop_check.py`), FAIR metadata (`croissant.json`, `ro-crate-metadata.json`, `codemeta.json`, `docs/DATASHEET.md`), and CI workflow configuration.
4. AMS-LaTeX manuscript drafting and Build Standard v1.2.0 documentation alignment.

## 3. Verification-First Anti-Hallucination Discipline

- **AI systems are NOT listed as authors** and cannot bear scholarly responsibility.
- **Zero unverified AI outputs are accepted as truth:**
  - Every one of the 78 theorems about `Spec.lean` in `lean/` is mechanically checked by **6 kernel checks** in CI (Lean default & paranoid modes + 4 independent checkers: `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`) with zero `sorry` in `Solution`, zero `native_decide`, and live negative elaboration & `sorry`-tamper rejection (`scripts/test_lean_negative.py`).
  - Every computational claim is backed by live native C11 execution, 3-OS / 2-ISA 0-bit SHA-256 parity, 26/26 falsification gates, 8/8 caught mutants, 1,000/1,000 ABI fuzz cases, and 2,720 property checks.
  - Every bibliographic DOI in `paper/refs.bib` is verified live against the Crossref REST API for reachability, title/author alignment, and zero retractions via `scripts/check_citations.py` (`36/36` OK -> `results/citation-audit.json`).
- **No peer review, arXiv ID, or Zenodo DOI is claimed** prior to confirmed external assignment.
