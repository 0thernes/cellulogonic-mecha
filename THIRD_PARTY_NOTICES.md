# Third-Party Notices & Toolchain Attributions

**CELLULOGONIC MECHA v1.5.0** ships **zero third-party vendored code or runtime packages** in its C11 simulation engine, Lean 4 formal proof suite (zero Mathlib), Python verification scripts (stdlib only), or WebGL2 3D Observatory (zero npm dependencies).

## External Build, Formal Verification & Typesetting Toolchains (Not Vendored)

| Tool / Component | License | Role in Repository |
|---|---|---|
| LLVM / Clang 23.1.0 + LLD | Apache-2.0 WITH LLVM-exception | Compiles Win64 PE64 binaries, DLL, and static archives (`build.ps1`) |
| GCC 13.3.0 | GPL-3.0-or-later WITH GCC-exception-3.1 | Compiles Linux ELF64 binaries, `.so`, and static archives (`build_linux.sh`, CI) |
| Lean 4 (`v4.35.0`) & Lake | Apache-2.0 | Machine-checks all formal proofs in `lean/` (zero Mathlib) |
| Independent Lean Kernels (`leanchecker`, `leanchecker-paranoid`, `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`) | Apache-2.0 / MIT | Multi-kernel replay & Palomar comparator verification |
| Tectonic TeX Engine | MIT | Compiles `paper/main.tex` to `dist/Donahue_2026_cellulogonic_mecha.pdf` |
| Node.js (v20/v22+) | MIT | Optional local HTTP bridge (`web/server.mjs`) for the WebGL2 3D Observatory |
| Python 3.11+ (stdlib only) | PSF-2.0 | Runs `scripts/build_paper.py`, `scripts/check_citations.py`, `scripts/check_readme_axioms.py` |
| GitHub Actions (`actions/checkout`, `actions/attest-build-provenance`) | MIT | Informational CI matrix, multi-kernel audit, and SLSA build provenance |

No third-party trademarks or institutional endorsements are implied.
