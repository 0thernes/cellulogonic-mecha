#ifndef CELLULOGONIC_INTERNAL_H
#define CELLULOGONIC_INTERNAL_H

#include "bare_crt.h"
#include "cellulogonic_abi.h"

#define CELL_MAX_ORGANELLES   8u
#define CELL_GENOME_BYTES     128u
#define CELL_MT_GENOME_BYTES  32u
#define CELL_VESICLE_C0       100LL
#define CELL_SNAPSHOT_MAGIC   0x434D4231u /* "CMB1" */

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
    uint8_t free_plasmid[32];      /* Extracellular lysed template fragment for HGT */
    uint8_t free_plasmid_valid;
    uint8_t free_plasmid_is_phage;
    uint8_t pad[6];
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
    int32_t*    vmem_delta;        /* num_slots conservative bioelectric Laplacian buffer */
    uint32_t*   bfs_queue;         /* num_slots scratch queue for tissue cluster sizing */
    uint8_t*    bfs_vis;           /* num_slots scratch visited bitmap */
};

/* Internal Function Declarations */
uint64_t cell_philox_u64(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
uint32_t cell_philox_u32(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
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
