module

public import CellulogonicProofs

/-!
# CELLULOGONIC MECHA v1.5.0 Formal Solution Entrypoint (`Solution.lean`)

Exports all 70 machine-checked theorems from `CellulogonicProofs` (`T01`–`T58`, `C59`–`C62`,
`NV01`–`NV08` across 9 proof modules + pure `Spec.lean`, with `autoImplicit = false`,
`relaxedAutoImplicit = false`, zero `sorry`, zero `native_decide`) for isolated judging
against `Challenge.lean` via `lake comparator --config comparator.json --paranoid`.

Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
SPDX-License-Identifier: Apache-2.0
-/
