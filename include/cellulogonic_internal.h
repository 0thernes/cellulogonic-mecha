#ifndef CELLULOGONIC_INTERNAL_H
#define CELLULOGONIC_INTERNAL_H

#include "bare_crt.h"
#include "cellulogonic_abi.h"

#define CELL_MAX_ORGANELLES   8u
#define CELL_MAX_BONDS        4u
#define CELL_MAX_AXONS        2u
#define CELL_MORPH_AXES       3u /* 0=Wnt(A-P), 1=BMP(D-V/Slit), 2=Shh(Midline/Netrin) */
#define CELL_GENOME_BYTES     128u
#define CELL_MT_GENOME_BYTES  32u
#define CELL_VESICLE_C0       100LL
#define CELL_VOXEL_NM         1000
#define CELL_SNAPSHOT_MAGIC   0x434D4233u /* "CMB3" (v1.2 3D Embryogenesis & GRN) */

/* Domain-Isolated Philox4x32-10 Stream IDs (Clean-Slate P4) */
#define RNG_DOMAIN_WORLD_EXOG   0x1001u
#define RNG_DOMAIN_ABIOGENESIS  0x2002u
#define RNG_DOMAIN_CELL_MUT     0x3003u
#define RNG_DOMAIN_MITOSIS_PART 0x4004u
#define RNG_DOMAIN_PHAGO_HGT    0x5005u

/* Exact Stoichiometric Constants Matching Lean 4 Proofs T01-T36 */
static const int64_t CELL_MASS_W[CELL_SPECIES_COUNT] = {
    4, 2, 6, 4, 8, 12, 16, 2
};
static const int64_t CELL_MU_POT[CELL_SPECIES_COUNT] = {
    48, 36, 80, 40, 104, 156, 212, 6
};

/* Solvent Physical Constants Matching Lean 4 T28-T30 */
typedef struct SolventParams {
    int64_t  e_pmf;            /* Free energy per PMF quantum: {10, 7, 10, 15} */
    int64_t  r2_pmf_yield;     /* PMF pumped by R2 (M->3W+k PMF): {6, 8, 6, 4} */
    int64_t  r2_heat;          /* Heat of R2: {2, 6, 2, 2} */
    uint8_t  min_cring;        /* Minimum thermodynamically valid c-ring nH: {3, 4, 3, 2} */
    uint8_t  optimal_lipid;    /* Matching lipid_class: {0, 1, 2, 3} */
    int64_t  burst_num;        /* Lysis threshold: C0*V^2 * 100 > burst_num * L^3 */
    int64_t  leak_mismatch;    /* Extra PMF leak if lipid_class != optimal_lipid */
    int64_t  decay_div;        /* Thermal decay rate divisor (higher = slower decay) */
    int64_t  diff_shift;       /* Spatial diffusion shift (higher = slower diffusion) */
} SolventParams;

static const SolventParams SOLVENT_TABLE[CELL_SOLVENT_COUNT] = {
    /* 0: TERRAN_H2O       */ { 10, 6, 2, 3, 0, 115, 2, 64, 4 },
    /* 1: TITAN_AZOTOSOME  */ {  7, 8, 6, 4, 1, 135, 2, 192, 5 },
    /* 2: VENT_TETRAETHER  */ { 10, 6, 2, 3, 2, 110, 5, 28, 3 },
    /* 3: VENUS_H2SO4      */ { 15, 4, 2, 2, 3, 110, 4, 36, 4 }
};

/* Internalized Nested Endosymbiont Organelle (Real Conserved Compartment!) */
typedef struct CellOrganelle {
    uint8_t  active;           /* 0=empty, 1=living endosymbiont */
    uint8_t  org_type;         /* 0=proto-mitochondrion, 1=proto-chloroplast */
    uint8_t  nesting_depth;    /* 1=primary endosymbiont, 2=secondary chimeric endosymbiont */
    uint8_t  c_ring_nH;        /* 2..5 */
    uint8_t  antiporter_share; /* 0..100 (% ATP exported to host; 0 = selfish cheater!) */
    uint8_t  lipid_class;      /* 0..3 */
    uint8_t  egt_transferred;  /* Number of loci migrated to host nucleus */
    uint8_t  pad;
    uint32_t lineage_id;
    int64_t  mol[CELL_SPECIES_COUNT];
    int64_t  pmf;
    uint8_t  mt_genome[CELL_MT_GENOME_BYTES];
} CellOrganelle;

/* Living Cellular Compartment on the Spatial Torus */
typedef struct CellEntity {
    uint8_t  alive_state;          /* 0=empty, 1=alive, 2=endospore */
    uint8_t  lipid_class;          /* 0..3 */
    uint8_t  c_ring_nH;            /* 2..5 */
    uint8_t  num_organelles;       /* 0..CELL_MAX_ORGANELLES */
    uint8_t  bond_mask;            /* Bit 0=N, 1=S, 2=E, 3=W gap-junction/adhesion bonds */
    uint8_t  diff_role;            /* 0=undiff, 1=soma, 2=germ, 3=stroma */
    uint8_t  phage_load;           /* Intracellular selfish phage/plasmid count */
    uint8_t  kill_switch_intact;   /* 1=intact (under DEGRADING), 0=deleted by backlash */
    uint32_t lineage_id;
    uint32_t generation;
    uint32_t age_epochs;
    uint32_t egt_genes;
    int32_t  v_mem_mv;             /* Transmembrane bioelectric voltage (-90..+40 mV) */
    int32_t  defect_strikes;       /* Consecutive selfish multicellular defections */
    /* Continuous 3D Off-Lattice Fluid Position, Swimming & Cytoskeletal Nanomechanics */
    int32_t  pos_nm[3];            /* Continuous 3D position in [0, W*1000)^3 nm */
    int32_t  vel_nm[3];            /* 3D overdamped Stokes velocity (nm/epoch) */
    int32_t  heading_q10[3];       /* 3D flagellar swimming / cytokinetic axis (norm ~1024) */
    int32_t  radius_nm;            /* Physical 3D membrane radius (220..1500 nm spectrum!) */
    int32_t  elong_q10;            /* Aspect ratio: <850 oblate/disc, 1000 sphere, >1300 rod/dumbbell */
    int32_t  ruffle_q10;           /* Amoeboid pseudopod / dendritic membrane ruffling (0..1000) */
    int32_t  breath_phase_mrad;    /* Actomyosin radial expansion-contraction phase (0..6283) */
    int32_t  breath_amp_q10;       /* Actomyosin expansion-contraction amplitude (0..350) */
    uint32_t mt_freq_mhz;          /* Microtubule/FtsZ GHz eigenfrequency in MHz (1200..14800) */
    uint16_t mt_coherence_q10;     /* Fröhlich-Pokorný coherent GHz excitation (0..1000) */
    uint8_t  mt_filaments;         /* Active radial microtubule/FtsZ filament bundle count */
    uint8_t  flagella_q10;         /* Flagellar/ciliary motility vigor (0..255) */
    uint8_t  num_bonds;            /* Active organic 3D cadherin/gap-junction bonds (0..4) */
    int16_t  bond_partners[CELL_MAX_BONDS]; /* Off-lattice 3D bonded cell indices (-1 = none) */
    /* 3D Developmental Embryogenesis: Conditional GRN, Cadherins, Apical Wedge, Axons, Eutely & CPG */
    uint8_t  tf[8];                /* 8-channel epigenetic Transcription Factors (inherited on mitosis!) */
    uint8_t  morph_fate;           /* Committed CellMorphFate (0..5: Pluri/Ecto/Neural/Muscle/Gut/Germ) */
    uint8_t  cad_type;             /* Differential Cadherin class (0=E, 1=N, 2=M, 3=G; Steinberg DAH!) */
    uint8_t  cleavage_mode;        /* Mitotic spindle cleavage rule: 0=rod, 1=sheet/ball, 2=3D branch */
    uint8_t  eutelic_rem;          /* Hardcoded eutelic division countdown (0 = terminal exit!) */
    uint8_t  contact_pressure;     /* Live 3D crowding overlap count (drives Hippo pathway!) */
    uint8_t  cpg_firing;           /* 1 if CPG neuron fired action potential this epoch (for 3D glow!) */
    uint16_t apical_wedge_q10;     /* Apical actomyosin constriction ring (0..1000 -> tube folding!) */
    int32_t  cpg_phase_mrad;       /* Central Pattern Generator pacemaker phase (0..6283) */
    int16_t  axon_target[CELL_MAX_AXONS]; /* Wired 3D long-range axon partner indices (-1 = none) */
    uint8_t  axon_weight[CELL_MAX_AXONS]; /* Synaptic weight (Hebbian LTP + use-it-or-lose-it pruning) */
    int64_t  mol[CELL_SPECIES_COUNT];
    int64_t  pmf;
    uint64_t autophage_dna_sig;
    uint8_t  genome[CELL_GENOME_BYTES];
    CellOrganelle org[CELL_MAX_ORGANELLES];
} CellEntity;

/* Kahan-Neumaier Compensated Accumulator */
typedef struct KahanSum {
    double sum;
    double c;
} KahanSum;

/* Spatial Voxel Environment */
typedef struct CellVoxel {
    int64_t env[CELL_SPECIES_COUNT];
    int64_t quorum;
    int64_t heat_local_uq;
    int32_t morph[CELL_MORPH_AXES]; /* 3D Chemical GPS morphogens: 0=Wnt(A-P), 1=BMP/Slit(D-V), 2=Shh/Netrin */
    uint8_t free_plasmid[32];       /* Extracellular lysed template fragment for HGT */
    uint8_t free_plasmid_valid;
    uint8_t free_plasmid_is_phage;
    uint8_t pad[2];
} CellVoxel;

/* Full Deterministic Cellular World */
struct CellWorld {
    CellWorldConfig cfg;
    uint32_t num_slots;
    uint64_t epoch;
    uint64_t total_cell_steps;
    uint64_t total_reactions;
    uint64_t post_warmup_allocs;
    uint32_t next_lineage_id;
    uint32_t rng_step_nonce;

    /* Exact Integer Conservation Ledger */
    int64_t  mass_initial_amu;
    int64_t  e_initial_uq;
    int64_t  e_inflow_cum_uq;
    int64_t  e_export_cum_uq;
    int64_t  e_heat_radiated_sink_uq;
    uint32_t second_law_violations;
    uint32_t pad0;

    /* Empirical Counters */
    uint64_t abiogenesis_vesicles;
    uint64_t symmetric_fissions;
    uint64_t asymmetric_buddings;
    uint64_t osmotic_lyses;
    uint64_t phagocytic_digestions;
    uint64_t endosymbiosis_captures;
    uint64_t secondary_captures;
    uint64_t egt_transfer_events;
    uint64_t mitophagy_events;
    uint64_t hgt_uptake_events;
    uint64_t phage_lysis_events;
    uint64_t cancer_defections;
    uint64_t apoptotic_policings;

    /* Constraint-Backlash Counters */
    uint64_t backlash_killswitch_deletions;
    uint64_t backlash_dormancy_spores;
    uint64_t backlash_hgt_smuggles;
    uint64_t backlash_metabolic_bypasses;

    /* Developmental Embryogenesis Counters */
    uint64_t axons_wired_cum;
    uint64_t axons_pruned_cum;
    uint64_t hippo_arrests_cum;
    uint64_t eutelic_stops_cum;
    uint64_t cpg_pulses_cum;

    /* Activity / Novelty Tracking (fixed preallocated buffers, 0 post-warmup alloc!) */
    uint32_t lineage_seen_hash[1024];
    uint32_t lineage_activity[1024];
    uint64_t novel_adaptations_cum;

    /* Preallocated Spatial & Scratch Arrays (Zero Post-Warmup Allocs!) */
    CellVoxel*  voxels;
    CellEntity* cells;
    int64_t*    diff_delta;        /* num_slots * 8 exact conservative flux buffer */
    int64_t*    heat_delta;        /* num_slots conservative heat flux buffer */
    int64_t*    quorum_delta;      /* num_slots quorum diffusion buffer */
    int32_t*    morph_delta;       /* num_slots * 3 morphogen GPS diffusion buffer (0 alloc!) */
    int32_t*    vmem_delta;        /* num_slots conservative bioelectric Laplacian buffer */
    uint32_t*   bfs_queue;         /* num_slots scratch queue for tissue cluster sizing */
    uint8_t*    bfs_vis;           /* num_slots scratch visited bitmap */
    int32_t*    hash_head;         /* num_slots 3D off-lattice spatial hash head (0 alloc!) */
    int32_t*    hash_next;         /* num_slots 3D off-lattice spatial hash linked list */
};

/* Helper: map a cell's continuous 3D off-lattice position to its local spatial fluid voxel */
static inline uint32_t cell_voxel_idx(const CellWorld* w, const CellEntity* c) {
    uint32_t W = w->cfg.width;
    uint32_t H = w->cfg.height;
    int32_t vx = (c->pos_nm[0] / CELL_VOXEL_NM) % (int32_t)W;
    int32_t vy = (c->pos_nm[1] / CELL_VOXEL_NM) % (int32_t)H;
    if (vx < 0) vx += (int32_t)W;
    if (vy < 0) vy += (int32_t)H;
    return (uint32_t)vy * W + (uint32_t)vx;
}

/* Internal Function Declarations */
uint64_t cell_philox_u64(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
uint32_t cell_philox_u32(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
uint64_t cell_hw_rdseed_u64(void);
double   cell_wall_seconds(void);

typedef struct CellSha256Ctx {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t  data[64];
    uint32_t datalen;
} CellSha256Ctx;

void cell_sha256_init(CellSha256Ctx* ctx);
void cell_sha256_update(CellSha256Ctx* ctx, const uint8_t* data, size_t len);
void cell_sha256_final(CellSha256Ctx* ctx, uint8_t out_hash[32]);
void cell_sha256_to_hex(const uint8_t hash[32], char hex[65]);
int  cell_verify_second_law(CellWorld* w, int64_t prev_sink_uq);
int64_t cell_hydrolyze_atp_exact(CellWorld* w, uint32_t idx, int64_t k_req);
int  cellulogonic_ingest_autophage_128b(CellWorld* w, uint32_t slot, const uint8_t genome_128b[128]);

/* Exact Stoichiometry & Chemoton Physics (src/chemoton_physics.c) */
int64_t cell_species_mass(const int64_t mol[CELL_SPECIES_COUNT]);
int64_t cell_species_energy(const int64_t mol[CELL_SPECIES_COUNT]);
int64_t cell_internal_volume(const CellEntity* c);
int64_t cell_organelle_volume(const CellOrganelle* o);
double  cell_reduced_Q(int64_t vol, int64_t lipid);
int     cell_can_symmetric_fission(int64_t vol, int64_t lipid);
int     cell_is_osmotic_burst(int64_t vol, int64_t lipid, uint32_t solvent, uint8_t lipid_class);
void    cell_decode_genome_traits(CellEntity* c);
void    cell_mutate_genome(CellEntity* c, uint64_t seed, uint64_t epoch, uint32_t slot, uint32_t rate_pct, uint8_t delimit_pct);
void    cell_step_chemoton(CellWorld* w, uint32_t idx);
void    cell_compute_ledger(const CellWorld* w, CellLedgerSnapshot* out);

/* Spatial World, Diffusion, Abiogenesis, Endosymbiosis & Bioelectricity (src/cellular_world.c) */
void    cell_init_world_state(CellWorld* w);
void    cell_step_one_epoch(CellWorld* w);

/* Metrology, COVI & Bridges (src/metrology_covi.c) */
void    cell_compute_telemetry(const CellWorld* w, CellTelemetry* out);

#endif /* CELLULOGONIC_INTERNAL_H */
