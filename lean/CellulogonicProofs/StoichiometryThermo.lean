module

public import CellulogonicProofs.Spec

/-!
  CELLULOGONIC MECHA v1.7.0 — Theorem I: Exact Multi-Solvent Stoichiometric Mass
  Conservation, First-Law Energy Closure & Strict Second-Law Impossibility of
  Perpetual-Motion Cellular Cycles across ALL 4 Planetary Solvents.

  Pure Lean 4.35.0 `module` (zero Mathlib, zero sorry, zero custom axioms).
  Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
  SPDX-License-Identifier: Apache-2.0
-/

namespace Cellulogonic.StoichiometryThermo

/-- T01: Every elementary reaction in the chemoton (including salvage & abiotic synthesis)
    strictly conserves atomic mass (0 amu residual). -/
public theorem T01_all_reactions_conserve_mass (r : RxnId) : rxnMassDelta r = 0 := by
  cases r <;> rfl

/-- T02: Every elementary reaction obeys exact First-Law energy closure (0 uq residual)
    across ALL 4 planetary solvents and ANY rotor stoichiometry nH. -/
public theorem T02_all_reactions_first_law_exact (sv : Solvent) (nH : Int) (r : RxnId) :
    rxnChemDelta r + dPmf sv nH r * pmfPotS sv + qHeat sv nH r = eInflow r := by
  cases sv <;> cases r <;>
    simp only [rxnChemDelta, sumSpecies, stoich, muPot, dPmf, r2PmfYield, pmfPotS, qHeat, eInflow] <;>
    omega

/-- T03: STRICT Second-Law dissipation (`qHeat > 0`) across ALL 4 planetary solvents
    (including Titan azotosome `e_pmf = 8, nH ≥ 4 -> q ≥ +4 > 0`) whenever the rotor meets
    the solvent's minimal gearing `nH ≥ minCRing sv`. -/
public theorem T03_all_reactions_strict_second_law (sv : Solvent) (nH : Int)
    (hGear : nH ≥ minCRing sv) (r : RxnId) : qHeat sv nH r > 0 := by
  cases sv <;> cases r <;>
    simp only [qHeat, r2PmfYield, pmfPotS, minCRing] at hGear ⊢ <;>
    omega

/-- T04: Uncoupled macromolecule synthesis from substrate S alone is strictly endergonic
    (ΔG > 0), proving ATP/PMF free-energy coupling is thermodynamically mandatory. -/
public theorem T04_endergonic_biosynthesis_requires_atp :
    (muPot .L - 2 * muPot .S > 0) ∧
    (muPot .P - 3 * muPot .S > 0) ∧
    (muPot .T - 4 * muPot .S > 0) := by
  decide

/-- T05: Chemiosmotic oxidative respiration (`R1 + R2 + 2·R3` across Terran `nH=3`, Hydrothermal `nH=3`,
    and Venus `nH=2` with exact 0 net PMF) nets 2 ATP per 1 S, whereas anaerobic fermentation (`R4`)
    yields 1 ATP per 2 S — an exact 4× yield ratio. -/
public theorem T05_respiration_quadruples_fermentation_yield :
    (stoich .R1_catab .S + stoich .R2_pump .S + 2 * stoich .R3_atp_synth .S = -1) ∧
    (stoich .R1_catab .E + stoich .R2_pump .E + 2 * stoich .R3_atp_synth .E = 2) ∧
    (dPmf .Terran 3 .R1_catab + dPmf .Terran 3 .R2_pump + 2 * dPmf .Terran 3 .R3_atp_synth = 0) ∧
    (dPmf .Hydrothermal 3 .R1_catab + dPmf .Hydrothermal 3 .R2_pump + 2 * dPmf .Hydrothermal 3 .R3_atp_synth = 0) ∧
    (dPmf .Venus 2 .R1_catab + dPmf .Venus 2 .R2_pump + 2 * dPmf .Venus 2 .R3_atp_synth = 0) ∧
    (stoich .R4_ferment .S = -2) ∧
    (stoich .R4_ferment .E = 1) := by
  decide

/-- Exact trajectory First-Law balance lemma across all 4 planetary solvents. -/
private theorem traj_first_law (sv : Solvent) (nH : Int) (rs : List RxnId) :
    trajFreeEnergyDelta sv nH rs + trajHeat sv nH rs = trajInflow rs := by
  induction rs with
  | nil => rfl
  | cons r rest ih =>
    simp only [trajFreeEnergyDelta, trajHeat, trajInflow]
    have h := T02_all_reactions_first_law_exact sv nH r
    omega

/-- Non-negativity of trajectory heat dissipation across all 4 planetary solvents. -/
private theorem traj_heat_nonneg (sv : Solvent) (nH : Int) (hGear : nH ≥ minCRing sv) (rs : List RxnId) :
    trajHeat sv nH rs ≥ 0 := by
  induction rs with
  | nil => simp [trajHeat]
  | cons r rest ih =>
    simp only [trajHeat]
    have h := T03_all_reactions_strict_second_law sv nH hGear r
    omega

/-- T06: Impossibility of Perpetual-Motion Cellular Cycles (Second Law across ALL 4 Solvents).
    Any non-empty closed thermodynamic cycle (zero net free-energy change) strictly
    requires positive external free-energy inflow equal to its strictly positive heat dissipation. -/
public theorem T06_closed_box_cycle_dissipation_strict (sv : Solvent) (nH : Int)
    (hGear : nH ≥ minCRing sv) (r : RxnId) (rs : List RxnId)
    (hCycle : trajFreeEnergyDelta sv nH (r :: rs) = 0) :
    trajHeat sv nH (r :: rs) > 0 ∧ trajInflow (r :: rs) = trajHeat sv nH (r :: rs) := by
  have h1 := traj_first_law sv nH (r :: rs)
  have h2 := T03_all_reactions_strict_second_law sv nH hGear r
  have h3 := traj_heat_nonneg sv nH hGear rs
  simp only [trajHeat] at *
  constructor <;> omega

private theorem sumSpecies_add (f g : Species → Int) :
    sumSpecies (fun s => f s + g s) = sumSpecies f + sumSpecies g := by
  unfold sumSpecies
  dsimp only
  omega

private theorem sumSpecies_scale (c : Int) (f : Species → Int) :
    sumSpecies (fun s => c * f s) = c * sumSpecies f := by
  unfold sumSpecies
  simp only [Int.mul_add]

/-- T07: Applying any reaction k times preserves exact total atomic mass (0 residual). -/
public theorem T07_step_preserves_total_mass (sv : Solvent) (nH : Int) (st : WorldState) (r : RxnId) (k : Int) :
    stateMass (applyRxn sv nH st r k) = stateMass st := by
  unfold stateMass applyRxn
  have hdist :
      sumSpecies (fun s => massW s * (st.conc s + k * stoich r s)) =
      sumSpecies (fun s => massW s * st.conc s) +
      k * sumSpecies (fun s => massW s * stoich r s) := by
    rw [← sumSpecies_scale, ← sumSpecies_add]
    unfold sumSpecies
    simp only [Int.mul_add, Int.mul_left_comm]
  rw [hdist]
  have hm : sumSpecies (fun s => massW s * stoich r s) = 0 := T01_all_reactions_conserve_mass r
  rw [hm]
  omega

/-- T08: Applying any reaction k times preserves exact First-Law ledger closure
    (E_total' - E_inflow' = E_total - E_inflow, exact 0 uq residual) across ALL 4 solvents. -/
public theorem T08_step_preserves_first_law_ledger (sv : Solvent) (nH : Int) (st : WorldState) (r : RxnId) (k : Int) :
    stateEnergy sv (applyRxn sv nH st r k) - (applyRxn sv nH st r k).inflow =
    stateEnergy sv st - st.inflow := by
  unfold stateEnergy applyRxn
  have hdist :
      sumSpecies (fun s => muPot s * (st.conc s + k * stoich r s)) =
      sumSpecies (fun s => muPot s * st.conc s) +
      k * rxnChemDelta r := by
    unfold rxnChemDelta
    rw [← sumSpecies_scale, ← sumSpecies_add]
    unfold sumSpecies
    simp only [Int.mul_add, Int.mul_left_comm]
  rw [hdist]
  have hfl := T02_all_reactions_first_law_exact sv nH r
  have hmul : k * (rxnChemDelta r + dPmf sv nH r * pmfPotS sv + qHeat sv nH r) = k * eInflow r :=
    congrArg (k * ·) hfl
  simp only [Int.mul_add, Int.add_mul, Int.mul_assoc] at hmul ⊢
  omega

/-- T09: For any non-negative reaction firing k ≥ 0 and valid c-ring `nH ≥ minCRing sv`,
    the radiated heat sink is strictly monotone non-decreasing across ALL 4 solvents. -/
public theorem T09_heat_sink_monotone (sv : Solvent) (nH : Int)
    (hGear : nH ≥ minCRing sv) (st : WorldState) (r : RxnId) (k : Int) (hk : k ≥ 0) :
    (applyRxn sv nH st r k).heatSink ≥ st.heatSink := by
  unfold applyRxn
  dsimp only
  have hq := T03_all_reactions_strict_second_law sv nH hGear r
  have hprod : k * qHeat sv nH r ≥ 0 := Int.mul_nonneg hk (Int.le_of_lt hq)
  omega

/-- T10: Binary fission / phagocytic engulfment partitioning (c1 + c2 = c_tot) strictly
    conserves both total atomic mass and total chemical free energy. -/
public theorem T10_fission_and_engulfment_exact_conservation
    (c1 c2 : Species → Int) :
    sumSpecies (fun s => massW s * (c1 s + c2 s)) =
      sumSpecies (fun s => massW s * c1 s) + sumSpecies (fun s => massW s * c2 s) ∧
    sumSpecies (fun s => muPot s * (c1 s + c2 s)) =
      sumSpecies (fun s => muPot s * c1 s) + sumSpecies (fun s => muPot s * c2 s) := by
  constructor
  · rw [← sumSpecies_add]
    unfold sumSpecies
    simp only [Int.mul_add]
  · rw [← sumSpecies_add]
    unfold sumSpecies
    simp only [Int.mul_add]

end Cellulogonic.StoichiometryThermo
