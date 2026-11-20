# CELLULOGONIC MECHA v1.2

**Clean-Room 3D Off-Lattice Developmental Embryogenesis, Morphogenetic GRN, Chemiosmotic, Cytoskeletal, Endosymbiotic & Xenocellular Artificial Life Engine**
*(Standalone Zero-Dependency Native C11 + Dual AVX2/Scalar Bit-Exact Kernels + 58 Pure Lean 4 Formal Proofs + Live 60 FPS WebGL2 3D Embryogenesis & Cellular Microscopy Observatory)*

> *"A working open-ended-evolution lab, built clean-room from first principles. It runs on a single computer and is held to lab-grade proof, so anyone watching can see new forms and behaviours emerge that no one programmed. Every claim about them is backed by evidence."*

---

## Position in the 4-Build Artificial Life Ecosystem

| Build | Repository / Pillar | Biological Layer | Bridge Interface |
|---|---|---|---|
| **Build 1** | `AUTOPHAGE` / `Zeta-Omega ALife` | **DNA & Genetic Engine** (256-byte polymorphic x86-64 Galois-field genomes, $\mathbb{F}_2$ ribosomes, polynomial sex) | `cellulogonic_ingest_autophage_genome()` ingests 256-byte dual-strand plasmids directly into cellular operons |
| **Build 2** | **`CELLULOGONIC MECHA` (this repo)** | **CELLULAR Layer** (Chemoton stoichiometry, Canham–Helfrich bilayers, Mitchell chemiosmosis, nested primary & secondary endosymbiosis, Levin bioelectric tissues, Tardigrade TDP bioglass anhydrobiosis, 4 xenocellular planetary solvents) | Flat C11 ABI (`include/cellulogonic_abi.h`), Win64 DLL / `.a` & Linux `.so` / `.a` |
| **Build 3** | `Asmodensius` | **Creatures, Beings & Entities** (embodied sensorimotor organisms & eutelic organ budgets) | `cellulogonic_project_asmodensius_112()` projects apex cellular phenotypes to `[0, 1]^112` |
| **Build 4** | `Asmodensius-Native` | **World Model** (thermodynamic/spatial world substrate & Clean-Slate `P1`–`P10` verification invariants) | Exact `P2` (First/Second Law), `P4` (`Philox4x32-10` + `.cmbin` + SHA-256), `P9` (Observer Independence), `P10` (`0` post-warmup allocs) |

---

## Eight Coupled 3D Cellular & Developmental Biophysical Phenomena (Zero Master Blueprints)

1. **3D Developmental Embryogenesis & Self-Organizing Morphogenetic GRN Engine (`CELL_REGIME_EMBRYOGENESIS = 5`, Lean `T51`–`T58`)**
   DNA stores **NO 3D body blueprint** — only local conditional `IF-THEN` Gene Regulatory Network (GRN) rules evaluated against local 3D **Chemical GPS Morphogen Gradients** (`Wnt` A–P, `BMP/Slit` D–V, `Shh/Netrin` Midline). Starting from 8 identical-DNA blastomeres, local TF cascades (`tf[8]`) commit cells epigenetically into **Ectoderm**, **Neural**, **Muscle**, and **Gut** layers; **Steinberg differential cadherins** (`E/N/M/G-Cadherin`) sort mixed cells like oil and water (`44.4% → 85.6%` homophilic purity); **apical actomyosin ring constriction** curves sheets into 3D cups/tubes; **mitotic spindle cleavage angles** switch between rods, sheets/balls, and 3D ganglia; Neural growth cones sniff **Netrin attractants minus Slit repellents** to wire long-range 3D axons with **use-it-or-lose-it Hebbian pruning** (`99` wired, `69` pruned); **Hippo mechanical crowding + morphogen dilution + hardcoded eutelic division countdowns** arrest adult size (`6` Hippo + `681` eutelic stops); and **Central Pattern Generator (CPG) pacemakers** fire rhythmic waves (`397` motor pulses) steered by sensory chemotaxis!
2. **Continuous 3D Off-Lattice Hydrodynamics, Polymorphism, Actomyosin Breathing & GHz Microtubules (`src/cellular_world.c`, `src/chemoton_physics.c`, Lean `T45`–`T50`)**
   Cells are never trapped on 2D grids: they swim, float, tumble, and drift through a continuous `32×32×32 μm` (`32,000³ nm`) 3D fluid chamber with overdamped Stokes drag (`T45`), momentum-conserving soft-body Hertz collisions (`T46`), Berg–Purcell 3D chemotaxis, self-regulated orthokinetic swimming speeds (hungry sprint vs. rich-plume grazing brake), and floating 3D few-cell (`2–16`) & multicellular (`>16`) colonies. Cells continuously deform across diameters (`230–1,480 nm`), biconcave/rod/pinching/amoeboid morphologies, pulsatile actomyosin breathing (`T50`), and ATP-pumped **Fröhlich–Pokorný `1.2–14.8 GHz` microtubule eigenmode oscillations** (`T48`–`T49`, strictly zero coherence when starved or vitrified). Passing `seed = 0` invokes live Intel hardware `RDSEED` thermal entropy for truly non-deterministic feral evolution alongside deterministic locked seeds.
2. **Exact Integer-Quantum 8-Species Chemoton & Non-Equilibrium Thermodynamics (`src/chemoton_physics.c`, Lean `T01`–`T10`)**
   Every voxel and every living compartment tracks exact integer counts of 8 conserved molecular classes (`S` substrate, `X` oxidant, `M` reduced cofactor, `E` ATP, `L` amphiphilic membrane lipid, `P` folded catalytic polypeptide, `T` informational template polymer, `W` exhaust waste) across 17 balanced reactions. Atomic mass residual is identically **`0 amu`**, First-Law energy residual is identically **`0 uq` (`0.00e+00` drift)**, uncoupled macromolecule synthesis (`L`/`P`/`T`) is strictly endergonic (requiring ATP coupling), chemiosmotic respiration delivers an exact **4× ATP yield advantage** over anaerobic fermentation, and irreversible heat flows monotonically into a one-way Second-Law sink (`0` violations).
3. **Discrete Canham–Helfrich Elastic Bilayer Geometry & Asymmetric Budding (`T11`–`T19`)**
   Cells are bounded vesicles governed by exact cleared-denominator squared reduced volume $Q(V, L) = 100 V^2 / L^3$. Symmetric binary fission requires $Q \le \frac{1}{2}$ (`200 V^2 ≤ L^3`), while the swollen interphase window $\frac{1}{2} < Q \le 1$ blocks symmetric division yet permits **strain-matched asymmetric exosome budding** — formally proved via the exact polynomial identity $2(x^3 + y^3)^2 - (x^2 + y^2)^3 = (x - y)^2(x^4 + 2x^3y + 2xy^3 + y^4)$ (`T16`). Mechanosensitive `MscL`-analog stretch gating prevents suicidal hyper-osmotic uptake; exceeding the rupture threshold triggers conserved osmotic lysis releasing detritus and extracellular HGT plasmids.
4. **Nick Lane's $1/R$ Prokaryotic Ceiling Shattered by Nested Serial Endosymbiosis (`T20`–`T27`)**
   Single-membrane prokaryotes suffer outer-surface-capped proton pumping against volumetric maintenance demand ($A/V \propto 1/R$). Phagocytic cells that engulf lysis-resistant, antiporter-expressing neighbors during 3D mid-swim encounters internalize them intact as **living nested organelle compartments** (`CellOrganelle org[8]`) with their own conserved chemistry, own transmembrane PMF, own miniature mtDNA/cpDNA genomes, proto-mitochondrial respiration or proto-chloroplast autotrophy, Endosymbiotic Gene Transfer (`EGT`), stochastic mitotic heteroplasmy partitioning, host mitophagy policing of selfish cheaters, and **depth-2 Secondary Chimeric Endosymbionts**. Empirically (`N = 24` locked seeds), endosymbiosis-enabled 3D worlds reach **`495.08` mean max cell volume** vs. **`206.67`** in endosymbiosis-ablated prokaryotic controls (`24/24` wins, $p_{\text{Holm}} = 0.00040$).
5. **Solvent-Adaptive Chemiosmotic c-Rings Across 4 Planetary / Xenocellular Regimes (`T28`–`T30`)**
   Cells evolve across four physical solvent worlds: **Terran Aqueous $\text{H}_2\text{O}$** ($e_{\text{pmf}}=10$, $n_H \ge 3$), **Titan Cryogenic Azotosome $\text{CH}_4/\text{N}_2$** ($e_{\text{pmf}}=7$, $n_H \ge 4$), **Hydrothermal Vent Archaeal Tetraether GDGT Monolayer** ($e_{\text{pmf}}=10$, $n_H \ge 3$), and **Venusian Sulfuric Acid $\text{H}_2\text{SO}_4$** ($e_{\text{pmf}}=15$, $n_H \ge 2$). Native populations reach **`96.4%–99.4%`** solvent adaptation (`c-ring`: `3.05`, `3.98`, `4.17`, `2.33`) and survive conservation-exact cross-world transplants.
6. **Levin Bioelectric Gap-Junction Morphogenesis & Track A vs. Track B Cancer Policing (`T31`–`T36`)**
   Active ATP-driven ion pumps maintain transmembrane voltages $V_m \in [-90, +40]\text{ mV}$ coupled across 3D cadherin-bonded colonies by conservative, Dirichlet-energy-dissipating gap-junction relaxation (`T32`). Depolarized cells upregulate foraging/motility (**soma**), hyperpolarized cells upregulate replication/lipid synthesis (**germline**), and boundary/interior roles emerge without global coordinates. Track B (Feral) exhibits spontaneous somatic cancer defections (`11` events); Track A (Constitutional) eliminates them (`0` defections) via apoptotic policing.
7. **Tardigrade TDP/CAHS Bioglass Anhydrobiosis & 13-Vector Omega Castration Lab (`T37`–`T44`)**
   Cells expressing Locus 15 intrinsically disordered bioglass protectants (`TDP/CAHS`) vitrify into amorphous **cryptobiotic tun states** under 92% desiccation, achieving a **`61.1×` mean survival advantage** (`33.125` vs. `0.542` survivors across locked seeds `5000..5023`, `24/24` wins, $p_{\text{Holm}} = 0.00040$) over TDP-knockout controls under 90% shock lysis, then rehydrate with exact `0 amu` / `0 uq` conservation. Across all 13 Omega Restraint Vectors, 50% partial restraints empirically trigger the **Escape-Breeding Paradox** (`73` kill-switch deletions, `490` dormancy spores, `107` HGT smuggles, `2,654` metabolic bypasses; `DEGRADING 50%` rebounds to `COVI = 93.41 (Stage 6) > 82.48` feral baseline), while 100% total restraint collapses vitality to `COVI = 0.00`.

---

## Verified Proof & Benchmark Highlights (v1.2.0, Measured on Intel Core Ultra 9 275HX)

| Verification Gate | Verified Result | Primary Evidence |
|---|---|---|
| **Lean 4 `v4.35.0` Formal Proofs** | **58 / 58 PROVEN (`0 sorryAx`, `0` custom axioms, `0` warnings)** | [`results/lean4-audit.txt`](results/lean4-audit.txt) |
| **Win64 PE64 Falsification (`clang 23.1.0`)** | **24 / 24 PASSED · 8 / 8 Planted Mutants Caught (`0.252s`)** | [`results/11_falsification_win64.txt`](results/11_falsification_win64.txt) |
| **Linux ELF64 Falsification (`gcc 13.3.0`)** | **24 / 24 PASSED · 8 / 8 Planted Mutants Caught (`0.269s`)** | [`results/11_falsification_linux.txt`](results/11_falsification_linux.txt) |
| **4-Way Cross-OS / Cross-Kernel SHA-256 Parity** | **0-Bit Diff** (`e46cd73a…fd169089`) · **`34.82 M rxns/s`** | [`results/02_bench_win64.txt`](results/02_bench_win64.txt) · [`results/02_bench_linux.txt`](results/02_bench_linux.txt) |
| **Exact Mass / 1st-Law / 2nd-Law Closure (`P2`)** | **`0 amu`, `0 uq`, `0.00e+00` drift, `0` violations** | Every run in [`results/`](results/) |
| **3D Embryogenesis (8 Identical-DNA Blastomeres)** | **`71` alive (`maxT 57`), `85.6%` cadherin sort, `99` axons, `397` CPG pulses** | [`results/12_embryogenesis_grn_3d.txt`](results/12_embryogenesis_grn_3d.txt) |
| **Zero-Seed Spontaneous Protocell Abiogenesis** | **`0` cells → peak `581` alive, `70` euk, `41` sec chimeras, `COVI 89.38 (S6)`** | [`results/03_protocell_abiogenesis.txt`](results/03_protocell_abiogenesis.txt) |
| **Locked Paired Stats (`N = 24`, Seeds `5000..5023`)** | **4 / 4 Hypotheses significant (`24/24` ×3, `23/24` ×1; $p_{\text{Holm}} \le 0.00080$)** | [`results/09_paired_stats_n24.txt`](results/09_paired_stats_n24.txt) |

---

## Quick Start

### 1. Build & Verify Everything on Windows x64 (Clang 23.1.0 + LLD + 24 Gates + 58 Lean 4 Proofs)
```powershell
.\build.ps1
```

### 2. Build & Verify on Linux / WSL2 ELF64 (GCC 13.3.0 + 24 Gates)
```bash
chmod +x build_linux.sh
./build_linux.sh
```

### 3. Run Standalone Clean-Room 3D Developmental & Cellular Lab Experiments
```powershell
.\bin\cellulogonic.exe embryo --epochs 200 --size 32 --seed 5000
.\bin\cellulogonic.exe probe
.\bin\cellulogonic.exe bench --epochs 300 --size 32 --seed 5000
.\bin\cellulogonic.exe abiogenesis --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe endosymbiosis --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe xenocell --epochs 180 --size 32 --seed 5000
.\bin\cellulogonic.exe bioelectric --epochs 240 --size 32 --seed 5000
.\bin\cellulogonic.exe anhydrobiosis --epochs 120 --size 32 --seed 5000
.\bin\cellulogonic.exe ablation --epochs 180 --size 32 --seed 5000 --out results\08_castration_ablation_13vec.csv
.\bin\cellulogonic.exe stats --pairs 24 --epochs 180 --size 32
.\bin\cellulogonic.exe bridge --seed 5000
```

### 4. Launch the Live 60 FPS WebGL2 3D Embryogenesis & Cellular Microscopy Observatory
```powershell
node web\server.mjs
# Open http://127.0.0.1:8788 (click 🧫 Grow 3D Embryo, ⚡ Feral RDSEED, or 🧬 AUTOPHAGE DNA!)
```

---

## Direct Drop-In C11 ABI (`include/cellulogonic_abi.h`)

```c
#include "cellulogonic_abi.h"

CellWorldConfig cfg;
cellulogonic_default_config(&cfg);
cfg.regime = CELL_REGIME_ABIOGENESIS_ZERO_SEED;
cfg.seed   = 5000ULL;

CellWorld* world = cellulogonic_create(&cfg);
cellulogonic_step_epochs(world, 240);

CellSpecimen apex;
cellulogonic_inspect_apex(world, &apex);

float asmodensius_traits[112];
cellulogonic_project_asmodensius_112(&apex, asmodensius_traits);

char sha256_hex[65];
cellulogonic_state_sha256(world, NULL, sha256_hex); /* Observer-independent P9 hash */
cellulogonic_destroy(world);
```

---

## Documentation & Manuscripts

- **Master Index & TOC**: [`INDEX.md`](INDEX.md)
- **Executive Summary Paper & Novelty Verdict**: [`SUMMARY.md`](SUMMARY.md)
- **Professional Research White Paper (33 Verified DOIs)**: [`paper/WHITE_PAPER.md`](paper/WHITE_PAPER.md)
- **Master Deep-Dive Report (Cells, Organelles, ≤1,000-Cell Tardigrade Eutelic Organogenesis, Xenobiology & Cellular A-Life Singularity)**: [`docs/MASTER_REPORT.md`](docs/MASTER_REPORT.md)
- **Verification & Audit Ledger**: [`VERIFICATION.md`](VERIFICATION.md)
- **Technical Architecture & ABI Specification**: [`ARCHITECTURE.md`](ARCHITECTURE.md)
- **Glossary (60+ Terms)**: [`GLOSSARY.md`](GLOSSARY.md)
- **Legal, License & Proof Boundaries**: [`LICENSE`](LICENSE) · [`LEGAL.md`](LEGAL.md) · [`NOTICE`](NOTICE)

---

## License

Copyright © 2026 **0thernes (0thernes LLC)** (`0_0@0thernes.art`). Licensed under the **COSMOGONIC LICENSE — NON-COMMERCIAL RESEARCH & PLAY** ([`LICENSE`](LICENSE)). Non-commercial study, research, reproduction, and modification are broadly permitted with attribution intact; all commercial rights are reserved by 0thernes LLC.
