# Runnable Examples (`examples/`)

This directory provides self-contained runnable demonstrations exercising the compiled native C11 `CELLULOGONIC MECHA v1.5.0` binary and verifying deterministic output invariants.

## Examples

1. [`01_embryogenesis_demo.py`](01_embryogenesis_demo.py) — Runs the zero-blueprint 5-fate 3D clonal embryogenesis simulation (`seed=5000`, `200` epochs) via the native binary and asserts exact conservation (`mass_res=0 amu`, `energy_res=0 uq`, `2nd_law_viol=0`), all 5 differentiated fates (`P=4, Ec=8, N=18, M=9, G=6, Gm=5`), and canonical `v1.5.0` SHA-256 `9526ca6a245c2f5887eb7f55d3a45e6ac0e667ec6dd88fd41f1ceb49bc3bbb89`.
2. [`quickstart_c_abi.c`](quickstart_c_abi.c) — Minimal compiled C11 ABI (`0x00010500`) quickstart built and verified automatically by `build.ps1` and `build_linux.sh`.

## Running

```bash
python3 examples/01_embryogenesis_demo.py
```
