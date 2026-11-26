<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# CELLULOGONIC MECHA v1.6.0 — Six-Kernel Pure Lean 4.35.0 Formal Proof Suite

This directory contains **78 machine-checked audited theorems** (`62` core theorems `T01`–`T58`, `C59`–`C62` across 8 biophysical modules + `8` concrete `NonVacuity` witness theorems `NV01`–`NV08` in Module 9 + `8` adversarial false-variant refutations `AT01`–`AT08` in Module 10) built using **core Lean 4 (`v4.35.0`) only** (zero Mathlib dependencies, `autoImplicit = false`, `relaxedAutoImplicit = false`, `linter.unusedVariables = true`, `linter.deprecated = true`, zero `sorryAx` in `Solution`, zero `native_decide`, zero custom axioms — strictly within `{propext, Classical.choice, Quot.sound}`).

## Palomar Isolated Specification & Six-Kernel Architecture

| File | Role |
|---|---|
| [`CellulogonicProofs/Spec.lean`](CellulogonicProofs/Spec.lean) | Pure shared mathematical definitions only (`module`, `public`, ZERO proofs, ZERO `sorry`; `12` core definitions pinned in `comparator.json`) |
| [`Challenge.lean`](Challenge.lean) | Isolated statement-only specification importing ONLY `CellulogonicProofs.Spec` (`78` intentional `sorry` targets matched by `lake comparator`) |
| [`Solution.lean`](Solution.lean) | Sorry-free solution entrypoint publicly re-exporting all 78 theorems from `CellulogonicProofs` |
| [`VerifyAxioms.lean`](VerifyAxioms.lean) | Standalone `#print axioms` audit across all 78 theorems (`../results/lean4-audit.txt`) |
| [`comparator.json`](comparator.json) | Machine-readable Palomar comparator config (`Challenge` vs `Solution`, 78 theorem names + 12 pinned `definition_names`, permitted axioms) |
| [`formalization.yaml`](formalization.yaml) | Standardized `v0.4`-schema-validated formalization metadata, scope boundaries, and CRediT/AI disclosure |

## Build, Multi-Kernel Replay & Live Negative Tamper Test

```bash
cd lean
lake build
lake build Challenge
lake env lean VerifyAxioms.lean > ../results/lean4-audit.txt
lake env leanchecker --fresh Solution
lake env leanchecker-paranoid --fresh Solution
# On Linux with bubblewrap (automated in .github/workflows/ci.yml):
lake check --paranoid
lake comparator --config comparator.json --paranoid
cd ..
python3 scripts/check_readme_axioms.py
python3 scripts/test_lean_negative.py
```

Archived verification transcripts:
- [`../results/lean4-audit.txt`](../results/lean4-audit.txt) (verbatim 78-theorem `#print axioms` log)
- [`../results/lean4-multi-kernel.txt`](../results/lean4-multi-kernel.txt) (independent multi-kernel replay & 6-kernel Palomar transcript)

## Module & Theorem Index (`78 / 78` Audited Theorems)

| Group | Module | Theorems | Machine-Checked Invariants |
|---|---|---|---|
| **Spec** | [`CellulogonicProofs/Spec.lean`](CellulogonicProofs/Spec.lean) | `12` Pinned Defs | 8 species, 4 solvents, 19 reactions, Canham–Helfrich, bioenergetics, COVI, 7-organ budget, Fröhlich ODE, GRN/Hippo/CPG, continuum biomechanics |
| **I** | [`CellulogonicProofs/StoichiometryThermo.lean`](CellulogonicProofs/StoichiometryThermo.lean) | `T01`–`T10` | 4-solvent 19-rxn exact mass (`0 amu`), 1st-Law (`0 uq`), strict 2nd-Law (`q_r > 0`, Titan `e_pmf=8`), ATP coupling, 3-solvent 4× respiration yield, no perpetual cycles |
| **II** | [`CellulogonicProofs/MembraneGeometry.lean`](CellulogonicProofs/MembraneGeometry.lean) | `T11`–`T19` | Canham–Helfrich $Q \le 1/2$ symmetric threshold, interphase window, constructive exosome escape, sextic **Global Minimax Theorem** (`T16`), osmotic lysis |
| **III** | [`CellulogonicProofs/BioenergeticsEndosymbiosis.lean`](CellulogonicProofs/BioenergeticsEndosymbiosis.lean) | `T20`–`T27` | Lane $1/R$ ceiling, endosymbiotic $O(1)$ liberation, $R/r$ amplification, secondary chimeras, heteroplasmy purifying selection, EGT |
| **IV** | [`CellulogonicProofs/ChemiosmosisAutopoiesis.lean`](CellulogonicProofs/ChemiosmosisAutopoiesis.lean) | `T28`–`T36` | Strict 2nd-Law c-ring floors (Terran $\ge 3$, Venus $\ge 2$, Titan $\ge 4$), Levin gap-junction Dirichlet dissipation, crystal/noise `COVI = 0`, Markov blanket |
| **V** | [`CellulogonicProofs/EutelicOrganogenesis.lean`](CellulogonicProofs/EutelicOrganogenesis.lean) | `T37`–`T44` | 3D hollow-tube lumen $\ge 3L$, 7-organ budget ($\ge 355$, 800-cell witness), syncytial compression, hydrostatic skeleton, TDP bioglass anhydrobiosis |
| **VI** | [`CellulogonicProofs/CytoskeletonNanomechanics.lean`](CellulogonicProofs/CytoskeletonNanomechanics.lean) | `T45`–`T50` | Stokes dissipation, 3D Hertz momentum, prolate area excess, stateful Fröhlich–Pokorný GHz relaxation ODE (`T48`), standing-wave scaling, ATP work bound |
| **VII** | [`CellulogonicProofs/MorphogeneticGRN.lean`](CellulogonicProofs/MorphogeneticGRN.lean) | `T51`–`T58` | Wolpert French-Flag, Steinberg sorting, apical wedge, eutelic tree invariant (`T54`), Hippo TF7 hysteresis (`T55`), Hebbian pruning, CPG phase (`T57`), `R8` zero-drift (`T58`) |
| **VIII** | [`CellulogonicProofs/ContinuumBiomechanics.lean`](CellulogonicProofs/ContinuumBiomechanics.lean) | `C59`–`C62` | XPBD numerical damping monotonicity (`C59`), 100% carcass-to-detritus conservation (`C60`), maternal yolk cleavage enthalpy (`C61`), biotensegrity elasticity (`C62`) |
| **IX** | [`CellulogonicProofs/NonVacuity.lean`](CellulogonicProofs/NonVacuity.lean) | `NV01`–`NV08` | Concrete inhabitants & sharp negative-boundary falsifiers proving non-vacuity of every hypothesis bundle across Groups I–VIII |
| **X** | [`CellulogonicProofs/AdversarialTests.lean`](CellulogonicProofs/AdversarialTests.lean) | `AT01`–`AT08` | Machine-checked negations (`¬P`) refuting weakened or sign-flipped variants across Groups I–VIII |
