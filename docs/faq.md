# Frequently Asked Questions (`docs/faq.md`)

### 1. Does `CELLULOGONIC MECHA` claim to solve abiogenesis, consciousness, or any Mega-Atlas open problem?
**No.** As explicitly recorded in [`docs/MEGA_ATLAS_CROSSCHECK.md`](MEGA_ATLAS_CROSSCHECK.md) and [`CLAIMS.md`](CLAIMS.md), **`0 / 1,900`** overarching open problems in `0thernes/unsolved-problems-mega-atlas` are claimed solved or retired. This repository is a conservation-exact, Lean-verified discrete computational laboratory, not wet-lab biology or proof of a new physical law.

### 2. Is the Fröhlich–Pokorný GHz microtubule module a quantum-consciousness (Orch-OR) claim?
**Strictly no.** `frohlichStep` (`T48`, `src/chemoton_physics.c`) is a purely classical, deterministic integer relaxation recurrence gated on metabolic ATP pumping (`sPump > sCrit`) with inverse-length standing-wave scaling (`T49`). No quantum-coherence or consciousness hypothesis is endorsed or claimed.

### 3. Why do `T01`–`T58` and `C59`–`C62` use `Classical.choice` in some theorems?
In Lean 4.35.0, the `grind` and `omega` tactics over integer/polynomial rings occasionally pull `Classical.choice` alongside `propext` and `Quot.sound` (18 of 70 theorems use `[propext, Classical.choice, Quot.sound]`; 42 use `[propext, Quot.sound]` or `[propext]`; 10 use zero axioms at all). All 70 theorems sit strictly inside Lean's standard three-axiom core with **zero `sorryAx`**, **zero `native_decide`**, **zero `Lean.ofReduceBool`**, and **zero custom axioms**, independently replayed by `leanchecker-paranoid`, `con-leche`, `con-ron`, and `nanoda_bin` (`unpermitted_axiom_hard_error: true`).

### 4. Why does local Windows multi-kernel replay run 5 checkers while CI runs all 6?
On Windows 11, Lean 4.35.0's `lean4lean.exe` cannot parse Windows `.olean` path headers and `lake check` / `lake comparator` require Linux `bubblewrap` (`bwrap`). Therefore, local Windows verification runs `lean`, `leanchecker --fresh`, `leanchecker-paranoid --fresh`, `con-leche --verified`, `con-ron --verified`, and `nanoda_bin` (logged in [`results/lean4-multi-kernel.txt`](../results/lean4-multi-kernel.txt)), while `.github/workflows/ci.yml` runs all **6/6 kernels** (adding `lean4lean`, `lake check --paranoid`, and `lake comparator --config comparator.json --paranoid`) on `ubuntu-latest` with `bubblewrap`.

### 5. Why does my local throughput differ from `33.50 M rxns/s`?
Wall-clock throughput depends on CPU architecture, turbo clock, and background load (e.g., `22.64 M rxns/s` Win64 vs. `15.75 M rxns/s` WSL2 Linux on the same Intel Core Ultra 9 275HX; see [`docs/benchmarks.md`](benchmarks.md)). **Bit-exact SHA-256 determinism (`681a9747...519eceff`) is 100% invariant to clock speed and load.**

### 6. Is this paper peer-reviewed or commercially restricted?
It is an **independent research preprint (not peer-reviewed)** authored by Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`). As of `v1.5.0`, all code, Lean proofs, and scripts are open-source under **Apache-2.0** ([`LICENSE`](../LICENSE)), and all papers, PDFs, figures, and documentation are open under **CC-BY-4.0** ([`LICENSE-DOCS`](../LICENSE-DOCS)).

### 7. How was AI used in creating this repository?
Substantive AI engineering and formalization assistance (Google DeepMind / Antigravity agent under human direction) is transparently disclosed in [`docs/AI_DISCLOSURE.md`](AI_DISCLOSURE.md) and `paper/main.tex`. AI systems are not listed as authors, and every formal, empirical, and bibliographic claim is independently verified by Lean kernels, native test gates, and live DOI handle checks.
