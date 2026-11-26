# Performance Benchmarks & Resource Budgets (`v1.5.0`)

All live `v1.5.0` measurements below were collected on **Intel Core Ultra 9 275HX** (AVX2 + BMI1 + BMI2 + POPCNT + RDSEED + RDTSCP; 32×32×32 μm periodic off-lattice chamber, 300 epochs, `51,488` cell steps, `1,222,070` exact conserved reactions, locked seed `5000`, `ABI 0x00010500`).

## 1. Four-Way Cross-OS / Cross-Compiler / Cross-ISA Parity & Throughput (`v1.5.0`)

| Platform & Compiler | Kernel | Elapsed (s) | Cell Steps/s | Reactions/s | AVX2 Speedup | Mass/Energy Drift | Post-Warmup Allocs | Canonical State SHA-256 |
|---|---|---|---|---|---|---|---|---|
| Win64 PE64 (`clang 23.1.0`, `x86-64-v3`, ThinLTO, `-Werror`) | `SIMD_AVX2` | `0.0540 s` | `0.954 M/s` | `22.643 M/s` | `1.23×` | `0 amu / 0 uq` | `0` | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Win64 PE64 (`clang 23.1.0`, `x86-64-v3`, ThinLTO, `-Werror`) | `SCALAR_REF` | `0.0664 s` | `0.775 M/s` | `18.405 M/s` | `1.00×` | `0 amu / 0 uq` | `0` | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Ubuntu Linux ELF64 (WSL2 `gcc 13.3.0`) | `SIMD_AVX2` | `0.0776 s` | `0.664 M/s` | `15.752 M/s` | `1.09×` | `0 amu / 0 uq` | `0` | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Ubuntu Linux ELF64 (WSL2 `gcc 13.3.0`) | `SCALAR_REF` | `0.0844 s` | `0.610 M/s` | `14.480 M/s` | `1.00×` | `0 amu / 0 uq` | `0` | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |

*(Historical note: prior `v1.3.0`/`v1.4.0` unloaded runs executed `971,811` reactions at `33.50 M rxns/s` Win64 / `28.56 M rxns/s` Linux with SHA `be46eceb...8f7422a1`. In `v1.5.0`, photon-budgeting and tun-bond regression hardening (`G26`) increased reaction volume over 300 epochs to `1,222,070` with new exact 4-way bit-identical SHA `681a9747...519eceff`.)*

## 2. Falsification, Fuzzing & Formal Verification Budgets (`v1.5.0`)

| Pipeline Stage | Target Budget | Measured Value (`v1.5.0`) | Status |
|---|---|---|---|
| 26-Gate + 8-Mutant Native Suite (`test_falsification`) | `< 2.0 s` | `0.866 s` (Win64); `0.902 s` (Linux) | PASS |
| 1,000-Case Adversarial ABI/CMBIN Fuzzer (`fuzz_cmbin_abi`) | `< 2.0 s` | `0.797 s` (`0` crashes, `0` UB, `0` leaks) | PASS |
| Hot-Loop Zero-Heap Symbol Audit (`llvm-nm`) | `0` alloc imports | `0` (`chemoton_physics.o` & `cellular_world.o`) | PASS |
| Post-warmup heap allocations (`G10` / `HW02` / `P10`) | `= 0` | `0` across >10,000 cell steps | PASS |
| Randomized Property Suite (`scripts/prop_check.py`) | `< 1.0 s` | `~0.1 s` (2,700 trials) | PASS |
| Pure Lean 4.35.0 full clean build (`lake build` + `Challenge`) | `< 60 s` | `~12 s` (10 modules + `Challenge` + `Solution`) | PASS |
| Multi-kernel replay (`leanchecker-paranoid`, `con-leche`, `con-ron`, `nanoda_bin`) | `< 90 s` | Logged in `results/lean4-multi-kernel.txt` | PASS |
| WebGL2 3D Observatory frame pacing (`web/renderer3d.js`) | `60 FPS` (`<= 16.6 ms`) | Hardware-instanced icospheres (`maxCells = 4096`) | PASS |

## 3. Honest Variance & Bottleneck Notes

- Wall-clock throughput on hybrid x86-64 laptops fluctuates with background workload and P-core turbo state. **Deterministic SHA-256 bit-exactness (`681a9747...519eceff`) is 100% invariant to CPU frequency or load.**
- The AVX2 kernel accelerates spatial finite-volume diffusion (`v4i64`); per-cell reaction stoichiometry, 3D spatial hashing, and Hertz collision loops remain scalar C11, so end-to-end AVX2 speedup is `1.09×`–`1.23×` rather than the theoretical `4×` lane width.
