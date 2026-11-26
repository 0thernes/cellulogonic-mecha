# ADR 0001: Exact Integer Conservation Core, Palomar Six-Kernel Lean 4.35.0 Boundary & Dual Open Licensing

- **Date:** 2026-11-25
- **Status:** Accepted (`v1.5.0`)
- **Author:** Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`)

## Context

Computational Artificial Life simulations frequently fail independent scrutiny for three distinct reasons:
1. Floating-point accumulation drift silently creates or destroys mass/energy, allowing evolutionary agents to exploit simulator bugs instead of physical trade-offs.
2. Monolithic formal proof files without isolated specification boundaries can pass trivially through vacuous hypotheses, `autoImplicit` typo-generalization, or mismatched definitions.
3. Non-commercial source-available licenses and informal institutional labels hinder open scientific replication, automated package indexing, and honest scholarly citation.

## Decision

1. **Exact `int64_t` Thermodynamic & Stoichiometric Ledger (`P2`/`P4`/`P9`/`P10`):** Retain exact integer counts for all 8 molecular species (`S, X, M, E, L, P, T, W`), 19 reactions (`R1`–`R15c`), and 4 planetary solvents with dual bit-exact `SIMD_AVX2` and `SCALAR_REF` kernels (`0-bit` SHA-256 parity across Win64 & Linux) and zero third-party runtime dependencies. Version public C11 ABI cleanly at `0x00010500` with explicit negative-bounds rejection (`G25`) and static zero-heap hot-loop auditing.
2. **Palomar Isolated Six-Kernel Lean 4.35.0 Architecture:** Refactor all formal proofs into Lean 4.35.0 `module` syntax with `autoImplicit = false` and `relaxedAutoImplicit = false`, placing pure definitions exclusively in `lean/CellulogonicProofs/Spec.lean`, statement-only `sorry` signatures in `lean/Challenge.lean`, sorry-free proofs (including explicit `NonVacuity.lean` witnesses `NV01`–`NV08`) in `lean/Solution.lean`, verified across independent kernels locally and all 6/6 kernels under `bubblewrap` in Ubuntu CI.
3. **Dual Open Licensing & Independent Researcher Attribution:** Permanently migrate all code, Lean proofs, scripts, and CI workflows to **Apache-2.0** (`LICENSE`), all papers, PDFs, documentation, and figures to **CC-BY-4.0** (`LICENSE-DOCS`), and standardize all attribution on `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`).
4. **Explicit Epistemic & Mega-Atlas Boundaries:** Maintain explicit separation between kernel-checked theorems, program-audited C11 gates, indicator-only observables, and unretired open conjectures (`0 / 1,900` Mega-Atlas problems retired).

## Consequences

- **Positive:** Any independent researcher can verify all 70 mathematical proofs across 6 independent kernels, reproduce bit-exact SHA-256 runs, verify 36 live DOIs, compile a zero-overflow AMS-LaTeX preprint, and reuse code/text legally under standard OSI/Creative Commons licenses.
- **Trade-off:** Coarse-grained integer quanta cannot model atomistic protein folding or wet-lab reaction rates, and 6th-kernel `lean4lean` / `bubblewrap` replay runs on Linux CI rather than native Windows (explicitly disclosed in `DEVIATIONS.md`).
