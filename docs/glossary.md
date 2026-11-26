# Glossary & Finding-Class Definitions (`docs/glossary.md`)

For the full 60+ term biological and computational glossary, also see root [`GLOSSARY.md`](../GLOSSARY.md).

## 1. The 10 Standard Finding Classes (Build Standard v1.2.0, Rule R7)

Every audit, review, or issue finding in this repository is classified into **exactly one** of these 10 canonical classes (also synced in [`.github/labels.tsv`](../.github/labels.tsv)):

| Class | Definition |
|---|---|
| `bug` | Code does something different from its spec or intent (a defect in program logic). |
| `error` | Wrong number, claim, doc, config, version, build, or command. |
| `issue` | Works and is not wrong, but is unclear, inconsistent, fragile, or hard to maintain. |
| `problem` | External obstacle (network, account, legal entity mismatch, toolchain) blocks progress or validity. |
| `concern` | Risk that has not caused harm yet (security, cost, legal, reliability). |
| `dilemma` | Trade-off between valid options where the owner decides. |
| `conflict` | Two requirements, licenses, or rules contradict each other. |
| `gap` | Something required or expected is missing; nothing present is wrong. |
| `hole` | Missing check or safeguard, so a defect could pass unnoticed. |
| `blindspot` | A user, platform, failure mode, or assumption nobody has examined yet. |

## 2. Core Biophysical & Verification Terms

- **Chemoton:** Tibor Gánti's minimal living unit coupling an autocatalytic metabolic network, an enclosing amphiphilic boundary, and an informational template polymer (`T01`–`T10`).
- **Squared Reduced Volume (`Q(V, L) = 100 V^2 / L^3`):** Cleared-denominator integer elastic vesicle tension metric governing osmotic lysis (`Q > 1`), swollen interphase budding (`1/2 < Q <= 1`), and symmetric binary fission (`Q <= 1/2`, `T11`–`T19`).
- **Global Minimax Theorem of Vesicle Division (`T16`):** Exact sextic algebraic identity $2(x^3+y^3)^2 - (x^2+y^2)^3 = (x-y)^2(x^4 + 2x^3y + 2xy^3 + y^4) \ge 0$ proving symmetric halving maximizes membrane area demand.
- **Proton-Motive Force (`e_pmf`) & c-Ring Stoichiometry (`n_H`):** Solvent-specific electrochemical quantum per translocated ion; strict Second-Law phosphorylation requires $n_H \cdot e_{\mathrm{pmf}} > 28\,\mathrm{uq}$ (`T28`–`T30`).
- **COVI (Cellular Omega Vitality Index):** Model-internal 8-pillar composite indicator gated multiplicatively on active metabolism, heritable templates, and active catalysts (`T34`–`T35`).
- **Eutely:** Fixed adult somatic cell count enforced by lineage countdown budgets $\sum 2^{\mathrm{rem}(c)} \le N_0 2^{d_0}$ (`T54`) and Hippo TF7 contact inhibition (`T55`).
- **Palomar Comparator Contract:** Isolated `Spec.lean` (pure defs) + `Challenge.lean` (statement-only `sorry`) + `Solution.lean` (sorry-free proofs) checked across 6 independent Lean 4.35.0 kernels.
- **`"CMB5"` `.cmbin` Snapshot:** Authenticated binary world-state serialization appending a NIST FIPS 180-4 SHA-256 integrity trailer verified alongside zero-padding (`_Static_assert`), bounds, bond-symmetry, no-trailing-EOF, and zero-residual ledger checks.
