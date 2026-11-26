# HANDOFF.md — `cellulogonic-mecha` (`v1.5.0`)

**Status:** Verified green across Windows 11 x64 PE64 (`clang 23.1.0 + lld`, portable `x86-64-v3`, `-Wall -Wextra -Wshadow -Wstrict-prototypes -Wconversion -Werror`), Ubuntu Linux ELF64 (`gcc 13.3.0`, `-Werror`, plus ASan + UBSan + AVX2 with 0 leaks / 0 UB), Pure Lean 4.35.0 (70/70 theorems, 5 local independent checkers + 6/6 CI Palomar comparator), Node Web Observatory security suite (12/12), clean-room Python exact oracle (6,586 checks + 2,700 property trials), Tectonic AMS-LaTeX preprint (0 overfull hboxes, 36/36 live DOIs), and Build Standard v1.2.0 (`100.0%` — `76/76` structure, `76/76` completion, `33/33` core).

## Repository Verification Summary (`v1.5.0`)
- **Public ABI & Snapshot Format:** `ABI 0x00010500`, `"CMB5"` `.cmbin` zero-padding & SHA-256 validated snapshots (`_Static_assert` layout enforced).
- **Falsification & Fuzzing:** **26 / 26 gates (`G01`–`G26`)**, **8 / 8 planted negative mutants (`M1`–`M8`)**, **1,000 / 1,000 dual-mode standalone + libFuzzer ABI/CMBIN fuzz cases (`test/fuzz_cmbin_abi.c`)**, **0 external `malloc`/`calloc`/`realloc`/`free` symbols in `chemoton_physics` and `cellular_world`**.
- **4-Way Cross-OS Bit-Exact Determinism:** Win64 Clang AVX2 == Win64 Clang Scalar == Linux GCC AVX2 == Linux GCC Scalar -> `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` (`0-bit diff`, `0 amu`, `0 uq`, `0` Second-Law violations, `0` post-warmup allocations).
- **Formal Proofs (Lean 4.35.0, Zero Mathlib):** **70 / 70 audited theorems** (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`), isolated `Spec.lean` / `Challenge.lean` / `Solution.lean`, strictly within `[propext, Classical.choice, Quot.sound]`, `0 sorryAx` in `Solution`, `0 native_decide`, `0` warnings.
- **Web Observatory (`web/`):** WCAG 2.2 AA accessible, zero `'unsafe-inline'` CSP, exact-port loopback Host/Origin DNS-rebinding & CSRF protection, `/healthz` readiness endpoint, WebGL2 graceful fallback, verified by `test/test_web_server.mjs` (`12/12` PASS).
- **Legal & Research Identity:** Author `Alexander Stephen Donahue` (`Independent Researcher, Spokane, WA, USA`, ORCID `0009-0006-8168-9648`); dual-licensed **Apache-2.0** (`LICENSE` — code/Lean/scripts) + **CC-BY-4.0** (`LICENSE-DOCS` — papers/docs/figures). Zero (`0/1,900`) `unsolved-problems-mega-atlas` open problems claimed solved.

## Quick Verification Commands
- Windows build & test: `.\build.ps1`
- Linux build & test: `./build_linux.sh`
- Oracle, properties, versions & web tests: `python3 scripts/verify_oracle_and_stats.py && python3 scripts/prop_check.py && python3 scripts/check_versions.py && node test/test_web_server.mjs`
- Paper & citations: `python3 scripts/check_citations.py && python3 scripts/build_paper.py`
- Build Standard audit: `bash scripts/audit-repo.sh .` (Linux) or `.\scripts\Audit-Repo.ps1` (Windows)
