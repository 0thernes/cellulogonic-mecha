module

/-!
# CellulogonicProofs.Spec — Pure Shared Definitions Only (Zero Proofs, Zero `sorry`)

Canonical mathematical, thermodynamic, geometric, bioenergetic, morphogenetic,
and continuum-biomechanics definitions shared identically by BOTH:
  - `Challenge.lean` (isolated statement-only specification — imports ONLY this file)
  - `CellulogonicProofs/*.lean` & `Solution.lean` (sorry-free proofs checked across 6 kernels)

Pure Lean 4.35.0 `module` (zero Mathlib, zero `sorry`, zero custom axioms).
Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
SPDX-License-Identifier: Apache-2.0
-/

-- ============================================================================
-- MODULE 1 DEFINITIONS: STOICHIOMETRY & 4-SOLVENT THERMODYNAMICS
-- ============================================================================
namespace Cellulogonic.StoichiometryThermo

/-- The 8 conserved cellular & environmental molecular species. -/
public inductive Species : Type where
  | S | X | M | E | L | P | T | W
  deriving DecidableEq, Repr

/-- All 4 Planetary & Xenocellular Solvent Regimes matching C11 `SOLVENT_TABLE`. -/
public inductive Solvent : Type where
  | Terran | Titan | Hydrothermal | Venus
  deriving DecidableEq, Repr

/-- Exact integer atomic mass weight w_i (in conserved amu quanta). -/
@[expose] public def massW : Species → Int
  | .S => 4
  | .X => 2
  | .M => 6
  | .E => 4
  | .L => 8
  | .P => 12
  | .T => 16
  | .W => 2

/-- Exact integer standard chemical free-energy potential μ_i (in uq / molecule). -/
@[expose] public def muPot : Species → Int
  | .S => 48
  | .X => 36
  | .M => 80
  | .E => 40
  | .L => 104
  | .P => 156
  | .T => 212
  | .W => 6

/-- Solvent-specific proton-motive force quantum e_pmf (uq per H+ equivalent). -/
@[expose] public def pmfPotS : Solvent → Int
  | .Terran       => 10
  | .Titan        => 8
  | .Hydrothermal => 10
  | .Venus        => 15

/-- Solvent-specific R2 proton pumping yield. -/
@[expose] public def r2PmfYield : Solvent → Int
  | .Terran       => 6
  | .Titan        => 7
  | .Hydrothermal => 6
  | .Venus        => 4

/-- Minimum thermodynamically valid ATP-synthase c-ring stoichiometry enforcing STRICT
    Second-Law dissipation (`nH * e_pmf > 28`) in each planetary solvent. -/
@[expose] public def minCRing : Solvent → Int
  | .Terran       => 3
  | .Titan        => 4
  | .Hydrothermal => 3
  | .Venus        => 2

/-- Terran baseline PMF quantum. -/
@[expose] public def pmfPot : Int := pmfPotS .Terran

/-- All 19 elementary chemoton, salvage, photochemical & abiotic synthesis reactions. -/
public inductive RxnId : Type where
  | R1_catab
  | R2_pump
  | R3_atp_synth
  | R4_ferment
  | R5_lipid_syn
  | R6_prot_syn
  | R7_tmpl_rep
  | R8_atp_hyd
  | R9_lipid_dec
  | R10_prot_dec
  | R11_tmpl_dec
  | R12_pmf_leak
  | R13a_salv_L
  | R13b_salv_P
  | R14a_photo_S
  | R14b_photo_X
  | R15a_abio_L
  | R15b_abio_P
  | R15c_abio_T
  deriving DecidableEq, Repr

/-- Exact integer stoichiometric matrix ν_{r,i} (products positive, reactants negative). -/
@[expose] public def stoich (r : RxnId) (s : Species) : Int :=
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
  | .R14a_photo_S, .W => -2 | .R14a_photo_S, .S => 1
  | .R14b_photo_X, .W => -1 | .R14b_photo_X, .X => 1
  | .R15a_abio_L, .S => -2 | .R15a_abio_L, .L => 1
  | .R15b_abio_P, .S => -3 | .R15b_abio_P, .P => 1
  | .R15c_abio_T, .S => -4 | .R15c_abio_T, .T => 1
  | _, _ => 0

/-- Net change in transmembrane proton-motive quanta Δp_r(sv, nH) per reaction event. -/
@[expose] public def dPmf (sv : Solvent) (nH : Int) : RxnId → Int
  | .R2_pump      => r2PmfYield sv
  | .R3_atp_synth => -nH
  | .R12_pmf_leak => -1
  | _             => 0

/-- Explicit external photon/geothermal free-energy inflow per reaction event (uq). -/
@[expose] public def eInflow : RxnId → Int
  | .R14a_photo_S => 46
  | .R14b_photo_X => 34
  | .R15a_abio_L  => 12
  | .R15b_abio_P  => 16
  | .R15c_abio_T  => 24
  | _             => 0

/-- Exact irreversible heat / entropy dissipation q_r(sv, nH) (uq) per reaction event. -/
@[expose] public def qHeat (sv : Solvent) (nH : Int) : RxnId → Int
  | .R1_catab     => 4
  | .R2_pump      => 62 - r2PmfYield sv * pmfPotS sv
  | .R3_atp_synth => nH * pmfPotS sv - 28
  | .R4_ferment   => 44
  | .R5_lipid_syn => 20
  | .R6_prot_syn  => 16
  | .R7_tmpl_rep  => 8
  | .R8_atp_hyd   => 28
  | .R9_lipid_dec => 80
  | .R10_prot_dec => 120
  | .R11_tmpl_dec => 164
  | .R12_pmf_leak => pmfPotS sv
  | .R13a_salv_L  => 44
  | .R13b_salv_P  => 48
  | .R14a_photo_S => 10
  | .R14b_photo_X => 4
  | .R15a_abio_L  => 4
  | .R15b_abio_P  => 4
  | .R15c_abio_T  => 4

/-- Sum over all 8 molecular species. -/
@[expose] public def sumSpecies (f : Species → Int) : Int :=
  f .S + f .X + f .M + f .E + f .L + f .P + f .T + f .W

/-- Atomic mass delta of reaction r: ∑_i w_i ν_{r,i}. -/
@[expose] public def rxnMassDelta (r : RxnId) : Int :=
  sumSpecies (fun s => massW s * stoich r s)

/-- Chemical free-energy delta of reaction r: ∑_i μ_i ν_{r,i}. -/
@[expose] public def rxnChemDelta (r : RxnId) : Int :=
  sumSpecies (fun s => muPot s * stoich r s)

/-- Total heat dissipated along a finite reaction trajectory in solvent `sv` with c-ring `nH`. -/
@[expose] public def trajHeat (sv : Solvent) (nH : Int) : List RxnId → Int
  | []      => 0
  | r :: rs => qHeat sv nH r + trajHeat sv nH rs

/-- Total external free-energy inflow along a reaction trajectory. -/
@[expose] public def trajInflow : List RxnId → Int
  | []      => 0
  | r :: rs => eInflow r + trajInflow rs

/-- Total chemical + PMF free-energy change along a reaction trajectory in solvent `sv`. -/
@[expose] public def trajFreeEnergyDelta (sv : Solvent) (nH : Int) : List RxnId → Int
  | []      => 0
  | r :: rs => (rxnChemDelta r + dPmf sv nH r * pmfPotS sv) + trajFreeEnergyDelta sv nH rs

/-- Cellular + spatial thermodynamic state for exact ledger verification. -/
public structure WorldState where
  conc     : Species → Int
  pmf      : Int
  heatSink : Int
  inflow   : Int

@[expose] public def stateMass (st : WorldState) : Int :=
  sumSpecies (fun s => massW s * st.conc s)

@[expose] public def stateEnergy (sv : Solvent) (st : WorldState) : Int :=
  sumSpecies (fun s => muPot s * st.conc s) + st.pmf * pmfPotS sv + st.heatSink

/-- Apply k ≥ 0 firings of reaction r to a WorldState in solvent `sv` with c-ring `nH`. -/
@[expose] public def applyRxn (sv : Solvent) (nH : Int) (st : WorldState) (r : RxnId) (k : Int) : WorldState :=
  { conc     := fun s => st.conc s + k * stoich r s
  , pmf      := st.pmf + k * dPmf sv nH r
  , heatSink := st.heatSink + k * qHeat sv nH r
  , inflow   := st.inflow + k * eInflow r }

end Cellulogonic.StoichiometryThermo

-- ============================================================================
-- MODULE 2 DEFINITIONS: CANHAM-HELFRICH VESICLE GEOMETRY
-- ============================================================================
namespace Cellulogonic.MembraneGeometry

@[expose] public def C0 : Nat := 100

@[expose] public def IsIntact (V L : Nat) : Prop :=
  C0 * V * V ≤ L * L * L

@[expose] public def CanSymmetricFission (V L : Nat) : Prop :=
  2 * C0 * V * V ≤ L * L * L

@[expose] public def ValidBinarySplit (V L V1 L1 V2 L2 : Nat) : Prop :=
  V1 > 0 ∧ V2 > 0 ∧ L1 > 0 ∧ L2 > 0 ∧
  V1 + V2 = V ∧ L1 + L2 = L ∧
  IsIntact V1 L1 ∧ IsIntact V2 L2

end Cellulogonic.MembraneGeometry

-- ============================================================================
-- MODULE 3 DEFINITIONS: BIOENERGETICS & ENDOSYMBIOSIS
-- ============================================================================
namespace Cellulogonic.BioenergeticsEndosymbiosis

@[expose] public def surfProxy (R : Nat) : Nat := 3 * R * R
@[expose] public def volProxy  (R : Nat) : Nat := R * R * R

@[expose] public def prokPower (cA R : Nat) : Nat := cA * surfProxy R

@[expose] public def maintCost (cV R : Nat) : Nat := cV * volProxy R

@[expose] public def eukPower (cA cEndo R K r : Nat) : Nat :=
  cA * surfProxy R + K * (cEndo * surfProxy r)

end Cellulogonic.BioenergeticsEndosymbiosis

-- ============================================================================
-- MODULE 4 DEFINITIONS: CHEMIOSMOSIS, BIOELECTRICITY & ANTI-GAMING COVI
-- ============================================================================
namespace Cellulogonic.ChemiosmosisAutopoiesis

@[expose] public def dG_ATP : Nat := 28

@[expose] public def coviGated (rawScore metabolicFlux templateCount activeCatalysts : Nat) : Nat :=
  let autopoieticGate := if metabolicFlux > 0 ∧ templateCount > 0 then 1 else 0
  let antiNoiseGate   := if activeCatalysts > 0 then 1 else 0
  rawScore * autopoieticGate * antiNoiseGate

@[expose] public def transportFlux (extAvail cap : Nat) : Nat :=
  min extAvail cap

end Cellulogonic.ChemiosmosisAutopoiesis

-- ============================================================================
-- MODULE 5 DEFINITIONS: 3D TUBE GEOMETRY, EUTELIC 7-ORGAN BUDGET & BIOGLASS
-- ============================================================================
namespace Cellulogonic.EutelicOrganogenesis

@[expose] public def EnclosesLumen (ringCells : Nat) : Prop :=
  ringCells ≥ 3

@[expose] public def tubeCellCount (ringCells lengthRings : Nat) : Nat :=
  ringCells * lengthRings

public structure OrganBudget where
  epidermis    : Nat
  nervous      : Nat
  musculature  : Nat
  gut          : Nat
  excretory    : Nat
  reproductive : Nat
  hydrostatic  : Nat

@[expose] public def totalCells (b : OrganBudget) : Nat :=
  b.epidermis + b.nervous + b.musculature + b.gut +
  b.excretory + b.reproductive + b.hydrostatic

@[expose] public def MeetsMinimal7Organ (b : OrganBudget) : Prop :=
  b.epidermis ≥ 150 ∧ b.nervous ≥ 100 ∧ b.musculature ≥ 50 ∧
  b.gut ≥ 20 ∧ b.excretory ≥ 5 ∧ b.reproductive ≥ 10 ∧ b.hydrostatic ≥ 20

@[expose] public def MeetsStandardEutelic (b : OrganBudget) : Prop :=
  b.epidermis ≥ 200 ∧ b.nervous ≥ 150 ∧ b.musculature ≥ 60 ∧
  b.gut ≥ 30 ∧ b.excretory ≥ 10 ∧ b.reproductive ≥ 20 ∧ b.hydrostatic ≥ 30 ∧
  totalCells b ≤ 1000

@[expose] public def isVitrified (waterPct wCrit tdpCount pMin : Nat) : Prop :=
  waterPct ≤ wCrit ∧ tdpCount ≥ pMin

@[expose] public def effectiveDecay (baseDecay glassFactor : Nat) (vitrified : Bool) : Nat :=
  if vitrified then baseDecay / glassFactor else baseDecay

end Cellulogonic.EutelicOrganogenesis

-- ============================================================================
-- MODULE 6 DEFINITIONS: CYTOSKELETAL NANOMECHANICS & FRÖHLICH ODE
-- ============================================================================
namespace Cellulogonic.CytoskeletonNanomechanics

@[expose] public def frohlichCoherentExcess (sPump sCrit : Nat) : Nat :=
  if sPump > sCrit then sPump - sCrit else 0

@[expose] public def frohlichStep (coh sPump sCrit : Nat) : Nat :=
  if sPump > sCrit then
    let target := Nat.min 1000 (sPump - sCrit)
    Nat.min 1000 ((coh * 3 + target + 2) / 4)
  else
    coh / 4

end Cellulogonic.CytoskeletonNanomechanics

-- ============================================================================
-- MODULE 7 DEFINITIONS: MORPHOGENETIC GRN, EUTELIC TRACE, HIPPO, AXONS, CPG
-- ============================================================================
namespace Cellulogonic.MorphogeneticGRN

open Cellulogonic.StoichiometryThermo

public inductive GermFate where
  | gutEndoderm
  | muscleMesoderm
  | neuralEctoderm
  deriving DecidableEq, Repr

@[expose] public def frenchFlagFate (tLow tHigh c : Nat) : GermFate :=
  if c < tLow then GermFate.gutEndoderm
  else if c < tHigh then GermFate.muscleMesoderm
  else GermFate.neuralEctoderm

@[expose] public def interfacialEnergy (jAA jBB jAB nAA nBB nAB : Int) : Int :=
  jAA * nAA + jBB * nBB + jAB * nAB

@[expose] public def eutelicBudget : List Nat → Nat
  | []      => 0
  | d :: ds => 2 ^ d + eutelicBudget ds

public inductive EutelicStep : List Nat → List Nat → Prop where
  | divide (pre post : List Nat) (d : Nat) :
      EutelicStep (pre ++ (d + 1) :: post) (pre ++ d :: d :: post)
  | die (pre post : List Nat) (d : Nat) :
      EutelicStep (pre ++ d :: post) (pre ++ post)

public inductive EutelicTrace : List Nat → List Nat → Prop where
  | refl (pop : List Nat) : EutelicTrace pop pop
  | step {p1 p2 p3 : List Nat} : EutelicStep p1 p2 → EutelicTrace p2 p3 → EutelicTrace p1 p3

@[expose] public def hippoUpdate (tf7 pressure hippoLim : Nat) : Nat :=
  let decayed := (tf7 * 30) / 32
  if pressure ≥ hippoLim then Nat.min 255 (decayed + 64) else decayed

@[expose] public def hippoMitosisPermitted (tf7 eutelicRem : Nat) : Bool :=
  decide (tf7 < 165 ∧ 0 < eutelicRem)

@[expose] public def updateSynapseWeight (w decay ltpGain : Nat) (fired : Bool) : Nat :=
  let w1 := if fired then Nat.min 255 (w + ltpGain) else w
  w1 - decay

public structure MorphWorld where
  thermo    : WorldState
  tfActive  : Nat
  cadType   : Nat
  axonCount : Nat
  cpgPhase  : Int

public inductive MorphStep where
  | signalUpdate (tf' cad' ax' : Nat) (cpg' : Int)
  | cpgAtpHyd    (k : Int) (hk : k ≥ 0) (cpg' : Int)

@[expose] public def applyMorphStep (sv : Solvent) (nH : Int) (mw : MorphWorld) : MorphStep → MorphWorld
  | .signalUpdate tf' cad' ax' cpg' =>
      { mw with tfActive := tf', cadType := cad', axonCount := ax', cpgPhase := cpg' }
  | .cpgAtpHyd k _ cpg' =>
      { mw with thermo := applyRxn sv nH mw.thermo .R8_atp_hyd k, cpgPhase := cpg' }

@[expose] public def runMorphTrace (sv : Solvent) (nH : Int) (mw : MorphWorld) : List MorphStep → MorphWorld
  | []         => mw
  | st :: rest => runMorphTrace sv nH (applyMorphStep sv nH mw st) rest

end Cellulogonic.MorphogeneticGRN

-- ============================================================================
-- MODULE 8 DEFINITIONS: CONTINUUM BIOMECHANICS
-- ============================================================================
namespace Cellulogonic.ContinuumBiomechanics

@[expose] public def xpbdDissipateStep (eDissNum dE : Int) : Int :=
  eDissNum + dE

@[expose] public def xpbdDissipateTrace (eDissNum : Int) : List Int → Int
  | []        => eDissNum
  | dE :: rem => xpbdDissipateTrace (xpbdDissipateStep eDissNum dE) rem

public structure EcoMassLedger where
  massSoma     : Int
  massStores   : Int
  massDetritus : Int
  massAmbient  : Int

@[expose] public def EcoMassLedger.totalMass (L : EcoMassLedger) : Int :=
  L.massSoma + L.massStores + L.massDetritus + L.massAmbient

@[expose] public def carcassDeathTransfer (L : EcoMassLedger) : EcoMassLedger :=
  { massSoma     := 0
    massStores   := 0
    massDetritus := L.massDetritus + L.massSoma + L.massStores
    massAmbient  := L.massAmbient }

@[expose] public def listSum : List Nat → Nat
  | []      => 0
  | x :: xs => x + listSum xs

end Cellulogonic.ContinuumBiomechanics
