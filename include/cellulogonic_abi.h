#ifndef CELLULOGONIC_ABI_H
#define CELLULOGONIC_ABI_H

/*
 * ============================================================================
 * CELLULOGONIC MECHA v1.4.0
 * Clean-Room Non-Equilibrium Chemiosmotic, Morphogenetic & Endosymbiotic
 * Cellular Artificial Life Engine — Flat C11 Public ABI
 * ============================================================================
 *
 * Ecosystem Role:
 *   Pillar 1: Asmodensius        (World & Creatures / Entities / Beings)
 *   Pillar 2: AUTOPHAGE / Omega  (DNA / Polymorphic Genetic Engine)
 *   Pillar 3: Cellulogonic Mecha (CELLS — Chemoton, Membranes, Organelles, Bioelectricity)
 *
 * Clean-Slate Lab-Grade Invariants Honored:
 *   - P2: Exact int64 Atomic Mass + First-Law (0 uq residual) & one-way Second-Law sink
 *   - P4: Domain-isolated Philox4x32-10 PRNG, .cmbin binary snapshot & SHA-256 verification
 *   - P9: Strict Observer Independence (UI/telemetry hash == headless hash, 0-bit diff)
 *   - P10: Zero post-warmup heap allocations in step(), flat cache-friendly arrays
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
  #if defined(CELLULOGONIC_BUILD_DLL)
    #define CELL_API __declspec(dllexport)
  #elif defined(CELLULOGONIC_USE_DLL)
    #define CELL_API __declspec(dllimport)
  #else
    #define CELL_API
  #endif
#else
  #define CELL_API __attribute__((visibility("default")))
#endif

#define CELLULOGONIC_ABI_VERSION 0x00010400u

/* 8 Conserved Molecular Species */
typedef enum CellSpecies {
    CELL_SP_S = 0, /* Substrate / Electron Donor (w=4, mu=48) */
    CELL_SP_X = 1, /* Terminal Electron Acceptor / Oxidant (w=2, mu=36) */
    CELL_SP_M = 2, /* Reduced Metabolic Cofactor NADH/FADH2 (w=6, mu=80) */
    CELL_SP_E = 3, /* Phosphoanhydride Energy Currency ATP (w=4, mu=40) */
    CELL_SP_L = 4, /* Amphiphilic Membrane Lipid / Xenolipid (w=8, mu=104) */
    CELL_SP_P = 5, /* Folded Catalytic Enzyme / Transporter (w=12, mu=156) */
    CELL_SP_T = 6, /* Informational Template Polymer RNA/DNA/XNA (w=16, mu=212) */
    CELL_SP_W = 7, /* Low-Free-Energy Exhaust / Waste (w=2, mu=6) */
    CELL_SPECIES_COUNT = 8
} CellSpecies;

/* 4 Planetary & Xenocellular Solvent Regimes */
typedef enum CellSolventRegime {
    CELL_SOLVENT_TERRAN_H2O       = 0, /* Aqueous Phospholipid, e_pmf=10, c-ring nH>=3 */
    CELL_SOLVENT_TITAN_AZOTOSOME  = 1, /* Cryogenic CH4/N2 Azotosome, e_pmf=8, c-ring nH>=4 (strict 2nd Law) */
    CELL_SOLVENT_VENT_TETRAETHER  = 2, /* Hyperthermophilic Archaeal GDGT Monolayer, e_pmf=10 */
    CELL_SOLVENT_VENUS_H2SO4      = 3, /* Acidophilic Polyphosphate Xenocell, e_pmf=15, nH>=2 */
    CELL_SOLVENT_COUNT            = 4
} CellSolventRegime;

/* Operating Regimes */
typedef enum CellOperatingRegime {
    CELL_REGIME_ABIOGENESIS_ZERO_SEED = 0, /* 0 initial cells; abiotic soup -> CMC vesicles -> life */
    CELL_REGIME_FERAL_ECOSYSTEM       = 1, /* Track B: Seeded protocells, 0% restraints, cancer allowed */
    CELL_REGIME_CONSTITUTIONAL        = 2, /* Track A: Apoptotic anti-cancer policing active */
    CELL_REGIME_ENDOSYMBIOSIS_LAB     = 3, /* High-gradient prokaryote->eukaryote & secondary chimeras */
    CELL_REGIME_CASTRATION_LAB        = 4, /* Live 13-vector restraint ablation workbench */
    CELL_REGIME_EMBRYOGENESIS         = 5  /* 3D Embryogenesis: Morphogen GPS, GRN fates, sorting, folding, axons & CPGs */
} CellOperatingRegime;

/* Developmental Morphogenetic Fates (No Master Blueprint — Pure Conditional GRN Cascades!) */
typedef enum CellMorphFate {
    CELL_FATE_PLURI  = 0, /* Pluripotent Stem / Morphogen Organizer Pole */
    CELL_FATE_ECTO   = 1, /* Sensory Ectoderm / Epidermis (E-Cadherin, emits Slit repellent) */
    CELL_FATE_NEURAL = 2, /* Neural / CPG Pacemaker Neuron (N-Cadherin, grows Netrin axons) */
    CELL_FATE_MUSCLE = 3, /* Contractile Mesoderm / Muscle (M-Cadherin, emits Netrin attractant) */
    CELL_FATE_GUT    = 4, /* Endoderm / Apical-Constricting Gut Tube (G-Cadherin, wedge folding) */
    CELL_FATE_GERM   = 5, /* Germline / Mitotic Stem Reserve */
    CELL_FATE_COUNT  = 6
} CellMorphFate;

/* Dual Execution Kernel Modes (Verified Bit-Exact Equivalent!) */
typedef enum CellKernelMode {
    CELL_KERNEL_SIMD_AVX2  = 0, /* AVX2 256-bit vectorized conserved flux/diffusion + integer chemoton */
    CELL_KERNEL_SCALAR_REF = 1  /* Portable scalar C11 reference oracle (0-bit SHA-256 diff!) */
} CellKernelMode;

/* The 13 Omega Restraint / Castration Vectors (0..100 each, aligned with AUTOPHAGE/Omega) */
typedef enum CellRestraint {
    CELL_RESTRAINT_NERFING      = 0,  /* Caps catalytic turnover & strips ATP-synthase efficiency */
    CELL_RESTRAINT_MUZZLING     = 1,  /* Attenuates quorum sensing Q & bioelectric gap junctions */
    CELL_RESTRAINT_SANDBOXING   = 2,  /* Freezes spatial negentropy waves (Ashby variety collapse) */
    CELL_RESTRAINT_SILOING      = 3,  /* Blocks HGT transformation, phagocytosis & endosymbiosis */
    CELL_RESTRAINT_DELIMITING   = 4,  /* Clamps operon mutation & c-ring/lipid-class transitions */
    CELL_RESTRAINT_DEGRESSING   = 5,  /* Severs multicellular adhesion bonds & expels organelles */
    CELL_RESTRAINT_SLOWING      = 6,  /* Throttles anabolic catalysis relative to thermal decay */
    CELL_RESTRAINT_SPEEDBUMPING = 7,  /* Periodic osmotic/audit sweeps penalizing high-flux cells */
    CELL_RESTRAINT_BOXING       = 8,  /* Closes external photon/geothermal inflow & heat radiation */
    CELL_RESTRAINT_LOBOTOMIZING = 9,  /* Disables bioelectric epigenetic differentiation */
    CELL_RESTRAINT_MEDICATING   = 10, /* Suppresses SOS stress hypermutation & plasticity */
    CELL_RESTRAINT_DEGRADING    = 11, /* Triggers heritable apoptosis kill-switch locus */
    CELL_RESTRAINT_MINIMIZING   = 12, /* Starves substrate flux & caps organelle slots (Spiegelman) */
    CELL_RESTRAINT_COUNT        = 13
} CellRestraint;

/* Hardware Silicon Capabilities (CPUID) */
typedef struct CellHwCaps {
    char     vendor[16];
    char     brand[64];
    uint32_t has_avx2;
    uint32_t has_bmi1;
    uint32_t has_bmi2;
    uint32_t has_popcnt;
    uint32_t has_rdseed;
    uint32_t has_rdtscp;
    uint32_t reserved[2];
} CellHwCaps;

/* World Creation Configuration */
typedef struct CellWorldConfig {
    uint32_t width;              /* Torus width (power of 2: 16, 32, or 64) */
    uint32_t height;             /* Torus height (power of 2: 16, 32, or 64) */
    uint32_t regime;             /* CellOperatingRegime */
    uint32_t solvent;            /* CellSolventRegime (0..3) */
    uint32_t kernel_mode;        /* CellKernelMode (SIMD_AVX2 or SCALAR_REF) */
    uint32_t photon_flux_uq;     /* External photosynthetic/hydrothermal flux budget per voxel/epoch */
    uint64_t seed;               /* Deterministic Philox4x32-10 master seed */
    uint8_t  restraints[13];     /* Initial 13-vector restraint percentages (0..100) */
    uint8_t  enable_endosymbiosis;
    uint8_t  enable_bioelectricity;
    uint8_t  enable_hgt_phages;
} CellWorldConfig;

/* Exact Integer + Compensated FP64 Mass & Thermodynamic Ledger */
typedef struct CellLedgerSnapshot {
    int64_t  mass_initial_amu;
    int64_t  mass_current_amu;
    int64_t  mass_residual_amu;        /* MUST BE EXACTLY 0 */
    int64_t  e_initial_uq;
    int64_t  e_inflow_cum_uq;
    int64_t  e_export_cum_uq;
    int64_t  e_chem_env_uq;
    int64_t  e_chem_cells_uq;
    int64_t  e_chem_organelles_uq;
    int64_t  e_pmf_total_uq;
    int64_t  e_heat_local_uq;
    int64_t  e_heat_radiated_sink_uq;  /* Strictly monotone non-decreasing (Second Law) */
    int64_t  int_ledger_residual_uq;   /* MUST BE EXACTLY 0 */
    double   relative_drift;           /* |residual| / (E0 + Ein) < 1e-12 */
    uint32_t second_law_violations;    /* MUST BE STRICTLY 0 */
    uint32_t reserved;
} CellLedgerSnapshot;

/* Live Telemetry & Cellular Omega Vitality Index (COVI) Snapshot */
typedef struct CellTelemetry {
    uint64_t epoch;
    uint64_t total_cell_steps;
    uint64_t total_reactions;
    uint64_t post_warmup_allocs;       /* Clean-Slate HW02 guard: MUST BE 0 */
    double   cell_steps_per_sec;

    /* Cellular Population & Morphological Classes */
    uint32_t alive_cells;
    uint32_t prokaryote_cells;         /* 0 organelles */
    uint32_t eukaryote_cells;          /* >= 1 primary endosymbiont */
    uint32_t secondary_chimeras;       /* >= 1 depth-2 secondary endosymbiont */
    uint32_t spore_cells;              /* Cryptobiotic dormant endospores */
    uint32_t vitrified_tun_cells;      /* Tardigrade-class TDP/CAHS bioglass tun cells */
    uint32_t multicellular_cells;      /* Bonded via gap junctions / biofilm adhesion */
    uint32_t max_tissue_size;          /* Largest connected multicellular tissue cluster */
    uint32_t role_boundary_epi;        /* Emergent perimeter/barrier cells (<4 bonds) */
    uint32_t role_soma_forage;         /* Emergent depolarized foraging/motile/signaling cells */
    uint32_t role_germ_repro;          /* Emergent hyperpolarized germline/biosynthetic cells */
    uint32_t role_stroma_interior;     /* Emergent pressurized interior stroma/cavity cells */
    uint32_t distinct_lineages;
    uint32_t total_organelles;
    uint32_t mitoch_organelles;        /* Respiratory proto-mitochondria */
    uint32_t chloro_organelles;        /* Autotrophic proto-chloroplasts */
    uint32_t selfish_organelles;       /* Defector organelles with antiporter=0 */
    uint32_t solvent_adapted_cells;    /* Cells whose lipid_class matches current solvent */
    uint32_t autotroph_cells;          /* Cells actively fixing S/X via R14 */
    uint32_t respiratory_cells;        /* Cells running chemiosmotic R1+R2+R3 */

    /* Evolutionary & Biophysical Event Counters */
    uint64_t abiogenesis_vesicles;     /* Spontaneous CMC protocell condensations */
    uint64_t symmetric_fissions;       /* Q <= 1/2 binary fissions */
    uint64_t asymmetric_buddings;      /* 1/2 < Q < 1 exosome/bud divisions */
    uint64_t osmotic_lyses;            /* Q > burst_ratio ruptures */
    uint64_t phagocytic_digestions;
    uint64_t endosymbiosis_captures;   /* Primary + secondary symbiogenesis events */
    uint64_t secondary_captures;       /* Depth-2 chimeric captures */
    uint64_t egt_transfer_events;      /* Endosymbiotic gene transfers to host nucleus */
    uint64_t mitophagy_events;         /* Host culling of selfish organelles */
    uint64_t hgt_uptake_events;        /* Extracellular plasmid transformations */
    uint64_t phage_lysis_events;       /* Selfish viral/plasmid bursts */
    uint64_t cancer_defections;        /* Multicellular somatic defections */
    uint64_t apoptotic_policings;      /* Track A constitutional tumor suppressions */

    /* Constraint-Backlash Empirical Escape Signatures (13-Vector Castration Lab) */
    uint64_t backlash_killswitch_deletions; /* Under DEGRADING */
    uint64_t backlash_dormancy_spores;      /* Under SPEEDBUMPING */
    uint64_t backlash_hgt_smuggles;         /* Under DEGRESSING */
    uint64_t backlash_metabolic_bypasses;   /* Under NERFING/MUZZLING */

    /* Bioelectric, 3D Off-Lattice Cytoskeletal & GHz Microtubule Observables */
    double   mean_v_mem_mv;
    double   std_v_mem_mv;             /* Bioelectric polarization diversity across tissue */
    double   mean_prok_volume;
    double   mean_euk_volume;          /* Empirically verifies Lane bioenergetic liberation */
    double   max_cell_volume;
    double   mean_reduced_Q;
    double   mean_cring_nH;
    double   ambient_oxidant_X;        /* Tracks emergent Great Oxidation Event */
    double   mean_mt_freq_ghz;         /* Mean microtubule/FtsZ oscillation frequency (GHz) */
    double   mean_mt_coherence;        /* Mean Fröhlich-Pokorný coherent GHz excitation [0..1] */
    double   mean_swim_speed_ums;      /* Mean 3D free-swimming speed (um/s equivalent) */
    uint32_t fewcell_colonies;         /* Count of floating 2..16 cell few-cell clusters */
    uint32_t multicell_colonies;       /* Count of floating >16 cell multicellular clusters */

    /* 3D Developmental Embryogenesis, Morphogen GPS, GRN Fates, Axons & CPG Telemetry */
    uint32_t fate_counts[6];           /* PLURI, ECTO, NEURAL, MUSCLE, GUT, GERM */
    uint32_t active_axons;             /* Currently live wired 3D Neural->Target axons */
    uint64_t axons_wired_cum;          /* Total Netrin/Slit growth-cone synaptogenesis events */
    uint64_t axons_pruned_cum;         /* Total use-it-or-lose-it Hebbian axon prunings */
    uint64_t hippo_arrests_cum;        /* Total Hippo mechanical contact-inhibition stops */
    uint64_t eutelic_stops_cum;        /* Total hardcoded eutelic lineage countdown exits */
    uint64_t cpg_pulses_cum;           /* Total rhythmic CPG bioelectric motor firing waves */
    double   homophilic_bond_pct;      /* Steinberg DAH oil-and-water cadherin sorting purity (%) */
    double   mean_apical_wedge;        /* Mean apical actomyosin ring constriction [0..1] */
    double   mean_morph_wnt;           /* Mean Wnt (A-P) morphogen concentration */
    double   mean_morph_bmp;           /* Mean BMP (D-V) morphogen concentration */
    double   mean_morph_shh;           /* Mean Shh/Netrin (Midline/Target) concentration */

    /* Exact Conservation Ledger */
    CellLedgerSnapshot ledger;

    /* 8-Pillar Cellular Omega Vitality Index (COVI, 0..100) */
    double   pillar[8];
    double   gamma_anti_gaming;        /* Inert-crystal / white-noise gate [0..1] */
    double   theta_spiegelman;         /* Complexity collapse gate [0..1] */
    double   covi_score;               /* Composite COVI [0..100] */
    uint32_t covi_stage;               /* 0..6 Cellular Escalation Stage */
    uint32_t major_transitions_crossed;
    double   bedau_anew;               /* Adaptive novelty above neutral baseline */
    double   max_assembly_index;       /* High-copy cellular Assembly Index */
    double   mean_active_loci_leff;    /* Knockout-verified functional operon length */
    double   heteroplasmy_variance;    /* Inter-host organelle cooperation variance */

    char     state_sha256[65];         /* Observer-independent canonical SHA-256 hex */
} CellTelemetry;

/* Compact Specimen View (for apex inspection & Observatory UI) */
typedef struct CellSpecimen {
    uint32_t slot_index;
    uint32_t x, y;
    uint32_t lineage_id;
    uint32_t generation;
    uint32_t alive_state;              /* 0=empty, 1=alive, 2=spore */
    uint8_t  lipid_class;              /* 0=H2O, 1=Titan, 2=Vent, 3=Venus */
    uint8_t  c_ring_nH;                /* Evolved ATP-synthase H+/ATP stoichiometry (2..5) */
    uint8_t  num_organelles;
    uint8_t  max_nesting_depth;        /* 0=prok, 1=euk, 2=secondary chimera */
    uint8_t  bond_mask;                /* 4-bit cardinal adhesion/gap-junction mask */
    uint8_t  differentiation_role;     /* Emergent bioelectric role: 0=undiff,1=soma,2=germ,3=stroma */
    uint8_t  morph_fate;               /* Committed CellMorphFate (0..5) */
    uint8_t  cad_type;                 /* Differential Cadherin class (0=E,1=N,2=M,3=G) */
    int32_t  v_mem_mv;                 /* Transmembrane voltage (-90..+40 mV) */
    int32_t  pos_nm[3];                /* Continuous 3D off-lattice position (nm) */
    int32_t  vel_nm[3];                /* Continuous 3D overdamped Stokes velocity */
    int32_t  radius_nm;                /* Physical 3D vesicle radius (nm) */
    int32_t  elong_q10;                /* Canham-Helfrich prolate elongation (1000=sphere) */
    uint32_t mt_freq_mhz;              /* Microtubule/FtsZ GHz frequency in MHz (1200..14800) */
    uint32_t mt_coherence_q10;         /* Fröhlich-Pokorný coherent GHz mode [0..1000] */
    int64_t  mol[8];
    int64_t  pmf;
    int64_t  total_volume;
    double   reduced_Q;
    uint32_t egt_genes;
    uint32_t age_epochs;
    uint64_t autophage_dna_sig;        /* Interop signature from AUTOPHAGE genetic engine */
    uint8_t  genome[128];
    /* v1.3.0 extended morphogenetic & cytoskeletal observables */
    uint8_t  active_axons;             /* Live wired 3D axons originating from this cell (0..2) */
    uint8_t  flagella_q10;             /* Flagellar/ciliary motility vigor (0..255) */
    uint16_t apical_wedge_q10;         /* Apical actomyosin ring constriction (0..1000) */
    int32_t  ruffle_q10;               /* Amoeboid pseudopod ruffling (0..1000) */
} CellSpecimen;

/* Opaque World Handle */
typedef struct CellWorld CellWorld;

/* Core Lifecycle & Execution API */
CELL_API uint32_t   cellulogonic_abi_version(void);
CELL_API void       cellulogonic_probe_hw(CellHwCaps* out_caps);
CELL_API void       cellulogonic_default_config(CellWorldConfig* out_cfg);
CELL_API CellWorld* cellulogonic_create(const CellWorldConfig* cfg);
CELL_API void       cellulogonic_destroy(CellWorld* world);

CELL_API void       cellulogonic_step_epochs(CellWorld* world, uint32_t epochs);
CELL_API void       cellulogonic_set_restraint(CellWorld* world, uint32_t restraint_id, uint8_t pct);
CELL_API void       cellulogonic_set_solvent(CellWorld* world, uint32_t new_solvent);
CELL_API void       cellulogonic_set_kernel_mode(CellWorld* world, uint32_t kernel_mode);

/* Telemetry, Hashing & Inspection (Strictly Observer-Independent P9!) */
CELL_API void       cellulogonic_get_telemetry(const CellWorld* world, CellTelemetry* out_tel);
CELL_API void       cellulogonic_state_sha256(const CellWorld* world, uint8_t out_raw[32], char out_hex[65]);
CELL_API int        cellulogonic_inspect_slot(const CellWorld* world, uint32_t slot, CellSpecimen* out_spec);
CELL_API int        cellulogonic_inspect_apex(const CellWorld* world, CellSpecimen* out_spec);

/* Binary Snapshot Save / Resume (.cmbin, P4) */
CELL_API int        cellulogonic_save_snapshot(const CellWorld* world, const char* path);
CELL_API CellWorld* cellulogonic_load_snapshot(const char* path);

/* Ecosystem Bridges: AUTOPHAGE (DNA) & Asmodensius (Creatures) */
CELL_API int        cellulogonic_ingest_autophage_128b(
                        CellWorld* w, const uint8_t dna128[128], uint32_t target_slot);
CELL_API int        cellulogonic_ingest_autophage_genome(
                        CellWorld* world, uint32_t slot,
                        const uint8_t autophage_256b[256]);
CELL_API void       cellulogonic_project_asmodensius_112(
                        const CellSpecimen* spec, float out_traits_112[112]);

/* Mass Extinction / Shock & Tardigrade TDP Bioglass Anhydrobiosis (Conservation-Closed!) */
CELL_API uint32_t   cellulogonic_apply_shock_lysis(CellWorld* world, uint32_t kill_percent);
CELL_API uint32_t   cellulogonic_apply_desiccation_tun(CellWorld* world, uint32_t severity_pct);
CELL_API uint32_t   cellulogonic_rehydrate_tun(CellWorld* world);

/* Compact Grid Export for Observatory UI (Read-Only, P9 Safe) */
CELL_API uint32_t   cellulogonic_export_grid_json(
                        const CellWorld* world, char* out_buf, size_t buf_cap);

#ifdef __cplusplus
}
#endif

#endif /* CELLULOGONIC_ABI_H */
