<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Frequently Asked Questions (`docs/faq.md`)

### 1. Does `CELLULOGONIC MECHA` claim to solve abiogenesis, consciousness, or any Mega-Atlas open problem?
**No.** As explicitly recorded in [`docs/MEGA_ATLAS_CROSSCHECK.md`](MEGA_ATLAS_CROSSCHECK.md) and [`CLAIMS.md`](../CLAIMS.md), **`0 / 1,900`** overarching open problems in `0thernes/unsolved-problems-mega-atlas` (`v2.6.0`, commit `d23b4d7`) are claimed solved or retired across all 13 adjacent catalog IDs. This repository is a conservation-exact, Lean-verified discrete computational laboratory, not wet-lab biology or proof of a new physical law.

### 2. Is the Fröhlich–Pokorný GHz microtubule module a quantum-consciousness (Orch-OR) claim?
**Strictly no.** `frohlichStep` (`T48`, `src/chemoton_physics.c`) is a purely classical, deterministic integer relaxation recurrence gated on metabolic ATP pumping (`sPump > sCrit`) with inverse-length standing-wave scaling (`T49`). No quantum-coherence or consciousness hypothesis is endorsed or claimed.

### 3. Why do some of the 78 Lean 4.35.0 theorems use `Classical.choice`?
In Lean 4.35.0, the `grind` and `omega` tactics over integer/polynomial rings occasionally pull `Classical.choice` alongside `propext` and `Quot.sound`. Every one of the **78 audited theorems** (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`, `AT01`–`AT08`) sits strictly inside Lean's standard three-axiom core with **zero `sorryAx`**, **zero `native_decide`**, **zero `Lean.ofReduceBool`**, and **zero custom axioms**, independently replayed across 6 kernels and verified by live negative tamper rejection (`scripts/test_lean_negative.py`).

### 4. Why does local Windows multi-kernel replay run 5 checkers while CI runs all 6?
On Windows 11, Lean 4.35.0's `lean4lean.exe` cannot parse Windows `.olean` path headers and `lake check` / `lake comparator` require Linux `bubblewrap` (`bwrap`). Therefore, `.github/workflows/ci.yml` runs **six kernel checks** (`lean`, `leanchecker`, `leanchecker-paranoid`, `lean4lean`, `nanoda_bin`, `con-leche`/`con-ron`, plus `lake check --paranoid`, `lake comparator --config comparator.json --paranoid`, and a live sorry-tampered comparator rejection gate) on `ubuntu-latest` with `bubblewrap`.

### 5. Why does my local throughput differ from `27.67 M rxns/s`?
Wall-clock throughput depends on CPU architecture, turbo clock, and background load (e.g., `27.665 M rxns/s` Win64 vs. `23.681 M rxns/s` WSL2 Linux on the same Intel Core Ultra 9 275HX; see [`docs/benchmarks.md`](benchmarks.md)). **Bit-exact SHA-256 determinism (`83317026...532c9bbe`) is 100% invariant to clock speed, OS, compiler, and ISA (x86-64 AVX2, x86-64 Scalar, macOS ARM64).**

### 6. Is this paper peer-reviewed or commercially restricted?
It is an **independent research preprint (not peer-reviewed)** authored by Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`). All code, Lean proofs, and scripts are open-source under **Apache-2.0** ([`LICENSE`](../LICENSE)), and all papers, PDFs, figures, and documentation are open under **CC-BY-4.0** ([`LICENSE-DOCS`](../LICENSE-DOCS), [`REUSE.toml`](../REUSE.toml)).

### 7. How was AI used in creating this repository?
Substantive AI engineering and formalization assistance (Google DeepMind / Antigravity agent under human direction) is transparently disclosed in [`docs/AI_DISCLOSURE.md`](AI_DISCLOSURE.md) and `paper/main.tex`. AI systems are not listed as authors, and claims are checked by different tools: the Lean theorems by six kernel checks (they cover the `Spec.lean` model only), the C engine by 3-OS native test gates, and citations by Crossref retraction checks (`results/citation-audit.json`). Lean does not check empirical or bibliographic claims.
