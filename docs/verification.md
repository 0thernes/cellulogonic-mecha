# Verification Architecture (`docs/verification.md`)

**CELLULOGONIC MECHA `v1.5.0`** combines four independent layers of verification so no single tool, compiler, or kernel acts as an unchecked single point of trust (also see root [`VERIFICATION.md`](../VERIFICATION.md)).

## Layer 1 — Six-Kernel Pure Lean 4.35.0 Formal Verification (`lean/`)
- **Scope:** 70 audited theorems (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`) across 10 `module` files (`Spec.lean` + 8 biophysical modules + `NonVacuity.lean`), zero Mathlib, `autoImplicit = false`, `relaxedAutoImplicit = false`, `linter.unusedVariables = true`, `linter.deprecated = true`.
- **Palomar Isolation:** `Challenge.lean` (445 lines, 19,487 bytes) imports ONLY pure `Spec.lean`; `Solution.lean` exports the 70 sorry-free proofs; `comparator.json` pins every name and permitted axiom (`[propext, Classical.choice, Quot.sound]`).
- **Independent Kernel Replay:**
  - Local Windows 11 (`results/lean4-multi-kernel.txt`): `lean`, `leanchecker --fresh`, `leanchecker-paranoid --fresh`, `con-leche --verified` (`3,326` decls), `con-ron --verified` (`3,326` decls), `nanoda_bin` (`3,735` decls, `unpermitted_axiom_hard_error: true`).
  - Ubuntu CI (`.github/workflows/ci.yml` with `bubblewrap`): all 6/6 kernels including `lean4lean`, `lake check --paranoid`, and `lake comparator --config comparator.json --paranoid`.
- **README Verbatim Lock:** `scripts/check_readme_axioms.py` verifies `README.md` matches `results/lean4-audit.txt` character-for-character with zero forbidden axioms.

## Layer 2 — Native C11 Falsification, Mutants, Property Testing, Sanitizers & Fuzzing
- **26-Gate + 8-Mutant Suite (`test/test_falsification.c`):** `26 / 26` PASS (`G01`–`G26`), `8 / 8` planted negative mutants (`M1`–`M8`) caught on both Win64 PE64 (`clang 23.1.0`, `-Werror`) and Linux ELF64 (`gcc 13.3.0`, `-Werror`), plus static `llvm-nm`/`nm` zero-heap hot-loop symbol verification.
- **4-Way Cross-OS / Cross-ISA Bit-Exact Parity:** `SIMD_AVX2` == `SCALAR_REF` on Win64 & Linux (`SHA-256 = 681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff`, 0-bit diff, `0 amu` / `0 uq` drift, `0` allocs).
- **Property-Based Suite (`scripts/prop_check.py`):** `2,700 / 2,700` randomized seeded trials across sextic minimax, 4-solvent stoichiometry, gap-junction Dirichlet energy, Hippo hysteresis, and continuum yolk/biotensegrity laws.
- **Sanitizers, Fuzzing, Python Oracle & Web Security Suite:** `-fsanitize=address,undefined -march=x86-64-v3 -fno-sanitize-recover=all` runs in Ubuntu CI & `just sanitize`; dual-mode `test/fuzz_cmbin_abi.c` runs `1,000 / 1,000` adversarial `"CMB5"` binary/ABI fuzz cases in every build (and exports `LLVMFuzzerTestOneInput` for `libFuzzer`); `reference/chemoton_oracle.py` + `scripts/verify_oracle_and_stats.py` execute `6,586` exact-rational oracle & differential checks; `test/test_web_server.mjs` verifies `12 / 12` Web Observatory HTTP/CSP/CSRF/DNS-rebinding invariants.

## Layer 3 — Statistical & Publication Verification
- **Locked Paired Statistics (`N = 24`, seeds `5000..5023`):** Deterministic 10,000-iteration sign-flip permutation test + step-down Holm–Bonferroni FWER across pre-registered `H1`–`H4` (`96/96` wins, `p_Holm = 0.00040`).
- **Live Citation & Zero-Overflow Paper Build:** `scripts/check_citations.py` verifies `36 / 36` BibTeX DOIs live against `doi.org/api/handles/`; `scripts/build_paper.py` compiles `paper/main.tex` with Tectonic enforcing `0` Overfull `\hbox`, `0` undefined refs/citations, and clean-room `dist/arxiv_submission.tar.gz` compilation.

## Layer 4 — Repository & Provenance Audit
- **Build Standard v1.2.0:** `scripts/Audit-Repo.ps1` / `scripts/audit-repo.sh` verifying `100.0%` Structure / `100.0%` Completion / `100.0%` Core.
- **Cryptographic Checksums & CI Security:** `CHECKSUMS.sha256`, `data/MANIFEST.sha256`, CI `SHA256SUMS`, `actions/attest-build-provenance@v2`, `cppcheck`, and `gitleaks`.
