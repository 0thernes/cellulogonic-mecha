<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Documentation Hub (`docs/index.md`)

Complete documentation index for **CELLULOGONIC MECHA `v1.6.0`** (`0thernes/cellulogonic-mecha`).

## Core Architecture & Specifications
- [`architecture.md`](architecture.md) — 4-build ecosystem pipeline, portable C11 core layout, AVX2/Scalar/ARM64 parity, and 78-theorem Lean 4.35.0 module graph (also see root [`ARCHITECTURE.md`](../ARCHITECTURE.md))
- [`configuration.md`](configuration.md) — CLI flags, `CellWorldConfig` fields, solvent/regime codes, and `.env.example` variables
- [`operations.md`](operations.md) — Local operation, `git bundle` offline backup, archive pinning, DigiCert RFC 3161 TSA, and Software Heritage offsite mirror policy
- [`threat-model.md`](threat-model.md) — STRIDE security analysis covering `"CMB5"` `.cmbin` binary parsing, CSRF & DNS-rebinding hardened Observatory API, and zero-dependency supply chain

## Formal Verification & Scientific Rigor
- [`verification.md`](verification.md) — End-to-end 6-kernel Lean 4.35.0 (`78/78` + live tamper reject), 26-gate / 8-mutant falsification, 1,000-case ABI fuzzing + `libFuzzer`, property checking, and 3-OS CI map
- [`statement-fidelity.md`](statement-fidelity.md) — Row-by-row Lean 4 ↔ exact C11 symbol fidelity & divergence audit matrix
- [`MEGA_ATLAS_CROSSCHECK.md`](MEGA_ATLAS_CROSSCHECK.md) — Unsolved Problems Mega-Atlas `v2.6.0` non-retirement cross-check (`0 / 1,900` retired across 13 verbatim IDs)
- [`preregistration.md`](preregistration.md) — Locked `N = 24` paired ablation hypotheses (`H1`–`H4`) & Holm–Bonferroni protocol
- [`benchmarks.md`](benchmarks.md) — Cross-OS throughput, unloaded vs. loaded timing transparency, and 0-alloc / 60 FPS resource budgets
- [`AI_DISCLOSURE.md`](AI_DISCLOSURE.md) — Transparent AI-assistance disclosure & CRediT authorship statement
- [`run-log.md`](run-log.md) — Chronological build, verification, and release execution log
- [`../SUBMISSION_READINESS.md`](../SUBMISSION_READINESS.md) — 43-item Defensible Independent AI Research Playbook v1.3 Gold Standard Checklist

## Governance, Decisions & Reviews
- [`adr/README.md`](adr/README.md) & [`adr/0001-architecture-and-verification-boundaries.md`](adr/0001-architecture-and-verification-boundaries.md) — Architecture Decision Records
- [`reviews/2026-11-25-10-pass-red-team-audit.md`](reviews/2026-11-25-10-pass-red-team-audit.md) — 10-Pass Red-Team Audit & Multi-Cycle Hardening Dossier
- [`faq.md`](faq.md) & [`glossary.md`](glossary.md) — Skeptical FAQ & canonical glossary (including all 10 finding classes)
- [`MASTER_REPORT.md`](MASTER_REPORT.md) — Comprehensive deep-dive scientific & operational monograph
- [`build-standard.md`](build-standard.md) — Build Standard for LLMs v1.2.0 reference specification
