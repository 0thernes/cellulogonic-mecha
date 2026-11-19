# Contributing to CELLULOGONIC MECHA

Non-commercial research contributions, independent verification audits, bug reports, and formal proof challenges are welcome under the terms of [LICENSE](../LICENSE).

## Verification Standards for Any Code or Proof PR
1. **Zero Conservation Drift**: Every new reaction, transport channel, lysis pathway, or engulfment transition MUST preserve exact integer atomic mass (`mass_residual == 0`) and exact First-Law energy (`int_ledger_residual_uq == 0`), with `second_law_violations == 0`.
2. **Bit-Exact Determinism**: SIMD/AVX2 and portable scalar kernels MUST agree to 0-bit SHA-256 diff across both Windows (Clang) and Linux (GCC).
3. **Zero `sorryAx`**: Any Lean 4 change MUST pass `lake build` and `#print axioms` audit with zero `sorryAx` and zero custom axioms.
4. **Honest Scientific Scope**: Clearly separate established biological/physical theory from newly formulated discrete theorems or observed emergent simulation phenomena.
