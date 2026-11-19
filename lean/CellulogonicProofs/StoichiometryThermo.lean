/-
  CELLULOGONIC MECHA v1.0 — Theorem I: Exact Stoichiometric Mass Conservation,
  First-Law Energy Closure & Second-Law Impossibility of Perpetual-Motion Cellular Cycles.

  Pure Lean 4 (v4.35.0, zero Mathlib, zero sorry, zero custom axioms).
  Copyright (c) 2026 0thernes (0thernes LLC) <0_0@0thernes.art>.
-/

namespace Cellulogonic.StoichiometryThermo

/-- The 8 conserved cellular & environmental molecular species:
    0: S (Substrate / Electron Donor)
    1: X (Oxidant / Terminal Electron Acceptor)
    2: M (Reduced Metabolic Cofactor)
    3: E (Energy Currency — ATP / Polyphosphate / Thioester)
    4: L (Amphiphilic Membrane Lipid / Azotosome / Tetraether)
    5: P (Catalytic Polypeptide / Enzyme / Transporter)
    6: T (Informational Template Polymer — RNA / DNA / XNA)
    7: W (Low-Free-Energy Exhaust / Waste) -/
inductive Species : Type where
  | S | X | M | E | L | P | T | W
  deriving DecidableEq, Repr

/-- Exact integer atomic mass weight w_i (in conserved amu quanta). -/
def massW : Species → Int
  | .S => 4
  | .X => 2
  | .M => 6
  | .E => 4
  | .L => 8
  | .P => 12
  | .T => 16
  | .W => 2

/-- Exact integer standard chemical free-energy potential μ_i (in uq / molecule). -/
def muPot : Species → Int
  | .S => 48
  | .X => 36
  | .M => 80
  | .E => 40
  | .L => 104
  | .P => 156
  | .T => 212
  | .W => 6

/-- Exact Terran proton-motive force quantum (uq per transmembrane H+ equivalent). -/
def pmfPot : Int := 10

/-- All 16 elementary cellular reactions in the Cellulogonic Mecha chemoton. -/
inductive RxnId : Type where
  | R1_catab       -- S + X -> M
  | R2_pump        -- M -> 3 W + 6 PMF
  | R3_atp_synth   -- 2 W + 3 PMF -> E
  | R4_ferment     -- 2 S -> E + 2 W
  | R5_lipid_syn   -- 2 S + E -> L + 2 W
  | R6_prot_syn    -- 3 S + E -> P + 2 W
  | R7_tmpl_rep    -- 4 S + E -> T + 2 W
  | R8_atp_hyd     -- E -> 2 W
  | R9_lipid_dec   -- L -> 4 W
  | R10_prot_dec   -- P -> 6 W
  | R11_tmpl_dec   -- T -> 8 W
  | R12_pmf_leak   -- 1 PMF -> Heat
  | R13a_salv_L    -- L -> S + 2 W
  | R13b_salv_P    -- P -> 2 S + 2 W
  | R13c_salv_T    -- T -> 3 S + 2 W
  | R14a_photo_S   -- 2 W + 46 uq_ext -> S
  | R14b_photo_X   -- W + 34 uq_ext -> X
  deriving DecidableEq, Repr

/-- Exact integer stoichiometric matrix ν_{r,i} (products positive, reactants negative). -/
def stoich (r : RxnId) (s : Species) : Int :=
  match r, s with
  | .R1_catab, .S => -1 | .R1_catab, .X => -1 | .R1_catab, .M => 1
  | .R2_pump,  .M => -1 | .R2_pump,  .W => 3
  | .R3_atp_synth, .W => -2 | .R3_atp_synth, .E => 1
  | .R4_ferment, .S => -2 | .R4_ferment, .E => 1 | .R4_ferment, .W => 2
  | .R5_lipid_syn, .S => -2 | .R5_lipid_syn, .E => -1 | .R5_lipid_syn, .L => 1 | .R5_lipid_syn, .W => 2
  | .R6_prot_syn, .S => -3 | .R6_prot_syn, .E => -1 | .R6_prot_syn, .P => 1 | .R6_prot_syn, .W => 2
  | .R7_tmpl_rep, .S => -4 | .R7_tmpl_rep, .E => -1 | .R7_tmpl_rep, .T => 1 | .R7_tmpl_rep, .W => 2
  | .R8_atp_hyd, .E => -1 | .R8_atp_hyd, .W => 2
  | .R9_lipid_dec, .L => -1 | .R9_lipid_dec, .W => 4
  | .R10_prot_dec, .P => -1 | .R10_prot_dec, .W => 6
  | .R11_tmpl_dec, .T => -1 | .R11_tmpl_dec, .W => 8
  | .R13a_salv_L, .L => -1 | .R13a_salv_L, .S => 1 | .R13a_salv_L, .W => 2
  | .R13b_salv_P, .P => -1 | .R13b_salv_P, .S => 2 | .R13b_salv_P, .W => 2
  | .R13c_salv_T, .T => -1 | .R13c_salv_T, .S => 3 | .R13c_salv_T, .W => 2
  | .R14a_photo_S, .W => -2 | .R14a_photo_S, .S => 1
  | .R14b_photo_X, .W => -1 | .R14b_photo_X, .X => 1
  | _, _ => 0

/-- Net change in transmembrane proton-motive quanta Δp_r per reaction event. -/
def dPmf : RxnId → Int
  | .R2_pump      => 6
  | .R3_atp_synth => -3
  | .R12_pmf_leak => -1
  | _             => 0

/-- Explicit external photon/geothermal free-energy inflow per reaction event (uq). -/
def eInflow : RxnId → Int
  | .R14a_photo_S => 46
  | .R14b_photo_X => 34
  | _             => 0

/-- Exact irreversible heat / entropy dissipation q_r (uq) per reaction event. -/
def qHeat : RxnId → Int
  | .R1_catab     => 4
  | .R2_pump      => 2
  | .R3_atp_synth => 2
  | .R4_ferment   => 44
  | .R5_lipid_syn => 20
  | .R6_prot_syn  => 16
  | .R7_tmpl_rep  => 8
  | .R8_atp_hyd   => 28
  | .R9_lipid_dec => 80
  | .R10_prot_dec => 120
  | .R11_tmpl_dec => 164
  | .R12_pmf_leak => 10
  | .R13a_salv_L  => 44
  | .R13b_salv_P  => 48
  | .R13c_salv_T  => 56
  | .R14a_photo_S => 10
  | .R14b_photo_X => 4

/-- Sum over all 8 molecular species. -/
def sumSpecies (f : Species → Int) : Int :=
  f .S + f .X + f .M + f .E + f .L + f .P + f .T + f .W

/-- Atomic mass delta of reaction r: ∑_i w_i ν_{r,i}. -/
def rxnMassDelta (r : RxnId) : Int :=
  sumSpecies (fun s => massW s * stoich r s)

/-- Chemical free-energy delta of reaction r: ∑_i μ_i ν_{r,i}. -/
def rxnChemDelta (r : RxnId) : Int :=
  sumSpecies (fun s => muPot s * stoich r s)

/-- T01: Every elementary reaction in the chemoton strictly conserves atomic mass. -/
theorem T01_all_reactions_conserve_mass (r : RxnId) : rxnMassDelta r = 0 := by
  cases r <;> rfl

/-- T02: Every elementary reaction obeys exact First-Law energy closure (0 uq residual). -/
theorem T02_all_reactions_first_law_exact (r : RxnId) :
    rxnChemDelta r + dPmf r * pmfPot + qHeat r = eInflow r := by
  cases r <;> rfl

/-- T03: Every elementary reaction dissipates strictly positive irreversible heat (Second Law). -/
theorem T03_all_reactions_strict_second_law (r : RxnId) : qHeat r > 0 := by
  cases r <;> decide

/-- T04: Uncoupled macromolecule synthesis from substrate S alone is strictly endergonic
    (ΔG > 0), proving ATP/PMF free-energy coupling is thermodynamically mandatory. -/
theorem T04_endergonic_biosynthesis_requires_atp :
    (muPot .L - 2 * muPot .S > 0) ∧
    (muPot .P - 3 * muPot .S > 0) ∧
    (muPot .T - 4 * muPot .S > 0) := by
  decide

/-- T05: Chemiosmotic oxidative respiration (R1 + R2 + 2·R3) nets 2 ATP per 1 S,
    whereas anaerobic fermentation (R4) yields 1 ATP per 2 S — an exact 4× yield ratio. -/
theorem T05_respiration_quadruples_fermentation_yield :
    (stoich .R1_catab .S + stoich .R2_pump .S + 2 * stoich .R3_atp_synth .S = -1) ∧
    (stoich .R1_catab .E + stoich .R2_pump .E + 2 * stoich .R3_atp_synth .E = 2) ∧
    (dPmf .R1_catab + dPmf .R2_pump + 2 * dPmf .R3_atp_synth = 0) ∧
    (stoich .R4_ferment .S = -2) ∧
    (stoich .R4_ferment .E = 1) := by
  decide

/-- Total heat dissipated along a finite reaction trajectory. -/
def trajHeat : List RxnId → Int
  | []      => 0
  | r :: rs => qHeat r + trajHeat rs

/-- Total external free-energy inflow along a reaction trajectory. -/
def trajInflow : List RxnId → Int
  | []      => 0
  | r :: rs => eInflow r + trajInflow rs

/-- Total chemical + PMF free-energy change along a reaction trajectory. -/
def trajFreeEnergyDelta : List RxnId → Int
  | []      => 0
  | r :: rs => (rxnChemDelta r + dPmf r * pmfPot) + trajFreeEnergyDelta rs

/-- Exact trajectory First-Law balance lemma. -/
theorem traj_first_law (rs : List RxnId) :
    trajFreeEnergyDelta rs + trajHeat rs = trajInflow rs := by
  induction rs with
  | nil => rfl
  | cons r rest ih =>
    simp only [trajFreeEnergyDelta, trajHeat, trajInflow]
    have h := T02_all_reactions_first_law_exact r
    omega

/-- Non-negativity of trajectory heat dissipation. -/
theorem traj_heat_nonneg (rs : List RxnId) : trajHeat rs ≥ 0 := by
  induction rs with
  | nil => decide
  | cons r rest ih =>
    simp only [trajHeat]
    have h := T03_all_reactions_strict_second_law r
    omega

/-- T06: Impossibility of Perpetual-Motion Cellular Cycles (Second Law).
    Any non-empty closed thermodynamic cycle (zero net free-energy change) strictly
    requires positive external free-energy inflow equal to its strictly positive heat dissipation. -/
theorem T06_closed_box_cycle_dissipation_strict (r : RxnId) (rs : List RxnId)
    (hCycle : trajFreeEnergyDelta (r :: rs) = 0) :
    trajHeat (r :: rs) > 0 ∧ trajInflow (r :: rs) = trajHeat (r :: rs) := by
  have h1 := traj_first_law (r :: rs)
  have h2 := T03_all_reactions_strict_second_law r
  have h3 := traj_heat_nonneg rs
  simp only [trajHeat] at *
  constructor
  · omega
  · omega

/-- Cellular + spatial thermodynamic state for exact ledger verification. -/
structure WorldState where
  conc     : Species → Int
  pmf      : Int
  heatSink : Int
  inflow   : Int

def stateMass (st : WorldState) : Int :=
  sumSpecies (fun s => massW s * st.conc s)

def stateEnergy (st : WorldState) : Int :=
  sumSpecies (fun s => muPot s * st.conc s) + st.pmf * pmfPot + st.heatSink

/-- Apply k ≥ 0 firings of reaction r to a WorldState. -/
def applyRxn (st : WorldState) (r : RxnId) (k : Int) : WorldState :=
  { conc     := fun s => st.conc s + k * stoich r s
  , pmf      := st.pmf + k * dPmf r
  , heatSink := st.heatSink + k * qHeat r
  , inflow   := st.inflow + k * eInflow r }

/-- Helper: sumSpecies distributes over linear combinations. -/
theorem sumSpecies_add (f g : Species → Int) :
    sumSpecies (fun s => f s + g s) = sumSpecies f + sumSpecies g := by
  unfold sumSpecies
  dsimp only
  omega

theorem sumSpecies_scale (c : Int) (f : Species → Int) :
    sumSpecies (fun s => c * f s) = c * sumSpecies f := by
  unfold sumSpecies
  simp only [Int.mul_add]

/-- T07: Applying any reaction k times preserves exact total atomic mass (0 residual). -/
theorem T07_step_preserves_total_mass (st : WorldState) (r : RxnId) (k : Int) :
    stateMass (applyRxn st r k) = stateMass st := by
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
    (E_total' - E_inflow' = E_total - E_inflow, exact 0 uq residual). -/
theorem T08_step_preserves_first_law_ledger (st : WorldState) (r : RxnId) (k : Int) :
    stateEnergy (applyRxn st r k) - (applyRxn st r k).inflow =
    stateEnergy st - st.inflow := by
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
  have hfl := T02_all_reactions_first_law_exact r
  have hmul : k * (rxnChemDelta r + dPmf r * pmfPot + qHeat r) = k * eInflow r :=
    congrArg (k * ·) hfl
  simp only [Int.mul_add, Int.add_mul, Int.mul_assoc] at hmul ⊢
  omega

/-- T09: For any non-negative reaction firing k ≥ 0, the radiated heat sink is strictly
    monotone non-decreasing (one-way Second-Law arrow). -/
theorem T09_heat_sink_monotone (st : WorldState) (r : RxnId) (k : Int) (hk : k ≥ 0) :
    (applyRxn st r k).heatSink ≥ st.heatSink := by
  unfold applyRxn
  dsimp only
  have hq := T03_all_reactions_strict_second_law r
  have hprod : k * qHeat r ≥ 0 := Int.mul_nonneg hk (Int.le_of_lt hq)
  omega

/-- T10: Binary fission / phagocytic engulfment partitioning (c1 + c2 = c_tot) strictly
    conserves both total atomic mass and total chemical free energy. -/
theorem T10_fission_and_engulfment_exact_conservation
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
