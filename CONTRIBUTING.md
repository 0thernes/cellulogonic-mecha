# Contributing to CELLULOGONIC MECHA (`CONTRIBUTING.md`)

Thank you for your interest in inspecting, replicating, challenging, or extending **CELLULOGONIC MECHA**!

## Workflow & Governance

- This is a solo-maintained open scientific repository (`Alexander Stephen Donahue`, Independent Researcher, Spokane, WA, USA) following a linear `main` workflow (Build Standard Rules `R1`–`R8`).
- **Issue reports, falsification challenges, and independent replications** are the highest-value contributions: open an issue via `.github/ISSUE_TEMPLATE/` and classify any finding into one of the 10 standard classes (`bug`, `error`, `issue`, `problem`, `concern`, `dilemma`, `conflict`, `gap`, `hole`, `blindspot`).
- **Code or Lean proof contributions:** Ensure zero third-party runtime dependencies, zero `sorry` in `Solution`, zero `native_decide`, zero compiler warnings (`-Werror`), `26/26` falsification gates + `8/8` mutants passing, `1,000/1,000` `fuzz_cmbin_abi` cases passing, `2,700/2,700` `prop_check` trials passing, and `scripts/check_readme_axioms.py` passing.

## Pre-Submission Checklist

```bash
./build_linux.sh          # or .\build.ps1 on Windows
python3 scripts/prop_check.py
python3 scripts/check_readme_axioms.py
python3 scripts/check_citations.py
python3 scripts/build_paper.py
pwsh -NoProfile -File scripts/Audit-Repo.ps1
```

## Licensing of Contributions

By submitting a contribution to this repository, you agree that code/Lean/script contributions are licensed under **Apache-2.0** ([`LICENSE`](LICENSE)) and documentation/paper/figure contributions are licensed under **CC-BY-4.0** ([`LICENSE-DOCS`](LICENSE-DOCS)).
