<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Verification Architecture (`docs/verification.md`)

**CELLULOGONIC MECHA `v1.6.0`** combines four independent layers of verification so no single tool, compiler, or kernel acts as an unchecked single point of trust (also see root [`VERIFICATION.md`](../VERIFICATION.md)).

## Layer 1 — Six-Kernel Pure Lean 4.35.0 Formal Verification (`lean/`)
- **Scope:** **78 audited theorems** (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`, `AT01`–`AT08`) across 11 `module` files (`Spec.lean` + 8 biophysical modules + `NonVacuity.lean` + `AdversarialTests.lean`), zero Mathlib, `autoImplicit = false`, `relaxedAutoImplicit = false`, `linter.unusedVariables = true`, `linter.deprecated = true`.
- **Palomar Isolation & Live Tamper Rejection:** `Challenge.lean` imports ONLY pure `Spec.lean`; `Solution.lean` exports the 78 sorry-free proofs; `comparator.json` pins all `78` theorems plus `12` core `Spec.lean` definitions and permitted axioms (`[propext, Classical.choice, Quot.sound]`). Both `scripts/test_lean_negative.py` (`sorry`, custom `axiom`, `native_decide`) and a CI sorry-tampered `lake comparator --paranoid` failure gate prove the verification gate catches real exploits.
- **Independent Kernel Replay & README Lock:** Ubuntu CI executes all 6/6 kernels under `bubblewrap`, diffs `results/lean4-audit.txt` verbatim against fresh `VerifyAxioms.lean` output, and runs `scripts/check_readme_axioms.py`.

## Layer 2 — Native C11 Falsification, Mutants, Property Testing, Sanitizers & Fuzzing
- **26-Gate + 8-Mutant Suite (`test/test_falsification.c`):** `26 / 26` PASS (`G01`–`G26`), `8 / 8` planted negative mutants (`M1`–`M8`, incl. simulation-level `M3`) caught across Windows x86-64 (`clang 23.1.0 -Werror`), Linux x86-64 (`gcc 13.3.0 -Werror`), and macOS ARM64 (`clang -Werror` in CI), plus 3-module `llvm-nm`/`nm` zero-heap hot-loop symbol verification (`chemoton_physics`, `cellular_world`, `metrology_covi`).
- **Cross-OS / Cross-ISA Bit-Exact Parity:** `SIMD_AVX2` == `SCALAR_REF` == macOS ARM64 (`SHA-256 = 83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe`, 0-bit diff, `0 amu` / `0 uq` drift, `0` allocs).
- **Property-Based Suite (`scripts/prop_check.py`):** `2,720 / 2,720` randomized trials across mathematical laws plus 20 randomized native C11 binary executions.
- **Sanitizers, Fuzzing, Python Oracle & Web Security Suite:** `-fsanitize=address,undefined -fno-sanitize-recover=all` + `clang -DCELL_LIBFUZZER -fsanitize=fuzzer,address,undefined` run in Ubuntu CI; `test/fuzz_cmbin_abi.c` runs `1,000 / 1,000` adversarial `"CMB5"` binary/ABI fuzz cases in every build; `reference/chemoton_oracle.py` + `scripts/verify_oracle_and_stats.py` execute `6,586` exact-rational oracle & `p_Holm <= 0.05` statistical checks; `test/test_web_server.mjs` verifies `13 / 13` Web Observatory HTTP/CSP/CSRF/DNS-rebinding/favicon invariants.

## Layer 3 — Statistical, Bibliographic & Publication Verification
- **Locked Paired Statistics (`N = 24`, seeds `5000..5023`):** Deterministic 10,000-iteration sign-flip permutation test + step-down Holm–Bonferroni FWER across pre-registered `H1`–`H4` (`96/96` wins, `p_Holm = 0.00040 <= 0.05`, `d_z ∈ [1.468, 3.560]`).
- **Live Crossref Retraction Audit & Zero-Overflow Paper Build:** `scripts/check_citations.py` verifies `36 / 36` BibTeX DOIs live against Crossref (`results/citation-audit.json`, `0` retractions, `0` title/year mismatches); `scripts/build_paper.py` compiles `paper/main.tex` with Tectonic enforcing `0` Overfull `\hbox`/`\vbox`, `0` undefined refs/citations/labels, and clean-room `dist/arxiv_submission.tar.gz` compilation.

## Layer 4 — Repository, Blueprint & Cryptographic Provenance Audit
- **Build Standard v1.2.0 & Schema Validation:** `scripts/Audit-Repo.ps1` / `scripts/audit-repo.sh` verifying `100.0%` Structure / `100.0%` Completion / `100.0%` Core, with `repo.yaml` strictly validated against `repo-standard.schema.json`, plus static Interactive Blueprint published via GitHub Pages (`.github/workflows/pages.yml`).
- **Cryptographic Checksums, TSA & CI Security:** `CHECKSUMS.sha256`, `data/MANIFEST.sha256`, DigiCert RFC 3161 TSA (`data/timestamps/CHECKSUMS.sha256.tsr`), SLSA v4 (`actions/attest-build-provenance@v4`), `cppcheck`, and SHA-256-verified `gitleaks 8.30.1` + `osv-scanner 2.7.1`.
