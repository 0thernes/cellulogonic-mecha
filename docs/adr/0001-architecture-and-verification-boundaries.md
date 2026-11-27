# ADR 0001: Exact Integer Conservation Core, Palomar Six-Kernel-Check Lean 4.35.0 Boundary & Dual Open Licensing

- **Date:** 2026-11-25 (amended `2026-11-26` for `v1.7.0`)
- **Status:** Accepted (`v1.7.0`)
- **Author:** Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`)

## Context

Computational Artificial Life simulations frequently fail independent scrutiny for three distinct reasons:
1. Floating-point accumulation drift silently creates or destroys mass/energy, allowing evolutionary agents to exploit simulator bugs instead of physical trade-offs.
2. Monolithic formal proof files without isolated specification boundaries can pass trivially through vacuous hypotheses, `autoImplicit` typo-generalization, or mismatched definitions.
3. Non-commercial source-available licenses and informal institutional labels hinder open scientific replication, automated package indexing, and honest scholarly citation.

## Decision

1. **Exact `int64_t` Thermodynamic & Stoichiometric Ledger (`P2`/`P4`/`P9`/`P10`):** Retain exact integer counts for all 8 molecular species (`S, X, M, E, L, P, T, W`), 19 reactions (`R1`–`R15c`), and 4 planetary solvents with portable `-march=x86-64` + isolated `target("avx2")` / `SCALAR_REF` / macOS ARM64 kernels (`0-bit` SHA-256 parity across 3 OSes / 2 ISAs) and zero third-party runtime dependencies. Version public C11 ABI cleanly at `0x00010700` with explicit negative-bounds rejection (`G25`) and 3-module static zero-heap hot-loop auditing.
2. **Palomar Isolated Six-Kernel-Check Lean 4.35.0 Architecture:** Maintain all formal proofs in Lean 4.35.0 `module` syntax (10 proof modules + `Spec.lean`, 15 `.lean` files) with `autoImplicit = false` and `relaxedAutoImplicit = false`, placing pure definitions exclusively in `lean/CellulogonicProofs/Spec.lean` (`12` pinned definitions), statement-only `sorry` signatures in `lean/Challenge.lean`, and sorry-free proofs (including 5 explicitly annotated bookkeeping helpers, `NonVacuity.lean` `NV01`–`NV08`, and `AdversarialTests.lean` `AT01`–`AT08`, `78/78` audited theorems about `Spec.lean`) in `lean/Solution.lean`, verified under **6 kernel checks** (Lean default & paranoid modes + 4 independent checkers: `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`) inside `bubblewrap` in Ubuntu CI with `6/6` live negative elaboration & `sorry`-tamper rejection (`scripts/test_lean_negative.py`).
3. **Dual Open Licensing & Independent Researcher Attribution:** Permanently maintain all code, Lean proofs, scripts, and CI workflows under **Apache-2.0** (`LICENSE`), all papers, PDFs, documentation, and figures under **CC-BY-4.0** (`LICENSE-DOCS`, `REUSE.toml`, `LICENSES/`), and standardize all attribution on `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`).
4. **Explicit Epistemic & Mega-Atlas Boundaries:** Maintain explicit separation between kernel-checked theorems, program-audited C11 gates, indicator-only observables, and unretired open conjectures (`0 / 1,900` Mega-Atlas `v2.6.0` problems retired across 13 verbatim adjacent catalog IDs).

## Consequences

- **Positive:** Any independent researcher can re-check the 78 Lean theorems about the `Spec.lean` model under six kernel checks, reproduce bit-exact SHA-256 runs across Linux/Windows/macOS ARM64, verify 36 live Crossref retraction-free DOIs, explore the interactive GitHub Pages Proof Blueprint, compile a zero-overflow AMS-LaTeX preprint, and reuse code/text legally under standard OSI/Creative Commons licenses.
- **Trade-off:** Coarse-grained integer quanta cannot model atomistic protein folding or wet-lab reaction rates, and 6th-kernel `lean4lean` / `bubblewrap` replay runs on Linux CI rather than native Windows (explicitly disclosed in `DEVIATIONS.md`).
