<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Clean-Clone Replication Protocol & Verification Log (`REPLICATION.md`)

This protocol enables any independent verifier to reproduce every native C11 benchmark, falsification gate, property check, 6-kernel Lean 4.35.0 proof audit, live Crossref citation audit, static Blueprint portal, and Tectonic AMS-LaTeX PDF build from a clean clone (Build Standard `B43` & `B44`).

## 1. Prerequisites

- **Windows 11 x64:** Elan + Lean `v4.35.0` (includes bundled `clang 23.1.0 + lld + llvm-ar`), Python `3.11+` (stdlib only), Tectonic (`tectonic.exe`), optional Node.js `20+` for WebGL2 Observatory.
- **Ubuntu Linux x64 / macOS ARM64:** `gcc 13+` or `clang`, `bubblewrap` (Linux 6-kernel comparator), Elan + Lean `v4.35.0`, Python `3.11+`, Tectonic, optional Node.js `20+`.

## 2. Step-by-Step Clean-Clone Commands

```bash
git clone https://github.com/0thernes/cellulogonic-mecha.git
cd cellulogonic-mecha
```

### A. Native C11 Build, 26-Gate Falsification, 8-Mutant & 1,000-Case ABI Fuzz Suite
- **Windows:** `.\build.ps1`
- **Linux / macOS ARM64:** `chmod +x build_linux.sh && ./build_linux.sh`
- **Expected:** `SUMMARY: 26 / 26 GATES PASSED | 8 / 8 PLANTED MUTANTS CAUGHT`, `1000` adversarial ABI/CMBIN fuzz cases PASS, 3-module zero-heap `nm` audit PASS, C11 quickstart PASS, exit code `0`.

### B. Bit-Exact Dual-Kernel Parity, Embryogenesis & Locked `N = 24` Statistics
```bash
# Windows (replace ./bin/cellulogonic_linux with .\bin\cellulogonic.exe)
./bin/cellulogonic_linux bench --epochs 300 --size 32 --seed 5000
./bin/cellulogonic_linux embryo --epochs 200 --size 32 --seed 5000
./bin/cellulogonic_linux stats --pairs 24 --epochs 180 --size 32
PYTHONDONTWRITEBYTECODE=1 python3 scripts/verify_oracle_and_stats.py
PYTHONDONTWRITEBYTECODE=1 python3 scripts/prop_check.py
PYTHONDONTWRITEBYTECODE=1 python3 scripts/check_versions.py
PYTHONDONTWRITEBYTECODE=1 python3 examples/01_embryogenesis_demo.py
node test/test_web_server.mjs
```
- **Expected Exact Tolerances (0-bit tolerance on hashes & integers):**
  - `bench` SHA-256 (`SIMD_AVX2` == `SCALAR_REF` == macOS ARM64): `83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe`, `50243` steps, `1224310` rxns, `mass_res=0 amu`, `energy_res=0 uq`, `2nd_law_viol=0`, `allocs=0`.
  - `embryo` SHA-256: `cafc83d04db7b5858876821cb16c2aa1afa7792645f508fa40ad35bd15bad443`, `alive=47`, `Fates[P= 4 Ec= 8 N=18 M= 5 G= 6 Gm= 6]`.
  - `stats` (`N=24`): `24/24` wins across all 4 arms (`H1`–`H4`), `p_Holm = 0.00040 <= 0.05`, Cohen's `dz` in `[1.468, 3.560]`.
  - `verify_oracle_and_stats.py`: `6,586` exact-rational Python oracle checks + 4-solvent differential & stats checks PASS.
  - `prop_check.py`: `2,720 / 2,720` trials (including randomized native C11 runs) PASS.
  - `test_web_server.mjs`: `13 / 13` Web Observatory HTTP/CSP/CSRF/DNS-rebinding/favicon tests PASS.

### C. Six-Kernel Pure Lean 4.35.0 Formal Audit & Live Negative Tamper Test
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
python3 scripts/test_lean_negative.py
```
- **Expected:** `78/78` audited theorems, `0 sorryAx` in `Solution`, `0` warnings, strictly within `[propext, Classical.choice, Quot.sound]`, `check_readme_axioms.py` and `test_lean_negative.py` exit `0`.

### D. White Paper, Clean-Room arXiv Tarball, Blueprint, Timestamps & Live Crossref Citation Audit
```bash
python3 scripts/check_citations.py --write-json results/citation-audit.json
python3 scripts/build_blueprint.py
python3 scripts/check_blueprint.py
python3 scripts/stamp_timestamps.py --verify
python3 scripts/build_paper.py
pwsh -NoProfile -File scripts/Audit-Repo.ps1
```
- **Expected:** `36/36` DOIs `OK (Crossref)` with `36/36` title & author alignment and `0` retractions, `0` Overfull `\hbox`/`\vbox`, `0` undefined refs/citations, clean-room `dist/arxiv_submission.tar.gz` compiles cleanly, `_site/` Blueprint generated and verified, dual RFC 3161 `.tsr` + Bitcoin OTS `.ots` verified, `Audit-Repo.ps1` reports `100.0%` Structure / `100.0%` Completion / `100.0%` Core.

## 3. Replication Log (Build Standard `B44`)

*Note: Rows below record author clean-clone and 3-OS GitHub Actions CI replays. Independent external third-party human replication is openly invited via GitHub Issues (`replication` label) and is not yet recorded.*

| Date | Verifier / Environment | Version / Commit | Scope Verified | Result | Notes |
|---|---|---|---|---|---|
| 2026-11-21 | Author (Win64 `clang 23.1.0` + WSL2 Ubuntu `gcc 13.3.0`, Core Ultra 9 275HX) | `v1.3.0` | 24/24 gates, 8/8 mutants, 4-way SHA-256, 58 Lean proofs | PASS | `33.50 M` Win64 / `28.56 M` Linux; 0-bit diff `be46eceb...8f7422a1` |
| 2026-11-22 | Author (Win64 `clang 23.1.0` ThinLTO + Linux `gcc 13.3.0`) | `v1.4.0` (`3b85b15`) | 24/24 gates, 8/8 mutants, 62 Lean proofs (`C59`–`C62`) | PASS | Bit-exact match preserved with ThinLTO |
| 2026-11-25 | Author + GitHub CI (`ubuntu-latest` + `windows-latest`, 6-kernel Lean) | `v1.5.0` (`1b64580`) | 26/26 gates, 8/8 mutants, 70 Lean proofs (6/6 CI kernels GREEN `#45752503579`), 36/36 DOIs, PDF + SLSA `#72484229` | PASS | 4-way exact SHA `681a9747...519eceff`, `96/96` stats wins |
| 2026-11-26 | Author (Win64 `clang 23.1.0` + WSL2 Linux `gcc 13.3.0` + ASan/UBSan + clean-clone verified) + 3-OS GitHub CI (`ubuntu-latest`, `windows-latest`, `macos-latest` ARM64) | `v1.6.0` | **26/26 gates**, 8/8 mutants (incl. sim `M3`), 1,000-case fuzz + `libFuzzer`, 3-module zero-heap `nm`, 6,586 Python oracle, **2,720 `prop_check`**, **13/13 Web tests**, **78/78 Lean proofs (`AT01`–`AT08`) + 6/6 CI kernels + live tamper rejection**, **36/36 Crossref retraction-free DOIs**, zero-overflow AMS-LaTeX PDF + clean-room arXiv, GitHub Pages Blueprint, DigiCert RFC 3161 `.tsr`, `100.0%` `audit-repo.sh` | PASS | 3-OS/2-ISA exact SHA `83317026...532c9bbe` (`bench`), `cafc83d0...15bad443` (`embryo`), `96/96` wins (`stats`, `H2=9.90×`, `H4=73.5×`); external human third-party replication openly invited |
