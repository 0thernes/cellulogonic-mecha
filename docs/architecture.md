# System Architecture (`docs/architecture.md`)

**CELLULOGONIC MECHA `v1.5.0`** operates as **Build 2 (CELLULAR)** in a clean-room four-build Artificial Life architecture, combining a zero-dependency native C11 simulation core with a 6-kernel Pure Lean 4.35.0 formal specification suite and a local 60 FPS WebGL2 3D Observatory.

```mermaid
flowchart LR
  B1["Build 1: AUTOPHAGE\n128B/256B GF(2^8) DNA"] -->|ingest_autophage_128b| B2["Build 2: CELLULOGONIC MECHA\nC11 + AVX2/Scalar Core\nABI 0x00010500"]
  B2 -->|project_asmodensius_112| B3["Build 3: Asmodensius\nEmbodied [0,1]^112 Creatures"]
  B3 -->|embodied| B4["Build 4: Asmodensius-Native\nWorld Substrate (P2/P4/P9/P10)"]
  L["Lean 4.35.0 (70 Theorems)\nSpec / Challenge / Solution\n6 Independent Kernels"] -.->|Formal Invariants| B2
  T["26 Gates + 8 Mutants\n+ 1,000 Fuzz + 2,700 prop_check\n+ Python Oracle + 12 Web Tests"] -.->|Executable Audit| B2
  B2 -->|JSON stream (const CellWorld*)| W["web/server.mjs (127.0.0.1:8788)\nCSRF-Hardened WebGL2 3D Observatory"]
```

## 1. Native C11 Simulation Core (`include/`, `src/`)

| File | Responsibility | Key Invariant |
|---|---|---|
| `include/cellulogonic_abi.h` | Flat public C11 ABI (`0x00010500`), enums (`CellSpecies`, `CellSolventRegime`, `CellOperatingRegime`, `CellMorphFate`, `CellRestraint`), structs, and `CELL_API` signatures | Zero third-party deps; negative-bounds hardened (`G25`) |
| `include/cellulogonic_internal.h` | Internal structs (`_Static_assert` explicit padding), 19-reaction matrix, 4-solvent `SOLVENT_TABLE`, RNG stream domains | Exact match to `lean/CellulogonicProofs/Spec.lean` |
| `src/crypto_rng.c` | Domain-isolated `Philox4x32-10` PRNG, CPUID-guarded `RDSEED`, NIST FIPS 180-4 SHA-256, QPC/monotonic timer | Bit-exact across Windows PE64 and Linux ELF64 |
| `src/chemoton_physics.c` | 19-reaction integer Chemoton (`apply_exact_rxn`), Gray-coded 4-byte operons, `MscL` transport, chemiosmosis, organelle metabolism, ledger audit | `0 amu` mass, `0 uq` First-Law, strict `q_r > 0` across all 4 solvents |
| `src/cellular_world.c` | Dual 256-bit AVX2 (`v4i64`) / Scalar finite-volume diffusion, 3D periodic Taylor–Green Stokes hydrodynamics, spatial hash, Hertz collisions, fission/budding/lysis, endosymbiosis, 5-fate clonal 3D embryogenesis, axons & CPG | 0-bit SHA-256 parity (`SIMD_AVX2 == SCALAR_REF`); `0` post-warmup allocs |
| `src/metrology_covi.c` | 8-pillar `COVI`, catalysis-gated LZ77 Assembly Index, smooth Lipschitz `AUTOPHAGE` 128B/256B decoder, `[0,1]^112` `Asmodensius` projection | Strictly `const CellWorld*` (`P9` observer independence) |
| `src/abi_exports.c` | Lifecycle with config clamping, complete-state SHA-256, authenticated `"CMB5"` `.cmbin` save/load, Tardigrade desiccation/shock/rehydration, JSON export | Full bounds, zero-padding, bond-symmetry, no-trailing-EOF & ledger checks |
| `src/main_cli.c` | Reproducible CLI workbench (`probe`, `bench`, `abiogenesis`, `endosymbiosis`, `xenocell`, `bioelectric`, `anhydrobiosis`, `ablation`, `stats`, `bridge`, `embryo`, `json`, `stream`) | Step-down Holm–Bonferroni inference |

## 2. Formal Verification Suite (`lean/`)

- `CellulogonicProofs/Spec.lean`: Pure shared definitions only (`module`, `public`, zero proofs, zero `sorry`).
- `CellulogonicProofs/{StoichiometryThermo,MembraneGeometry,BioenergeticsEndosymbiosis,ChemiosmosisAutopoiesis,EutelicOrganogenesis,CytoskeletonNanomechanics,MorphogeneticGRN,ContinuumBiomechanics,NonVacuity}.lean`: 70 sorry-free machine-checked theorems (`T01`–`T58`, `C59`–`C62`, `NV01`–`NV08`).
- `Challenge.lean` ↔ `Solution.lean` ↔ `comparator.json`: Palomar isolated specification contract verified locally across 5 Windows-capable kernels and across all 6/6 kernels under `bubblewrap` in CI.

See root [`ARCHITECTURE.md`](../ARCHITECTURE.md) for full byte-level memory layout and operon locus tables.
