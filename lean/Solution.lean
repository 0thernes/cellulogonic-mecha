module

public import CellulogonicProofs

/-!
# CELLULOGONIC MECHA v1.7.0 Formal Solution Entrypoint (`Solution.lean`)

Exports all 78 machine-checked theorems from `CellulogonicProofs` (`T01`–`T58`, `C59`–`C62`,
`NV01`–`NV08`, `AT01`–`AT08` across 10 proof modules + pure `Spec.lean`, with `autoImplicit = false`,
`relaxedAutoImplicit = false`, zero `sorry`, zero `native_decide`) for isolated judging
against `Challenge.lean` via `lake comparator --config comparator.json --paranoid`.

Copyright (c) 2026 Alexander Stephen Donahue <0_0@0thernes.art>.
SPDX-License-Identifier: Apache-2.0
-/
