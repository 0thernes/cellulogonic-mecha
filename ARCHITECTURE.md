# CELLULOGONIC MECHA v1.1 — Technical Architecture & ABI Specification

## 1. Four-Build Ecosystem Architecture

**CELLULOGONIC MECHA** serves as **Build 2 (CELLULAR)** in the 0thernes 4-build clean-room Artificial Life stack:

```
┌────────────────────────────────────────────────────────────────────────────┐
│ Build 4 · WORLD MODEL (Asmodensius-Native)                                 │
│ Conserved macro-environment, Clean-Slate P1-P10, locked seed bands         │
└───────────────────────────────▲────────────────────────────────────────────┘
                                │ 112-trait physiological vector + P2/P4/P9/P10
┌───────────────────────────────┴────────────────────────────────────────────┐
│ Build 3 · CREATURES / BEINGS / ENTITIES (Asmodensius)                      │
│ Embodied sensorimotor bodies, connectomes, <=1,000-cell eutelic organs     │
└───────────────────────────────▲────────────────────────────────────────────┘
                                │ cellulogonic_project_asmodensius_112()
┌───────────────────────────────┴────────────────────────────────────────────┐
│ Build 2 · 3D CELLULAR ENGINE (CELLULOGONIC MECHA v1.1 — THIS REPOSITORY)   │
│ Continuous 3D Off-Lattice Fluid · Polymorphic Breathing · GHz Microtubules │
│ 8-Species Chemoton · Canham-Helfrich Bilayers · Mitchell Chemiosmosis      │
│ Nested Primary/Secondary Endosymbiosis · Levin Bioelectric 3D Colonies     │
│ Tardigrade TDP Bioglass Anhydrobiosis · 4 Xenocellular Planetary Solvents  │
└───────────────────────────────▲────────────────────────────────────────────┘
                                │ cellulogonic_ingest_autophage_genome()
┌───────────────────────────────┴────────────────────────────────────────────┐
│ Build 1 · DNA / GENETICS (AUTOPHAGE / Zeta-Omega ALife Package)            │
│ 256-byte dual-strand polymorphic x86-64 Galois-field organisms & ribosomes │
└────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Zero-Allocation 3D Off-Lattice Memory Layout (Clean-Slate `HW02` / `P10`)

Every heap buffer is allocated once inside `cellulogonic_create()` via `calloc()`. During `cellulogonic_step_epochs()`, **zero dynamic allocations occur** (`post_warmup_allocs == 0`).

| Buffer | Type | Count | Purpose |
|---|---|---|---|
| `voxels` | `CellVoxel` | $W \times H$ | Conserved extracellular `env[8]`, `quorum`, `heat_local_uq`, `free_plasmid[32]` |
| `cells` | `CellEntity` | $W \times H$ | Free-swimming 3D cells (`pos_nm[3]`, `vel_nm[3]`, `heading_q10[3]`, `radius_nm`, `elong_q10`, `ruffle_q10`, `breath_*`, `mt_freq_mhz`, `mt_coherence_q10`, `bond_partners[4]`, `mol[8]`, `pmf`, `v_mem_mv`, `genome[128]`, nested `org[8]`) |
| `hash_head` / `hash_next` | `int32_t` | $W \times H$ | Zero-allocation $O(N)$ 3D spatial hash linked list for soft-body collisions, docking & phagocytosis |
| `diff_delta` | `int64_t` | $8 W H$ | Exact pairwise anti-symmetric finite-volume chemical flux accumulator |
| `heat_delta` | `int64_t` | $W H$ | Exact conservative thermal diffusion accumulator |
| `quorum_delta` | `int64_t` | $W H$ | Autoinducer morphogen diffusion accumulator |
| `vmem_delta` | `int32_t` | $W H$ | Exact conservative gap-junction voltage Laplacian buffer (`T32`) |
| `bfs_queue` / `bfs_vis` | `uint32_t` / `uint8_t` | $W H$ | Preallocated scratch (read-only telemetry uses stack arrays so `P9` never mutates world!) |

---

## 3. Exact 8-Species Stoichiometric Matrix & Reaction Potentials (`T01`–`T10`)

Species mass weights $\mathbf{w} = (4, 2, 6, 4, 8, 12, 16, 2)\text{ amu}$ and standard chemical free energies $\boldsymbol{\mu} = (48, 36, 80, 40, 104, 156, 212, 6)\text{ uq}$ for `(S, X, M, E, L, P, T, W)`:

| ID | Biological Pathway | Balanced Stoichiometry | $\Delta \text{Mass}$ | $\Delta \text{PMF}$ | $E_{\text{inflow}}$ | $q_{\text{heat}}$ (Terran, $e_{\text{pmf}}=10$) |
|---|---|---|---:|---:|---:|---:|
| `R1` | Catabolic redox activation | $\text{S} + \text{X} \to \text{M}$ | `0` | `0` | `0` | `+4 uq` |
| `R2` | Chemiosmotic proton pumping | $\text{M} \to 3\text{W} + k_p\,\text{PMF}$ | `0` | `+6` | `0` | `+2 uq` |
| `R3` | Rotary F₁F₀ ATP synthase | $2\text{W} + n_H\,\text{PMF} \to \text{E}$ | `0` | `−3` (`−n_H`) | `0` | `+2 uq` ($10 n_H - 28$) |
| `R4` | Anaerobic fermentation | $2\text{S} \to \text{E} + 2\text{W}$ | `0` | `0` | `0` | `+44 uq` |
| `R5` | Lipid bilayer synthesis | $2\text{S} + \text{E} \to \text{L} + 2\text{W}$ | `0` | `0` | `0` | `+20 uq` |
| `R6` | Polypeptide translation | $3\text{S} + \text{E} \to \text{P} + 2\text{W}$ | `0` | `0` | `0` | `+16 uq` |
| `R7` | Template replication | $4\text{S} + \text{E} \to \text{T} + 2\text{W}$ | `0` | `0` | `0` | `+8 uq` |
| `R8` | ATP maintenance / work | $\text{E} \to 2\text{W}$ | `0` | `0` | `0` | `+28 uq` |
| `R9` | Lipid turnover / decay | $\text{L} \to 4\text{W}$ | `0` | `0` | `0` | `+80 uq` |
| `R10` | Proteolysis | $\text{P} \to 6\text{W}$ | `0` | `0` | `0` | `+120 uq` |
| `R11` | Nuclease / acid depurination | $\text{T} \to 8\text{W}$ | `0` | `0` | `0` | `+164 uq` |
| `R12` | Passive transmembrane leak | $1\,\text{PMF} \to \text{Heat}$ | `0` | `−1` | `0` | `+10 uq` ($e_{\text{pmf}}$) |
| `R13a/b/c` | Autophagic salvage | $\text{L}\to\text{S}+2\text{W}$, $\text{P}\to2\text{S}+2\text{W}$, $\text{T}\to3\text{S}+2\text{W}$ | `0` | `0` | `0` | `+44 / +48 / +56` |
| `R14a` | Phototrophic carbon fixation | $2\text{W} + 46\,\text{uq} \to \text{S}$ | `0` | `0` | `46` | `+10 uq` |
| `R14b` | Oxygenic oxidant fixation | $\text{W} + 34\,\text{uq} \to \text{X}$ | `0` | `0` | `34` | `+4 uq` |

---

## 4. Four Planetary / Xenocellular Solvent Regimes (`T28`–`T30`)

| ID | Solvent Regime | $e_{\text{pmf}}$ | `R2` PMF Yield | Min c-Ring $n_H$ | Optimal `lipid_class` | Osmotic Burst (%) | Thermal Decay Divisor | Diffusion Shift |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| `0` | **Terran Aqueous $\text{H}_2\text{O}$** | `10` | `6` | `3` | `0` (Phospholipid) | `115%` | `64` | `4` (`/16`) |
| `1` | **Titan Cryogenic Azotosome** | `7` | `8` | `4` | `1` (Azotosome) | `135%` | `192` | `5` (`/32`) |
| `2` | **Hyperthermophilic Vent GDGT** | `10` | `6` | `3` | `2` (Tetraether) | `110%` | `28` | `3` (`/8`) |
| `3` | **Venusian Acid $\text{H}_2\text{SO}_4$** | `15` | `4` | `2` | `3` (Polyphos-Xeno) | `110%` | `36` | `4` (`/16`) |

---

## 5. Evolvable 128-Byte Catalytic & Regulatory Operon Map

| Locus | Byte Range | Encoded Biophysical Machinery |
|---|---|---|
| `0` | `0..7` | `R1` catabolic dehydrogenase rate (`genome[0]`) & allosteric gain |
| `1` | `8..15` | `R2` respiratory electron-transport proton pump density (`genome[8]`) |
| `2` | `16..23` | F₁F₀ ATP-synthase c-ring stoichiometry $n_H = 2 + (\text{genome}[16] \bmod 4)$ & turbine rate (`genome[17]`) |
| `3` | `24..31` | `R4` anaerobic glycolytic fermentation enzyme (`genome[24]`) |
| `4` | `32..39` | `R5` amphiphilic lipid synthase (`genome[32]`) |
| `5` | `40..47` | `R6` ribosomal polypeptide translation rate (`genome[40]`) |
| `6` | `48..55` | `R7` polymerase template replication rate (`genome[48]`) |
| `7` | `56..63` | `R14a` phototrophic fixation (`genome[56]`) & `R14b` oxygenesis (`genome[57]`) |
| `8` | `64..71` | Xenocellular membrane chemistry class `lipid_class = genome[64] & 3` |
| `9` | `72..79` | Selective `S` permease (`genome[72]`) & `X` permease (`genome[73]`) gated by `MscL` stretch channel |
| `10` | `80..87` | Phagocytosis receptor (`80`), protease digestion (`81`), antiporter ATP sharing (`82`), lysis resistance (`83`), host mitophagy policing (`84`) |
| `11` | `88..95` | Asymmetric budding/exosome trigger (`88`), cryptobiotic endospore dormancy switch (`90`) |
| `12` | `96..103` | Levin bioelectric ion-pump resting target $V_m$ (`96`), quorum polarization shift (`97`), gap-junction connexin conductance (`98`) |
| `13` | `104..111` | Quorum autoinducer secretion (`104`), multicellular tissue adhesion (`105`), cooperative vs. somatic-cancer allele (`106`) |
| `14` | `112..119` | Natural HGT transformation competence (`112`), CRISPR/restriction phage defense (`113`) |
| `15` | `120..127` | Polymerase proofreading (`120`), kill-switch repair/excision (`121`), **Tardigrade TDP/CAHS bioglass protectant (`122`)** |

---

## 6. Dual-Kernel Bit-Exact AVX2 vs. Scalar Equivalence (`G07`)

Spatial finite-volume diffusion updates all 8 conserved species across toroidal East and South faces via pairwise anti-symmetric fluxes $f_e = \mathrm{trunc}((v_i - v_e) / 2^s)$, $f_s = \mathrm{trunc}((v_i - v_s) / 2^s)$.
- **Scalar kernel (`diffuse_fields_scalar`)**: standard C11 signed integer division `(a - b) / (1LL << s)`.
- **AVX2 kernel (`diffuse_fields_avx2`)**: processes 4 × `int64_t` species simultaneously using GCC/Clang native 256-bit vector types (`v4i64`, lowering directly to YMM `vpaddq`/`vpsubq`/`vpcmpgtq`/`vpsrlq`/`vpor` with zero `<immintrin.h>` header dependency) via exact truncation-toward-zero bias:
  $$\mathrm{sdiv}_{2^s}(v) = \mathrm{asr}\bigl(v + ((v < 0) \mathbin{\&} (2^s - 1)),\; s\bigr)$$
- Because every intermediate bit is identically defined across signed 64-bit two's complement arithmetic, **Win64 Clang AVX2 == Win64 Clang Scalar == Linux GCC AVX2 == Linux GCC Scalar** with **0-bit SHA-256 diff** (`5d515453…e515174a`).

---

## 7. Observer Independence (`P9`) & Binary Snapshot Format (`.cmbin`, `P4`)

- `cellulogonic_get_telemetry()`, `cellulogonic_state_sha256()`, `cellulogonic_inspect_slot()`, `cellulogonic_inspect_apex()`, and `cellulogonic_export_grid_json()` take strictly `const CellWorld*` and allocate zero heap memory (`compute_max_tissue_cluster` uses local stack arrays), guaranteeing 0-bit observer invariance verified by Gate `G08` + Planted Mutant `M4`.
- `.cmbin` binary snapshots serialize `SnapshotHeader` (`magic = 0x434D4231 "CMB1"`, `abi_version = 0x00010000`, full exact ledger & counters) followed by exact `voxels[]` and `cells[]` arrays, permitting bit-exact save on `SIMD_AVX2` and cross-kernel resume on `SCALAR_REF` (`G09`).
