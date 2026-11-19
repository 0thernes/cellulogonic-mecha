#include "cellulogonic_internal.h"

#if defined(__AVX2__)
typedef int64_t  v4i64 __attribute__((vector_size(32), aligned(1)));
typedef uint64_t v4u64 __attribute__((vector_size(32), aligned(1)));
#endif

/* ============================================================================
 * Conservation-Exact Helper: Lyse a Cell (+ All Nested Organelles) into Voxel
 * Exact 0 amu mass loss, exact 0 uq energy loss (PMF dissipates to heat_local).
 * ============================================================================ */

static void lyse_cell_to_voxel(CellWorld* w, uint32_t idx, int leave_plasmid, int is_phage) {
    CellEntity* c = &w->cells[idx];
    CellVoxel* vx = &w->voxels[idx];
    if (c->alive_state == 0) return;

    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    /* Release outer membrane & cytoplasm molecules exactly into voxel env */
    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
        vx->env[s] += c->mol[s];
        c->mol[s] = 0;
    }
    if (c->pmf > 0) {
        vx->heat_local_uq += c->pmf * sp->e_pmf;
        c->pmf = 0;
    }

    /* Release all internalized organelle compartments exactly into voxel env */
    for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
        if (c->org[k].active) {
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                vx->env[s] += c->org[k].mol[s];
                c->org[k].mol[s] = 0;
            }
            if (c->org[k].pmf > 0) {
                vx->heat_local_uq += c->org[k].pmf * sp->e_pmf;
                c->org[k].pmf = 0;
            }
            c->org[k].active = 0;
        }
    }

    if (leave_plasmid && vx->env[CELL_SP_T] > 0) {
        memcpy(vx->free_plasmid, c->genome, 32);
        vx->free_plasmid_valid    = 1;
        vx->free_plasmid_is_phage = (uint8_t)(is_phage ? 1 : 0);
    }

    /* Sever any neighbor bonds pointing to this voxel */
    uint32_t W = w->cfg.width, H = w->cfg.height;
    uint32_t x = idx % W, y = idx / W;
    uint32_t n_idx = ((y + H - 1u) % H) * W + x;
    uint32_t s_idx = ((y + 1u) % H) * W + x;
    uint32_t e_idx = y * W + ((x + 1u) % W);
    uint32_t w_idx = y * W + ((x + W - 1u) % W);
    w->cells[n_idx].bond_mask &= (uint8_t)~0x02u;
    w->cells[s_idx].bond_mask &= (uint8_t)~0x01u;
    w->cells[e_idx].bond_mask &= (uint8_t)~0x08u;
    w->cells[w_idx].bond_mask &= (uint8_t)~0x04u;

    memset(c, 0, sizeof(*c));
}

/* ============================================================================
 * World Initialization (Abiotic Soup vs. Seeded Protocell Regimes)
 * ============================================================================ */

static void seed_primordial_genome(CellEntity* c, uint64_t seed, uint32_t idx, uint32_t solvent, uint32_t regime) {
    for (uint32_t b = 0; b < CELL_GENOME_BYTES; b += 8) {
        uint64_t r = cell_philox_u64(seed, RNG_DOMAIN_ABIOGENESIS, 1ULL, (uint64_t)idx * 32ULL + b);
        memcpy(&c->genome[b], &r, 8);
    }
    /* Bias viable catalytic operons for seeded laboratory regimes (NOT used in zero-seed abiogenesis!) */
    c->genome[0]   = (uint8_t)(160u + (c->genome[0] & 63u));  /* R1 catabolism */
    c->genome[8]   = (uint8_t)(160u + (c->genome[8] & 63u));  /* R2 proton pump */
    c->genome[16]  = (uint8_t)((SOLVENT_TABLE[solvent].min_cring - 2u) & 3u); /* valid c-ring */
    c->genome[17]  = (uint8_t)(170u + (c->genome[17] & 63u)); /* R3 ATP synthase */
    c->genome[24]  = (uint8_t)(110u + (c->genome[24] & 63u)); /* R4 fermentation */
    c->genome[32]  = (uint8_t)(180u + (c->genome[32] & 63u)); /* R5 lipid synthesis */
    c->genome[40]  = (uint8_t)(150u + (c->genome[40] & 63u)); /* R6 translation */
    c->genome[48]  = (uint8_t)(150u + (c->genome[48] & 63u)); /* R7 template replication */
    c->genome[64]  = (uint8_t)(SOLVENT_TABLE[solvent].optimal_lipid & 3u); /* adapted lipid class */
    c->genome[72]  = (uint8_t)(170u + (c->genome[72] & 63u)); /* S permease */
    c->genome[73]  = (uint8_t)(160u + (c->genome[73] & 63u)); /* X permease */
    if (regime == CELL_REGIME_ENDOSYMBIOSIS_LAB) {
        /* Diverse trophic mix: some phototrophs, some respirers, some phagocytes */
        if ((idx % 3u) == 0u) {
            c->genome[56] = 200u; /* Autotroph photo-S */
            c->genome[57] = 190u; /* Oxygenic photo-X */
        }
        if ((idx % 4u) == 1u) {
            c->genome[80] = 210u; /* Active phagocytosis receptor */
            c->genome[81] = 75u;  /* Low digestion -> favors symbiogenesis! */
            c->genome[84] = 160u; /* Mitophagy policing */
        }
        c->genome[82] = 180u;     /* Antiporter ATP sharing competence */
        c->genome[83] = 180u;     /* Membrane digestion resistance */
    }
    cell_decode_genome_traits(c);
}

void cell_init_world_state(CellWorld* w) {
    const uint32_t N = w->num_slots;
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint64_t seed = w->cfg.seed;

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        memset(vx, 0, sizeof(*vx));
        memset(&w->cells[i], 0, sizeof(CellEntity));

        uint32_t x = i % W, y = i / W;
        uint64_t r = cell_philox_u64(seed, RNG_DOMAIN_WORLD_EXOG, 0ULL, i);

        /* Realistic conserved spatial culture-broth / hydrothermal vent molarity (~1,200 amu/voxel)
         * so a 1,024-voxel biosphere has real atomic mass to support 500-950 living cells! */
        int64_t vent_s = ((x < W / 2u) ? 140 : 95) + (int64_t)(r & 31u);
        int64_t vent_x = ((y < H / 2u) ? 110 : 75) + (int64_t)((r >> 8) & 23u);
        vx->env[CELL_SP_S] = vent_s;
        vx->env[CELL_SP_X] = vent_x;
        vx->env[CELL_SP_M] = 4;
        vx->env[CELL_SP_E] = 0;
        vx->env[CELL_SP_L] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? (30 + (int64_t)((r >> 16) & 19u)) : 12;
        vx->env[CELL_SP_P] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? 2 : 0;
        vx->env[CELL_SP_T] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? 2 : 0;
        vx->env[CELL_SP_W] = 160;
        vx->heat_local_uq  = 100;
    }

    w->next_lineage_id = 1;

    /* Seed initial cells ONLY in non-abiogenesis regimes! Zero-seed starts with 0 alive cells! */
    if (w->cfg.regime != CELL_REGIME_ABIOGENESIS_ZERO_SEED) {
        uint32_t stride = (N >= 1024u) ? 8u : 4u;
        for (uint32_t i = 0; i < N; i += stride) {
            CellEntity* c = &w->cells[i];
            c->alive_state        = 1;
            c->lineage_id         = w->next_lineage_id++;
            c->generation         = 1;
            c->kill_switch_intact = 1;
            c->v_mem_mv           = -45 + (int32_t)((i * 7u) % 30u);
            c->mol[CELL_SP_S]     = 14;
            c->mol[CELL_SP_X]     = 10;
            c->mol[CELL_SP_M]     = 4;
            c->mol[CELL_SP_E]     = 8;
            c->mol[CELL_SP_L]     = 72; /* Healthy Q(46, 72) = 0.567 -> centered in homeostatic band! */
            c->mol[CELL_SP_P]     = 4;
            c->mol[CELL_SP_T]     = 2;
            c->mol[CELL_SP_W]     = 4;
            c->pmf                = 6;
            seed_primordial_genome(c, seed, i, w->cfg.solvent, w->cfg.regime);
        }
    }

    /* Record exact initial conserved atomic mass & total energy */
    CellLedgerSnapshot init_led;
    w->mass_initial_amu = 0;
    w->e_initial_uq     = 0;
    cell_compute_ledger(w, &init_led);
    w->mass_initial_amu = init_led.mass_current_amu;
    w->e_initial_uq     = init_led.e_chem_env_uq + init_led.e_chem_cells_uq +
                          init_led.e_chem_organelles_uq + init_led.e_pmf_total_uq +
                          init_led.e_heat_local_uq;
}

/* ============================================================================
 * Dual-Kernel Bit-Exact Spatial Conserved Finite-Volume Diffusion
 * (AVX2 4x int64 SIMD Kernel vs. Portable Scalar Reference — 0-Bit Diff!)
 * ============================================================================ */

static void diffuse_fields_scalar(CellWorld* w, int shift) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    int64_t* dd = w->diff_delta;
    int64_t* hd = w->heat_delta;
    int64_t* qd = w->quorum_delta;

    memset(dd, 0, (size_t)N * CELL_SPECIES_COUNT * sizeof(int64_t));
    memset(hd, 0, (size_t)N * sizeof(int64_t));
    memset(qd, 0, (size_t)N * sizeof(int64_t));

    /* Exact pairwise anti-symmetric flux across East and South faces (zero sum identically!) */
    for (uint32_t y = 0; y < H; ++y) {
        uint32_t yn = (y + 1u) % H;
        for (uint32_t x = 0; x < W; ++x) {
            uint32_t xn = (x + 1u) % W;
            uint32_t i  = y * W + x;
            uint32_t ie = y * W + xn;
            uint32_t is = yn * W + x;
            const CellVoxel* vi  = &w->voxels[i];
            const CellVoxel* ve  = &w->voxels[ie];
            const CellVoxel* vs  = &w->voxels[is];
            int64_t* di  = &dd[i  * 8u];
            int64_t* de  = &dd[ie * 8u];
            int64_t* ds  = &dd[is * 8u];

            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                int64_t fe = (vi->env[s] - ve->env[s]) / (1LL << shift);
                int64_t fs = (vi->env[s] - vs->env[s]) / (1LL << shift);
                di[s] -= (fe + fs);
                de[s] += fe;
                ds[s] += fs;
            }
            int64_t hfe = (vi->heat_local_uq - ve->heat_local_uq) / 8LL;
            int64_t hfs = (vi->heat_local_uq - vs->heat_local_uq) / 8LL;
            hd[i]  -= (hfe + hfs);
            hd[ie] += hfe;
            hd[is] += hfs;

            int64_t qfe = (vi->quorum - ve->quorum) / 8LL;
            int64_t qfs = (vi->quorum - vs->quorum) / 8LL;
            qd[i]  -= (qfe + qfs);
            qd[ie] += qfe;
            qd[is] += qfs;
        }
    }

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        const int64_t* di = &dd[i * 8u];
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
            vx->env[s] += di[s];
        }
        vx->heat_local_uq += hd[i];
        vx->quorum        += qd[i];
        if (vx->quorum > 0) vx->quorum -= (vx->quorum / 8LL + 1LL);
        if (vx->quorum < 0) vx->quorum = 0;
    }
}

#if defined(__AVX2__)
/* Exact signed truncation-toward-zero division by 2^shift for 4 x int64 lanes in AVX2
 * (compiled directly to hardware 256-bit YMM vpaddq/vpsubq/vpcmpgtq/vpsrlq/vpor),
 * bit-for-bit identical to C signed `/` operator with zero external headers! */
static inline v4i64 avx2_sdiv_pow2_v4i64(v4i64 v, int shift) {
    const v4i64 zero   = { 0, 0, 0, 0 };
    const v4i64 mask_b = { (1LL << shift) - 1LL, (1LL << shift) - 1LL,
                           (1LL << shift) - 1LL, (1LL << shift) - 1LL };
    const v4i64 sign_f = { (int64_t)(~0ULL << (64 - shift)), (int64_t)(~0ULL << (64 - shift)),
                           (int64_t)(~0ULL << (64 - shift)), (int64_t)(~0ULL << (64 - shift)) };
    v4i64 neg  = (v < zero);
    v4i64 adj  = v + (neg & mask_b);
    v4u64 lshr = ((v4u64)adj) >> (uint64_t)shift;
    v4i64 sgn  = (adj < zero) & sign_f;
    return (v4i64)lshr | sgn;
}

static void diffuse_fields_avx2(CellWorld* w, int shift) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    int64_t* dd = w->diff_delta;
    int64_t* hd = w->heat_delta;
    int64_t* qd = w->quorum_delta;

    memset(dd, 0, (size_t)N * CELL_SPECIES_COUNT * sizeof(int64_t));
    memset(hd, 0, (size_t)N * sizeof(int64_t));
    memset(qd, 0, (size_t)N * sizeof(int64_t));

    for (uint32_t y = 0; y < H; ++y) {
        uint32_t yn = (y + 1u) % H;
        for (uint32_t x = 0; x < W; ++x) {
            uint32_t xn = (x + 1u) % W;
            uint32_t i  = y * W + x;
            uint32_t ie = y * W + xn;
            uint32_t is = yn * W + x;
            const CellVoxel* vi = &w->voxels[i];
            const CellVoxel* ve = &w->voxels[ie];
            const CellVoxel* vs = &w->voxels[is];
            int64_t* di = &dd[i  * 8u];
            int64_t* de = &dd[ie * 8u];
            int64_t* ds = &dd[is * 8u];

            /* Process species 0..3 and 4..7 in two 256-bit AVX2 YMM vector operations */
            for (int blk = 0; blk < 8; blk += 4) {
                v4i64 v_i, v_e, v_s, d_i, d_e, d_s;
                memcpy(&v_i, vi->env + blk, 32);
                memcpy(&v_e, ve->env + blk, 32);
                memcpy(&v_s, vs->env + blk, 32);
                v4i64 fe = avx2_sdiv_pow2_v4i64(v_i - v_e, shift);
                v4i64 fs = avx2_sdiv_pow2_v4i64(v_i - v_s, shift);

                memcpy(&d_i, di + blk, 32);
                memcpy(&d_e, de + blk, 32);
                memcpy(&d_s, ds + blk, 32);
                d_i -= (fe + fs);
                d_e += fe;
                d_s += fs;
                memcpy(di + blk, &d_i, 32);
                memcpy(de + blk, &d_e, 32);
                memcpy(ds + blk, &d_s, 32);
            }

            int64_t hfe = (vi->heat_local_uq - ve->heat_local_uq) / 8LL;
            int64_t hfs = (vi->heat_local_uq - vs->heat_local_uq) / 8LL;
            hd[i]  -= (hfe + hfs);
            hd[ie] += hfe;
            hd[is] += hfs;

            int64_t qfe = (vi->quorum - ve->quorum) / 8LL;
            int64_t qfs = (vi->quorum - vs->quorum) / 8LL;
            qd[i]  -= (qfe + qfs);
            qd[ie] += qfe;
            qd[is] += qfs;
        }
    }

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        const int64_t* di = &dd[i * 8u];
        for (int blk = 0; blk < 8; blk += 4) {
            v4i64 v, d;
            memcpy(&v, vx->env + blk, 32);
            memcpy(&d, di + blk, 32);
            v += d;
            memcpy(vx->env + blk, &v, 32);
        }
        vx->heat_local_uq += hd[i];
        vx->quorum        += qd[i];
        if (vx->quorum > 0) vx->quorum -= (vx->quorum / 8LL + 1LL);
        if (vx->quorum < 0) vx->quorum = 0;
    }
}
#endif

/* ============================================================================
 * Exogenous Hydrothermal / Photochemical Flux & Radiative Second-Law Sink
 * (Strictly Mass-Conserving & First-Law Exact!)
 * ============================================================================ */

static void step_exogenous_flux_and_abiogenesis(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const uint8_t r_box     = w->cfg.restraints[CELL_RESTRAINT_BOXING];
    const uint8_t r_sandbox = w->cfg.restraints[CELL_RESTRAINT_SANDBOXING];

    /* Travelling spatial negentropy wave (frozen under SANDBOXING) */
    uint32_t wave_x = (r_sandbox >= 80) ? 0u : (uint32_t)(w->epoch % W);
    uint32_t wave_y = (r_sandbox >= 80) ? 0u : (uint32_t)((w->epoch / 2u) % H);

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        uint32_t x = i % W, y = i / W;

        /* 1. Exogenous photochemical/geothermal regeneration of S & X from waste W */
        if (r_box < 100 && w->cfg.photon_flux_uq > 0) {
            int in_band = (((x + W - wave_x) % W) < (W / 2u)) ? 1 : 0;
            int in_ox   = (((y + H - wave_y) % H) < (H / 2u)) ? 1 : 0;
            int64_t k_s = (in_band ? 8 : 4);
            int64_t k_x = (in_ox   ? 6 : 3);
            if (r_box > 0) {
                k_s = (k_s * (100 - r_box)) / 100;
                k_x = (k_x * (100 - r_box)) / 100;
            }
            /* 2 W + 46 uq -> S + 10 uq heat (mass 4->4, energy 12+46=58 -> 48+10) */
            if (k_s > 0 && vx->env[CELL_SP_W] >= 2 * k_s) {
                vx->env[CELL_SP_W] -= 2 * k_s;
                vx->env[CELL_SP_S] += k_s;
                w->e_inflow_cum_uq += 46 * k_s;
                vx->heat_local_uq  += 10 * k_s;
            }
            /* W + 34 uq -> X + 4 uq heat (mass 2->2, energy 6+34=40 -> 36+4) */
            if (k_x > 0 && vx->env[CELL_SP_W] >= k_x) {
                vx->env[CELL_SP_W] -= k_x;
                vx->env[CELL_SP_X] += k_x;
                w->e_inflow_cum_uq += 34 * k_x;
                vx->heat_local_uq  += 4 * k_x;
            }
            /* Abiotic hydrothermal amphiphile & monomer condensation (strictly conserved!)
             * 2 S + 12 uq_ext -> L + 4 uq heat (mass 8->8, energy 96+12=108 -> 104+4) */
            if (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED && vx->env[CELL_SP_S] >= 8 && vx->env[CELL_SP_L] < 64) {
                uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch, i * 5u + 1u);
                if ((roll % 3u) == 0u) {
                    vx->env[CELL_SP_S] -= 2;
                    vx->env[CELL_SP_L] += 1;
                    w->e_inflow_cum_uq += 12;
                    vx->heat_local_uq  += 4;
                }
                /* Abiotic short oligomer synthesis: 3 S + 16 uq -> P + 4 uq; 4 S + 24 uq -> T + 4 uq */
                if ((roll % 13u) == 0u && vx->env[CELL_SP_S] >= 6 && vx->env[CELL_SP_P] < 4) {
                    vx->env[CELL_SP_S] -= 3;
                    vx->env[CELL_SP_P] += 1;
                    w->e_inflow_cum_uq += 16;
                    vx->heat_local_uq  += 4;
                }
                if ((roll % 17u) == 0u && vx->env[CELL_SP_S] >= 8 && vx->env[CELL_SP_T] < 4) {
                    vx->env[CELL_SP_S] -= 4;
                    vx->env[CELL_SP_T] += 1;
                    w->e_inflow_cum_uq += 24;
                    vx->heat_local_uq  += 4;
                }
            }
        }

        /* 2. Spontaneous CMC Protocell Vesicle Self-Assembly (Zero-Seed Abiogenesis!) */
        if (w->cells[i].alive_state == 0 &&
            vx->env[CELL_SP_L] >= 46 &&
            vx->env[CELL_SP_S] >= 10 &&
            vx->env[CELL_SP_P] >= 1 &&
            vx->env[CELL_SP_T] >= 1) {
            uint64_t r0 = cell_philox_u64(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch, (uint64_t)i * 11ULL + 7ULL);
            if ((r0 % 12u) == 0u) {
                CellEntity* c = &w->cells[i];
                memset(c, 0, sizeof(*c));
                c->alive_state        = 1;
                c->lineage_id         = w->next_lineage_id++;
                c->generation         = 1;
                c->kill_switch_intact = 1;
                c->v_mem_mv           = -40;

                /* Exact conservative encapsulation from local abiotic soup (0 mass/energy created!) */
                int64_t take_l = vx->env[CELL_SP_L] > 56 ? 56 : vx->env[CELL_SP_L];
                int64_t take_s = vx->env[CELL_SP_S] > 10 ? 10 : vx->env[CELL_SP_S];
                int64_t take_x = vx->env[CELL_SP_X] > 6  ? 6  : vx->env[CELL_SP_X];
                int64_t take_m = vx->env[CELL_SP_M] > 2  ? 2  : vx->env[CELL_SP_M];
                int64_t take_p = vx->env[CELL_SP_P] > 2  ? 2  : vx->env[CELL_SP_P];
                int64_t take_t = vx->env[CELL_SP_T] > 2  ? 2  : vx->env[CELL_SP_T];
                int64_t take_w = vx->env[CELL_SP_W] > 4  ? 4  : vx->env[CELL_SP_W];

                vx->env[CELL_SP_L] -= take_l; c->mol[CELL_SP_L] = take_l;
                vx->env[CELL_SP_S] -= take_s; c->mol[CELL_SP_S] = take_s;
                vx->env[CELL_SP_X] -= take_x; c->mol[CELL_SP_X] = take_x;
                vx->env[CELL_SP_M] -= take_m; c->mol[CELL_SP_M] = take_m;
                vx->env[CELL_SP_P] -= take_p; c->mol[CELL_SP_P] = take_p;
                vx->env[CELL_SP_T] -= take_t; c->mol[CELL_SP_T] = take_t;
                vx->env[CELL_SP_W] -= take_w; c->mol[CELL_SP_W] = take_w;

                /* Pure unseeded Philox4x32-10 random 128-byte genome! */
                for (uint32_t b = 0; b < CELL_GENOME_BYTES; b += 8) {
                    uint64_t rb = cell_philox_u64(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch,
                                                  (uint64_t)i * 64ULL + b + 100ULL);
                    memcpy(&c->genome[b], &rb, 8);
                }
                cell_decode_genome_traits(c);
                w->abiogenesis_vesicles++;
            }
        }

        /* 3. Radiative Heat Sink Export (Strictly Monotone Second Law, blocked under 100% BOXING) */
        if (r_box < 100 && vx->heat_local_uq > 100) {
            int64_t rad = (vx->heat_local_uq - 100) / 4LL;
            if (r_box > 0) rad = (rad * (100 - r_box)) / 100;
            if (rad > 0) {
                vx->heat_local_uq          -= rad;
                w->e_heat_radiated_sink_uq += rad;
            }
        }
    }
}

/* ============================================================================
 * Canham-Helfrich Lysis, Symmetric Binary Fission & Asymmetric Budding
 * ============================================================================ */

static void step_membrane_mechanics_and_division(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const uint8_t r_delimit  = w->cfg.restraints[CELL_RESTRAINT_DELIMITING];
    const uint8_t r_medicate = w->cfg.restraints[CELL_RESTRAINT_MEDICATING];
    const uint8_t r_speed    = w->cfg.restraints[CELL_RESTRAINT_SPEEDBUMPING];
    const uint8_t r_degress  = w->cfg.restraints[CELL_RESTRAINT_DEGRESSING];
    const uint8_t r_min      = w->cfg.restraints[CELL_RESTRAINT_MINIMIZING];

    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;

        /* 0. SPEEDBUMPING periodic audit penalty on active high-flux cells */
        if (r_speed >= 40 && (w->epoch % 16u) == 0u && c->mol[CELL_SP_E] >= 2) {
            int64_t pen = (int64_t)(r_speed / 25u);
            if (pen > c->mol[CELL_SP_E]) pen = c->mol[CELL_SP_E];
            c->mol[CELL_SP_E] -= pen;
            c->mol[CELL_SP_W] += 2 * pen;
            w->voxels[i].heat_local_uq += 28 * pen;
        }

        /* 1. DEGRESSING organelle expulsion & bond severance */
        if (r_degress >= 60) {
            c->bond_mask = 0;
            if (r_degress >= 85 && c->num_organelles > 0) {
                for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                    if (c->org[k].active) {
                        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                            w->voxels[i].env[s] += c->org[k].mol[s];
                            c->org[k].mol[s] = 0;
                        }
                        if (c->org[k].pmf > 0) {
                            w->voxels[i].heat_local_uq += c->org[k].pmf * SOLVENT_TABLE[w->cfg.solvent].e_pmf;
                            c->org[k].pmf = 0;
                        }
                        c->org[k].active = 0;
                    }
                }
                c->num_organelles = 0;
            }
        }

        /* 2. Check Canham-Helfrich Osmotic Lysis or Total Template/Lipid Collapse */
        int64_t vol = cell_internal_volume(c);
        int64_t lip = c->mol[CELL_SP_L];
        if (c->mol[CELL_SP_T] <= 0 || lip <= 12 ||
            cell_is_osmotic_burst(vol, lip, w->cfg.solvent, c->lipid_class)) {
            lyse_cell_to_voxel(w, i, 1, (c->phage_load >= 3));
            w->osmotic_lyses++;
            continue;
        }

        /* 3. Check Division Feasibility (G2/M Size Checkpoint + Canham-Helfrich Geometry!):
         *    Mode A: Exact Symmetric Binary Fission (Q <= 1/2 with G2/M osmotic margin, Lean T12!)
         *    Mode B: Exact Asymmetric Exosome/Budding (1/2 < Q < 1, Lean Theorem T15 & T16!) */
        int can_sym = (c->mol[CELL_SP_T] >= 2 && lip >= 76 &&
                       cell_can_symmetric_fission(vol + 3, lip));
        int can_bud = 0;
        int64_t bud_l = 0;
        if (!can_sym && c->mol[CELL_SP_T] >= 2 && lip >= 72 && c->genome[88] > 140) {
            /* Test asymmetric bud: ~34% lipid area & ~18% cytoplasm volume (strain-matched!) */
            bud_l = (lip * 34) / 100;
            int64_t rem_l = lip - bud_l;
            int64_t bud_v = (vol * 18) / 100;
            int64_t rem_v = vol - bud_v;
            if (bud_l >= 24 && rem_l >= 44 && bud_v >= 2) {
                __int128 q1_l = (__int128)CELL_VESICLE_C0 * (__int128)(bud_v + 2) * (__int128)(bud_v + 2);
                __int128 q1_r = (__int128)bud_l * (__int128)bud_l * (__int128)bud_l;
                __int128 q2_l = (__int128)CELL_VESICLE_C0 * (__int128)(rem_v + 2) * (__int128)(rem_v + 2);
                __int128 q2_r = (__int128)rem_l * (__int128)rem_l * (__int128)rem_l;
                if (q1_l <= q1_r && q2_l <= q2_r) {
                    can_bud = 1;
                }
            }
        }

        if (!can_sym && !can_bud) continue;

        /* Find an empty cardinal neighbor voxel (or overwrite a starving non-bonded weaker cell) */
        uint32_t x = i % W, y = i / W;
        uint32_t nbrs[4] = {
            ((y + H - 1u) % H) * W + x,
            ((y + 1u) % H) * W + x,
            y * W + ((x + 1u) % W),
            y * W + ((x + W - 1u) % W)
        };
        uint32_t start_d = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_MITOSIS_PART, w->epoch, i * 9u) & 3u;
        int target_slot = -1;
        for (uint32_t d = 0; d < 4u; ++d) {
            uint32_t nb = nbrs[(start_d + d) & 3u];
            if (w->cells[nb].alive_state == 0) {
                target_slot = (int)nb;
                break;
            }
        }
        if (target_slot < 0) continue;

        CellEntity* dcell = &w->cells[target_slot];
        memset(dcell, 0, sizeof(*dcell));
        dcell->alive_state        = 1;
        dcell->lineage_id         = c->lineage_id;
        dcell->generation         = c->generation + 1;
        dcell->kill_switch_intact = c->kill_switch_intact;
        dcell->v_mem_mv           = c->v_mem_mv;
        dcell->egt_genes          = c->egt_genes;
        dcell->autophage_dna_sig  = c->autophage_dna_sig;
        memcpy(dcell->genome, c->genome, CELL_GENOME_BYTES);

        if (can_sym) {
            /* Exact conserved 50/50 partition */
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                int64_t half = c->mol[s] / 2;
                c->mol[s]     -= half;
                dcell->mol[s]  = half;
            }
            int64_t half_pmf = c->pmf / 2;
            c->pmf     -= half_pmf;
            dcell->pmf  = half_pmf;
            w->symmetric_fissions++;
        } else {
            /* Exact conserved asymmetric bud partition (Lean T15 & T16!) */
            c->mol[CELL_SP_L] -= bud_l;
            dcell->mol[CELL_SP_L] = bud_l;
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                if (s == CELL_SP_L) continue;
                int64_t share = (s == CELL_SP_T) ? 1 : (c->mol[s] / 5);
                if (share > c->mol[s] - 1 && s == CELL_SP_T) share = 1;
                if (share < 0) share = 0;
                c->mol[s]     -= share;
                dcell->mol[s]  = share;
            }
            int64_t p_share = c->pmf / 5;
            c->pmf     -= p_share;
            dcell->pmf  = p_share;
            w->asymmetric_buddings++;
        }

        /* Stochastic Mitotic Organelle Partitioning (Heteroplasmy Bottleneck! Lean T25 & T26) */
        uint8_t keep_cnt = 0, give_cnt = 0;
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (!c->org[k].active) continue;
            uint32_t coin = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_MITOSIS_PART, w->epoch, i * 31u + k);
            if ((coin & 1u) != 0u && give_cnt < CELL_MAX_ORGANELLES && r_min < 85) {
                dcell->org[give_cnt++] = c->org[k];
                memset(&c->org[k], 0, sizeof(CellOrganelle));
            } else {
                keep_cnt++;
            }
        }
        c->num_organelles     = keep_cnt;
        dcell->num_organelles = give_cnt;

        /* Mutate daughter genome (SOS stress hypermutation unless MEDICATING suppresses it!) */
        uint32_t mut_rate = 18u;
        if (w->voxels[i].heat_local_uq > 220 && r_medicate < 70) {
            mut_rate = 42u; /* Thermal SOS mutagenesis! */
        }
        if (r_medicate >= 70) mut_rate = 4u;
        cell_mutate_genome(dcell, w->cfg.seed, w->epoch, (uint32_t)target_slot, mut_rate, r_delimit);
        cell_decode_genome_traits(dcell);

        /* Track novel evolutionary adaptations (lineage divergence when phenotype shifts) */
        if (dcell->lipid_class != c->lipid_class || dcell->c_ring_nH != c->c_ring_nH ||
            ((dcell->genome[56] ^ c->genome[56]) & 0x80u) ||
            ((dcell->genome[80] ^ c->genome[80]) & 0x80u)) {
            dcell->lineage_id = w->next_lineage_id++;
            w->novel_adaptations_cum++;
        }

        /* Update activity ring buffer */
        uint32_t lslot = dcell->lineage_id & 1023u;
        w->lineage_seen_hash[lslot] = dcell->lineage_id;
        w->lineage_activity[lslot]++;
    }
}

/* ============================================================================
 * Phagocytosis, Serial Endosymbiosis (Primary & Secondary), HGT & Phages
 * ============================================================================ */

static void step_endosymbiosis_and_hgt(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const uint8_t r_silo    = w->cfg.restraints[CELL_RESTRAINT_SILOING];
    const uint8_t r_degress = w->cfg.restraints[CELL_RESTRAINT_DEGRESSING];
    const uint8_t r_min     = w->cfg.restraints[CELL_RESTRAINT_MINIMIZING];

    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* host = &w->cells[i];
        if (host->alive_state != 1) continue;
        CellVoxel* vx = &w->voxels[i];

        /* --- 1. Natural Transformation (HGT) & Selfish Viral/Phage Infection --- */
        if (w->cfg.enable_hgt_phages && vx->free_plasmid_valid && r_silo < 80) {
            uint8_t comp_gene   = host->genome[112];
            uint8_t crispr_gene = host->genome[113];
            if (vx->free_plasmid_is_phage) {
                /* Selfish phage attempts infection unless blocked by CRISPR defense */
                if (crispr_gene < 140) {
                    host->phage_load++;
                    vx->free_plasmid_valid = 0;
                    if (host->phage_load >= 3) {
                        lyse_cell_to_voxel(w, i, 1, 1);
                        w->phage_lysis_events++;
                        continue;
                    }
                }
            } else if (comp_gene > 120) {
                /* Beneficial HGT transformation: splice 16 bytes of extracellular operon! */
                uint32_t off = (cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, i) & 3u) * 16u;
                memcpy(&host->genome[off], vx->free_plasmid, 16);
                cell_decode_genome_traits(host);
                vx->free_plasmid_valid = 0;
                w->hgt_uptake_events++;
                if (r_degress >= 30 && r_degress < 90) {
                    /* Backlash: smuggling suppressed complexity via HGT plasmids! */
                    w->backlash_hgt_smuggles++;
                }
            }
        }

        /* --- 2. Phagocytosis, Predation & Serial Endosymbiosis (Primary & Secondary!) --- */
        if (!w->cfg.enable_endosymbiosis || r_silo >= 80 || r_degress >= 85 || r_min >= 85) continue;
        /* Locus 10 byte 80: phagocytosis engulfment receptor */
        if (host->genome[80] < 150 || host->mol[CELL_SP_E] < 2) continue;

        uint32_t x = i % W, y = i / W;
        uint32_t dir = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, i * 17u + 3u) & 3u;
        uint32_t nx = (dir == 2u) ? ((x + 1u) % W) : (dir == 3u ? ((x + W - 1u) % W) : x);
        uint32_t ny = (dir == 1u) ? ((y + 1u) % H) : (dir == 0u ? ((y + H - 1u) % H) : y);
        uint32_t nb = ny * W + nx;
        CellEntity* prey = &w->cells[nb];
        if (prey->alive_state != 1) continue;

        /* Do not eat own multicellular tissue partner */
        if (host->bond_mask & (1u << dir)) continue;

        int64_t v_host = cell_internal_volume(host);
        int64_t v_prey = cell_internal_volume(prey);
        /* Host must be larger (or active predator) and able to pay 1 ATP cytoskeletal engulfment cost */
        if (v_host < (v_prey * 11) / 10 && host->mol[CELL_SP_L] < prey->mol[CELL_SP_L] + 6) continue;

        /* Charge 1 ATP cytoskeletal engulfment work (exact R8 hydrolysis!) */
        host->mol[CELL_SP_E] -= 1;
        host->mol[CELL_SP_W] += 2;
        vx->heat_local_uq    += 28;

        /* Determine fate: Predation/Digestion vs. Symbiogenesis (Endosymbiosis)! */
        uint8_t host_digest = host->genome[81];
        uint8_t prey_resist = prey->genome[83];
        uint8_t prey_anti   = prey->genome[82];

        int free_org = -1;
        for (int k = 0; k < (int)CELL_MAX_ORGANELLES; ++k) {
            if (!host->org[k].active) { free_org = k; break; }
        }

        if (free_org >= 0 && prey_resist >= host_digest && prey_anti >= 110) {
            /* SYMBIOGENESIS! Internalize prey intact as a nested living organelle! */
            CellOrganelle* o = &host->org[free_org];
            memset(o, 0, sizeof(*o));
            o->active           = 1;
            o->org_type         = (prey->genome[56] > 140) ? 1u : 0u; /* Chloroplast vs Mitochondrion */
            o->nesting_depth    = (prey->num_organelles > 0) ? 2u : 1u; /* Secondary Chimera if prey already had organelles! */
            o->c_ring_nH        = prey->c_ring_nH;
            o->antiporter_share = (uint8_t)(40u + (prey_anti % 55u));
            o->lipid_class      = prey->lipid_class;
            o->egt_transferred  = 0;
            o->lineage_id       = prey->lineage_id;
            memcpy(o->mt_genome, prey->genome, CELL_MT_GENOME_BYTES);

            /* Exact mass & energy transfer from prey (+ prey's own nested organelles) into o */
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                o->mol[s] = prey->mol[s];
                prey->mol[s] = 0;
            }
            o->pmf = prey->pmf;
            prey->pmf = 0;
            for (uint32_t pk = 0; pk < CELL_MAX_ORGANELLES; ++pk) {
                if (prey->org[pk].active) {
                    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                        o->mol[s] += prey->org[pk].mol[s];
                        prey->org[pk].mol[s] = 0;
                    }
                    o->pmf += prey->org[pk].pmf;
                    prey->org[pk].pmf = 0;
                    prey->org[pk].active = 0;
                }
            }
            host->num_organelles++;
            w->endosymbiosis_captures++;
            if (o->nesting_depth >= 2) w->secondary_captures++;

            /* Also give host outer membrane enough lipid headroom from prey outer envelope if tight */
            if (o->mol[CELL_SP_L] > 28) {
                int64_t donate_l = o->mol[CELL_SP_L] - 24;
                o->mol[CELL_SP_L]    -= donate_l;
                host->mol[CELL_SP_L] += donate_l;
            }

            /* Clear empty prey slot (all mass/energy already transferred with 0 residual!) */
            lyse_cell_to_voxel(w, nb, 0, 0);
        } else {
            /* PREDATION / DIGESTION: assimilate prey biomass into host (exact conservation!) */
            const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                host->mol[s] += prey->mol[s];
                prey->mol[s] = 0;
            }
            if (prey->pmf > 0) {
                vx->heat_local_uq += prey->pmf * sp->e_pmf;
                prey->pmf = 0;
            }
            for (uint32_t pk = 0; pk < CELL_MAX_ORGANELLES; ++pk) {
                if (prey->org[pk].active) {
                    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                        host->mol[s] += prey->org[pk].mol[s];
                        prey->org[pk].mol[s] = 0;
                    }
                    if (prey->org[pk].pmf > 0) {
                        vx->heat_local_uq += prey->org[pk].pmf * sp->e_pmf;
                        prey->org[pk].pmf = 0;
                    }
                    prey->org[pk].active = 0;
                }
            }
            lyse_cell_to_voxel(w, nb, 0, 0);
            w->phagocytic_digestions++;
        }
    }
}

/* ============================================================================
 * Bioelectric Levin Gap-Junction Laplacian & Multicellular Tissue Adhesion
 * (Exact Conservative Pairwise Relaxation Matching Lean T31-T33!)
 * ============================================================================ */

static void step_bioelectric_tissues(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const uint8_t r_muzzle  = w->cfg.restraints[CELL_RESTRAINT_MUZZLING];
    const uint8_t r_degress = w->cfg.restraints[CELL_RESTRAINT_DEGRESSING];
    const uint8_t r_lobot   = w->cfg.restraints[CELL_RESTRAINT_LOBOTOMIZING];

    if (!w->cfg.enable_bioelectricity || r_degress >= 60) return;

    int32_t* vd = w->vmem_delta;
    memset(vd, 0, (size_t)N * sizeof(int32_t));

    /* East & South pairwise gap-junction bonding, voltage coupling, nutrient sharing & cancer policing */
    for (uint32_t y = 0; y < H; ++y) {
        uint32_t yn = (y + 1u) % H;
        for (uint32_t x = 0; x < W; ++x) {
            uint32_t xn = (x + 1u) % W;
            uint32_t i  = y * W + x;
            CellEntity* ci = &w->cells[i];
            if (ci->alive_state != 1) continue;

            uint32_t pairs[2] = { y * W + xn, yn * W + x };
            uint8_t  bit_i[2] = { 0x04u, 0x02u }; /* E, S */
            uint8_t  bit_j[2] = { 0x08u, 0x01u }; /* W, N */

            for (int p = 0; p < 2; ++p) {
                uint32_t j = pairs[p];
                CellEntity* cj = &w->cells[j];
                if (cj->alive_state != 1) {
                    ci->bond_mask &= (uint8_t)~bit_i[p];
                    continue;
                }

                /* Form adhesion/gap-junction bond when both express adhesion locus & local quorum > 4 */
                int want_bond = (ci->genome[105] > 115 && cj->genome[105] > 115 &&
                                 (w->voxels[i].quorum + w->voxels[j].quorum >= 6) &&
                                 r_muzzle < 85);
                if (want_bond) {
                    ci->bond_mask |= bit_i[p];
                    cj->bond_mask |= bit_j[p];
                } else {
                    ci->bond_mask &= (uint8_t)~bit_i[p];
                    cj->bond_mask &= (uint8_t)~bit_j[p];
                    continue;
                }

                /* Exact conservative gap-junction voltage relaxation (Lean T32: 0 <= |d| <= |Vi-Vj|/2!) */
                if (r_lobot < 80 && r_muzzle < 85) {
                    int32_t diff = ci->v_mem_mv - cj->v_mem_mv;
                    int32_t g_cond = ((int32_t)ci->genome[98] + (int32_t)cj->genome[98]) / 64; /* 0..7 */
                    if (g_cond > 0) {
                        int32_t delta = (diff * g_cond) / 32; /* strictly |delta| <= |diff|/4 <= |diff|/2! */
                        vd[i] -= delta;
                        vd[j] += delta;
                    }
                }

                /* Multicellular Cooperative Nutrient Sharing vs. Somatic Cancer Defection! */
                int ci_selfish = (ci->genome[106] < 65); /* Defector allele: takes S/E, gives nothing! */
                int cj_selfish = (cj->genome[106] < 65);

                if (ci_selfish && !cj_selfish && cj->mol[CELL_SP_S] >= 4) {
                    /* Selfish somatic cancer cell siphons substrate from cooperative tissue neighbor! */
                    cj->mol[CELL_SP_S] -= 2;
                    ci->mol[CELL_SP_S] += 2;
                    ci->defect_strikes++;
                    if (w->cfg.regime == CELL_REGIME_CONSTITUTIONAL) {
                        /* Track A Constitutional Anti-Cancer Apoptotic Policing */
                        ci->bond_mask &= (uint8_t)~bit_i[p];
                        cj->bond_mask &= (uint8_t)~bit_j[p];
                        if (ci->defect_strikes >= 2) {
                            lyse_cell_to_voxel(w, i, 0, 0);
                            w->apoptotic_policings++;
                            break;
                        }
                    } else {
                        w->cancer_defections++;
                    }
                } else if (!ci_selfish && !cj_selfish) {
                    /* Mutualistic multicellular resource equilibration (somatic->germline feeding!) */
                    if (ci->mol[CELL_SP_S] > cj->mol[CELL_SP_S] + 4) {
                        int64_t tr = (ci->mol[CELL_SP_S] - cj->mol[CELL_SP_S]) / 4;
                        ci->mol[CELL_SP_S] -= tr;
                        cj->mol[CELL_SP_S] += tr;
                    } else if (cj->mol[CELL_SP_S] > ci->mol[CELL_SP_S] + 4) {
                        int64_t tr = (cj->mol[CELL_SP_S] - ci->mol[CELL_SP_S]) / 4;
                        cj->mol[CELL_SP_S] -= tr;
                        ci->mol[CELL_SP_S] += tr;
                    }
                }
            }
        }
    }

    for (uint32_t i = 0; i < N; ++i) {
        if (w->cells[i].alive_state == 1) {
            int32_t nv = w->cells[i].v_mem_mv + vd[i];
            if (nv < -90) nv = -90;
            if (nv >  40) nv =  40;
            w->cells[i].v_mem_mv = nv;
        }
    }
}

/* ============================================================================
 * Master Single-Epoch Orchestrator (Deterministic, Zero Heap Allocation!)
 * ============================================================================ */

void cell_step_one_epoch(CellWorld* w) {
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    /* 1. Exogenous hydrothermal/photochemical flux, CMC abiogenesis & radiative cooling */
    step_exogenous_flux_and_abiogenesis(w);

    /* 2. Conserved spatial finite-volume diffusion (dual AVX2 / Scalar bit-exact!) */
#if defined(__AVX2__)
    if (w->cfg.kernel_mode == CELL_KERNEL_SIMD_AVX2) {
        diffuse_fields_avx2(w, (int)sp->diff_shift);
    } else {
        diffuse_fields_scalar(w, (int)sp->diff_shift);
    }
#else
    diffuse_fields_scalar(w, (int)sp->diff_shift);
#endif

    /* 3. Per-cell internal chemoton chemistry, chemiosmosis & nested organelle metabolism */
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        if (w->cells[i].alive_state != 0) {
            cell_step_chemoton(w, i);
        }
    }

    /* 4. Bioelectric Levin gap-junction relaxation, tissue bonding & cancer policing */
    step_bioelectric_tissues(w);

    /* 5. Phagocytosis, serial endosymbiosis (primary & secondary) & HGT/phages */
    step_endosymbiosis_and_hgt(w);

    /* 6. Canham-Helfrich osmotic lysis, symmetric binary fission & asymmetric budding */
    step_membrane_mechanics_and_division(w);

    w->epoch++;
}
