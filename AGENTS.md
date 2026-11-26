# AGENTS.md — CELLULOGONIC MECHA (v1.5.0) Canonical Agent Instructions

**Repository:** `0thernes/cellulogonic-mecha` (`v1.5.0`)
**Author:** Alexander Stephen Donahue (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`, `0_0@0thernes.art`)
**Dual License:** Code `Apache-2.0` (`LICENSE`) · Papers & Documentation `CC-BY-4.0` (`LICENSE-DOCS`)
**Role:** Build 2 (CELLULAR) — 3D Off-Lattice Developmental Embryogenesis, Chemiosmotic, Endosymbiotic & Xenocellular Artificial Life Engine

---

## 1. Hard Rules (Build Standard v1.2.0, R1–R8)

- **R1–R2 (Solo Linear Main):** Commit and push straight to `main`. No PRs, no feature branches, no commit/tag signing, no git hooks, no blocking branch protection.
- **R3 (Informational CI):** `.github/workflows/ci.yml` reports on every push to `main` and tag `v*`.
- **R4 (Banned Files):** NEVER add `.github/dependabot.yml`, `renovate.json`, `PULL_REQUEST_TEMPLATE*`, `.github/copilot-instructions.md`, or `FUNDING.yml`.
- **R5 (Identity & License):** Always attribute `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`). Never use `0thernes LLC` or non-commercial licenses in active files.
- **R6 (Zero Fabrication):** Never fabricate any metric, benchmark, SHA-256 digest, Lean proof result, DOI, citation, or peer-review status. Every number must come from a live run of the current tree.
- **R7 (10 Finding Classes):** Classify every audit finding into exactly one of: `bug`, `error`, `issue`, `problem`, `concern`, `dilemma`, `conflict`, `gap`, `hole`, `blindspot`.
- **R8 (PowerShell Bracket Safety):** Because this workspace lives under `Z:\[Vibe Coded (AI)]\ANTIGRAVITY\cellulogonic-mecha`, ALWAYS pass `-LiteralPath` to PowerShell file cmdlets (`Test-Path -LiteralPath`, `Set-Location -LiteralPath`, `Get-Content -LiteralPath`, etc.).

## 2. Canonical Verification Commands

| Task | Command |
|---|---|
| Build Win64 C11 + 24 gates + Lean audit | `.\build.ps1` |
| Build Linux ELF64 + 24 gates + Lean audit | `./build_linux.sh` |
| Build Lean 4.35.0 (`Solution` + `Challenge`) | `cd lean && lake build && lake build Challenge` |
| Verbatim Lean axiom audit | `cd lean && lake env lean VerifyAxioms.lean > ../results/lean4-audit.txt` |
| Verify README verbatim axioms | `python3 scripts/check_readme_axioms.py` |
| Property-based suite (2,700 trials) | `python3 scripts/prop_check.py` |
| Live DOI citation audit (`36/36` DOIs) | `python3 scripts/check_citations.py` |
| Compile AMS-LaTeX PDF + clean-room arXiv | `python3 scripts/build_paper.py` |
| Build Standard v1.2.0 self-audit | `pwsh -NoProfile -File scripts/Audit-Repo.ps1` |
| Full one-word reproduction (`just`) | `just reproduce` |

## 3. Verified Baseline (`v1.5.0`)

- **Native C11:** 26/26 falsification gates + 8/8 planted mutants caught (Win64 `clang 23.1.0 -Werror` & Linux `gcc 13.3.0`), 1,000/1,000 adversarial ABI/CMBIN fuzz cases PASS, hot-loop zero-heap `llvm-nm` verified, 4-way 0-bit SHA-256 parity (`681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff`), `0 amu` / `0 uq` drift, `0` post-warmup allocs, `2,700/2,700` `prop_check` trials.
- **Pure Lean 4.35.0:** 70/70 audited theorems (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`), zero Mathlib, `autoImplicit = false`, zero `sorry` in `Solution`, zero `native_decide`, strictly `[propext, Classical.choice, Quot.sound]`, Palomar `Spec`/`Challenge`/`Solution` contract verified locally across 5 Windows-capable checkers (`lean`, `leanchecker`, `leanchecker-paranoid`, `con-leche`, `con-ron`, `nanoda_bin`) and 6/6 kernels under `bubblewrap` in Ubuntu CI.
- **Mega-Atlas Non-Retirement:** `0 / 1,900` overarching open problems in `0thernes/unsolved-problems-mega-atlas` are retired (`docs/MEGA_ATLAS_CROSSCHECK.md`).
