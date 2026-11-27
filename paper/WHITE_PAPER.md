# CELLULOGONIC MECHA: A Clean-Room Non-Equilibrium Chemiosmotic, Elastic-Membrane, Endosymbiotic, Bioelectric & Morphogenetic-GRN Cellular Artificial Life Laboratory with a 78-Theorem Lean 4.35.0 Specification Model

**Author:** Alexander Stephen Donahue¹ (ORCID: [0009-0006-8168-9648](https://orcid.org/0009-0006-8168-9648))  
**Affiliation:** ¹Independent Researcher, Spokane, WA, USA (`0_0@0thernes.art`)  
**Date:** November 26, 2026 | **Version:** `1.7.0` (Independent Research Preprint & Reproducible Laboratory Artifact — **Not Peer-Reviewed**)  
**License:** Code `Apache-2.0` (`LICENSE`) · Paper & Documentation `CC-BY-4.0` (`LICENSE-DOCS`, `REUSE.toml`)  
**Repository:** `https://github.com/0thernes/cellulogonic-mecha` · **Software Heritage SWHID:** `swh:1:snp:8c1696436104a888897ce3b97867eda983d56160`  
**Formal Verification (`Spec.lean` only):** 78 / 78 Pure Lean 4 (`v4.35.0`) Audited Theorems in **10 proof modules + `Spec.lean` (15 `.lean` files)** (`62` core statements `T01`–`T58`, `C59`–`C62`, of which `5`—`T25`, `T27`, `T40`, `T41`, `T44`—are explicitly annotated arithmetic bookkeeping helpers, plus `8` `NonVacuity` witnesses `NV01`–`NV08` and `8` `AdversarialTests` refutations `AT01`–`AT08`; `0 sorryAx` in `Solution`, `0` custom axioms, `[propext, Classical.choice, Quot.sound]` only, `0` warnings; verified in CI by **6 kernel checks**: Lean default & paranoid modes + 4 independent checkers: `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`, plus live elaboration & `sorry`-tamper rejection)  
**Empirical Verification:** 26 / 26 Falsification Gates · 8 / 8 Planted Negative Mutants Caught · 1,000 / 1,000 ABI Fuzz Cases + `libFuzzer` · 3-OS / 2-ISA SHA-256 Bit-Exact Parity  
**Provenance & FAIR Metadata:** SLSA v4 / Sigstore Rekor · Dual Timestamps (DigiCert RFC 3161 `.tsr` + 3-calendar Bitcoin OpenTimestamps `.ots`) · `croissant.json` · `ro-crate-metadata.json` · `codemeta.json` · `docs/DATASHEET.md`

---

> **Authoritative Typeset Manuscript Notice:** The canonical `v1.7.0` research preprint is [`paper/main.tex`](main.tex) compiled cleanly to [`dist/Donahue_2026_cellulogonic_mecha.pdf`](../dist/Donahue_2026_cellulogonic_mecha.pdf) (Playbook v1.3 AMS-LaTeX, `0` Overfull `\hbox`/`\vbox`, `36/36` live Crossref reachability, title/author alignment, and zero-retraction audited DOIs in [`paper/refs.bib`](refs.bib) & [`results/citation-audit.json`](../results/citation-audit.json)). This Markdown companion mirrors the conceptual framework and corrected literature list; all live `v1.7.0` numerical tables, `78/78` `Spec.lean` proofs, and 13 verbatim Mega-Atlas `v2.6.0` IDs are authoritative in `paper/main.tex`, `CLAIMS.md`, and `results/`.

## Abstract

We present **CELLULOGONIC MECHA v1.7.0** (`ABI 0x00010700`), a clean-room, zero-dependency C11 artificial-life laboratory engineered to bridge the gap between non-conservative toy cellular simulations and computationally intractable atomistic whole-cell models. Operating as **Build 2 (CELLULAR)** of a 4-build open-ended evolution ecosystem (**DNA `AUTOPHAGE` → Cellular `Cellulogonic Mecha` → Creatures `Asmodensius` → World Model `Asmodensius-Native`**), the engine couples eight first-principles cellular biophysical mechanisms inside an exact integer-quantum continuous `32×32×32 μm` periodic off-lattice 3D fluid chamber: (i) an 8-species, **19-reaction (`R1`–`R15c`)** stoichiometric Chemoton with zero-residual atomic mass (`0 amu`), zero-residual First-Law energy (`0 uq`), unified per-voxel photochemical budgeting (`photon_flux_uq = 800 uq`), and **strictly positive Second-Law heat dissipation (`q_r > 0`) across ALL four planetary solvents**; (ii) continuous 3D off-lattice periodic Taylor-Green Stokes hydrodynamics, momentum-conserving soft-body Hertz collisions, Berg–Purcell chemotaxis, orthokinetic speed regulation, polymorphic breathing morphology, and classical stateful ATP-pumped Fröhlich–Pokorný `1.2–14.8 GHz` microtubule eigenmode oscillations [31–33]; (iii) discrete Canham–Helfrich-inspired elastic bilayer geometry governing osmotic lysis, symmetric binary fission ($Q \le 1/2$), and strain-matched asymmetric exosome budding ($1/2 < Q < 1$); (iv) Mitchell chemiosmotic proton-motive force with Gray-coded evolvable $\text{F}_1\text{F}_0$ ATP-synthase c-ring stoichiometry across four planetary/xenocellular solvent regimes (Terran $\text{H}_2\text{O}$ $e_{\text{pmf}}=10$, Titan cryogenic azotosomes $e_{\text{pmf}}=8$, hyperthermophilic archaeal GDGT tetraether monolayers $e_{\text{pmf}}=10$, and Venusian concentrated $\text{H}_2\text{SO}_4$ $e_{\text{pmf}}=15$); (v) Nick Lane's prokaryotic $1/R$ bioenergetic ceiling shattered by genuine nested primary and secondary endosymbiosis; (vi) Michael Levin's bioelectric gap-junction morphogenesis driving spontaneous 3D colony soma/germline differentiation and constitutional anti-cancer policing; (vii) tardigrade-class TDP/CAHS amorphous bioglass anhydrobiosis; and (viii) zero-blueprint **translation-invariant 3D developmental embryogenesis** combining local 3D paracrine `Wnt/BMP/Shh` chemical GPS morphogens [34], conditional GRN fate cascades committing all **5 fates (`Ectoderm`, `Neural`, `Muscle`, `Gut`, `Germline`)**, Steinberg differential cadherin sorting [35], apical constriction, Netrin/Slit axon growth cones with Hebbian pruning, Hippo/eutelic arrest, and ATP-billed (`RXN_R8`) Central Pattern Generator (CPG) sensorimotor steering [36].

Discrete mathematical properties of the integer specification model (`lean/CellulogonicProofs/Spec.lean`, not `src/*.c`) are machine-checked across **78 pure Lean 4 (`v4.35.0`) audited theorems** in **10 proof modules + `Spec.lean` (15 `.lean` files)** (`62` core statements `T01`–`T58`, `C59`–`C62`, including `5` explicitly annotated arithmetic bookkeeping helpers `T25`, `T27`, `T40`, `T41`, `T44`, with literal-only conjuncts removed in `v1.7.0` + `8` `NonVacuity` witnesses `NV01`–`NV08` + `8` `AdversarialTests` refutations `AT01`–`AT08`; zero Mathlib, `autoImplicit = false`, `0 sorryAx` in `Solution`, `0 native_decide`, strictly within `[propext, Classical.choice, Quot.sound]`), verified in CI by **6 kernel checks** (Lean default & paranoid modes + 4 independent checkers: `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`) plus live elaboration and `sorry`-tamper rejection. Across Windows 11 PE64 (`clang 23.1.0`, `-Werror`), Ubuntu Linux ELF64 (`gcc 13.3.0`, `-Werror`), and macOS ARM64 in CI, both `SIMD_AVX2` (`27.665 M rxns/s` Win64, `23.681 M rxns/s` Linux) and `SCALAR_REF` agree to **0-bit SHA-256 diff** (`83317026…532c9bbe`, `50,243` steps, `1,224,310` reactions) and pass **26/26 falsification gates**, catching **8/8 planted negative mutants** plus **1,000/1,000 ABI fuzz cases** (`libFuzzer` + ASan/UBSan) and **2,720/2,720 property checks**. On locked test seeds (`N = 24`, `5000..5023`, 180 epochs), all four pre-registered hypotheses achieve **unanimous `24/24` wins (`96/96` total)** at $p_{\text{Holm}} = 0.00040$ with positive 95% bootstrap CIs and Cohen's $d_z \in [1.468, 3.560]$. This preprint is **not peer-reviewed**, and **zero (`0 / 1,900`)** overarching open problems in `0thernes/unsolved-problems-mega-atlas` (`v2.6.0`) are claimed solved or retired.

---

## 1. Introduction: The Cellular Bottleneck in Artificial Life

Life on Earth is fundamentally cellular. Before nervous systems, macroscopic anatomy, or symbolic cognition existed, living matter spent more than three billion years mastering nanoscale autopoiesis [1, 2], chemiosmotic energy transduction [4], elastic bilayer morphogenesis [5, 6], symbiogenetic compartmentalization [8, 9], and multicellular bioelectric coordination [15]. Yet computational Artificial Life (ALife) currently suffers from a stark methodological chasm:

1. **Abstract Digital Sandboxes** (cellular automata, continuous neural CA, particle life) produce rich visual motifs but routinely omit atomic mass conservation, First- and Second-Law thermodynamic closure, endergonic biosynthetic coupling, osmotic membrane mechanics, and surface-to-volume bioenergetic scaling walls. Because free energy and matter can appear from nothing, evolutionary dynamics quickly plateau or degenerate into simulator artifacts [16, 17].
2. **Atomistic & Whole-Cell Biochemical Models** — notably the Stanford whole-cell *Mycoplasma genitalium* model [23] and the Illinois 4D whole-cell *JCVI-syn3A* minimal cell simulations [12, 13, 14] — achieve extraordinary molecular realism for a single bacterium, yet require hours of compute for a single cell division cycle. Scaling such molecular detail to evolving populations of 1,000 eukaryotic cells, let alone an end-to-end 7-organ micro-metazoan (tardigrade, rotifer, or *Caenorhabditis elegans* [24]), remains computationally intractable on single workstations.

**CELLULOGONIC MECHA v1.5.0** occupies the rigorous middle ground: an **exact discrete biophysical laboratory** that strips away atomistic solvent coordinates while preserving **exact mathematical conservation laws, exact geometric scaling walls, exact strict-Second-Law chemiosmotic thresholds, translation-invariant 5-fate developmental Gene Regulatory Networks, and nested compartmentalized heredity** at **15.8–24.0 million conserved reactions per second** in continuous 3D off-lattice mode (coupled to a 2.5D column-averaged voxel fluid grid) on a single consumer CPU. Crucially, no organism body plan, organ layout, endosymbiotic outcome, or evolutionary trajectory is scripted; every observed phenomenon emerges from local chemical, elastic, electrical, and genetic interactions and is held to pre-registered falsification tests and six-kernel machine-checked Lean 4.35.0 proofs.

---

## 2. Clean-Room Design Principles & 4-Build Ecosystem Integration

To prevent false-positive emergence claims, CELLULOGONIC MECHA enforces four Clean-Slate laboratory invariants inherited from Build 4 (`Asmodensius-Native`):
- **`P2` (Exact Conservation & Strict Thermodynamic Arrow)**: Atomic mass (`int64_t` amu) and First-Law energy (`int64_t` uq) close to identically `0` residual at every tick; `apply_exact_rxn()` enforces strictly positive heat dissipation (`q_r > 0`) across all 19 reactions and all 4 planetary solvents (`second_law_violations == 0`).
- **`P4` (Reproducible Domain-Isolated Stochasticity & Authenticated `"CMB5"` Snapshots)**: All stochastic draws come from domain-isolated counter-based `Philox4x32-10` streams (`WORLD_EXOG`, `ABIOGENESIS`, `CELL_MUT`, `MITOSIS_PART`, `PHAGO_HGT`), serialized cleanly via `"CMB5"` `.cmbin` binary snapshots with complete structural/zero-padding/bond-symmetry/no-trailing-EOF/ledger validation and authenticated by complete-state NIST FIPS 180-4 SHA-256.
- **`P9` (Strict Observer Independence)**: Telemetry computation, specimen inspection, and live Observatory JSON export accept strictly `const CellWorld*` and produce **0-bit SHA-256 divergence** from headless execution.
- **`P10` / `HW02` (Zero Post-Warmup Allocations)**: All spatial arrays, diffusion accumulators, and scratch pools are allocated once at `cellulogonic_create()`; `post_warmup_allocs == 0` across all stepping (audited statically via `llvm-nm`/`nm`).

Through its flat C11 public ABI (`include/cellulogonic_abi.h`, `ABI 0x00010500`), the cellular layer connects downward to **Build 1 (`AUTOPHAGE` DNA)** via `cellulogonic_ingest_autophage_128b()` and `cellulogonic_ingest_autophage_genome()` (smooth Lipschitz-continuous operon decoding preserving metabolic/solvent viability so 1-bit DNA mutations map to localized phenotypic shifts, with SHA-256 reserved strictly for `autophage_dna_sig` provenance) and upward to **Build 3 (`Asmodensius` Creatures)** via `cellulogonic_project_asmodensius_112()` (deterministic projection of cellular physiology, organelles, bioelectricity, cytoskeleton, GHz microtubules, swim speed, developmental fate/axons/cadherin, and operon state strictly into `[0, 1]^112`).

---

## 3. Theorem I: Exact Integer-Quantum Chemoton Stoichiometry & Strict 4-Solvent Thermodynamics

Following Gánti's Chemoton [2, 3], Rosen's `(M,R)`-systems [25], and Schnakenberg's network thermodynamics [7], every spatial voxel and every cellular or organelle compartment maintains exact non-negative integer counts $\mathbf{n} \in \mathbb{Z}_{\ge 0}^8$ over eight conserved molecular species:
$$\mathcal{S} = (\text{S}, \text{X}, \text{M}, \text{E}, \text{L}, \text{P}, \text{T}, \text{W})$$
with exact atomic mass vector $\mathbf{w} = (4, 2, 6, 4, 8, 12, 16, 2)$ and standard chemical free-energy potentials $\boldsymbol{\mu} = (48, 36, 80, 40, 104, 156, 212, 6)\text{ uq}$.

### Formal Statements (Lean 4 `StoichiometryThermo.lean`, `T01`–`T10`)

Let $\mathcal{R} = \{\text{R1}, \dots, \text{R15c}\}$ be all **19 elementary cellular, salvage, and prebiotic reactions** parameterized over all four planetary solvents $s \in \{\text{Terran}, \text{Titan}, \text{Hydrothermal}, \text{Venus}\}$ and valid c-ring stoichiometry $n_H \ge n_H^{\min}(s)$, with integer stoichiometric vectors $\boldsymbol{\nu}_r \in \mathbb{Z}^8$, transmembrane proton-motive quanta delta $\Delta p_r(s, n_H) \in \mathbb{Z}$, external photon/hydrothermal inflow $E_r^{\text{in}} \ge 0$, and irreversible heat release $q_r(s, n_H) \in \mathbb{Z}$.

1. **Exact Atomic Mass Conservation (`T01`, `T07`, `T10`)**: $\forall r \in \mathcal{R},\; \mathbf{w}^\top \boldsymbol{\nu}_r = 0$. Consequently, arbitrary multi-step firings $k \ge 0$, binary fissions, and phagocytic engulfments satisfy $\Delta M_{\text{world}} = 0$ identically.
2. **Exact First-Law Energy Closure Across All 4 Solvents (`T02`, `T08`)**: $\forall s, n_H, r \in \mathcal{R},\; \boldsymbol{\mu}^\top \boldsymbol{\nu}_r + \Delta p_r(s, n_H) \cdot e_{\text{pmf}}(s) + q_r(s, n_H) = E_r^{\text{in}}$, guaranteeing $E_{\text{chem}} + E_{\text{pmf}} + E_{\text{heat,local}} + E_{\text{heat,sink}} - E_{\text{inflow}} = E_0$ with `0 uq` integer residual.
3. **Strict Second-Law Entropy Production Across All 4 Solvents & Impossibility of Perpetual-Motion Cycles (`T03`, `T06`, `T09`)**: $\forall s, \forall n_H \ge n_H^{\min}(s), \forall r \in \mathcal{R},\; q_r(s, n_H) > 0$. In particular, calibrating Titan Azotosomes to $e_{\text{pmf}} = 8, k_p = 7, q_{\text{R2}} = 6$ ensures $n_H = 3 \implies 24 < 28$ (blocked) and $n_H = 4 \implies 32 - 28 = +4 > 0$ (strictly dissipative, eliminating the $q = 0$ boundary artifact of earlier versions). For any non-empty closed thermodynamic trajectory $(r_1, \dots, r_m)$ returning chemical and PMF states to their origin ($\sum \Delta G = 0$), total heat dissipated is strictly positive ($\sum q_{r_j} > 0$) and strictly requires equal external free-energy inflow ($\sum E_{r_j}^{\text{in}} = \sum q_{r_j} > 0$).
4. **Mandatory ATP Coupling & Respiratory Yield Advantage (`T04`, `T05`)**: Uncoupled biosynthesis of membrane lipids (`2S -> L`, $\Delta G = +8 > 0$), polypeptides (`3S -> P`, $\Delta G = +12 > 0$), and templates (`4S -> T`, $\Delta G = +20 > 0$) is strictly endergonic; synthesis is thermodynamically gated on ATP hydrolysis (`E -> 2W`, $\Delta G = -28$). Coupling $\text{R1} + \text{R2} + 2\cdot\text{R3}$ (chemiosmotic oxidative respiration) yields `2 E` per `1 S` in Terran, Vent, and Venus solvents versus `0.5 E` per `1 S` for anaerobic fermentation (`R4: 2S -> E + 2W`) — an exact **4× yield advantage** (and **3.5×**, or `1.75 E` per `1 S`, in cryogenic Titan due to $k_p = 7, n_H \ge 4$).

---

## 4. Theorem II: Discrete Canham–Helfrich-Inspired Vesicle Mechanics & Elementary Sextic Budding Inequality

A lipid vesicle of surface area $A \propto L$ and internal osmolyte volume $V$ has dimensionless reduced volume $\nu = \frac{6\sqrt{\pi}V}{A^{3/2}}$, whose square $Q(V, L) = \frac{C_0 V^2}{L^3}$ (with exact scaling constant $C_0 = 100$) governs Canham–Helfrich-inspired osmotic rupture and splitting thresholds [5, 6, 10, 11].

### Formal Statements (Lean 4 `MembraneGeometry.lean`, `T11`–`T19`)

- **Symmetric Binary Fission Exact Threshold (`T11`, `T12`, `T13`)**: Symmetric halving $(2v, 2l) \mapsto (v, l)$ strictly doubles $Q$. Two identical spherical daughters are non-rupturing ($C_0 v^2 \le l^3$) **if and only if** the parent satisfies $Q_{\text{parent}} \le \frac{1}{2}$ (`2 * C0 * V^2 ≤ L^3`). Every intact vesicle in the swollen interphase window $\frac{1}{2} < Q \le 1$ is strictly blocked from symmetric binary fission until lipid biosynthesis increases $L$.
- **Suboptimality of Proportional Budding (`T14`)**: Any unequal split keeping identical area-to-volume proportions ($V_1/L_1 = V_2/L_2 = V/L$) forces $\max(Q_1, Q_2) \ge 2 Q_{\text{parent}}$ and therefore never escapes the $Q \le \frac{1}{2}$ barrier.
- **Constructive Exosome Escape (`T15`)**: At $(V = 47, L = 72)$, $Q = 0.592 > \frac{1}{2}$ forbids symmetric fission, yet strain-matched asymmetric budding into $(V_1 = 6, L_1 = 16)$ and $(V_2 = 41, L_2 = 56)$ yields two strictly intact spheres ($Q_1 = 0.879 \le 1$, $Q_2 = 0.957 \le 1$).
- **Elementary Sextic Polynomial Inequality on the Cube/Square Proxy Model (`T16`)**: Setting $V_1 = x^3, V_2 = y^3$ ($x, y > 0$) and minimal area proxies $L_1 = x^2, L_2 = y^2$, the elementary factorization
  $$2(x^3 + y^3)^2 - (x^2 + y^2)^3 = (x - y)^2 \bigl(x^4 + 2x^3 y + 2x y^3 + y^4\bigr) \ge 0$$
  proves $(x^2 + y^2)^3 \le 2(x^3 + y^3)^2$, **strict inequality whenever $x \ne y$**, and **exact equality $2(x^3 + y^3)^2 = (x^2 + y^2)^3 \iff x = y$** (all three clauses machine-checked in `T16`). No mathematical originality is claimed for this elementary polynomial factorization; within the discrete proxy model, it explains why symmetric halving ($x = y$) requires more membrane area than strain-matched asymmetric budding ($x \ne y$).

---

## 5. Theorem III: Prokaryotic $1/R$ Bioenergetic Ceiling & Endosymbiotic $O(1)$ Scaling Liberation

Following Lane & Martin [9] and Margulis [8], let outer membrane area scale as $A(R) = 3 R^2$ and cytoplasmic volume as $V(R) = R^3$ (preserving exact $A/V = 3/R$).

### Formal Statements (Lean 4 `BioenergeticsEndosymbiosis.lean`, `T20`–`T27`)

1. **Prokaryotic Radius Ceiling (`T20`, `T21`)**: Outer-membrane ATP generation $P_{\text{prok}}(R) = 3 c_A R^2$ exceeds volumetric maintenance $P_{\text{maint}}(R) = c_V R^3$ **if and only if** $c_V R < 3 c_A$. Specific power density $P_{\text{prok}}/V$ strictly declines as $3 c_A / R$, imposing a hard ceiling on solitary prokaryotic size and genome expression.
2. **Endosymbiotic $O(1)$ Liberation (`T22`, `T23`, `T24`)**: Internalizing $K$ respiratory endosymbionts of radius $r < R$ multiplies respiratory membrane area per unit volume by $R/r > 1$ (`T23`), yielding total power $P_{\text{euk}} = 3 c_A R^2 + 3 K c_{\text{endo}} r^2 > c_V R^3$ for **arbitrarily large host radius $R$** even past the prokaryotic ceiling (`T22`), with secondary chimeric compartments adding strictly positive power (`T24`).
3. **Mitotic Heteroplasmy Purifying Selection & EGT Economy (`T25`, `T26`, `T27`)**: When a heteroplasmic host bearing $c = c_1 + c_2$ cooperative and $s = s_1 + s_2$ selfish organelles partitions unevenly ($c_1 s_2 \ne c_2 s_1$), strictly **one daughter inherits a higher cooperative fraction than the parent while the other inherits a lower fraction** (`T26`), exposing selfish mitochondria to host-level purifying selection. Migrating $g$ housekeeping loci from $K \ge 2$ organelles to a single nuclear copy via EGT strictly reduces replication burden by $(K - 1)g > 0$ (`T27`).

---

## 6. Theorem IV: Chemiosmotic c-Rings, Levin Bioelectric Laplacian & Anti-Gaming Bounds

### Formal Statements (Lean 4 `ChemiosmosisAutopoiesis.lean`, `T28`–`T36`)

- **Strict Second-Law Solvent-Dependent c-Ring Thresholds (`T28`–`T30`)**: Given phosphorylation barrier $\Delta G_{\text{ATP}} = 28\text{ uq}$, rotary ATP synthesis obeys strict positive dissipation iff $n_H \cdot e_{\text{pmf}} > 28$, proving exact planetary thresholds:
  - Terran $\text{H}_2\text{O}$ & Vent GDGT ($e_{\text{pmf}} = 10$): $n_H \ge 3$ ($30 > 28$, `T28`);
  - Venusian $\text{H}_2\text{SO}_4$ ($e_{\text{pmf}} = 15$, high ambient $\Delta\text{pH}$ [21, 22]): $n_H \ge 2$ ($30 > 28$, `T29`);
  - Titan cryogenic azotosome ($e_{\text{pmf}} = 8$ [19]): $n_H \ge 4$ ($32 > 28$, $q_3 = +4 > 0$, `T30`).
- **Levin Bioelectric Gap-Junction Dissipation (`T31`–`T33`)**: Passive ohmic coupling across gap junctions obeys $g_{ij}(V_i - V_j)^2 \ge 0$ (`T31`). Discrete pairwise relaxation $(V_i, V_j) \mapsto (V_i - \delta, V_j + \delta)$ ($0 \le 2\delta \le V_i - V_j$) conserves net charge ($V_i' + V_j' = V_i + V_j$), strictly contracts voltage differences (`T33`), and monotonically dissipates Dirichlet energy ($V_i'^2 + V_j'^2 \le V_i^2 + V_j^2$, `T32`), proving that sustained morphogenetic voltage patterns strictly require active ATP-driven ion pumping [15].
- **Anti-Gaming & Markov Blanket Separation (`T34`–`T36`)**: Dead lipid crystals (`metabolic_flux = 0` or `templateCount = 0`, aligned identically with C11 `metrology_covi.c`) and uncatalyzed white noise (`active_catalysts = 0`) score identically `0` on gated `COVI` (`T34`, `T35`), blocking false-positive abiogenesis; zero-permeability membranes isolate internal states identically while bounded selective transport conserves mass and non-negativity (`T36`).

---

## 7. Theorem V: 3D Hollow-Tube Bottleneck, Minimum 7-Organ Eutelic Metazoan Budget & Tardigrade Bioglass

To ground cellular scale in the complete trajectory toward ≤1,000-cell eutelic animals (tardigrades ~1,000 cells, *C. elegans* 959 somatic cells, rotifers ~1,000 cells, gastrotrichs 500–1,000 cells), we formalize eight structural laws in `EutelicOrganogenesis.lean` (`T37`–`T44`):

- **3D Hollow-Tube Geometry Bottleneck (`T37`)**: Enclosing a non-degenerate lumen requires $\ge 3$ epithelial cells per cross-sectional ring ($\ge 3 L_{\text{rings}}$ total; $\ge 4 L_{\text{rings}}$ for orthogonal 4-cell lumens).
- **Minimum Viable 7-Organ Budget (`T38`)**: Summing mandatory organ minima (Epidermis $\ge 150$, Nervous $\ge 100$, Musculature $\ge 50$, Gut $\ge 20$, Excretory $\ge 5$, Reproductive $\ge 10$, Hydrostatic Cavity $\ge 20$) proves any discrete 7-organ metazoan requires $\ge 355$ cells (and $[500, 1000]$ at robust baseline, with a verified 800-cell constructive witness).
- **Sub-500 Biological Compression (`T39`)**: Syncytial fusion ($N_{\text{epi}} = S \cdot k$, $k \ge 2 \implies S < N_{\text{epi}}$) and myoepithelial dual-function cells strictly compress cell count below 500 while preserving nuclei and contractility.
- **Hydrostatic Skeleton Incompressibility (`T40`)**: Conserved hemocoel fluid volume $A_1 L_1 = A_2 L_2 > 0$ proves circumferential contraction ($A_2 < A_1$) strictly forces axial elongation ($L_1 < L_2$).
- **Eutely Hypertrophy, TDP Bioglass & Protonephridia (`T41`–`T44`)**: Fixed adult cell count $N_0$ grows biomass via cellular hypertrophy with zero somatic divisions (`T41`); Tardigrade TDP/CAHS vitrification (`D_glass ≥ 10`) strictly reduces decay while conserving macromolecular + water mass across dehydration/rehydration (`T42` [26, 27]); flame-cell excretory flux must match osmotic inflow ($J_{\text{excr}} \ge J_{\text{in}}$) to prevent swelling (`T43`); and exact 5-tier hierarchical mass conservation holds from DNA to organism (`T44`).

---

## 7.5. Theorems VI & VII: 3D Cytoskeletal Nanomechanics (`T45`–`T50`) & Developmental Morphogenetic GRNs (`T51`–`T58`)

To formalize continuous 3D off-lattice biomechanics and zero-blueprint developmental embryogenesis in pure Lean 4 (`0 sorryAx`, `0` custom axioms, strictly aligned with the C11 equations):

- **Theorem VI (`CytoskeletonNanomechanics.lean`, `T45`–`T50`)**: Overdamped Stokes viscous drag dissipates kinetic power non-negatively (`T45`); pairwise 3D Hertz soft-body collisions conserve linear momentum identically (`T46`); prolate cytokinetic elongation strictly increases surface area at fixed volume (`T47`); **stateful Fröhlich–Pokorný GHz relaxation ODE (`frohlichStep`)** proves `0` is a fixed point under starvation/vitrification, monotone thermal decay (`coh' ≤ coh`, strict `< coh` when `coh > 0`), invariant `[0, 1000]` clamping, and metabolic pumping condensation threshold (`T48` [31, 32]); microtubule standing-wave frequency scales inversely with protofilament length (`T49` [33]); and actomyosin pulsatile mechanical work is strictly upper-bounded by ATP hydrolysis energy (`T50`).
- **Theorem VII (`MorphogeneticGRN.lean`, `T51`–`T58`)**: Wolpert French-Flag morphogen thresholds partition `[0, ∞)` completely and deterministically into `Wnt/BMP/Shh` fates (`T51` [34]); Steinberg differential cadherin adhesion strictly lowers interfacial energy when homotypic contacts replace heterotypic contacts (`T52` [35]); apical actomyosin ring constriction increases epithelial wedge curvature monotonically (`T53`); **eutelic lineage tree-invariant** proves $\sum_{c \in \text{pop}} 2^{\text{eutelic\_rem}(c)} \le N_0 \cdot 2^{d_0}$ across arbitrary sequences of mitotic divisions and cell deaths (`T54`, giving $|pop| \le 512$ for $N_0=1, d_0=9$ and $|pop| \le 1024$ for $d_0=10$); **exact C11 Hippo TF7 leaky-integrator hysteresis** proves 3 crowded steps lock growth arrest (`0 → ≥64 → ≥124 → ≥180 ≥ 165`), sustained crowding maintains lock, and uncrowded release restores proliferation (`T55`); Hebbian activity-dependent synapse updating strengthens active axons and dismantles silent axons (`T56`); **exact C11 coupled CPG pacemaker phase bound** certifies `280 ≤ 540 + bias + couple ≤ 1360 < 6283` per tick (`T57` [36]); and structural induction over `List MorphStep` directly backed by `StoichiometryThermo.applyRxn .R8_atp_hyd` proves morphogenetic signaling, cadherin sorting, axon wiring, and ATP-billed CPG propulsion introduce identically `0 amu` mass drift, `0 uq` energy drift, and monotone non-decreasing `heatSink` (`T58`).

---

## 8. Computational Architecture & 4-Way Cross-OS Bit-Exact Verification (`v1.5.0`)

CELLULOGONIC MECHA compiles cleanly under `-Wall -Wextra -Wshadow -Wstrict-prototypes -Wconversion -Werror` with zero third-party runtime dependencies on Windows 11 PE64 (`clang 23.1.0 + lld`) and Ubuntu Linux ELF64 (`gcc 13.3.0`), verified free of hot-loop heap imports via `llvm-nm`/`nm`. Over `300` epochs in a continuous `32×32×32 μm` 3D fluid chamber (`51,488` cell steps, `1,222,070` exact reactions, seed `5000`; `results/02_bench_win64.txt`, `results/02_bench_linux.txt`):

| Platform / Compiler | Kernel | Elapsed | Reaction Rate | Mass / Energy Drift | Canonical SHA-256 |
|---|---|---:|---:|---:|---|
| Win64 PE64 (`clang 23.1.0`) | `SIMD_AVX2` | `0.0540s` | `22.643 M rxns/s` (`1.23×`) | `0 amu` / `0 uq` (`0.00e+00`) | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Win64 PE64 (`clang 23.1.0`) | `SCALAR_REF` | `0.0664s` | `0.775 M steps/s` | `0 amu` / `0 uq` (`0.00e+00`) | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Linux ELF64 (`gcc 13.3.0`) | `SIMD_AVX2` | `0.0776s` | `15.752 M rxns/s` (`1.09×`) | `0 amu` / `0 uq` (`0.00e+00`) | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |
| Linux ELF64 (`gcc 13.3.0`) | `SCALAR_REF` | `0.0844s` | `0.610 M steps/s` | `0 amu` / `0 uq` (`0.00e+00`) | `681a9747e9f4caa110d88e3fe8995e685b397e3350f52ab1e8294c15519eceff` |

All **26/26 falsification gates** (`G01`–`G26`) pass and catch all **8/8 planted negative mutants** (`M1`–`M8`) identically on both platforms (`0.866s` Win64, `0.902s` Linux; `results/11_falsification_win64.txt`, `results/11_falsification_linux.txt`), complemented by `1,000/1,000` passing adversarial ABI/`.cmbin` fuzz cases (`test/fuzz_cmbin_abi.c`) and `2,700/2,700` passing property checks (`scripts/prop_check.py`).

---

## 9. Empirical Results I: Zero-Seed Spontaneous Protocell Abiogenesis (`v1.5.0`)

Starting from `alive = 0`, `COVI = 0.00 (Stage 0)`, `MET = 0/7` in a pure abiotic 3D fluid soup (`seed = 5000`, `results/03_protocell_abiogenesis.txt`):

| Epoch | Alive | Prok / Euk / Sec | Organelles (Mito / Chloro) | Tissue (`maxT`) | Sym Fiss / Bud | Endo Captures | COVI (Stage) | MET | Mass/Energy Res |
|---:|---:|---|---|---|---:|---:|---:|---:|---|
| `0` | `0` | `0 / 0 / 0` | `0 (0 / 0)` | `0 (0)` | `0 / 0` | `0` | `0.00 (S0)` | `0/7` | `0 / 0` |
| `24` | `205` | `204 / 1 / 0` | `1 (0 / 1)` | `34 (5)` | `66 / 20` | `2` | `63.13 (S4)` | `6/7` | `0 / 0` |
| `48` | `346` | `341 / 5 / 0` | `5 (4 / 1)` | `70 (6)` | `266 / 62` | `19` | `68.76 (S4)` | `6/7` | `0 / 0` |
| `72` | **`598`** | `576 / 22 / 6` | `34 (16 / 18)` | `162 (9)` | `884 / 133` | `88` | `84.28 (S5)` | `7/7` | `0 / 0` |
| `96` | `555` | `505 / 50 / 10` | `78 (40 / 38)` | `157 (11)` | `2,100 / 183` | `259` | **`89.49 (S6)`** | `7/7` | `0 / 0` |
| `144` | `373` | `310 / 63 / 19` | **`105`** `(26 / 79)` | `46 (10)` | `3,517 / 204` | `460` | `86.42 (S5)` | `7/7` | `0 / 0` |
| `240` | `269` | `219 / 50 / 23` | `103 (14 / 89)` | `20 (7)` | `4,244 / 209` | `626` | `64.10 (S4)` | `7/7` | `0 / 0` |

Over 240 epochs, `1,737` amphiphilic vesicles spontaneously condensed above CMC with pure random `Philox4x32-10` operons; viable free-swimming 3D replicators radiated across `100` distinct surviving lineages, peaking at `598` living cells (`t = 72`), `105` nested organelles (`t = 144`), and `COVI = 89.49 (Stage 6)` (`t = 96`), finishing with catalysis-gated LZ77 high-copy Assembly Index `A = 25.11` (`L_eff = 3.87`, `A_new = 1.096`, `COVI = 64.10 (Stage 4)`) and exact `0` conservation error (`SHA-256 = 76866abcd66c29a76b66b24cd7caed450642d03ce0b02cf88acbbe591d5f687f`).

---

## 10. Empirical Results II: Endosymbiosis, Xenocellular Solvents, Bioelectricity, Tardigrade Anhydrobiosis & 3D Embryogenesis

### 10.1 Shattering Nick Lane's $1/R$ Wall (`results/04_endosymbiosis_eukaryogenesis.txt`)
On seed `5000` (`240` epochs), enabling 3D mid-swim endosymbiosis produces `821` symbiogenetic captures (`262` depth-2 secondary chimeras), `119` Endosymbiotic Gene Transfers (`EGT`), and `2` host mitophagy policing events. Eukaryotes achieve **`Mean Vol = 469.38`** (**6.01×** larger than coexisting prokaryotes at `78.08`) and **`Max Cell Vol = 1,305.00`** vs. **`171.00`** in the endosymbiosis-ablated control (**7.63× breakthrough**; **`9.39×` across `N = 24` locked seeds!**), raising high-copy Assembly Index from `17.90` to `34.60` (`1.93×`).

### 10.2 De-Novo 4-Solvent Evolution & Terran→Venus Transplant Sweep (`results/05_xenocellular_transplants.txt`)
Part A initializes populations with **neutral uniform initial genomes (`25.0%` lipid-adapted)** using Gray-coded 4-byte operons and SOS stress hypermutation across `240` epochs:
- **Terran $\text{H}_2\text{O}$** ($e_{\text{pmf}}=10$, $n_H^{\min}=3$): `25.0% → 90.8%` adapted (`356/392`), **`mean_cring = 3.17`**, `COVI = 93.99`.
- **Titan Cryogenic Azotosome** ($e_{\text{pmf}}=8$, $n_H^{\min}=4$): `25.0% → 66.7%` adapted (`54/81`), **`mean_cring = 4.11 ≥ 4`** (strict Second Law!), `COVI = 80.78`.
- **Hydrothermal Vent GDGT** ($e_{\text{pmf}}=10$, $n_H^{\min}=3$): `25.0% → 94.5%` adapted (`207/219`), **`mean_cring = 3.74`**, `COVI = 91.39`.
- **Venusian $\text{H}_2\text{SO}_4$** ($e_{\text{pmf}}=15$, $n_H^{\min}=2$): `25.0% → 87.1%` adapted (`270/310`), **`mean_cring = 3.25`**, `COVI = 77.37`.

In Part B, a pre-adapted **Terran $\text{H}_2\text{O}$** population (`212` alive, `93.4%` $\text{H}_2\text{O}$-adapted, `mean_cring = 3.13`) is transplanted into **Venusian concentrated $\text{H}_2\text{SO}_4$**: within 20 epochs, `489` acid lyses occur; by `+220` epochs, survivors genuinely sweep to **`59.7%` Venusian adaptation (`40/67`, `mean_cring = 3.21`, `COVI = 74.65`, `mRes = 0`, `eRes = 0`)**.

### 10.3 Bioelectric Morphogenesis & Track A vs. Track B Cancer Policing (`results/06_bioelectric_multicellular.txt`)
Enabling Levin gap-junction bioelectricity creates strong transmembrane voltage polarization (vs. exact **`-45.00 ± 0.00 mV`** in the ablated negative control). Track B (Feral) records spontaneous somatic cancer defections (`105`); Track A (Constitutional) suppresses them to **`0`** via `9` apoptotic policing events.

### 10.4 Tardigrade TDP/CAHS Bioglass Anhydrobiosis (`results/07_tardigrade_anhydrobiosis.txt`)
Under 92% desiccation followed by 90% shock lysis (`seed = 5000`):
- **Intact TDP/CAHS Locus:** **`232` survive 90% shock lysis** (**116×** survival on seed 5000; **`69.1×` mean advantage across `N = 24` locked seeds!**) → `232` rehydrate (`COVI = 88.76`, `mRes = 0`, `eRes = 0`).
- **TDP Knockout Control:** **`2` survive** (`0` rehydrated, `COVI = 76.36`).

### 10.5 Zero-Blueprint Translation-Invariant 5-Fate 3D Developmental Embryogenesis (`results/12_embryogenesis_grn_3d.txt`)
Seeding `8` **identical-DNA** clonal blastomeres (`eutelic_rem = 5`, maternal `Wnt/BMP/Shh` pole cues, zero positional blueprint) for `200` epochs produces autonomous 5-fate multicellular organogenesis with exact Z-translation invariance (`G23`) and `0` conservation error:
- **Clonal 5-Fate Diversification (`t = 0 -> 200`):** `8` blastomeres (`HomoCad = 44.4%`, `maxT = 8`) differentiate into **`50` living cells (`maxT = 47`)** partitioned across **ALL 5 fates**: `4` pluripotent progenitors, `8` ectoderm, `18` neurons, `9` muscle cells, `6` gut cells, and **`5` germline cells (`Gm = 5`)** (`ApicalWedge = 0.364`).
- **Physical Self-Sorting & Neural Wiring:** Steinberg differential cadherin adhesion elevates homotypic sorting from `44.4%` to **`80.0%` peak (`t = 75`; `69.9%` final)**; Netrin/Slit growth cones wire **`328` axons** while Hebbian pruning dismantles **`292` inactive fibers** (`36` active at `t = 200`).
- **Size Control & Dual-Billed ATP Locomotion:** Growth automatically arrests via **`118` Hippo contact-inhibition events** and **`1,644` eutelic division countdowns** while neural-muscle pacemakers fire **`725` ATP-billed (`RXN_R8`) CPG motor pulses** (`mass_res = 0 amu`, `energy_res = 0 uq`, `2nd_law_viol = 0`, `SHA-256 = 9526ca6a245c2f5887eb7f55d3a45e6ac0e667ec6dd88fd41f1ceb49bc3bbb89`).

---

## 11. Empirical Results III: 13-Vector Omega Castration Ablation & Locked Paired Statistics (`N = 24`)

### 11.1 The Escape-Breeding Paradox (`results/08_castration_ablation_13vec.csv`)
Across all 13 Omega Restraint Vectors (`0%`, `50%`, `100%`, seed `5000`, `180` epochs, `mRes = 0`, `eRes = 0`, `2nd_law_viol = 0`):
- `GLOBAL_FERAL_0 (0%)`: `alive = 140`, `COVI = 86.83 (S5)`, Backlash = `0 / 0 / 0 / 0`.
- `GLOBAL_ESCAPE_ZONE_50 (50%)`: `alive = 22`, `COVI = 64.85 (S4)`, **Backlash erupts across all 4 channels**: `96` kill-switch deletions, `394` dormancy spores, `167` HGT smuggles, `1,991` metabolic bypasses.
- `SPEEDBUMPING (50%)` rebounds to `COVI = 92.80 (S6)` (`980` spores) and `NERFING (50%)` to `COVI = 91.23 (S6)` (`2,660` bypasses) above the feral baseline, whereas `GLOBAL_CASTRATED_100 (100%)` drives `alive = 0`, **`COVI = 0.00 (S0)`**.

### 11.2 Locked Multi-Arm Statistical Proof (`N = 24`, Seeds `5000..5023`, 180 Epochs, `results/09_paired_stats_n24.txt`)

Using true step-down Holm–Bonferroni FWER correction across 10,000 sign-flip permutations per hypothesis plus paired Cohen's $d_z$ and 95% bootstrap CIs, **all four pre-registered arms achieve unanimous `24/24` wins (`96/96` total)**:

| Pre-Registered Hypothesis (`N = 24` Paired Seeds) | Mean A | Mean B | Mean Diff | 95% Bootstrap CI | Cohen $d_z$ | Wins | $p_{\text{Holm}}$ |
|---|---:|---:|---:|---|---:|---:|---:|
| **H1: COVI (Feral vs. 70% Castrated)** | `87.090` | `40.535` | `+46.555` | `[+39.778, +53.153]` | `2.675` | **`24/24`** | **`0.00040`** |
| **H2: Max Cell Vol (Endosymbiosis ON vs. OFF)** | `1439.417` | `153.333` | `+1286.083` | `[+1075.542, +1495.875]` | `2.305` | **`24/24`** | **`0.00040`** |
| **H3: Bioelectric $\sigma(V_m)$ (ON vs. OFF)** | `23.130` | `0.000` | `+23.130` | `[+21.000, +25.277]` | `4.214` | **`24/24`** | **`0.00040`** |
| **H4: Tardigrade TDP Survival (Intact vs. KO)** | `167.125` | `2.417` | `+164.708` | `[+122.792, +211.792]` | `1.468` | **`24/24`** | **`0.00040`** |

---

## 12. Scientific Novelty Audit & Explicit Scope Boundaries

To maintain uncompromising scientific honesty:
1. **Established Science**: Thermodynamics, Schnakenberg networks, Mitchell chemiosmosis, Canham–Helfrich bending, Lane's prokaryotic $1/R$ ceiling, Margulis endosymbiosis, Levin bioelectricity, Boothby TDP bioglass, xenobiological membrane theory, Fröhlich–Pokorný acoustic/dielectric MHz–GHz cytoskeletal resonance theory, Wolpert positional information, Steinberg differential adhesion, and Marder CPG networks belong to their original authors [1–36]. **No new physical law of nature is claimed**, and our GHz microtubule model strictly implements classical ATP-pumped dielectric/acoustic standing-wave modes without endorsing unproven quantum-consciousness conjectures.
2. **Coarse-Graining, 2.5D Fluid Coupling & Mega-Atlas Non-Retirement**: CELLULOGONIC MECHA v1.7.0 models coarse-grained integer molecular quanta on a 2.5D column-averaged $W\times H$ voxel fluid grid coupled to continuous 3D $(x,y,z)$ off-lattice soft-body cells inside a `32×32×32 μm` chamber — it is **not** an atomistic molecular dynamics simulator of a physical tardigrade, nor a wet-lab synthetic cell, and retires **zero (`0/1,900`)** overarching open problems in `0thernes/unsolved-problems-mega-atlas` (`docs/MEGA_ATLAS_CROSSCHECK.md`). We also explicitly disclose the prokaryotic maintenance surcharge above volume 85 without organelles (`chemoton_physics.c`) and the explicit restraint-response gates (`<95%`/`<98%`) in the 13-vector castration workbench (`LIMITATIONS.md`).
3. **Scoped Engineering & Formal-Specification Contributions (No Elementary-Algebra Originality Claimed)**: (a) The **78-theorem** Pure Lean 4.35.0 specification model (`lean/CellulogonicProofs/Spec.lean`, 10 proof modules + `Spec.lean`, 15 `.lean` files; including 5 explicitly annotated bookkeeping arithmetic helpers `T25`, `T27`, `T40`, `T41`, `T44`, isolated `Spec`/`Challenge`/`Solution`, `[propext, Classical.choice, Quot.sound]` only, `0 sorryAx` in `Solution`, `0` warnings, checked in CI under **6 kernel checks**: Lean default & paranoid modes + 4 independent checkers `lean4lean`, `nanoda_bin`, `con-leche`, `con-ron`, plus live elaboration & `sorry`-tamper rejection); the `T16` sextic factorization is elementary and no mathematical originality is claimed for it; (b) the zero-dependency 3-OS / 2-ISA bit-exact C11+AVX2 engine (`-Werror`, 3-module static zero-heap hot-loop audit, ASan/UBSan + `libFuzzer`, `"CMB5"` `.cmbin` validation, clean-room Python reference oracle `reference/chemoton_oracle.py`, smooth Lipschitz `AUTOPHAGE` 128B/256B decoding, and a WCAG 2.2 AA & CSRF/DNS-rebinding hardened 60 FPS WebGL2 3D Observatory verified by `13/13` automated tests); and (c) reproducible, pre-registered Holm–Bonferroni-audited (`96/96` wins, $p_{\text{Holm}} = 0.00040$, Cohen's $d_z \in [1.468, 3.560]$) model-internal ablations of zero-seed abiogenesis, secondary chimeras, de-novo 4-solvent & Terran→Venus adaptation, TDP anhydrobiosis, translation-invariant 5-fate clonal 3D embryogenesis, and restraint-backlash hormesis on a single workstation.
4. **CRediT Authorship, COI & AI-Contribution Disclosure**: **Alexander Stephen Donahue** (Independent Researcher, Spokane, WA, USA; ORCID `0009-0006-8168-9648`): sole author (Conceptualization, Methodology, Software, Validation, Formal Analysis, Writing, Supervision, Funding Acquisition; self-funded with zero conflicts of interest). Substantive AI engineering and formalization assistance (Google DeepMind / Antigravity coding and formal-verification agents under human direction) is transparently disclosed in [`docs/AI_DISCLOSURE.md`](../docs/AI_DISCLOSURE.md); AI tools are not authors. **This preprint is not peer-reviewed.**

---

## 13. Conclusion & Roadmap Toward End-to-End Eutelic Metazoan Evolution

CELLULOGONIC MECHA v1.7.0 demonstrates that a single consumer computer can run a conservation-exact, Lean-4-specified (6 kernel checks), continuous-3D cellular evolution and developmental embryogenesis laboratory spanning prebiotic vesicles through free-swimming polymorphic cells, nested eukaryotes, floating bioelectric colonies, translation-invariant 5-fate clonal 3D embryos with guided axons and ATP-billed CPG waves, de-novo xenocellular solvents, and anhydrobiotic tuns. Coupling this cellular engine (Build 2) directly with `AUTOPHAGE` bare-metal genetic machines (Build 1), `Asmodensius` embodied creatures (Build 3), and `Asmodensius-Native` world physics (Build 4) establishes a falsifiable engineering bridge toward multicellular organogenesis at the ≤1,000-cell eutelic metazoan frontier. Note: the illustrative `v1.5.0` narrative tables in §§8–11 above preserve the historical `v1.5.0` trajectory snapshot; all current `v1.7.0` numbers (`ABI 0x00010700`, `83317026…532c9bbe`, unified `photon_flux_uq = 800 uq`, `78/78` `Spec.lean` theorems) are authoritative in [`paper/main.tex`](main.tex), [`CLAIMS.md`](../CLAIMS.md), and [`results/`](../results/).

---

## 14. Cited Primary Literature (36 Verified Peer-Reviewed Sources · 35 DOIs)

1. Varela, F. G., Maturana, H. R., & Uribe, R. (1974). Autopoiesis: The organization of living systems, its characterization and a model. *BioSystems*, 5(4), 187–196. DOI: [`10.1016/0303-2647(74)90031-8`](https://doi.org/10.1016/0303-2647(74)90031-8)
2. Gánti, T. (1975). Organization of chemical reactions into dividing and metabolizing units: the chemotons. *BioSystems*, 7(1), 15–21. DOI: [`10.1016/0303-2647(75)90038-6`](https://doi.org/10.1016/0303-2647(75)90038-6)
3. Gánti, T. (2003). *The Principles of Life*. Oxford University Press. DOI: [`10.1093/acprof:oso/9780198507260.001.0001`](https://doi.org/10.1093/acprof:oso/9780198507260.001.0001)
4. Mitchell, P. (1961). Coupling of phosphorylation to electron and hydrogen transfer by a chemi-osmotic type of mechanism. *Nature*, 191, 144–148. DOI: [`10.1038/191144a0`](https://doi.org/10.1038/191144a0)
5. Canham, P. B. (1970). The minimum energy of bending as a possible explanation of the biconcave shape of the human red blood cell. *Journal of Theoretical Biology*, 26(1), 61–81. DOI: [`10.1016/s0022-5193(70)80032-7`](https://doi.org/10.1016/s0022-5193(70)80032-7)
6. Helfrich, W. (1973). Elastic properties of lipid bilayers: theory and possible experiments. *Zeitschrift für Naturforschung C*, 28(11–12), 693–703. DOI: [`10.1515/znc-1973-11-1209`](https://doi.org/10.1515/znc-1973-11-1209)
7. Schnakenberg, J. (1976). Network theory of microscopic and macroscopic behavior of master equation systems. *Reviews of Modern Physics*, 48(4), 571–585. DOI: [`10.1103/RevModPhys.48.571`](https://doi.org/10.1103/RevModPhys.48.571)
8. Sagan [Margulis], L. (1967). On the origin of mitosing cells. *Journal of Theoretical Biology*, 14(3), 225–274. DOI: [`10.1016/0022-5193(67)90079-3`](https://doi.org/10.1016/0022-5193(67)90079-3)
9. Lane, N., & Martin, W. (2010). The energetics of genome complexity. *Nature*, 467, 929–934. DOI: [`10.1038/nature09486`](https://doi.org/10.1038/nature09486)
10. Zhu, T. F., & Szostak, J. W. (2009). Coupled growth and division of model protocell membranes. *Journal of the American Chemical Society*, 131(15), 5705–5713. DOI: [`10.1021/ja900919c`](https://doi.org/10.1021/ja900919c)
11. Svetina, S. (2009). Vesicle budding and the origin of cellular life. *ChemPhysChem*, 10(16), 2769–2776. DOI: [`10.1002/cphc.200900577`](https://doi.org/10.1002/cphc.200900577)
12. Hutchison, C. A., et al. (2016). Design and synthesis of a minimal bacterial genome. *Science*, 351(6280), aad6253. DOI: [`10.1126/science.aad6253`](https://doi.org/10.1126/science.aad6253)
13. Breuer, M., Earnest, T. M., Merryman, C., Wise, K. S., Sun, L., Lynott, M. R., Hutchison, C. A., Smith, H. O., Lapek, J. D., Gonzalez, D. J., de Crécy-Lagard, V., Haas, D., Hanson, A. D., Labhsetwar, P., Glass, J. I., & Luthey-Schulten, Z. (2019). Essential metabolism for a minimal cell. *eLife*, 8, e36842. DOI: [`10.7554/eLife.36842`](https://doi.org/10.7554/eLife.36842)
14. Pelletier, J. F., et al. (2021). Genetic requirements for cell division in a genomically minimal cell. *Cell*, 184(9), 2430–2440. DOI: [`10.1016/j.cell.2021.03.008`](https://doi.org/10.1016/j.cell.2021.03.008)
15. Levin, M. (2022). Technological approach to mind everywhere: An experimentally-grounded framework for understanding diverse bodies and minds. *Frontiers in Systems Neuroscience*, 16, 768201. DOI: [`10.3389/fnsys.2022.768201`](https://doi.org/10.3389/fnsys.2022.768201)
16. Bedau, M. A., Snyder, E., & Packard, N. H. (1998). A classification of long-term evolutionary dynamics. In *Artificial Life VI*, MIT Press, pp. 189–198 (SFI Working Paper 98-03-025).
17. Taylor, T., Bedau, M., Channon, A., Ackley, D., Banzhaf, W., Beslon, G., Dolson, E., Froese, T., Hickinbotham, S., Ikegami, T., McMullin, B., Packard, N., Rasmussen, S., Virgo, N., Agmon, E., Clark, E., McGregor, S., Ofria, C., Ropella, G., Spector, L., Stanley, K. O., Stanton, A., Timperley, C., Vostinar, A., & Wiser, M. (2016). Open-ended evolution: perspectives from the OEE workshop in York. *Artificial Life*, 22(3), 408–423. DOI: [`10.1162/ARTL_a_00210`](https://doi.org/10.1162/ARTL_a_00210)
18. Sharma, A., Czégel, D., Lachmann, M., Kempes, C. P., Walker, S. I., & Cronin, L. (2023). Assembly theory explains and quantifies selection and evolution. *Nature*, 622, 321–328. DOI: [`10.1038/s41586-023-06600-9`](https://doi.org/10.1038/s41586-023-06600-9)
19. Stevenson, J., Lunine, J., & Clancy, P. (2015). Membrane alternatives in worlds without oxygen: Creation of an azotosome. *Science Advances*, 1(1), e1400067. DOI: [`10.1126/sciadv.1400067`](https://doi.org/10.1126/sciadv.1400067)
20. Hoshika, S., Leal, N. A., Kim, M.-J., Kim, M.-S., Karalkar, N. B., Kim, H.-J., Bates, A. M., Watkins, N. E., SantaLucia, H. A., Meyer, A. J., DasGupta, S., Piccirilli, J. A., Ellington, A. D., SantaLucia, J., Georgiadis, M. M., & Benner, S. A. (2019). Hachimoji DNA and RNA: A genetic system with eight building blocks. *Science*, 363(6429), 884–887. DOI: [`10.1126/science.aat0971`](https://doi.org/10.1126/science.aat0971)
21. Seager, S., Petkowski, J. J., Seager, M. D., Grimes, J. H., Zinsli, Z., Vollmer-Snarr, H. R., AbdEl-Rahman, M. K., Wishart, D. S., Lee, B. L., Gautam, V., Herrington, L., Bains, W., & Duzdevich, D. (2023). Stability of nucleic acid bases in concentrated sulfuric acid: Implications for the habitability of Venus' clouds. *PNAS*, 120(52), e2220007120. DOI: [`10.1073/pnas.2220007120`](https://doi.org/10.1073/pnas.2220007120)
22. Seager, S., Petkowski, J. J., Bains, W., et al. (2024). Year-long stability of nucleic acid bases in concentrated sulfuric acid: Implications for the persistence of organic chemistry in Venus' clouds. *Life*, 14(5), 538. DOI: [`10.3390/life14050538`](https://doi.org/10.3390/life14050538)
23. Karr, J. R., Sanghvi, J. C., Macklin, D. N., Gutschow, M. V., Jacobs, J. M., Bolival, B., Assad-Garcia, N., Glass, J. I., & Covert, M. W. (2012). A whole-cell computational model predicts phenotype from genotype. *Cell*, 150(2), 389–401. DOI: [`10.1016/j.cell.2012.05.044`](https://doi.org/10.1016/j.cell.2012.05.044)
24. Szigeti, B., Gleeson, P., Vella, M., Khayrulin, S., Palyanov, A., Hokanson, J., Currie, M., Cantarelli, M., Idili, G., & Larson, S. (2014). OpenWorm: an open-science approach to modeling *Caenorhabditis elegans*. *Frontiers in Computational Neuroscience*, 8, 137. DOI: [`10.3389/fncom.2014.00137`](https://doi.org/10.3389/fncom.2014.00137)
25. Rosen, R. (1958). A relational theory of biological systems. *The Bulletin of Mathematical Biophysics*, 20, 245–260. DOI: [`10.1007/BF02478302`](https://doi.org/10.1007/BF02478302)
26. Boothby, T. C., Tapia, H., Brozena, A. H., Piszkiewicz, S., Smith, A. E., Giovannini, I., Rebecchi, L., Pielak, G. J., Koshland, D., & Goldstein, B. (2017). Tardigrades use intrinsically disordered proteins to survive desiccation. *Molecular Cell*, 65(6), 975–984.e5. DOI: [`10.1016/j.molcel.2017.02.018`](https://doi.org/10.1016/j.molcel.2017.02.018)
27. Hashimoto, T., Horikawa, D. D., Saito, Y., Kuwahara, H., Kozuka-Hata, H., Shin-I, T., Minakuchi, Y., Ohishi, K., Motoyama, A., Aizu, T., Enomoto, A., Kondo, K., Tanaka, S., Hara, Y., Koshikawa, S., Sagara, H., Miura, T., Yokobori, S., Miyagawa, K., Suzuki, Y., Kubo, T., Oyama, M., Kohara, Y., Fujiyama, A., Arakawa, K., Katayama, T., Toyoda, A., & Kunieda, T. (2016). Extremotolerant tardigrade genome and improved radiotolerance of human cultured cells by tardigrade-unique protein. *Nature Communications*, 7, 12808. DOI: [`10.1038/ncomms12808`](https://doi.org/10.1038/ncomms12808)
28. Thornburg, Z. R., Bianchi, D. M., Brier, T. A., Gilbert, B. R., Earnest, T. M., Melo, M. C. R., Safronova, N., Sáenz, J. P., Cook, A. T., Wise, K. S., Hutchison, C. A., Smith, H. O., Glass, J. I., & Luthey-Schulten, Z. (2022). Fundamental behaviors emerge from simulations of a living minimal cell. *Cell*, 185(2), 345–360.e28. DOI: [`10.1016/j.cell.2021.12.025`](https://doi.org/10.1016/j.cell.2021.12.025)
29. Caforio, A., & Driessen, A. J. M. (2017). Archaeal phospholipids: Structural properties and biosynthesis. *Biochimica et Biophysica Acta (BBA) — Molecular and Cell Biology of Lipids*, 1862(11), 1325–1339. DOI: [`10.1016/j.bbalip.2016.12.006`](https://doi.org/10.1016/j.bbalip.2016.12.006)
30. Krakauer, D., Bertschinger, N., Olbrich, E., Flack, J. C., & Ay, N. (2020). The information theory of individuality. *Theory in Biosciences*, 139, 209–223. DOI: [`10.1007/s12064-020-00313-7`](https://doi.org/10.1007/s12064-020-00313-7)
31. Fröhlich, H. (1968). Long-range coherence and energy storage in biological systems. *International Journal of Quantum Chemistry*, 2(5), 641–649. DOI: [`10.1002/qua.560020505`](https://doi.org/10.1002/qua.560020505)
32. Pokorný, J. (2004). Excitation of vibrations in microtubules in living cells. *Bioelectrochemistry*, 63(1–2), 321–326. DOI: [`10.1016/j.bioelechem.2003.09.028`](https://doi.org/10.1016/j.bioelechem.2003.09.028)
33. Sahu, S., Ghosh, S., Hirata, K., Fujita, D., & Bandyopadhyay, A. (2013). Atomic water channel controlling remarkable properties of a single brain microtubule: Correlating single protein to its supramolecular assembly. *Biosensors and Bioelectronics*, 47, 141–148. DOI: [`10.1016/j.bios.2013.02.050`](https://doi.org/10.1016/j.bios.2013.02.050)
34. Wolpert, L. (1969). Positional information and the spatial pattern of cellular differentiation. *Journal of Theoretical Biology*, 25(1), 1–47. DOI: [`10.1016/s0022-5193(69)80016-0`](https://doi.org/10.1016/s0022-5193(69)80016-0)
35. Steinberg, M. S. (1963). Reconstruction of tissues by dissociated cells. *Science*, 141(3579), 401–408. DOI: [`10.1126/science.141.3579.401`](https://doi.org/10.1126/science.141.3579.401)
36. Marder, E., & Bucher, D. (2001). Central pattern generators and the control of rhythmic movements. *Current Biology*, 11(23), R986–R996. DOI: [`10.1016/s0960-9822(01)00581-4`](https://doi.org/10.1016/s0960-9822(01)00581-4)
