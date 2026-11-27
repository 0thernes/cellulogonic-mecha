# Roadmap (`ROADMAP.md`)

**Maintenance Status:** `active` (`v1.7.0`, November 2026)

## Completed Milestones
- **`v1.0.0`–`v1.2.1`:** Clean-room C11 + dual AVX2/Scalar bit-exact cellular engine, 8-species 19-reaction Chemoton, Canham–Helfrich vesicles, serial endosymbiosis, Levin bioelectricity, Tardigrade TDP bioglass, 3D off-lattice fluid chamber, 5-fate clonal 3D embryogenesis, 60 FPS WebGL2 Observatory.
- **`v1.3.0`–`v1.4.0`:** Strict 4-solvent Second Law (`q_r > 0`), de-novo neutral-genome solvent adaptation, translation-invariant 3D paracrine morphogens, `"CMB4"` cryptographic snapshot validation, `ContinuumBiomechanics.lean` (`C59`–`C62`), AVX-VNNI + ThinLTO builds.
- **`v1.5.0`–`v1.6.0`:** Dual open licensing (`Apache-2.0` / `CC-BY-4.0`), canonical independent researcher attribution, 78-theorem Palomar Lean 4.35.0 formal suite (`Spec`/`Challenge`/`Solution`/`NonVacuity`/`AdversarialTests`), portable `-march=x86-64` + isolated `target("avx2")` + macOS ARM64 parity, `"CMB5"` snapshot validation, Playbook v1.3 AMS-LaTeX white paper (`36/36` live-verified DOIs), `prop_check` + `libFuzzer` + ASan/UBSan CI, GitHub Pages Proof Blueprint, and 100.0% Build Standard v1.2.0 compliance.
- **`v1.7.0`:** Resolved all 9 formal-code and manuscript errata from the 2026-11-26 statement audit (strengthened `T16` with explicit equality iff, removed literal-only conjuncts from `T41`/`T54`/`C62`/`NV08`, annotated 5 bookkeeping helpers, strengthened `T39`/`T43` with non-underflow conservation + `Int` companion, upgraded `test_lean_negative.py` to 6/6 elaboration + live `sorry`-tamper rejection, upgraded `check_citations.py` with Crossref title+author alignment, added FAIR metadata `croissant.json` / `ro-crate-metadata.json` / `codemeta.json` / `docs/DATASHEET.md`, and dual DigiCert RFC 3161 `.tsr` + 3-calendar Bitcoin OpenTimestamps `.ots`).

## Planned Next
- **Extended Scale & GPU Parity Study:** Evaluate deterministic fixed-point diffusion kernels on larger `64×64×64` chambers while preserving 0-bit SHA-256 parity against the scalar C11 oracle.
- **External Replication Registry:** Incorporate independent third-party reproduction logs into [`REPLICATION.md`](REPLICATION.md) and publish archived Zenodo/Software Heritage snapshots once externally assigned.
- **Expanded C11 → Lean Differential Harness:** Automate direct JSON trace export from `bin/cellulogonic` into Lean 4 concrete trace replay tests.

## Explicitly Not Planned
- **Atomistic Molecular Dynamics:** We will not add explicit water-molecule MD or quantum-chemical force fields; that belongs to dedicated whole-cell MD projects (e.g., *JCVI-syn3A* 4D Lattice Microbes).
- **Quantum Consciousness / Unfalsifiable Speculation:** The classical Fröhlich–Pokorný dielectric damping ODE (`T48`) will never be marketed as quantum consciousness or Orch-OR.
- **Proprietary Lock-In or Telemetry:** This repository will never collect user telemetry, phone home, or revert from open Apache-2.0 / CC-BY-4.0 licensing.
