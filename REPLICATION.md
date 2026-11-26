# Clean-Clone Replication Protocol & Verification Log (`REPLICATION.md`)

This protocol enables any independent verifier to reproduce every native C11 benchmark, falsification gate, property check, 6-kernel Lean 4.35.0 proof audit, live DOI citation verification, and Tectonic AMS-LaTeX PDF build from a clean clone (Build Standard `B43` & `B44`).

## 1. Prerequisites

- **Windows 11 x64:** Elan + Lean `v4.35.0` (includes bundled `clang 23.1.0 + lld + llvm-ar`), Python `3.11+` (stdlib only), Tectonic (`tectonic.exe`), optional Node.js `20+` for WebGL2 Observatory.
- **Ubuntu Linux x64:** `gcc 13+` (or `clang`), `bubblewrap` (`sudo apt-get install -y bubblewrap`), Elan + Lean `v4.35.0`, Python `3.11+`, Tectonic, optional Node.js `20+`.

## 2. Step-by-Step Clean-Clone Commands

```bash
git clone https://github.com/0thernes/cellulogonic-mecha.git
cd cellulogonic-mecha
```

### A. Native C11 Build, 26-Gate Falsification, 8-Mutant & 1,000-Case ABI Fuzz Suite
- **Windows:** `.\build.ps1`
- **Linux:** `chmod +x build_linux.sh && ./build_linux.sh`
- **Expected:** `SUMMARY: 26 / 26 GATES PASSED | 8 / 8 PLANTED MUTANTS CAUGHT`, `1000` adversarial ABI/CMBIN fuzz cases PASS, C11 quickstart PASS, exit code `0`.

### B. Bit-Exact Dual-Kernel Parity, Embryogenesis & Locked `N = 24` Statistics
```bash
# Windows (replace ./bin/cellulogonic_linux with .\bin\cellulogonic.exe)
./bin/cellulogonic_linux bench --epochs 300 --size 32 --seed 5000
./bin/cellulogonic_linux embryo --epochs 200 --size 32 --seed 5000
./bin/cellulogonic_linux stats --pairs 24 --epochs 180 --size 32
PYTHONDONTWRITEBYTECODE=1 python3 scripts/verify_oracle_and_stats.py
PYTHONDONTWRITEBYTECODE=1 python3 scripts/prop_check.py
PYTHONDONTWRITEBYTECODE=1 python3 scripts/check_versions.py
node test/test_web_server.mjs
```
- **Expected Exact Tolerances (0-bit tolerance on hashes & integers):**
  - `bench` SHA-256 (`SIMD_AVX2` == `SCALAR_REF`): `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff`, `51488` steps, `1222070` rxns, `mass_res=0 amu`, `energy_res=0 uq`, `2nd_law_viol=0`, `allocs=0`.
  - `embryo` SHA-256: `9526ca6a245c2f5887eb7f55d3a45e6ac0e667ec6dd88fd41f1ceb49bc3bbb89`, `alive=50`, `Fates[P= 4 Ec= 8 N=18 M= 9 G= 6 Gm= 5]`.
  - `stats` (`N=24`): `24/24` wins across all 4 arms (`H1`–`H4`), `p_Holm = 0.00040`, Cohen's `dz` in `[1.468, 4.214]`.
  - `verify_oracle_and_stats.py`: `6,586` exact-rational Python oracle checks + 4-solvent differential & stats checks PASS.
  - `prop_check.py`: `2,700 / 2,700` trials PASS.
  - `test_web_server.mjs`: `12 / 12` Web Observatory HTTP/CSP/CSRF/DNS-rebinding tests PASS.

### C. Six-Kernel Pure Lean 4.35.0 Formal Audit
```bash
cd lean
lake build
lake build Challenge
lake env lean VerifyAxioms.lean > ../results/lean4-audit.txt
lake env leanchecker --fresh Solution
lake env leanchecker-paranoid --fresh Solution
# On Ubuntu Linux with bubblewrap (automated in .github/workflows/ci.yml):
lake check --paranoid
lake comparator --config comparator.json --paranoid
cd ..
python3 scripts/check_readme_axioms.py
```
- **Expected:** `70/70` audited theorems, `0 sorryAx` in `Solution`, `0` warnings, strictly within `[propext, Classical.choice, Quot.sound]`, `check_readme_axioms.py` exits `0`.

### D. White Paper, Clean-Room arXiv Tarball & Live Citation Audit
```bash
python3 scripts/check_citations.py
python3 scripts/build_paper.py
pwsh -NoProfile -File scripts/Audit-Repo.ps1
```
- **Expected:** `36/36` DOIs `DOI OK (200)`, `0` Overfull `\hbox`, `0` undefined refs/citations, clean-room `dist/arxiv_submission.tar.gz` compiles cleanly, `Audit-Repo.ps1` reports `100.0%` Structure / `100.0%` Completion / `100.0%` Core.

## 3. Replication Log (Build Standard `B44`)

*Note: External independent third-party replications are warmly invited via GitHub Issues (`replication` label); rows below honestly distinguish author/CI replays from awaited external replications.*

| Date | Verifier / Environment | Version / Commit | Scope Verified | Result | Notes |
|---|---|---|---|---|---|
| 2026-11-21 | Author (Win64 `clang 23.1.0` + WSL2 Ubuntu `gcc 13.3.0`, Core Ultra 9 275HX) | `v1.3.0` | 24/24 gates, 8/8 mutants, 4-way SHA-256, 58 Lean proofs | PASS | `33.50 M` Win64 / `28.56 M` Linux; 0-bit diff `be46eceb...8f7422a1` |
| 2026-11-22 | Author (Win64 `clang 23.1.0` ThinLTO + Linux `gcc 13.3.0`) | `v1.4.0` (`3b85b15`) | 24/24 gates, 8/8 mutants, 62 Lean proofs (`C59`–`C62`) | PASS | Bit-exact match preserved with AVX-VNNI + ThinLTO |
| 2026-11-25 | Author (Win64 `clang 23.1.0` + WSL2 Linux `gcc 13.3.0`, Lean `v4.35.0`, Tectonic, Python 3.12, Node.js; **clean-clone verified**) | `v1.5.0` | **26/26 gates**, 8/8 mutants, 1,000-case ABI/CMBIN fuzz, ASan+UBSan+AVX2 clean sweep, C11 quickstart, 6,586 Python oracle checks, 2,700 `prop_check`, 12/12 Web Observatory security tests, 70 Lean theorems across 5 Windows-capable kernels (`lean`, `leanchecker`, `leanchecker-paranoid`, `con-leche`, `con-ron`, `nanoda_bin`), 36/36 live DOIs, zero-overflow AMS-LaTeX PDF (12 pages visually audited) + clean-room arXiv, `100.0%` `audit-repo.sh` | PASS | 4-way exact SHA `681a9747...519eceff` (`bench`), `9526ca6a...bc3bbb89` (`embryo`), `96/96` wins (`stats`, `dz` `[1.47, 4.21]`) |
| 2026-11-25 | GitHub Actions CI (`ubuntu-latest` + `windows-latest`, bubblewrap 6-kernel Lean) | `v1.5.0` | Cross-OS C11 + ASan/UBSan + 6/6 Lean kernels (`lean4lean`, `lake check --paranoid`, `lake comparator --paranoid`) + Tectonic PDF | Automated on push | Independent external human third-party replication still **awaiting community submission** |
