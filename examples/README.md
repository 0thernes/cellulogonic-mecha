<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Runnable Examples (`examples/`)

This directory provides self-contained runnable demonstrations exercising the compiled native C11 `CELLULOGONIC MECHA v1.6.0` binary and verifying deterministic output invariants.

## Examples

1. [`01_embryogenesis_demo.py`](01_embryogenesis_demo.py) — Runs the zero-blueprint 5-fate 3D clonal embryogenesis simulation (`seed=5000`, `200` epochs) via the OS-appropriate native binary (or `CELLULOGONIC_BIN`) and asserts exact conservation (`mass_res=0 amu`, `energy_res=0 uq`, `2nd_law_viol=0`), all 5 differentiated fates (`P=4, Ec=8, N=18, M=5, G=6, Gm=6`), and SIMD == Scalar SHA-256 parity (`cafc83d04db7b5858876821cb16c2aa1afa7792645f508fa40ad35bd15bad443`).
2. [`quickstart_c_abi.c`](quickstart_c_abi.c) — Portable minimal compiled C11 ABI (`0x00010600`) quickstart built and verified automatically by `build.ps1` and `build_linux.sh` across Windows, Linux, and macOS ARM64.

## Running

```bash
python3 examples/01_embryogenesis_demo.py
```
