module

public import CellulogonicProofs.Spec
public import CellulogonicProofs.StoichiometryThermo
public import CellulogonicProofs.MembraneGeometry
public import CellulogonicProofs.BioenergeticsEndosymbiosis
public import CellulogonicProofs.ChemiosmosisAutopoiesis
public import CellulogonicProofs.EutelicOrganogenesis
public import CellulogonicProofs.CytoskeletonNanomechanics
public import CellulogonicProofs.MorphogeneticGRN
public import CellulogonicProofs.ContinuumBiomechanics
public import CellulogonicProofs.NonVacuity
public import CellulogonicProofs.AdversarialTests

/-!
# CELLULOGONIC MECHA v1.6.0 — Root Formal Verification Library (`CellulogonicProofs.lean`)

Aggregates all 11 pure Lean 4.35.0 `module` files (`Spec` + 8 core theorem modules +
`NonVacuity` + `AdversarialTests`, totaling 78 machine-checked theorems: `T01`–`T58`,
`C59`–`C62`, `NV01`–`NV08`, `AT01`–`AT08`), with zero Mathlib dependencies, zero `sorry`,
zero `native_decide`, `autoImplicit = false`, and `relaxedAutoImplicit = false`.

Axiom auditing (`#print axioms`) is run exclusively via standalone `VerifyAxioms.lean`
(`lake env lean VerifyAxioms.lean`) so library builds emit zero stdout noise while every
declaration's exact axiom footprint is recorded verbatim in `results/lean4-audit.txt`.

Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
SPDX-License-Identifier: Apache-2.0
-/
