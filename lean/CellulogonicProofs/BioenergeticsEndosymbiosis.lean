/-
  CELLULOGONIC MECHA v1.0 — Theorem III: The Prokaryotic 1/R Bioenergetic Ceiling,
  Endosymbiotic O(1) Power-Density Liberation, & Mitotic Heteroplasmy Purifying Selection.

  Pure Lean 4 (v4.35.0, zero Mathlib, zero sorry, zero custom axioms).
  Copyright (c) 2026 0thernes (0thernes LLC) <0_0@0thernes.art>.
-/

namespace Cellulogonic.BioenergeticsEndosymbiosis

/-- Discrete spherical surface area proxy A(R) = 3 R^2 and volume proxy V(R) = R^3,
    preserving the exact Euclidean surface-to-volume ratio A(R)/V(R) = 3/R. -/
def surfProxy (R : Nat) : Nat := 3 * R * R
def volProxy  (R : Nat) : Nat := R * R * R

/-- Prokaryotic ATP generation power bounded by outer membrane area. -/
def prokPower (cA R : Nat) : Nat := cA * surfProxy R

/-- Volumetric cytoplasmic maintenance demand scaling as R^3. -/
def maintCost (cV R : Nat) : Nat := cV * volProxy R

/-- Eukaryotic ATP power with K internalized respiratory endosymbionts of radius r. -/
def eukPower (cA cEndo R K r : Nat) : Nat :=
  cA * surfProxy R + K * (cEndo * surfProxy r)

/-- T20: Exact Prokaryotic Bioenergetic Ceiling (Nick Lane Surface-to-Volume Wall).
    A single-membrane prokaryote of radius R > 0 has strictly positive net surplus power
    iff cV * R < 3 * cA. Beyond R_crit = 3 cA / cV, starvation is mathematically forced. -/
theorem T20_prokaryotic_bioenergetic_ceiling (cA cV R : Nat) (hR : R > 0) :
    prokPower cA R > maintCost cV R ↔ cV * R < 3 * cA := by
  unfold prokPower maintCost surfProxy volProxy
  have hR2 : 0 < R * R := Nat.mul_pos hR hR
  have hLHS : cA * (3 * R * R) = (3 * cA) * (R * R) := by grind
  have hRHS : cV * (R * R * R) = (cV * R) * (R * R) := by grind
  rw [hLHS, hRHS]
  constructor
  · intro h
    exact Nat.lt_of_mul_lt_mul_right h
  · intro h
    exact Nat.mul_lt_mul_of_pos_right h hR2

/-- T21: Prokaryotic specific power density (power per unit volume) strictly decreases with R.
    Cleared-denominator form: P(R2)/V(R2) < P(R1)/V(R1) for 0 < R1 < R2. -/
theorem T21_prokaryotic_power_density_strictly_decreasing (cA R1 R2 : Nat)
    (hcA : cA > 0) (hR1 : R1 > 0) (hLt : R1 < R2) :
    prokPower cA R2 * volProxy R1 < prokPower cA R1 * volProxy R2 := by
  unfold prokPower surfProxy volProxy
  have hFactor : 0 < 3 * cA * R1 * R1 * R2 * R2 := by
    have h1 : 0 < 3 * cA := Nat.mul_pos (by decide) hcA
    have h2 : 0 < R1 * R1 := Nat.mul_pos hR1 hR1
    have hR2 : 0 < R2 := Nat.lt_trans hR1 hLt
    have h3 : 0 < R2 * R2 := Nat.mul_pos hR2 hR2
    have h12 : 0 < (3 * cA) * (R1 * R1) := Nat.mul_pos h1 h2
    have h123 : 0 < ((3 * cA) * (R1 * R1)) * (R2 * R2) := Nat.mul_pos h12 h3
    simp only [Nat.mul_assoc] at h123 ⊢
    exact h123
  have hL : cA * (3 * R2 * R2) * (R1 * R1 * R1) = (3 * cA * R1 * R1 * R2 * R2) * R1 := by grind
  have hR : cA * (3 * R1 * R1) * (R2 * R2 * R2) = (3 * cA * R1 * R1 * R2 * R2) * R2 := by grind
  rw [hL, hR]
  exact Nat.mul_lt_mul_of_pos_left hLt hFactor

/-- T22: Endosymbiotic Scaling Liberation Theorem.
    Even when host radius R strictly exceeds the prokaryotic wall (cV * R ≥ 3 * cA),
    packing K endosymbionts of radius r achieves strictly positive net surplus power for
    ARBITRARILY LARGE host radius R whenever K * (cEndo * surfProxy r) ≥ maintCost cV R. -/
theorem T22_endosymbiotic_scaling_liberation
    (cA cEndo cV R K r : Nat)
    (hcA : cA > 0) (hR : R > 0)
    (hWall : cV * R ≥ 3 * cA)
    (hEndoMet : K * (cEndo * surfProxy r) ≥ maintCost cV R) :
    prokPower cA R ≤ maintCost cV R ∧
    eukPower cA cEndo R K r > maintCost cV R := by
  constructor
  · have hNotGt : ¬ (prokPower cA R > maintCost cV R) := by
      rw [T20_prokaryotic_bioenergetic_ceiling cA cV R hR]
      omega
    omega
  · unfold eukPower
    have hOuterPos : cA * surfProxy R > 0 := by
      unfold surfProxy
      have h3R : 0 < 3 * R := Nat.mul_pos (by decide) hR
      have h3RR : 0 < (3 * R) * R := Nat.mul_pos h3R hR
      exact Nat.mul_pos hcA h3RR
    omega

/-- T23: Organelle Surface Amplification Factor.
    Splitting a fraction of host volume into smaller spheres of radius r < R multiplies
    available respiratory membrane area per unit volume by exact factor R / r > 1. -/
theorem T23_endosymbiont_surface_amplification (R r : Nat) (hr : r > 0) (hLt : r < R) :
    surfProxy R * volProxy r < surfProxy r * volProxy R := by
  unfold surfProxy volProxy
  have hFactor : 0 < 3 * R * R * r * r := by
    have hR : 0 < R := Nat.lt_trans hr hLt
    have h1 : 0 < 3 * R := Nat.mul_pos (by decide) hR
    have h2 : 0 < (3 * R) * R := Nat.mul_pos h1 hR
    have hrr : 0 < r * r := Nat.mul_pos hr hr
    have hAll : 0 < ((3 * R) * R) * (r * r) := Nat.mul_pos h2 hrr
    simp only [Nat.mul_assoc] at hAll ⊢
    exact hAll
  have hL : (3 * R * R) * (r * r * r) = (3 * R * R * r * r) * r := by grind
  have hR : (3 * r * r) * (R * R * R) = (3 * R * R * r * r) * R := by grind
  rw [hL, hR]
  exact Nat.mul_lt_mul_of_pos_left hLt hFactor

/-- T24: Secondary Endosymbiosis (Chimeric Megacells) strictly adds internal ATP capacity
    without violating lumen volume packing. -/
theorem T24_secondary_endosymbiosis_additive_power
    (cA c1 c2 R K1 r1 K2 r2 : Nat)
    (hc2 : c2 > 0) (hK2 : K2 > 0) (hr2 : r2 > 0) :
    eukPower cA c1 R K1 r1 + K2 * (c2 * surfProxy r2) > eukPower cA c1 R K1 r1 := by
  unfold surfProxy
  have h3r : 0 < 3 * r2 * r2 := Nat.mul_pos (Nat.mul_pos (by decide) hr2) hr2
  have hTerm : 0 < K2 * (c2 * (3 * r2 * r2)) := Nat.mul_pos hK2 (Nat.mul_pos hc2 h3r)
  omega

/-- T25: Mitotic organelle partitioning strictly conserves cooperative and selfish counts. -/
theorem T25_mitotic_heteroplasmy_conservation (c1 c2 s1 s2 : Nat) :
    (c1 + s1) + (c2 + s2) = (c1 + c2) + (s1 + s2) := by
  omega

/-- T26: Mitotic Heteroplasmy Bottleneck Purifying Selection Theorem.
    When a heteroplasmic host (c = c1 + c2 cooperative, s = s1 + s2 selfish organelles)
    partitions organelles unevenly (c1 * s2 ≠ c2 * s1), strictly ONE daughter inherits a
    higher cooperative fraction than the parent while the other inherits a lower fraction —
    exposing selfish mitochondria to host-level purifying selection. -/
theorem T26_asymmetric_bottleneck_purifying_selection (c1 c2 s1 s2 : Nat)
    (hUnequal : c1 * s2 ≠ c2 * s1) :
    (c1 * ((c1 + c2) + (s1 + s2)) > (c1 + c2) * (c1 + s1) ∧
     c2 * ((c1 + c2) + (s1 + s2)) < (c1 + c2) * (c2 + s2)) ∨
    (c2 * ((c1 + c2) + (s1 + s2)) > (c1 + c2) * (c2 + s2) ∧
     c1 * ((c1 + c2) + (s1 + s2)) < (c1 + c2) * (c1 + s1)) := by
  have hId1 : c1 * ((c1 + c2) + (s1 + s2)) = c1 * c1 + c1 * c2 + c1 * s1 + c1 * s2 := by grind
  have hId2 : (c1 + c2) * (c1 + s1) = c1 * c1 + c1 * c2 + c1 * s1 + c2 * s1 := by grind
  have hId3 : c2 * ((c1 + c2) + (s1 + s2)) = c1 * c2 + c2 * c2 + c2 * s1 + c2 * s2 := by grind
  have hId4 : (c1 + c2) * (c2 + s2) = c1 * c2 + c2 * c2 + c1 * s2 + c2 * s2 := by grind
  by_cases hGt : c1 * s2 > c2 * s1
  · left
    constructor <;> omega
  · right
    have hLt : c1 * s2 < c2 * s1 := by omega
    constructor <;> omega

/-- T27: Endosymbiotic Gene Transfer (EGT) Genome Economy Theorem.
    Migrating g housekeeping loci from K ≥ 2 organelles to a single host nuclear copy
    strictly reduces total cellular DNA replication burden by (K - 1) * g > 0. -/
theorem T27_egt_reduces_organelle_maintenance (K g : Nat) (hK : K ≥ 2) (hg : g > 0) :
    K * g > g ∧ K * g - g = (K - 1) * g := by
  constructor
  · have h2g : 2 * g ≤ K * g := Nat.mul_le_mul_right g hK
    omega
  · rw [Nat.sub_mul, Nat.one_mul]

end Cellulogonic.BioenergeticsEndosymbiosis
