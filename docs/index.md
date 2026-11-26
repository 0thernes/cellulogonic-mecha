# Documentation Hub (`docs/index.md`)

Complete documentation index for **CELLULOGONIC MECHA `v1.5.0`** (`0thernes/cellulogonic-mecha`).

## Core Architecture & Specifications
- [`architecture.md`](architecture.md) — 4-build ecosystem pipeline, C11 core layout, AVX2/Scalar parity, and Lean 4.35.0 module graph (also see root [`ARCHITECTURE.md`](../ARCHITECTURE.md))
- [`configuration.md`](configuration.md) — CLI flags, `CellWorldConfig` fields, solvent/regime codes, and `.env.example` variables
- [`operations.md`](operations.md) — Local operation, `git bundle` offline backup, archive pinning, and Software Heritage offsite mirror policy
- [`threat-model.md`](threat-model.md) — STRIDE security analysis covering `"CMB4"` `.cmbin` binary parsing, CSRF-hardened Observatory API, and zero-dependency supply chain

## Formal Verification & Scientific Rigor
- [`verification.md`](verification.md) — End-to-end 6-kernel Lean 4.35.0, 26-gate / 8-mutant falsification, 1,000-case ABI fuzzing, property checking, and CI verification map
- [`statement-fidelity.md`](statement-fidelity.md) — Row-by-row Lean 4 ↔ C11 fidelity & divergence audit matrix
- [`MEGA_ATLAS_CROSSCHECK.md`](MEGA_ATLAS_CROSSCHECK.md) — Unsolved Problems Mega-Atlas non-retirement cross-check (`0 / 1,900` retired)
- [`preregistration.md`](preregistration.md) — Locked `N = 24` paired ablation hypotheses (`H1`–`H4`) & Holm–Bonferroni protocol
- [`benchmarks.md`](benchmarks.md) — Cross-OS throughput, unloaded vs. loaded timing transparency, and 0-alloc / 60 FPS resource budgets
- [`AI_DISCLOSURE.md`](AI_DISCLOSURE.md) — Transparent AI-assistance disclosure & CRediT authorship statement
- [`run-log.md`](run-log.md) — Chronological build, verification, and release execution log

## Governance, Decisions & Reviews
- [`adr/README.md`](adr/README.md) & [`adr/0001-architecture-and-verification-boundaries.md`](adr/0001-architecture-and-verification-boundaries.md) — Architecture Decision Records
- [`reviews/2026-11-25-10-pass-red-team-audit.md`](reviews/2026-11-25-10-pass-red-team-audit.md) — Full 10-Pass Red-Team Audit & 3-Cycle Hardening Dossier
- [`faq.md`](faq.md) & [`glossary.md`](glossary.md) — Skeptical FAQ & canonical glossary (including all 10 finding classes)
- [`MASTER_REPORT.md`](MASTER_REPORT.md) — Comprehensive deep-dive scientific & operational monograph
- [`build-standard.md`](build-standard.md) — Build Standard for LLMs v1.2.0 reference specification
