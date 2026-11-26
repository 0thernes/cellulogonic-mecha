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
#define CELL_SNAPSHOT_MAGIC   0x434D4235u /* "CMB5" (v1.5.0 hardened zero-padding validated snapshot) */

/* Domain-Isolated Philox4x32-10 Stream IDs (Clean-Slate P4) */
#define RNG_DOMAIN_WORLD_EXOG   0x1001u
#define RNG_DOMAIN_ABIOGENESIS  0x2002u
#define RNG_DOMAIN_CELL_MUT     0x3003u
#define RNG_DOMAIN_MITOSIS_PART 0x4004u
#define RNG_DOMAIN_PHAGO_HGT    0x5005u

/* Exact Stoichiometric Constants Matching Lean 4 Proofs T01-T58 & C59-C62 */
static const int64_t CELL_MASS_W[CELL_SPECIES_COUNT] = {
    4, 2, 6, 4, 8, 12, 16, 2
};
static const int64_t CELL_MU_POT[CELL_SPECIES_COUNT] = {
    48, 36, 80, 40, 104, 156, 212, 6
};

/* Solvent Physical Constants Matching Lean 4 T01-T03 & T28-T30 (Strict 2nd Law q > 0 on ALL 4!) */
typedef struct SolventParams {
    int64_t  e_pmf;            /* Free energy per PMF quantum: {10, 8, 10, 15} */
    int64_t  r2_pmf_yield;     /* PMF pumped by R2 (M->3W+k PMF): {6, 7, 6, 4} */
    int64_t  r2_heat;          /* Heat of R2: {2, 6, 2, 2} — strictly > 0 across all 4 solvents! */
    uint8_t  min_cring;        /* Minimum thermodynamically valid c-ring nH: {3, 4, 3, 2} */
    uint8_t  optimal_lipid;    /* Matching lipid_class: {0, 1, 2, 3} */
    int64_t  burst_num;        /* Lysis threshold: C0*V^2 * 100 > burst_num * L^3 */
    int64_t  leak_mismatch;    /* Extra PMF leak if lipid_class != optimal_lipid */
    int64_t  decay_div;        /* Thermal decay rate divisor (higher = slower decay) */
    int64_t  diff_shift;       /* Spatial diffusion shift (higher = slower diffusion) */
} SolventParams;

static const SolventParams SOLVENT_TABLE[CELL_SOLVENT_COUNT] = {
    /* 0: TERRAN_H2O       */ { 10, 6, 2, 3, 0, 115, 3,  64, 4 },
    /* 1: TITAN_AZOTOSOME  */ {  8, 7, 6, 4, 1, 135, 5, 192, 5 },
    /* 2: VENT_TETRAETHER  */ { 10, 6, 2, 3, 2, 110, 6,  38, 3 },
    /* 3: VENUS_H2SO4      */ { 15, 4, 2, 2, 3, 110, 8,  36, 4 }
};

/* Internalized Nested Endosymbiont Organelle (Real Conserved Compartment, Zero Implicit Padding!) */
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
    uint32_t pad2;             /* Explicit zeroed alignment padding before int64_t mol[8] */
    int64_t  mol[CELL_SPECIES_COUNT];
    int64_t  pmf;
    uint8_t  mt_genome[CELL_MT_GENOME_BYTES];
} CellOrganelle;

/* Living Cellular Compartment on the Spatial Torus (Zero Implicit Padding!) */
typedef struct CellEntity {
    uint8_t  alive_state;          /* 0=empty, 1=alive, 2=endospore, 3=vitrified tun */
    uint8_t  lipid_class;          /* 0..3 (Gray-coded from Locus 8 bytes 64..67) */
    uint8_t  c_ring_nH;            /* 2..5 (Gray-coded from Locus 2 bytes 16, 18..20) */
    uint8_t  num_organelles;       /* 0..CELL_MAX_ORGANELLES */
    uint8_t  bond_mask;            /* Bit 0..3 adhesion/gap-junction mask */
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
    uint8_t  pad_bonds;            /* Explicit zeroed padding before int16_t bond_partners[4] */
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
    uint16_t pad_cpg;              /* Explicit zeroed padding before int32_t cpg_phase_mrad */
    int32_t  cpg_phase_mrad;       /* Central Pattern Generator pacemaker phase (0..6283) */
    int16_t  axon_target[CELL_MAX_AXONS]; /* Wired 3D long-range axon partner indices (-1 = none) */
    uint8_t  axon_weight[CELL_MAX_AXONS]; /* Synaptic weight (Hebbian LTP + use-it-or-lose-it pruning) */
    uint16_t pad_mol;              /* Explicit zeroed padding before int64_t mol[8] */
    int64_t  mol[CELL_SPECIES_COUNT];
    int64_t  pmf;
    uint64_t autophage_dna_sig;
    uint8_t  genome[CELL_GENOME_BYTES];
    CellOrganelle org[CELL_MAX_ORGANELLES];
} CellEntity;

_Static_assert(sizeof(CellWorldConfig) == 48,   "CellWorldConfig must have zero implicit padding");
_Static_assert(sizeof(CellOrganelle)   == 120,  "CellOrganelle must have zero implicit padding");
_Static_assert(sizeof(CellEntity)      == 1304, "CellEntity must have zero implicit padding");

/* Spatial Voxel Environment (2.5D column-averaged chemistry + 3D continuous off-lattice cells) */
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

_Static_assert(sizeof(CellVoxel) == 128, "CellVoxel must have zero implicit padding");

/* Full Deterministic Cellular World */
struct CellWorld {
    CellWorldConfig cfg;
    uint32_t num_slots;
    uint32_t cpu_has_avx2;         /* Runtime OSXSAVE+XGETBV+CPUID AVX2 guard */
    uint64_t epoch;
    uint64_t total_cell_steps;
    uint64_t total_reactions;
    uint64_t post_warmup_allocs;
    uint32_t next_lineage_id;
    uint32_t pad_hdr;

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

    /* Activity / Novelty Tracking */
    uint64_t novel_adaptations_cum;

    /* Preallocated Spatial & Scratch Arrays (Zero Post-Warmup Allocs!) */
    CellVoxel*  voxels;
    CellEntity* cells;
    int64_t*    diff_delta;        /* num_slots * 8 exact conservative flux buffer */
    int64_t*    heat_delta;        /* num_slots conservative heat flux buffer */
    int64_t*    quorum_delta;      /* num_slots quorum diffusion buffer */
    int32_t*    morph_delta;       /* num_slots * 3 morphogen GPS diffusion buffer (0 alloc!) */
    int32_t*    vmem_delta;        /* num_slots conservative bioelectric Laplacian buffer */
    int32_t*    hash_head;         /* num_slots 3D off-lattice spatial hash head (0 alloc!) */
    int32_t*    hash_next;         /* num_slots 3D off-lattice spatial hash linked list */
};

/* Helper: mathematical floor division by CELL_VOXEL_NM in int64_t (overflow-free even at INT32_MIN!) */
static inline int32_t cell_floor_div_voxel(int32_t p) {
    int64_t p64 = (int64_t)p;
    if (p64 >= 0) return (int32_t)(p64 / (int64_t)CELL_VOXEL_NM);
    return (int32_t)(-((-p64 + (int64_t)CELL_VOXEL_NM - 1LL) / (int64_t)CELL_VOXEL_NM));
}

/* Helper: map a cell's continuous 3D off-lattice position to its local spatial fluid column */
static inline uint32_t cell_voxel_idx(const CellWorld* w, const CellEntity* c) {
    int32_t W = (int32_t)w->cfg.width;
    int32_t H = (int32_t)w->cfg.height;
    int32_t vx = cell_floor_div_voxel(c->pos_nm[0]) % W;
    int32_t vy = cell_floor_div_voxel(c->pos_nm[1]) % H;
    if (vx < 0) vx += W;
    if (vy < 0) vy += H;
    return (uint32_t)vy * (uint32_t)W + (uint32_t)vx;
}

/* Helper: exact integer floor square root for non-negative int64_t (0 FP64 nondeterminism!) */
static inline int32_t cell_isqrt32(int64_t x) {
    if (x <= 0) return 0;
    if (x >= 4611686014132420609LL) return 2147483647; /* (2^31 - 1)^2 cap */
    uint64_t ux = (uint64_t)x;
    uint64_t res = 0ULL;
    uint64_t bit = 1ULL << 62;
    while (bit > ux) bit >>= 2;
    while (bit != 0ULL) {
        if (ux >= res + bit) {
            ux -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return (int32_t)res;
}

/* Helper: canonical little-endian uint64 load/store (host-endianness independent!) */
static inline void cell_store_le64(uint8_t out[8], uint64_t v) {
    for (int i = 0; i < 8; ++i) out[i] = (uint8_t)((v >> (i * 8)) & 0xFFu);
}
static inline uint64_t cell_load_le64(const uint8_t in[8]) {
    uint64_t v = 0ULL;
    for (int i = 0; i < 8; ++i) v |= ((uint64_t)in[i]) << (i * 8);
    return v;
}

/* Internal Function Declarations */
uint64_t cell_philox_u64(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
uint32_t cell_philox_u32(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo);
uint64_t cell_hw_rdseed_u64(void);
int      cell_runtime_has_avx2(void);
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

/* Exact Stoichiometry & Chemoton Physics (src/chemoton_physics.c) */
int64_t cell_species_mass(const int64_t mol[CELL_SPECIES_COUNT]);
int64_t cell_species_energy(const int64_t mol[CELL_SPECIES_COUNT]);
int64_t cell_internal_volume(const CellEntity* c);
int64_t cell_organelle_volume(const CellOrganelle* o);
double  cell_reduced_Q(int64_t vol, int64_t lipid);
int     cell_can_symmetric_fission(int64_t vol, int64_t lipid);
int     cell_is_osmotic_burst(int64_t vol, int64_t lipid, uint32_t solvent, uint8_t lipid_class);
void    cell_decode_genome_traits(CellEntity* c);
void    cell_encode_lipid_class(CellEntity* c, uint8_t target_class);
void    cell_encode_cring_nH(CellEntity* c, uint8_t target_nH);
void    cell_mutate_genome(CellEntity* c, uint64_t seed, uint64_t epoch, uint32_t slot, uint32_t rate_pct, uint8_t delimit_pct);
void    cell_step_chemoton(CellWorld* w, uint32_t idx);
void    cell_compute_ledger(const CellWorld* w, CellLedgerSnapshot* out);

/* Spatial World, Diffusion, Abiogenesis, Endosymbiosis & Bioelectricity (src/cellular_world.c) */
void    cell_clear_bonds_and_axons(CellWorld* w, uint32_t idx);
void    cell_lyse_to_voxel(CellWorld* w, uint32_t idx, int leave_plasmid, int is_phage);
void    cell_init_world_state(CellWorld* w);
void    cell_step_one_epoch(CellWorld* w);

/* Metrology, COVI & Bridges (src/metrology_covi.c) */
void    cell_compute_telemetry(const CellWorld* w, CellTelemetry* out);

#endif /* CELLULOGONIC_INTERNAL_H */
