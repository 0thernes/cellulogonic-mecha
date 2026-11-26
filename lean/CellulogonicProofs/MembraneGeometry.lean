module

public import CellulogonicProofs.Spec

/-!
  CELLULOGONIC MECHA v1.5.0 — Theorem II: Discrete Canham-Helfrich Isoperimetric Fission
  Trichotomy, Symmetric Binary Fission Threshold (Q ≤ 1/2), & Exact Algebraic Proof of
  Asymmetric Budding Minimax Optimality.

  Pure Lean 4.35.0 `module` (zero Mathlib, zero sorry, zero custom axioms).
  Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
  SPDX-License-Identifier: Apache-2.0
-/

namespace Cellulogonic.MembraneGeometry

/-- T11: Exact algebraic scaling law — symmetric halving (2v, 2l) → (v, l)
    strictly doubles the squared reduced volume Q. -/
public theorem T11_symmetric_fission_doubles_Q (c v l : Nat) :
    2 * (c * (2 * v) * (2 * v)) * (l * l * l) =
    (c * v * v) * ((2 * l) * (2 * l) * (2 * l)) := by
  grind

/-- T12: Exact Necessary and Sufficient Threshold for Symmetric Binary Fission.
    Two identical spherical daughters (v, l) are intact iff the parent (2v, 2l) satisfies
    Q_parent ≤ 1/2 (i.e. 2 * C0 * (2v)^2 ≤ (2l)^3). -/
public theorem T12_symmetric_fission_iff_half_threshold (v l : Nat) :
    IsIntact v l ↔ CanSymmetricFission (2 * v) (2 * l) := by
  unfold IsIntact CanSymmetricFission C0
  have h1 : 2 * 100 * (2 * v) * (2 * v) = 8 * (100 * v * v) := by grind
  have h2 : (2 * l) * (2 * l) * (2 * l) = 8 * (l * l * l) := by grind
  rw [h1, h2]
  constructor
  · intro h
    exact Nat.mul_le_mul_left 8 h
  · intro h
    exact Nat.le_of_mul_le_mul_left h (by decide : 0 < 8)

/-- T13: The Swollen Interphase Window (1/2 < Q ≤ 1).
    Any vesicle with L^3 < 2 * C0 * V^2 ≤ 2 * L^3 is intact (non-lysed) yet strictly
    blocked from symmetric binary fission until further lipid synthesis lowers Q. -/
public theorem T13_high_Q_blocks_symmetric_fission (V L : Nat)
    (hLo : L * L * L < 2 * C0 * V * V)
    (hHi : C0 * V * V ≤ L * L * L) :
    IsIntact V L ∧ ¬ CanSymmetricFission V L := by
  unfold IsIntact CanSymmetricFission
  exact ⟨hHi, Nat.not_le_of_gt hLo⟩

/-- T14: Equal-ratio asymmetric splits never improve over the Q ≤ 1/2 symmetric threshold.
    If two daughters split volume and lipid area in the same proportion (V1*L = L1*V,
    V2*L = L2*V), then if both daughters are intact, the parent MUST already satisfy Q ≤ 1/2. -/
public theorem T14_equal_ratio_asymmetric_split_suboptimal (V L V1 L1 V2 L2 : Nat)
    (hV1 : V1 > 0) (hV2 : V2 > 0)
    (hSumV : V1 + V2 = V) (_hSumL : L1 + L2 = L)
    (hProp1 : V1 * L = L1 * V)
    (hProp2 : V2 * L = L2 * V)
    (hInt1 : IsIntact V1 L1)
    (hInt2 : IsIntact V2 L2) :
    CanSymmetricFission V L := by
  unfold IsIntact CanSymmetricFission at *
  have hEither : 2 * V1 ≤ V ∨ 2 * V2 ≤ V := by omega
  cases hEither with
  | inl hle =>
    have hVpos : V > 0 := by omega
    have hCube1 : (L1 * V) * (L1 * V) * (L1 * V) = (V1 * L) * (V1 * L) * (V1 * L) := by
      rw [hProp1]
    have hStep1 : (C0 * V1 * V1) * (V * V * V) ≤ (L1 * L1 * L1) * (V * V * V) :=
      Nat.mul_le_mul_right (V * V * V) hInt1
    have hReL : (L1 * L1 * L1) * (V * V * V) = (L1 * V) * (L1 * V) * (L1 * V) := by grind
    have hReR : (V1 * L) * (V1 * L) * (V1 * L) = (V1 * V1) * (V1 * (L * L * L)) := by grind
    have hReA : (C0 * V1 * V1) * (V * V * V) = (V1 * V1) * (C0 * V * V * V) := by grind
    rw [hReL, hCube1, hReR] at hStep1
    rw [hReA] at hStep1
    have hCancel1 : C0 * V * V * V ≤ V1 * (L * L * L) :=
      Nat.le_of_mul_le_mul_left hStep1 (Nat.mul_pos hV1 hV1)
    have hMul2 : 2 * (C0 * V * V * V) ≤ 2 * (V1 * (L * L * L)) :=
      Nat.mul_le_mul_left 2 hCancel1
    have hBound : 2 * (V1 * (L * L * L)) ≤ V * (L * L * L) := by
      have hAssoc : 2 * (V1 * (L * L * L)) = (2 * V1) * (L * L * L) := by grind
      rw [hAssoc]
      exact Nat.mul_le_mul_right (L * L * L) hle
    have hTrans : V * (2 * C0 * V * V) ≤ V * (L * L * L) := by
      have hEq : V * (2 * C0 * V * V) = 2 * (C0 * V * V * V) := by grind
      rw [hEq]
      exact Nat.le_trans hMul2 hBound
    exact Nat.le_of_mul_le_mul_left hTrans hVpos
  | inr hle =>
    have hVpos : V > 0 := by omega
    have hCube2 : (L2 * V) * (L2 * V) * (L2 * V) = (V2 * L) * (V2 * L) * (V2 * L) := by
      rw [hProp2]
    have hStep2 : (C0 * V2 * V2) * (V * V * V) ≤ (L2 * L2 * L2) * (V * V * V) :=
      Nat.mul_le_mul_right (V * V * V) hInt2
    have hReL : (L2 * L2 * L2) * (V * V * V) = (L2 * V) * (L2 * V) * (L2 * V) := by grind
    have hReR : (V2 * L) * (V2 * L) * (V2 * L) = (V2 * V2) * (V2 * (L * L * L)) := by grind
    have hReA : (C0 * V2 * V2) * (V * V * V) = (V2 * V2) * (C0 * V * V * V) := by grind
    rw [hReL, hCube2, hReR] at hStep2
    rw [hReA] at hStep2
    have hCancel2 : C0 * V * V * V ≤ V2 * (L * L * L) :=
      Nat.le_of_mul_le_mul_left hStep2 (Nat.mul_pos hV2 hV2)
    have hMul2 : 2 * (C0 * V * V * V) ≤ 2 * (V2 * (L * L * L)) :=
      Nat.mul_le_mul_left 2 hCancel2
    have hBound : 2 * (V2 * (L * L * L)) ≤ V * (L * L * L) := by
      have hAssoc : 2 * (V2 * (L * L * L)) = (2 * V2) * (L * L * L) := by grind
      rw [hAssoc]
      exact Nat.mul_le_mul_right (L * L * L) hle
    have hTrans : V * (2 * C0 * V * V) ≤ V * (L * L * L) := by
      have hEq : V * (2 * C0 * V * V) = 2 * (C0 * V * V * V) := by grind
      rw [hEq]
      exact Nat.le_trans hMul2 hBound
    exact Nat.le_of_mul_le_mul_left hTrans hVpos

/-- T15: Constructive Proof of Asymmetric Exosome / Budding Escape Above Q = 1/2.
    For (V = 47, L = 72), Q ≈ 0.592 > 1/2 makes symmetric fission strictly impossible,
    yet an optimal-area-allocated asymmetric bud (6, 16) + (41, 56) is strictly intact. -/
public theorem T15_exosome_asymmetric_budding_escapes_half_barrier :
    IsIntact 47 72 ∧
    ¬ CanSymmetricFission 47 72 ∧
    ValidBinarySplit 47 72 6 16 41 56 := by
  unfold ValidBinarySplit IsIntact CanSymmetricFission C0
  decide

private theorem vesicle_minimax_poly_identity (x y : Int) :
    2 * (x^3 + y^3)^2 - (x^2 + y^2)^3 =
    (x - y)^2 * (x^4 + 2 * x^3 * y + 2 * x * y^3 + y^4) := by
  grind

/-- T16: Global Minimax Theorem of Vesicle Division (Exact Algebraic Form).
    With daughter volumes V1 = x^3, V2 = y^3 (x, y > 0) and strain-matched minimal areas
    L1 = x^2, L2 = y^2, total required area satisfies (L1 + L2)^3 ≤ 2 * (V1 + V2)^2,
    with STRICT inequality whenever V1 ≠ V2 (x ≠ y), and equality iff V1 = V2.
    Symmetric binary fission is therefore the unique global maximum of membrane area demand. -/
public theorem T16_optimal_area_partition_strictly_improves_budding (x y : Int)
    (hx : x > 0) (hy : y > 0) :
    (x^2 + y^2)^3 ≤ 2 * (x^3 + y^3)^2 ∧
    (x ≠ y → (x^2 + y^2)^3 < 2 * (x^3 + y^3)^2) := by
  have hid := vesicle_minimax_poly_identity x y
  have hx2 : x^2 > 0 := by
    have : x^2 = x * x := by grind
    rw [this]; exact Int.mul_pos hx hx
  have hy2 : y^2 > 0 := by
    have : y^2 = y * y := by grind
    rw [this]; exact Int.mul_pos hy hy
  have hx4 : x^4 > 0 := by
    have : x^4 = x^2 * x^2 := by grind
    rw [this]; exact Int.mul_pos hx2 hx2
  have hy4 : y^4 > 0 := by
    have : y^4 = y^2 * y^2 := by grind
    rw [this]; exact Int.mul_pos hy2 hy2
  have hx3y : x^3 * y > 0 := by
    have : x^3 * y = (x^2 * x) * y := by grind
    rw [this]; exact Int.mul_pos (Int.mul_pos hx2 hx) hy
  have hxy3 : x * y^3 > 0 := by
    have : x * y^3 = x * (y^2 * y) := by grind
    rw [this]; exact Int.mul_pos hx (Int.mul_pos hy2 hy)
  have hFactorPos : x^4 + 2 * x^3 * y + 2 * x * y^3 + y^4 > 0 := by
    have h1 : 2 * x^3 * y = 2 * (x^3 * y) := by grind
    have h3 : 2 * x * y^3 = 2 * (x * y^3) := by grind
    omega
  have hSqNonneg : (x - y)^2 ≥ 0 := by
    have hsq : (x - y)^2 = (x - y) * (x - y) := by grind
    rw [hsq]
    by_cases h : x - y ≥ 0
    · exact Int.mul_nonneg h h
    · have hneg : -(x - y) ≥ 0 := by omega
      have heq : (x - y) * (x - y) = (-(x - y)) * (-(x - y)) := by grind
      rw [heq]
      exact Int.mul_nonneg hneg hneg
  constructor
  · have hProd : (x - y)^2 * (x^4 + 2 * x^3 * y + 2 * x * y^3 + y^4) ≥ 0 :=
      Int.mul_nonneg hSqNonneg (Int.le_of_lt hFactorPos)
    omega
  · intro hne
    have hSqPos : (x - y)^2 > 0 := by
      have hsq : (x - y)^2 = (x - y) * (x - y) := by grind
      rw [hsq]
      by_cases h : x - y > 0
      · exact Int.mul_pos h h
      · have hneg : -(x - y) > 0 := by omega
        have heq : (x - y) * (x - y) = (-(x - y)) * (-(x - y)) := by grind
        rw [heq]
        exact Int.mul_pos hneg hneg
    have hProd : (x - y)^2 * (x^4 + 2 * x^3 * y + 2 * x * y^3 + y^4) > 0 :=
      Int.mul_pos hSqPos hFactorPos
    omega

/-- T17: Increasing internal osmolyte volume V strictly raises osmotic turgor strain. -/
public theorem T17_osmotic_lysis_monotone_in_volume (V1 V2 : Nat) (hLt : V1 < V2) :
    C0 * V1 * V1 < C0 * V2 * V2 := by
  unfold C0
  have h1 : V1 * V1 < V2 * V2 := by
    have hle : V1 * V1 ≤ V1 * V2 := Nat.mul_le_mul_left V1 (Nat.le_of_lt hLt)
    have hst : V1 * V2 < V2 * V2 := Nat.mul_lt_mul_of_pos_right hLt (by omega)
    exact Nat.lt_of_le_of_lt hle hst
  have h2 : 100 * (V1 * V1) < 100 * (V2 * V2) := Nat.mul_lt_mul_of_pos_left h1 (by decide)
  simp only [Nat.mul_assoc] at h2 ⊢
  exact h2

/-- T18: Synthesizing additional membrane lipid L strictly lowers osmotic rupture strain. -/
public theorem T18_lipid_synthesis_relieves_membrane_stress (L1 L2 : Nat) (hLt : L1 < L2) :
    L1 * L1 * L1 < L2 * L2 * L2 := by
  have hsq : L1 * L1 < L2 * L2 := by
    have hle : L1 * L1 ≤ L1 * L2 := Nat.mul_le_mul_left L1 (Nat.le_of_lt hLt)
    have hst : L1 * L2 < L2 * L2 := Nat.mul_lt_mul_of_pos_right hLt (by omega)
    exact Nat.lt_of_le_of_lt hle hst
  have h1 : (L1 * L1) * L1 ≤ (L1 * L1) * L2 := Nat.mul_le_mul_left (L1 * L1) (Nat.le_of_lt hLt)
  have h2 : (L1 * L1) * L2 < (L2 * L2) * L2 := Nat.mul_lt_mul_of_pos_right hsq (by omega)
  exact Nat.lt_of_le_of_lt h1 h2

/-- T19: Endosymbiotic double-membrane engulfment preserves outer lipid budget while
    strictly bounding total internalized organelle volume inside the host lumen. -/
public theorem T19_double_membrane_vesicle_preservation
    (V_host V_endo L_host : Nat)
    (hInt : IsIntact (V_host + V_endo) L_host) :
    IsIntact V_host L_host := by
  unfold IsIntact at *
  have hLe : V_host ≤ V_host + V_endo := Nat.le_add_right V_host V_endo
  have hSq : C0 * V_host * V_host ≤ C0 * (V_host + V_endo) * (V_host + V_endo) := by
    have hs : V_host * V_host ≤ (V_host + V_endo) * (V_host + V_endo) :=
      Nat.mul_le_mul hLe hLe
    have hm := Nat.mul_le_mul_left C0 hs
    simp only [Nat.mul_assoc] at hm ⊢
    exact hm
  exact Nat.le_trans hSq hInt

end Cellulogonic.MembraneGeometry
