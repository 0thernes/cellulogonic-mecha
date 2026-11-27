<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Performance Benchmarks & Resource Budgets (`v1.7.0`)

All live `v1.7.0` (bit-identical to `v1.6.0` simulation kernel) measurements below were collected on **Intel Core Ultra 9 275HX** (AVX2 + BMI1 + BMI2 + POPCNT + RDSEED + RDTSCP; 32×32×32 μm periodic off-lattice chamber, 300 epochs, `50,243` cell steps, `1,224,310` exact conserved reactions, locked seed `5000`, `ABI 0x00010700`). Native macOS Apple Silicon ARM64 (`clang`) builds and executes the full 26-gate, 1,000-case fuzz, oracle, property, demo, and Web bridge suite in CI.

## 1. Cross-OS / Cross-Compiler / Cross-ISA Parity & Throughput (`v1.7.0`)

| Platform & Compiler | Kernel | Elapsed (s) | Cell Steps/s | Reactions/s | AVX2 Speedup | Mass/Energy Drift | Post-Warmup Allocs | Canonical State SHA-256 |
|---|---|---|---|---|---|---|---|---|
| Win64 PE64 (`clang 23.1.0`, `-march=x86-64` + `target("avx2")`, ThinLTO, `-Werror`) | `SIMD_AVX2` | `0.0443 s` | `1.135 M/s` | `27.665 M/s` | `1.19×` | `0 amu / 0 uq` | `0` | `83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe` |
| Win64 PE64 (`clang 23.1.0`, `-march=x86-64`, ThinLTO, `-Werror`) | `SCALAR_REF` | `0.0527 s` | `0.954 M/s` | `23.232 M/s` | `1.00×` | `0 amu / 0 uq` | `0` | `83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe` |
| Ubuntu Linux ELF64 (WSL2 `gcc 13.3.0`, `-march=x86-64` + `target("avx2")`) | `SIMD_AVX2` | `0.0517 s` | `0.972 M/s` | `23.681 M/s` | `1.15×` | `0 amu / 0 uq` | `0` | `83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe` |
| Ubuntu Linux ELF64 (WSL2 `gcc 13.3.0`, `-march=x86-64`) | `SCALAR_REF` | `0.0595 s` | `0.844 M/s` | `20.577 M/s` | `1.00×` | `0 amu / 0 uq` | `0` | `83317026b088e005eda739e0c7180b40d15ef9ac4bf9be2c32b690c6532c9bbe` |

*(Historical note: `v1.5.0` executed `1,222,070` reactions at `22.64 M rxns/s` Win64 / `15.75 M rxns/s` Linux with SHA `681a9747...519eceff`. In `v1.6.0`, enforcing unified per-voxel `photon_rem_uq[i]` budgeting at `800 uq`, deduplicating the `8×8` spatial-hash stencil, and preventing same-epoch organelle birth double-stepping via `born_org_mask` increased reaction throughput to `27.665 M rxns/s` Win64 / `23.681 M rxns/s` Linux with exact cross-OS SHA `83317026...532c9bbe`.)*

## 2. Falsification, Fuzzing & Formal Verification Budgets (`v1.7.0`)

| Pipeline Stage | Target Budget | Measured Value (`v1.7.0`) | Status |
|---|---|---|---|
| 26-Gate + 8-Mutant Native Suite (`test_falsification`) | `< 2.0 s` | `0.692 s` (Win64); `0.802 s` (Linux); `2.906 s` (ASan+UBSan) | PASS |
| 1,000-Case Adversarial ABI/CMBIN Fuzzer (`fuzz_cmbin_abi`) | `< 5.0 s` | `1.254 s` Win64; `4.601 s` Linux (`0` crashes, `0` UB, `0` leaks) | PASS |
| 3-Module Hot-Loop Zero-Heap Symbol Audit (`llvm-nm`/`nm`) | `0` alloc imports | `0` (`chemoton_physics.o`, `cellular_world.o`, `metrology_covi.o`) | PASS |
| Post-warmup heap allocations (`G10` / `HW02` / `P10`) | `= 0` | `0` across >10,000 cell steps | PASS |
| Randomized Property & Native C11 Suite (`scripts/prop_check.py`) | `< 5.0 s` | `~0.6 s` (`2,720` trials incl. 20 randomized native C11 runs) | PASS |
| Pure Lean 4.35.0 full clean build (`lake build` + `Challenge`) | `< 60 s` | `~14 s` (10 proof modules + `Spec` + `Challenge` + `Solution`, 78 model theorems) | PASS |
| Live Negative Lean Elaboration & Tamper Rejection (`test_lean_negative.py`) | `< 20 s` | `6/6` exploits caught (2 false-claim elaboration failures, live `Spec` `sorry`-tamper, standalone `sorry`, custom `axiom`, `native_decide`) | PASS |
| WebGL2 3D Observatory frame pacing (`web/renderer3d.js`) | `60 FPS` (`<= 16.6 ms`) | Hardware-instanced icospheres (`maxCells = 4096`), epoch-gated 2D scope | PASS |

## 3. Honest Variance & Bottleneck Notes

- Wall-clock throughput on hybrid x86-64 laptops fluctuates with background workload and P-core turbo state. **Deterministic SHA-256 bit-exactness (`83317026...532c9bbe`) is 100% invariant to CPU frequency or load.**
- The isolated AVX2 kernel (`__attribute__((target("avx2")))`) accelerates spatial finite-volume diffusion (`v4i64`); per-cell reaction stoichiometry, 3D spatial hashing, and Hertz collision loops remain scalar C11, so end-to-end AVX2 speedup is `1.15×`–`1.19×` rather than the theoretical `4×` lane width.
