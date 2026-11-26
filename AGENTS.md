<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# AGENTS.md — CELLULOGONIC MECHA (v1.6.0) Canonical Agent Instructions

**Repository:** `0thernes/cellulogonic-mecha` (`v1.6.0`)
**Author:** Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`, `0_0@0thernes.art`)
**Dual License:** Code `Apache-2.0` (`LICENSE`) · Papers & Documentation `CC-BY-4.0` (`LICENSE-DOCS`, `REUSE.toml`)
**Role:** Build 2 (CELLULAR) — 3D Off-Lattice Developmental Embryogenesis, Chemiosmotic, Endosymbiotic & Xenocellular Artificial Life Engine

---

## 1. Hard Rules (Build Standard v1.2.0, R1–R8)

- **R1–R2 (Solo Linear Main):** Commit and push straight to `main`. No PRs, no feature branches, no commit/tag signing, no git hooks, no blocking branch protection.
- **R3 (Informational CI):** `.github/workflows/ci.yml` reports on every push to `main` and tag `v*` (3-OS C11 + 6-kernel Lean + paper + security); `.github/workflows/pages.yml` deploys the static 78-theorem Lean Blueprint.
- **R4 (Banned Files):** NEVER add `.github/dependabot.yml`, `renovate.json`, `PULL_REQUEST_TEMPLATE*`, `.github/copilot-instructions.md`, or `FUNDING.yml`.
- **R5 (Identity & License):** Always attribute `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`). Never use `0thernes LLC` or non-commercial licenses in active files.
- **R6 (Zero Fabrication):** Never fabricate any metric, benchmark, SHA-256 digest, Lean proof result, DOI, citation, or peer-review status. Every number must come from a live run of the current tree.
- **R7 (10 Finding Classes):** Classify every audit finding into exactly one of: `bug`, `error`, `issue`, `problem`, `concern`, `dilemma`, `conflict`, `gap`, `hole`, `blindspot`.
- **R8 (PowerShell Bracket Safety):** Because this workspace lives under `Z:\[Vibe Coded (AI)]\ANTIGRAVITY\cellulogonic-mecha`, ALWAYS pass `-LiteralPath` to PowerShell file cmdlets (`Test-Path -LiteralPath`, `Set-Location -LiteralPath`, `Get-Content -LiteralPath`, etc.).

## 2. Canonical Verification Commands

| Task | Command |
|---|---|
| Build Win64 C11 + 26 gates + 78-proof Lean audit | `.\build.ps1` |
| Build Linux/macOS C11 + 26 gates + Lean audit | `./build_linux.sh` |
| Build Lean 4.35.0 (`Solution` + `Challenge`) | `cd lean && lake build && lake build Challenge` |
| Verbatim Lean axiom audit (`78/78`) | `cd lean && lake env lean VerifyAxioms.lean > ../results/lean4-audit.txt` |
| Verify README verbatim axioms + live negative Lean | `python3 scripts/check_readme_axioms.py && python3 scripts/test_lean_negative.py` |
| Exact Python oracle & paired stats + 2,720 `prop_check` | `python3 scripts/verify_oracle_and_stats.py && python3 scripts/prop_check.py` |
| Web Observatory 13/13 security & bridge tests | `node test/test_web_server.mjs` |
| Live Crossref citation & retraction audit (`36/36`) | `python3 scripts/check_citations.py --write-json results/citation-audit.json` |
| Build interactive 78-theorem GitHub Pages Blueprint | `python3 scripts/build_blueprint.py` |
| Compile AMS-LaTeX PDF + clean-room arXiv | `python3 scripts/build_paper.py` |
| Build Standard v1.2.0 self-audit | `pwsh -NoProfile -File scripts/Audit-Repo.ps1` |
| Full one-word reproduction (`just`) | `just reproduce` |

## 3. Verified Baseline (`v1.6.0`)

- **Native C11 (`ABI 0x00010600`):** Portable `-march=x86-64` + isolated `target("avx2")` (plus macOS ARM64 in CI), 26/26 falsification gates + 8/8 planted mutants caught (`0.692s` Win64 / `0.802s` Linux + ASan/UBSan clean), 1,000/1,000 ABI/CMBIN fuzz + `libFuzzer`, 3-module zero-heap `nm` verified, cross-OS/ISA 0-bit SHA-256 parity (`83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe`), `0 amu` / `0 uq` drift, `0` post-warmup allocs, `2,720/2,720` `prop_check` trials.
- **Pure Lean 4.35.0:** **78/78** audited theorems (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`, `AT01`–`AT08`), `12` pinned `Spec` definitions, zero Mathlib, `autoImplicit = false`, zero `sorry` in `Solution`, zero `native_decide`, strictly `[propext, Classical.choice, Quot.sound]`, 6/6 kernels + live negative tamper rejection under `bubblewrap` in Ubuntu CI.
- **Mega-Atlas `v2.6.0` Non-Retirement:** `0 / 1,900` overarching open problems in `0thernes/unsolved-problems-mega-atlas` (`d23b4d7`) are retired across 13 verbatim adjacent catalog IDs (`docs/MEGA_ATLAS_CROSSCHECK.md`).
