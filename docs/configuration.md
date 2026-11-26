<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Configuration & CLI Reference (`docs/configuration.md`)

All runtime parameters for **CELLULOGONIC MECHA `v1.6.0`** are configured through deterministic CLI flags, the C11 `CellWorldConfig` struct, or optional environment variables documented in [`.env.example`](../.env.example). **Zero secrets or external API keys are required.**

## 1. Environment Variables (`.env.example`)

| Variable | Default | Scope | Description |
|---|---|---|---|
| `PORT` | `8788` | `web/server.mjs` | Local loopback HTTP port (`127.0.0.1` only) for the WebGL2 3D Observatory |
| `CELLULOGONIC_BIN` | OS auto-detected | `web/server.mjs`, `examples/`, `scripts/` | Optional explicit path to the compiled native C11 binary |
| `TECTONIC_BIN` | auto-detected | `scripts/build_paper.py` | Optional override path to the `tectonic` TeX compiler |
| `CC` | `gcc` (Linux) / `clang` (macOS) | `build_linux.sh` | C11 compiler override on POSIX systems |

## 2. Native CLI Subcommands (`bin/cellulogonic`)

All numeric CLI flags (`--seed`, `--epochs`, `--size`, `--solvent`, `--pairs`) are validated strictly by `parse_u64_strict()`; malformed values (e.g., `--seed abc`) exit immediately with status code `2`.

| Command | Canonical Example | Purpose |
|---|---|---|
| `probe` | `cellulogonic probe` | Prints ABI version (`0x00010600`), CPU vendor/brand (x86-64 CPUID or ARM64), ISA flags |
| `bench` | `cellulogonic bench --epochs 300 --size 32 --seed 5000` | Runs `SIMD_AVX2` vs. `SCALAR_REF` parity and throughput benchmark |
| `abiogenesis` | `cellulogonic abiogenesis --epochs 240 --size 32 --seed 5000` | Zero-seed spontaneous protocell condensation from abiotic soup |
| `endosymbiosis` | `cellulogonic endosymbiosis --epochs 240 --size 32 --seed 5000` | Endosymbiosis ON vs. prokaryotic-only OFF comparison |
| `xenocell` | `cellulogonic xenocell --epochs 240 --size 32 --seed 5000` | Neutral-genome de-novo 4-solvent adaptation & Terran→Venus transplant |
| `bioelectric` | `cellulogonic bioelectric --epochs 240 --size 32 --seed 5000` | Track B feral vs. Track A constitutional vs. bioelectricity-ablated control |
| `anhydrobiosis` | `cellulogonic anhydrobiosis --epochs 120 --size 32 --seed 5000` | Intact Tardigrade TDP bioglass tun vs. TDP-knockout shock survival |
| `ablation` | `cellulogonic ablation --epochs 180 --size 32 --seed 5000 --out results/08_castration_ablation_13vec.csv` | Full 13-vector restraint sweep (`0% / 50% / 100%`) |
| `stats` | `cellulogonic stats --pairs 24 --epochs 180 --size 32` | Locked paired permutation tests (`H1`–`H4`) with Holm–Bonferroni FWER |
| `embryo` | `cellulogonic embryo --epochs 200 --size 32 --seed 5000` | Zero-blueprint 5-fate 3D clonal embryogenesis from 8 identical-DNA blastomeres |
| `bridge` | `cellulogonic bridge --seed 5000` | Verifies Build 1 `AUTOPHAGE` 128B/256B ingestion → Build 3 `Asmodensius` `[0,1]^112` projection |
| `stream` | `cellulogonic stream` | Interactive stdin/stdout JSON bridge driven by `web/server.mjs` |

## 3. C11 `CellWorldConfig` Clamping Guarantees

`cellulogonic_create()` defensively clamps untrusted config fields before allocation:
- `restraints[0..12]` clamped strictly to `[0, 100]`.
- `regime` clamped to `[0, CELL_REGIME_EMBRYOGENESIS (5)]`.
- `solvent` clamped to `[0, CELL_SOLVENT_COUNT - 1 (3)]`.
- `photon_flux_uq` defaults to `800u` (unified per-voxel budget shared by abiotic photochemistry, host autotrophs, and chloroplasts).
- `seed == 0` explicitly invokes hardware `RDSEED`/`RDRAND` or portable OS entropy; any nonzero `seed` guarantees 0-bit deterministic replay.
