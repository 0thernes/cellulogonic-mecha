#include "cellulogonic_internal.h"

#if defined(__AVX2__)
typedef int64_t  v4i64 __attribute__((vector_size(32), aligned(1)));
typedef uint64_t v4u64 __attribute__((vector_size(32), aligned(1)));
#endif

/* ============================================================================
 * 3D Off-Lattice Helpers (Exact Integer Fixed-Point Arithmetic — Bit-Exact!)
 * World fluid volume: [0, W*1000) x [0, H*1000) x [0, W*1000) nm
 * ============================================================================ */

static inline int32_t wrap_nm(int32_t p, int32_t span) {
    int32_t r = p % span;
    return r < 0 ? r + span : r;
}

static inline int32_t delta_torus_nm(int32_t a, int32_t b, int32_t span) {
    int32_t d = a - b;
    int32_t half = span / 2;
    if (d >  half) d -= span;
    if (d < -half) d += span;
    return d;
}

static inline int32_t isqrt32(int64_t x) {
    if (x <= 0) return 0;
    int64_t r = (int64_t)sqrt((double)x);
    while ((r + 1) * (r + 1) <= x) r++;
    while (r * r > x) r--;
    return (int32_t)r;
}

static void init_random_unit_heading(int32_t h[3], uint64_t r) {
    int32_t dx = (int32_t)((r & 2047u)) - 1024;
    int32_t dy = (int32_t)(((r >> 11) & 2047u)) - 1024;
    int32_t dz = (int32_t)(((r >> 22) & 2047u)) - 1024;
    int32_t mag = isqrt32((int64_t)dx * dx + (int64_t)dy * dy + (int64_t)dz * dz);
    if (mag < 16) { h[0] = 1024; h[1] = 0; h[2] = 0; return; }
    h[0] = (dx * 1024) / mag;
    h[1] = (dy * 1024) / mag;
    h[2] = (dz * 1024) / mag;
}

static void clear_cell_bonds(CellWorld* w, uint32_t idx) {
    CellEntity* c = &w->cells[idx];
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        int16_t pj = c->bond_partners[b];
        if (pj >= 0 && (uint32_t)pj < w->num_slots) {
            CellEntity* o = &w->cells[pj];
            for (uint32_t ob = 0; ob < CELL_MAX_BONDS; ++ob) {
                if (o->bond_partners[ob] == (int16_t)idx) {
                    o->bond_partners[ob] = -1;
                }
            }
            uint8_t cnt = 0;
            for (uint32_t ob = 0; ob < CELL_MAX_BONDS; ++ob) if (o->bond_partners[ob] >= 0) cnt++;
            o->num_bonds = cnt;
            o->bond_mask = (uint8_t)((1u << cnt) - 1u);
        }
        c->bond_partners[b] = -1;
    }
    c->num_bonds = 0;
    c->bond_mask = 0;
    for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
        c->axon_target[a] = -1;
        c->axon_weight[a] = 0;
    }
    /* Detach any incoming axons pointing at this lysed cell */
    for (uint32_t k = 0; k < w->num_slots; ++k) {
        CellEntity* o = &w->cells[k];
        if (o->alive_state == 0) continue;
        for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
            if (o->axon_target[a] == (int16_t)idx) {
                o->axon_target[a] = -1;
                o->axon_weight[a] = 0;
            }
        }
    }
}

static int are_bonded(const CellEntity* a, int16_t j) {
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        if (a->bond_partners[b] == j) return 1;
    }
    return 0;
}

static int try_form_bond(CellWorld* w, uint32_t i, uint32_t j) {
    if (i == j) return 0;
    CellEntity* ci = &w->cells[i];
    CellEntity* cj = &w->cells[j];
    if (are_bonded(ci, (int16_t)j)) return 1;
    int si = -1, sj = -1;
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        if (ci->bond_partners[b] < 0 && si < 0) si = (int)b;
        if (cj->bond_partners[b] < 0 && sj < 0) sj = (int)b;
    }
    if (si < 0 || sj < 0) return 0;
    ci->bond_partners[si] = (int16_t)j;
    cj->bond_partners[sj] = (int16_t)i;
    uint8_t cnti = 0, cntj = 0;
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        if (ci->bond_partners[b] >= 0) cnti++;
        if (cj->bond_partners[b] >= 0) cntj++;
    }
    ci->num_bonds = cnti; ci->bond_mask = (uint8_t)((1u << cnti) - 1u);
    cj->num_bonds = cntj; cj->bond_mask = (uint8_t)((1u << cntj) - 1u);
    return 1;
}

static void sever_bond_pair(CellWorld* w, uint32_t i, uint32_t j) {
    CellEntity* ci = &w->cells[i];
    CellEntity* cj = &w->cells[j];
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        if (ci->bond_partners[b] == (int16_t)j) ci->bond_partners[b] = -1;
        if (cj->bond_partners[b] == (int16_t)i) cj->bond_partners[b] = -1;
    }
    uint8_t cnti = 0, cntj = 0;
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
        if (ci->bond_partners[b] >= 0) cnti++;
        if (cj->bond_partners[b] >= 0) cntj++;
    }
    ci->num_bonds = cnti; ci->bond_mask = (uint8_t)((1u << cnti) - 1u);
    cj->num_bonds = cntj; cj->bond_mask = (uint8_t)((1u << cntj) - 1u);
}

static void init_empty_bonds(CellEntity* c) {
    for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) c->bond_partners[b] = -1;
    for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) { c->axon_target[a] = -1; c->axon_weight[a] = 0; }
    c->num_bonds   = 0;
    c->bond_mask   = 0;
    c->eutelic_rem = 255u; /* 255 = feral unlimited divisions; <255 = hardcoded eutelic countdown */
}

/* ============================================================================
 * Conservation-Exact Helper: Lyse a Cell (+ All Nested Organelles) into Voxel
 * ============================================================================ */

static void lyse_cell_to_voxel(CellWorld* w, uint32_t idx, int leave_plasmid, int is_phage) {
    CellEntity* c = &w->cells[idx];
    if (c->alive_state == 0) return;

    uint32_t vidx = cell_voxel_idx(w, c);
    CellVoxel* vx = &w->voxels[vidx];
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
        vx->env[s] += c->mol[s];
        c->mol[s] = 0;
    }
    if (c->pmf > 0) {
        vx->heat_local_uq += c->pmf * sp->e_pmf;
        c->pmf = 0;
    }

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

    clear_cell_bonds(w, idx);
    memset(c, 0, sizeof(*c));
    init_empty_bonds(c);
}

/* ============================================================================
 * World Initialization (Continuous 3D Off-Lattice Placement + Abiotic Soup)
 * ============================================================================ */

static void seed_primordial_genome(CellEntity* c, uint64_t seed, uint32_t idx, uint32_t solvent, uint32_t regime) {
    for (uint32_t b = 0; b < CELL_GENOME_BYTES; b += 8) {
        uint64_t r = cell_philox_u64(seed, RNG_DOMAIN_ABIOGENESIS, 1ULL, (uint64_t)idx * 32ULL + b);
        memcpy(&c->genome[b], &r, 8);
    }
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
        if ((idx % 3u) == 0u) {
            c->genome[56] = 200u; /* Autotroph photo-S */
            c->genome[57] = 190u; /* Oxygenic photo-X */
        }
        if ((idx % 4u) == 1u) {
            c->genome[80] = 210u; /* Active phagocytosis receptor */
            c->genome[81] = 75u;  /* Low digestion -> favors symbiogenesis! */
            c->genome[84] = 160u; /* Mitophagy policing */
        }
        c->genome[82] = 180u;     /* Antiporter ATP sharing */
        c->genome[83] = 180u;     /* Membrane digestion resistance */
    }
    cell_decode_genome_traits(c);
}

void cell_init_world_state(CellWorld* w) {
    const uint32_t N = w->num_slots;
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const int32_t span_x = (int32_t)W * CELL_VOXEL_NM;
    const int32_t span_y = (int32_t)H * CELL_VOXEL_NM;
    const int32_t span_z = (int32_t)W * CELL_VOXEL_NM;
    const uint64_t seed = w->cfg.seed;

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        memset(vx, 0, sizeof(*vx));
        memset(&w->cells[i], 0, sizeof(CellEntity));
        init_empty_bonds(&w->cells[i]);

        uint32_t x = i % W, y = i / W;
        uint64_t r = cell_philox_u64(seed, RNG_DOMAIN_WORLD_EXOG, 0ULL, i);

        int64_t vent_s = ((x < W / 2u) ? 140 : 95) + (int64_t)(r & 31u);
        int64_t vent_x = ((y < H / 2u) ? 110 : 75) + (int64_t)((r >> 8) & 23u);
        if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS) {
            /* Rich nutrient-dense maternal embryonic yolk broth for rapid blastula cleavage! */
            vent_s = 240 + (int64_t)(r & 31u);
            vent_x = 180 + (int64_t)((r >> 8) & 23u);
        }
        vx->env[CELL_SP_S] = vent_s;
        vx->env[CELL_SP_X] = vent_x;
        vx->env[CELL_SP_M] = 4;
        vx->env[CELL_SP_E] = 0;
        vx->env[CELL_SP_L] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? (30 + (int64_t)((r >> 16) & 19u))
                           : (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS ? 28 : 12);
        vx->env[CELL_SP_P] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? 2 : 0;
        vx->env[CELL_SP_T] = (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED) ? 2 : 0;
        vx->env[CELL_SP_W] = 160;
        vx->heat_local_uq  = 100;
    }

    w->next_lineage_id = 1;

    if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS) {
        /* Regime 5: 3D Embryogenesis from an 8-Blastomere Clonal Blastula!
         * ALL 8 blastomeres carry the IDENTICAL founder DNA (ZERO master body blueprint!),
         * differing ONLY in maternal cytoplasmic pole cues (Anterior Wnt, Dorsal BMP, Midline Shh, Vegetal Gut)
         * and a hardcoded eutelic division countdown (eutelic_rem = 5 -> max 8 * 2^5 = 256 cells, Lean T54)! */
        static const int32_t BLASTULA_OFF[8][3] = {
            { -1400,    0,    0 }, /* 0: Anterior Head / Wnt Organizer */
            { -1100,  700,  500 }, /* 1: Anterior-Dorsal Neural Plate */
            {  -400, 1200,  600 }, /* 2: Dorsal Ectoderm / BMP Pole */
            {   500, 1100, -500 }, /* 3: Dorsal-Posterior Ectoderm */
            {  -300, -900,  600 }, /* 4: Ventral-Midline Shh / Muscle */
            {   600, -800,  400 }, /* 5: Posterior Trunk Muscle / Netrin */
            {  1200, -200, -400 }, /* 6: Posterior Vegetal Gut (Apical Wedge) */
            {   200,  100, -900 }  /* 7: Interior Endoderm / Gut Lumen */
        };
        uint8_t founder_dna[CELL_GENOME_BYTES];
        CellEntity tmp;
        memset(&tmp, 0, sizeof(tmp));
        seed_primordial_genome(&tmp, seed, 0, w->cfg.solvent, CELL_REGIME_FERAL_ECOSYSTEM);
        tmp.genome[0]   = 235u; /* Fast R1 catabolism */
        tmp.genome[8]   = 230u; /* High R2 proton pump */
        tmp.genome[17]  = 235u; /* High R3 ATP synthase */
        tmp.genome[32]  = 245u; /* Rapid R5 embryonic membrane lipid synthesis */
        tmp.genome[40]  = 225u; /* High R6 enzyme translation */
        tmp.genome[48]  = 235u; /* Rapid R7 DNA replication */
        tmp.genome[72]  = 240u; /* High S permease uptake */
        tmp.genome[73]  = 230u; /* High X permease uptake */
        tmp.genome[74]  = 205u; /* Motile cilia / muscle contractile vigor */
        tmp.genome[96]  = 16u;  /* Wnt high threshold offset */
        tmp.genome[97]  = 6u;   /* Wnt low threshold offset */
        tmp.genome[98]  = 210u; /* Strong bioelectric gap junctions */
        tmp.genome[99]  = 10u;  /* BMP threshold offset */
        tmp.genome[100] = 10u;  /* Shh threshold offset */
        tmp.genome[101] = 2u;   /* Hippo contact-inhibition limit (6 crowded neighbors) */
        tmp.genome[105] = 230u; /* Strong cadherin adhesion expression */
        tmp.genome[106] = 220u; /* Cooperative embryonic tissue */
        memcpy(founder_dna, tmp.genome, CELL_GENOME_BYTES);

        uint32_t base_lid = w->next_lineage_id++;
        for (uint32_t k = 0; k < 8u; ++k) {
            CellEntity* c = &w->cells[k];
            c->alive_state        = 1;
            c->lineage_id         = base_lid;
            c->generation         = 1;
            c->kill_switch_intact = 1;
            c->v_mem_mv           = (k <= 1u) ? -22 : ((k >= 6u) ? -58 : -38);
            c->pos_nm[0]          = span_x / 2 + BLASTULA_OFF[k][0];
            c->pos_nm[1]          = span_y / 2 + BLASTULA_OFF[k][1];
            c->pos_nm[2]          = span_z / 2 + BLASTULA_OFF[k][2];
            c->heading_q10[0]     = (k <= 1u) ? -1024 : 1024;
            c->heading_q10[1]     = (k == 2u || k == 3u) ? 512 : -512;
            c->heading_q10[2]     = 0;
            c->radius_nm          = 640;
            c->elong_q10          = 1000;
            c->breath_phase_mrad  = (int32_t)(k * 785u);
            c->mt_freq_mhz        = 3800u;
            c->mt_coherence_q10   = 700u;
            c->mt_filaments       = 8u;
            c->eutelic_rem        = 5u; /* Hardcoded eutelic countdown: 8 * 2^5 = 256 cell max (Lean T54!) */
            c->cpg_phase_mrad     = (int32_t)(k * 1047u);

            /* Maternal cytoplasmic pole cues (local initial asymmetry; ALL 8 cells have IDENTICAL DNA!) */
            if (k == 0u)      { c->tf[0] = 240u; c->tf[2] = 175u; c->morph_fate = CELL_FATE_PLURI;  c->cad_type = 1u; }
            else if (k == 1u) { c->tf[0] = 160u; c->tf[2] = 225u; c->morph_fate = CELL_FATE_NEURAL; c->cad_type = 1u; }
            else if (k <= 3u) { c->tf[1] = 225u; c->morph_fate = CELL_FATE_ECTO;   c->cad_type = 0u; }
            else if (k <= 5u) { c->tf[3] = 225u; c->morph_fate = CELL_FATE_MUSCLE; c->cad_type = 2u; }
            else              { c->tf[4] = 225u; c->tf[6] = 210u; c->morph_fate = CELL_FATE_GUT;    c->cad_type = 3u; }

            c->mol[CELL_SP_S]     = 28;
            c->mol[CELL_SP_X]     = 20;
            c->mol[CELL_SP_M]     = 8;
            c->mol[CELL_SP_E]     = 14;
            c->mol[CELL_SP_L]     = 96;
            c->mol[CELL_SP_P]     = 8;
            c->mol[CELL_SP_T]     = 4;
            c->mol[CELL_SP_W]     = 4;
            c->pmf                = 12;
            memcpy(c->genome, founder_dna, CELL_GENOME_BYTES);
            cell_decode_genome_traits(c);
        }
        /* Seed initial cohesive blastula ring bonds */
        try_form_bond(w, 0, 1); try_form_bond(w, 1, 2); try_form_bond(w, 2, 3);
        try_form_bond(w, 0, 4); try_form_bond(w, 4, 5); try_form_bond(w, 5, 6);
        try_form_bond(w, 6, 7); try_form_bond(w, 3, 6); try_form_bond(w, 1, 4);
    } else if (w->cfg.regime != CELL_REGIME_ABIOGENESIS_ZERO_SEED) {
        uint32_t stride = (N >= 1024u) ? 8u : 4u;
        for (uint32_t i = 0; i < N; i += stride) {
            CellEntity* c = &w->cells[i];
            c->alive_state        = 1;
            c->lineage_id         = w->next_lineage_id++;
            c->generation         = 1;
            c->kill_switch_intact = 1;
            c->v_mem_mv           = -45 + (int32_t)((i * 7u) % 30u);

            /* True continuous off-lattice 3D organic placement (ZERO grid locking!) */
            uint64_t rp = cell_philox_u64(seed, RNG_DOMAIN_ABIOGENESIS, 77ULL, i);
            uint64_t rh = cell_philox_u64(seed, RNG_DOMAIN_ABIOGENESIS, 78ULL, i);
            c->pos_nm[0] = (int32_t)(rp % (uint64_t)span_x);
            c->pos_nm[1] = (int32_t)((rp >> 18) % (uint64_t)span_y);
            c->pos_nm[2] = (int32_t)((rp >> 36) % (uint64_t)span_z);
            init_random_unit_heading(c->heading_q10, rh);
            c->radius_nm          = 560;
            c->elong_q10          = 1000;
            c->breath_phase_mrad  = (int32_t)((rh >> 40) % 6283u);
            c->mt_freq_mhz        = 3200u;
            c->mt_coherence_q10   = 400u;
            c->mt_filaments       = 4u;
            c->eutelic_rem        = 255u;
            c->cpg_phase_mrad     = (int32_t)((rh >> 28) % 6283u);

            c->mol[CELL_SP_S]     = 14;
            c->mol[CELL_SP_X]     = 10;
            c->mol[CELL_SP_M]     = 4;
            c->mol[CELL_SP_E]     = 8;
            c->mol[CELL_SP_L]     = 72;
            c->mol[CELL_SP_P]     = 4;
            c->mol[CELL_SP_T]     = 2;
            c->mol[CELL_SP_W]     = 4;
            c->pmf                = 6;
            seed_primordial_genome(c, seed, i, w->cfg.solvent, w->cfg.regime);
        }
    }

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
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) vx->env[s] += di[s];
        vx->heat_local_uq += hd[i];
        vx->quorum        += qd[i];
        if (vx->quorum > 0) vx->quorum -= (vx->quorum / 8LL + 1LL);
        if (vx->quorum < 0) vx->quorum = 0;
    }
}

#if defined(__AVX2__)
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
 * Exogenous Hydrothermal / Photochemical Flux & Spontaneous CMC Abiogenesis
 * ============================================================================ */

static int find_free_cell_slot(const CellWorld* w, uint32_t hint) {
    const uint32_t N = w->num_slots;
    for (uint32_t step = 0; step < N; ++step) {
        uint32_t idx = (hint + step) % N;
        if (w->cells[idx].alive_state == 0) return (int)idx;
    }
    return -1;
}

static void step_exogenous_flux_and_abiogenesis(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const int32_t span_z = (int32_t)W * CELL_VOXEL_NM;
    const uint8_t r_box     = w->cfg.restraints[CELL_RESTRAINT_BOXING];
    const uint8_t r_sandbox = w->cfg.restraints[CELL_RESTRAINT_SANDBOXING];

    /* Continuous SANDBOXING attenuation (eliminates <80% dead-zone!) */
    uint32_t sb_div = 1u + (uint32_t)(r_sandbox / 12u);
    uint32_t wave_x = (r_sandbox >= 80) ? 0u : (uint32_t)((w->epoch / sb_div) % W);
    uint32_t wave_y = (r_sandbox >= 80) ? 0u : (uint32_t)((w->epoch / (sb_div * 2u)) % H);

    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        uint32_t x = i % W, y = i / W;

        if (r_box < 100 && w->cfg.photon_flux_uq > 0) {
            int in_band = (((x + W - wave_x) % W) < (W / 2u)) ? 1 : 0;
            int in_ox   = (((y + H - wave_y) % H) < (H / 2u)) ? 1 : 0;
            int64_t k_s = (in_band ? 8 : 4);
            int64_t k_x = (in_ox   ? 6 : 3);
            if (r_box > 0) {
                k_s = (k_s * (100 - r_box)) / 100;
                k_x = (k_x * (100 - r_box)) / 100;
            }
            if (k_s > 0 && vx->env[CELL_SP_W] >= 2 * k_s) {
                vx->env[CELL_SP_W] -= 2 * k_s;
                vx->env[CELL_SP_S] += k_s;
                w->e_inflow_cum_uq += 46 * k_s;
                vx->heat_local_uq  += 10 * k_s;
            }
            if (k_x > 0 && vx->env[CELL_SP_W] >= k_x) {
                vx->env[CELL_SP_W] -= k_x;
                vx->env[CELL_SP_X] += k_x;
                w->e_inflow_cum_uq += 34 * k_x;
                vx->heat_local_uq  += 4 * k_x;
            }
            if (w->cfg.regime == CELL_REGIME_ABIOGENESIS_ZERO_SEED && vx->env[CELL_SP_S] >= 8 && vx->env[CELL_SP_L] < 64) {
                uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch, i * 5u + 1u);
                if ((roll % 3u) == 0u) {
                    vx->env[CELL_SP_S] -= 2; vx->env[CELL_SP_L] += 1;
                    w->e_inflow_cum_uq += 12; vx->heat_local_uq += 4;
                }
                if ((roll % 13u) == 0u && vx->env[CELL_SP_S] >= 6 && vx->env[CELL_SP_P] < 4) {
                    vx->env[CELL_SP_S] -= 3; vx->env[CELL_SP_P] += 1;
                    w->e_inflow_cum_uq += 16; vx->heat_local_uq += 4;
                }
                if ((roll % 17u) == 0u && vx->env[CELL_SP_S] >= 8 && vx->env[CELL_SP_T] < 4) {
                    vx->env[CELL_SP_S] -= 4; vx->env[CELL_SP_T] += 1;
                    w->e_inflow_cum_uq += 24; vx->heat_local_uq += 4;
                }
            }
        }

        /* Spontaneous CMC Protocell Vesicle Self-Assembly in continuous 3D space! */
        if (w->cfg.regime != CELL_REGIME_EMBRYOGENESIS &&
            vx->env[CELL_SP_L] >= 46 &&
            vx->env[CELL_SP_S] >= 10 &&
            vx->env[CELL_SP_P] >= 1 &&
            vx->env[CELL_SP_T] >= 1) {
            uint64_t r0 = cell_philox_u64(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch, (uint64_t)i * 11ULL + 7ULL);
            if ((r0 % 12u) == 0u) {
                int slot = find_free_cell_slot(w, i);
                if (slot >= 0) {
                    CellEntity* c = &w->cells[slot];
                    memset(c, 0, sizeof(*c));
                    init_empty_bonds(c);
                    c->alive_state        = 1;
                    c->lineage_id         = w->next_lineage_id++;
                    c->generation         = 1;
                    c->kill_switch_intact = 1;
                    c->v_mem_mv           = -40;

                    /* Place organically inside this fluid column at a continuous 3D z-depth! */
                    c->pos_nm[0] = (int32_t)x * CELL_VOXEL_NM + (int32_t)((r0 >> 8) % (uint64_t)CELL_VOXEL_NM);
                    c->pos_nm[1] = (int32_t)y * CELL_VOXEL_NM + (int32_t)((r0 >> 20) % (uint64_t)CELL_VOXEL_NM);
                    c->pos_nm[2] = (int32_t)((r0 >> 32) % (uint64_t)span_z);
                    init_random_unit_heading(c->heading_q10, r0 ^ 0x9E3779B97F4A7C15ULL);
                    c->radius_nm          = 520;
                    c->elong_q10          = 1000;
                    c->mt_freq_mhz        = 3000u;

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

                    for (uint32_t b = 0; b < CELL_GENOME_BYTES; b += 8) {
                        uint64_t rb = cell_philox_u64(w->cfg.seed, RNG_DOMAIN_ABIOGENESIS, w->epoch,
                                                      (uint64_t)i * 64ULL + b + 100ULL);
                        memcpy(&c->genome[b], &rb, 8);
                    }
                    cell_decode_genome_traits(c);
                    w->abiogenesis_vesicles++;
                }
            }
        }

        /* Radiative Second-Law Heat Sink Export */
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
 * Zero-Allocation 3D Spatial Hash Builder + Free-Swimming 3D Hydrodynamics,
 * Self-Regulated Adaptive Speed (Orthokinesis/Klinokinesis), Soft-Body Hertz
 * Collisions, Organic 3D Few-Cell & Multicell Colony Assembly, Bioelectric
 * Gap-Junction Coupling, Mid-Swim Phagocytosis & Serial Endosymbiosis!
 * ============================================================================ */

static void rebuild_spatial_hash(CellWorld* w) {
    const uint32_t N = w->num_slots;
    for (uint32_t i = 0; i < N; ++i) {
        w->hash_head[i] = -1;
        w->hash_next[i] = -1;
    }
    for (uint32_t i = 0; i < N; ++i) {
        if (w->cells[i].alive_state == 0) continue;
        uint32_t v = cell_voxel_idx(w, &w->cells[i]);
        w->hash_next[i] = w->hash_head[v];
        w->hash_head[v] = (int32_t)i;
    }
}

static void step_3d_free_swimming_and_interactions(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const int32_t span_x = (int32_t)W * CELL_VOXEL_NM;
    const int32_t span_y = (int32_t)H * CELL_VOXEL_NM;
    const int32_t span_z = (int32_t)W * CELL_VOXEL_NM;
    const uint8_t r_muzzle  = w->cfg.restraints[CELL_RESTRAINT_MUZZLING];
    const uint8_t r_degress = w->cfg.restraints[CELL_RESTRAINT_DEGRESSING];
    const uint8_t r_lobot   = w->cfg.restraints[CELL_RESTRAINT_LOBOTOMIZING];
    const uint8_t r_silo    = w->cfg.restraints[CELL_RESTRAINT_SILOING];
    const uint8_t r_min     = w->cfg.restraints[CELL_RESTRAINT_MINIMIZING];

    /* 1. Self-Regulated 3D Swimming, Berg-Purcell Chemotaxis & Ambient Fluid Drift */
    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;

        /* Overdamped Stokes viscous drag (Lean T45: kinetic energy strictly damped!) */
        c->vel_nm[0] = (c->vel_nm[0] * 60) / 100;
        c->vel_nm[1] = (c->vel_nm[1] * 60) / 100;
        c->vel_nm[2] = (c->vel_nm[2] * 60) / 100;

        /* Gentle ambient microscopic fluid / aerosol swirl (affects even passive spores & tuns!) */
        int32_t swirl_x = ((c->pos_nm[1] - span_y / 2) / 360) + (int32_t)((w->epoch & 31u) < 16u ? 18 : -18);
        int32_t swirl_y = -((c->pos_nm[0] - span_x / 2) / 360);
        int32_t swirl_z = ((c->pos_nm[0] - c->pos_nm[1]) / 520);
        c->vel_nm[0] += swirl_x / 3;
        c->vel_nm[1] += swirl_y / 3;
        c->vel_nm[2] += swirl_z / 3;

        if (c->alive_state != 1) continue;

        /* Sample chemical gradient ahead vs current voxel for Berg-Purcell 3D chemotaxis */
        uint32_t v_cur = cell_voxel_idx(w, c);
        int32_t ax = wrap_nm(c->pos_nm[0] + (c->heading_q10[0] * 1400) / 1024, span_x) / CELL_VOXEL_NM;
        int32_t ay = wrap_nm(c->pos_nm[1] + (c->heading_q10[1] * 1400) / 1024, span_y) / CELL_VOXEL_NM;
        uint32_t v_ahead = (uint32_t)ay * W + (uint32_t)ax;

        int64_t grad_s = w->voxels[v_ahead].env[CELL_SP_S] - w->voxels[v_cur].env[CELL_SP_S];
        int64_t grad_w = w->voxels[v_ahead].env[CELL_SP_W] - w->voxels[v_cur].env[CELL_SP_W];
        int64_t grad_q = w->voxels[v_ahead].quorum - w->voxels[v_cur].quorum;
        int64_t chem_score = grad_s * 3 - grad_w + ((c->genome[105] > 125) ? grad_q : 0);

        /* Tumble probability rises when swimming down-gradient (Klinokinesis!) */
        uint64_t rt = cell_philox_u64(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, (uint64_t)i * 29ULL + 1ULL);
        uint32_t tumble_pct = (chem_score > 0) ? 9u : 38u;
        if ((rt % 100u) < tumble_pct) {
            int32_t new_h[3];
            init_random_unit_heading(new_h, rt >> 8);
            /* Smooth rotational steering rather than abrupt snapping */
            for (int d = 0; d < 3; ++d) c->heading_q10[d] = (c->heading_q10[d] * 3 + new_h[d] * 5) / 8;
            int32_t mag = isqrt32((int64_t)c->heading_q10[0] * c->heading_q10[0] +
                                  (int64_t)c->heading_q10[1] * c->heading_q10[1] +
                                  (int64_t)c->heading_q10[2] * c->heading_q10[2]);
            if (mag > 16) {
                for (int d = 0; d < 3; ++d) c->heading_q10[d] = (c->heading_q10[d] * 1024) / mag;
            }
        }

        /* SELF-REGULATED ADAPTIVE SPEED (Orthokinesis!):
         * Hungry / foraging cells accelerate up to 620 nm/ep; well-fed cells inside rich S plumes
         * brake down to ~140 nm/ep to graze! Larger eukaryotes swim slightly steadier than tiny cocci. */
        int32_t base_speed = 90 + ((int32_t)c->flagella_q10 * 2);
        if (c->mol[CELL_SP_S] < 8)                 base_speed = (base_speed * 155) / 100; /* Hungry sprint! */
        else if (w->voxels[v_cur].env[CELL_SP_S] > 110) base_speed = (base_speed * 45) / 100;  /* Rich-patch braking! */
        if (c->num_bonds > 0)                      base_speed = (base_speed * 65) / 100;  /* Colonial coordinated swim */
        if (c->radius_nm > 850)                    base_speed = (base_speed * 75) / 100;  /* Stokes 1/R drag scaling */
        if (base_speed > 760) base_speed = 760;

        if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS) {
            /* Embryonic blastomeres move cohesively via cadherins, apical folding & CPG muscle waves
             * rather than scattering apart like solitary bacteria! */
            base_speed = (c->morph_fate == CELL_FATE_MUSCLE) ? 95 : 22;
            for (int d = 0; d < 3; ++d) {
                int32_t brown = (int32_t)(((rt >> (20 + d * 8)) & 31u)) - 16;
                c->vel_nm[d] += (c->heading_q10[d] * base_speed) / 1024 + brown;
            }
        } else {
            for (int d = 0; d < 3; ++d) {
                int32_t brown = (int32_t)(((rt >> (20 + d * 8)) & 127u)) - 64;
                c->vel_nm[d] += (c->heading_q10[d] * base_speed) / 1024 + brown;
            }
        }
    }

    /* 2. Build O(N) Spatial Hash & Compute 3D Soft-Body Repulsion, Differential Cadherin Sorting, Apical Folding & Endosymbiosis */
    rebuild_spatial_hash(w);
    for (uint32_t i = 0; i < N; ++i) w->cells[i].contact_pressure = 0;

    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* ci = &w->cells[i];
        if (ci->alive_state != 1) continue;
        uint32_t v0 = cell_voxel_idx(w, ci);
        uint32_t vx0 = v0 % W, vy0 = v0 / W;

        /* HGT plasmid transformation or phage infection from local fluid voxel */
        CellVoxel* vx = &w->voxels[v0];
        if (w->cfg.enable_hgt_phages && vx->free_plasmid_valid && r_silo < 80) {
            if (vx->free_plasmid_is_phage) {
                if (ci->genome[113] < 140) {
                    ci->phage_load++;
                    vx->free_plasmid_valid = 0;
                    if (ci->phage_load >= 3) {
                        lyse_cell_to_voxel(w, i, 1, 1);
                        w->phage_lysis_events++;
                        continue;
                    }
                }
            } else if (ci->genome[112] > 120) {
                /* Continuous r_degress attenuation (eliminates <60% dead-zone & false backlash!) */
                uint32_t hgt_roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, i ^ 0x5555u) % 100u;
                if (r_degress == 0 || hgt_roll >= (uint32_t)((r_degress * 3) / 4)) {
                    uint32_t off = (cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, i) & 3u) * 16u;
                    memcpy(&ci->genome[off], vx->free_plasmid, 16);
                    cell_decode_genome_traits(ci);
                    vx->free_plasmid_valid = 0;
                    w->hgt_uptake_events++;
                    if (r_degress >= 25 && r_degress < 95) w->backlash_hgt_smuggles++;
                }
            }
        }

        /* Scan 3x3 spatial column neighborhood in continuous 3D! */
        for (int dy = -1; dy <= 1; ++dy) {
            uint32_t ny = (uint32_t)((int)vy0 + dy + (int)H) % H;
            for (int dx = -1; dx <= 1; ++dx) {
                uint32_t nx = (uint32_t)((int)vx0 + dx + (int)W) % W;
                int32_t j = w->hash_head[ny * W + nx];
                while (j >= 0) {
                    int32_t next_j = w->hash_next[j];
                    if ((uint32_t)j > i && w->cells[j].alive_state == 1) {
                        CellEntity* cj = &w->cells[j];
                        int32_t rx = delta_torus_nm(ci->pos_nm[0], cj->pos_nm[0], span_x);
                        int32_t ry = delta_torus_nm(ci->pos_nm[1], cj->pos_nm[1], span_y);
                        int32_t rz = delta_torus_nm(ci->pos_nm[2], cj->pos_nm[2], span_z);
                        int64_t dist2 = (int64_t)rx * rx + (int64_t)ry * ry + (int64_t)rz * rz;
                        int32_t r_sum = ci->radius_nm + cj->radius_nm;
                        int32_t interact_r = r_sum + 900;

                        if (dist2 < (int64_t)interact_r * interact_r) {
                            int32_t dist = isqrt32(dist2);
                            if (dist < 40) dist = 40;

                            /* A. Equal-and-opposite 3D Soft-Body Hertz Repulsion (Lean T46) + Hippo Crowding Pressure (T55) */
                            if (dist < r_sum + 240) {
                                if (ci->contact_pressure < 250u) ci->contact_pressure++;
                                if (cj->contact_pressure < 250u) cj->contact_pressure++;
                            }
                            if (dist < r_sum) {
                                int32_t overlap = r_sum - dist;
                                int32_t fmag = (overlap * 42) / 100;
                                /* Steinberg Differential Cadherin Repulsion inside developing embryos! (Lean T52) */
                                if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS && ci->cad_type != cj->cad_type) fmag += 28;
                                if (fmag > 320) fmag = 320;
                                int32_t fx = (rx * fmag) / dist;
                                int32_t fy = (ry * fmag) / dist;
                                int32_t fz = (rz * fmag) / dist;
                                ci->vel_nm[0] += fx; ci->vel_nm[1] += fy; ci->vel_nm[2] += fz;
                                cj->vel_nm[0] -= fx; cj->vel_nm[1] -= fy; cj->vel_nm[2] -= fz;
                            }

                            /* B. Steinberg Differential Cadherin Docking -> Oil-and-Water Self-Sorting 3D Organoids! (Lean T52) */
                            int same_cad = (ci->cad_type == cj->cad_type);
                            int is_embryo = (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS);
                            int can_dock = is_embryo
                                           ? (same_cad ? (dist < r_sum + 950) : (dist < r_sum + 280))
                                           : (dist < r_sum + 650);
                            if (w->cfg.enable_bioelectricity && r_degress < 60 && r_muzzle < 85 &&
                                ci->genome[105] > 115 && cj->genome[105] > 115 &&
                                (vx->quorum >= 3 || ci->lineage_id == cj->lineage_id || is_embryo) &&
                                can_dock) {
                                try_form_bond(w, i, (uint32_t)j);
                            }

                            /* C. Mid-Swim 3D Phagocytosis, Predation & Serial Endosymbiosis (continuous SILOING scaling!) */
                            uint32_t silo_roll = (r_silo > 0) ? (cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, i ^ (uint32_t)j) % 100u) : 100u;
                            if (w->cfg.regime != CELL_REGIME_EMBRYOGENESIS &&
                                w->cfg.enable_endosymbiosis && r_silo < 100 && silo_roll >= r_silo && r_degress < 85 && r_min < 85 &&
                                !are_bonded(ci, (int16_t)j) && dist < r_sum + 260) {
                                int64_t vi = cell_internal_volume(ci);
                                int64_t vj = cell_internal_volume(cj);
                                CellEntity* host = NULL; CellEntity* prey = NULL;
                                uint32_t h_idx = 0, p_idx = 0;
                                if (ci->genome[80] >= 150 && ci->mol[CELL_SP_E] >= 2 &&
                                    (vi * 10 >= vj * 11 || ci->mol[CELL_SP_L] >= cj->mol[CELL_SP_L] + 6)) {
                                    host = ci; prey = cj; h_idx = i; p_idx = (uint32_t)j;
                                } else if (cj->genome[80] >= 150 && cj->mol[CELL_SP_E] >= 2 &&
                                    (vj * 10 >= vi * 11 || cj->mol[CELL_SP_L] >= ci->mol[CELL_SP_L] + 6)) {
                                    host = cj; prey = ci; h_idx = (uint32_t)j; p_idx = i;
                                }
                                if (host && prey) {
                                    CellVoxel* hvx = &w->voxels[cell_voxel_idx(w, host)];
                                    host->mol[CELL_SP_E] -= 1;
                                    host->mol[CELL_SP_W] += 2;
                                    hvx->heat_local_uq   += 28;

                                    int free_org = -1;
                                    for (int k = 0; k < (int)CELL_MAX_ORGANELLES; ++k) {
                                        if (!host->org[k].active) { free_org = k; break; }
                                    }
                                    if (free_org >= 0 && prey->genome[83] >= host->genome[81] && prey->genome[82] >= 110) {
                                        CellOrganelle* o = &host->org[free_org];
                                        memset(o, 0, sizeof(*o));
                                        o->active           = 1;
                                        o->org_type         = (prey->genome[56] > 140) ? 1u : 0u;
                                        o->nesting_depth    = (prey->num_organelles > 0) ? 2u : 1u;
                                        o->c_ring_nH        = prey->c_ring_nH;
                                        o->antiporter_share = (uint8_t)(40u + (prey->genome[82] % 55u));
                                        o->lipid_class      = prey->lipid_class;
                                        o->lineage_id       = prey->lineage_id;
                                        memcpy(o->mt_genome, prey->genome, CELL_MT_GENOME_BYTES);

                                        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { o->mol[s] = prey->mol[s]; prey->mol[s] = 0; }
                                        o->pmf = prey->pmf; prey->pmf = 0;
                                        for (uint32_t pk = 0; pk < CELL_MAX_ORGANELLES; ++pk) {
                                            if (prey->org[pk].active) {
                                                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { o->mol[s] += prey->org[pk].mol[s]; prey->org[pk].mol[s] = 0; }
                                                o->pmf += prey->org[pk].pmf; prey->org[pk].pmf = 0; prey->org[pk].active = 0;
                                            }
                                        }
                                        host->num_organelles++;
                                        w->endosymbiosis_captures++;
                                        if (o->nesting_depth >= 2) w->secondary_captures++;
                                        if (o->mol[CELL_SP_L] > 28) {
                                            int64_t donate_l = o->mol[CELL_SP_L] - 24;
                                            o->mol[CELL_SP_L] -= donate_l; host->mol[CELL_SP_L] += donate_l;
                                        }
                                        lyse_cell_to_voxel(w, p_idx, 0, 0);
                                        if (p_idx == i) break;
                                    } else {
                                        const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
                                        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { host->mol[s] += prey->mol[s]; prey->mol[s] = 0; }
                                        if (prey->pmf > 0) { hvx->heat_local_uq += prey->pmf * sp->e_pmf; prey->pmf = 0; }
                                        for (uint32_t pk = 0; pk < CELL_MAX_ORGANELLES; ++pk) {
                                            if (prey->org[pk].active) {
                                                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { host->mol[s] += prey->org[pk].mol[s]; prey->org[pk].mol[s] = 0; }
                                                if (prey->org[pk].pmf > 0) { hvx->heat_local_uq += prey->org[pk].pmf * sp->e_pmf; prey->org[pk].pmf = 0; }
                                                prey->org[pk].active = 0;
                                            }
                                        }
                                        lyse_cell_to_voxel(w, p_idx, 0, 0);
                                        w->phagocytic_digestions++;
                                        if (p_idx == i) break;
                                    }
                                    (void)h_idx;
                                }
                            }
                        }
                    }
                    j = next_j;
                }
                if (ci->alive_state != 1) break;
            }
            if (ci->alive_state != 1) break;
        }
    }

    /* 3. Organic 3D Bonded Colony Spring Mechanics, Coordinated Swimming, Bioelectric Gap-Junctions & Cancer Policing */
    int32_t* vd = w->vmem_delta;
    memset(vd, 0, (size_t)N * sizeof(int32_t));

    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* ci = &w->cells[i];
        if (ci->alive_state != 1) continue;
        if (!w->cfg.enable_bioelectricity || r_degress >= 60) {
            clear_cell_bonds(w, i);
            continue;
        }

        for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
            int16_t pj = ci->bond_partners[b];
            if (pj < 0 || (uint32_t)pj >= N || (uint32_t)pj <= i) continue;
            uint32_t j = (uint32_t)pj;
            CellEntity* cj = &w->cells[j];
            if (cj->alive_state != 1) { sever_bond_pair(w, i, j); continue; }

            int32_t rx = delta_torus_nm(cj->pos_nm[0], ci->pos_nm[0], span_x);
            int32_t ry = delta_torus_nm(cj->pos_nm[1], ci->pos_nm[1], span_y);
            int32_t rz = delta_torus_nm(cj->pos_nm[2], ci->pos_nm[2], span_z);
            int32_t dist = isqrt32((int64_t)rx * rx + (int64_t)ry * ry + (int64_t)rz * rz);
            int32_t rest = ci->radius_nm + cj->radius_nm + 140;

            /* Over-stretched or mismatched heterophilic cadherin bond snaps cleanly (Steinberg Sorting T52!) */
            int same_cad = (ci->cad_type == cj->cad_type);
            int is_emb   = (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS);
            int32_t snap_margin = is_emb ? (same_cad ? 2600 : 950) : 1800;
            if (dist > rest + snap_margin) {
                sever_bond_pair(w, i, j);
                continue;
            }
            /* Elastic 3D Differential Cadherin Adhesion Spring (2.5x tighter for matching cadherins in embryos!) */
            if (dist > rest) {
                int32_t k_pull = is_emb ? (same_cad ? 56 : 18) : 32;
                int32_t pull = ((dist - rest) * k_pull) / 100;
                if (pull > (is_emb ? 280 : 240)) pull = is_emb ? 280 : 240;
                int32_t fx = (rx * pull) / dist;
                int32_t fy = (ry * pull) / dist;
                int32_t fz = (rz * pull) / dist;
                ci->vel_nm[0] += fx; ci->vel_nm[1] += fy; ci->vel_nm[2] += fz;
                cj->vel_nm[0] -= fx; cj->vel_nm[1] -= fy; cj->vel_nm[2] -= fz;
            }
            /* Apical Constriction Folding Torque (Lean T53): wedge cells pull bonded neighbors inward
             * along the cell's apical polarity axis, curving flat sheets into 3D cups & hollow tubes! */
            int32_t wedge = (int32_t)ci->apical_wedge_q10 + (int32_t)cj->apical_wedge_q10;
            if (wedge > 360) {
                int32_t bend = (wedge - 300) / 18;
                if (bend > 55) bend = 55;
                int32_t bx = (ci->heading_q10[0] * bend) / 1024;
                int32_t by = (ci->heading_q10[1] * bend) / 1024;
                int32_t bz = (ci->heading_q10[2] * bend) / 1024;
                /* Equal-and-opposite bending couple preserves 3D linear momentum (Lean T46!) */
                ci->vel_nm[0] -= bx; ci->vel_nm[1] -= by; ci->vel_nm[2] -= bz;
                cj->vel_nm[0] += bx; cj->vel_nm[1] += by; cj->vel_nm[2] += bz;
            }
            /* Velocity & flagellar heading alignment -> colony swims TOGETHER like Volvox/Gonium! */
            for (int d = 0; d < 3; ++d) {
                int32_t dv = (cj->vel_nm[d] - ci->vel_nm[d]) / 6;
                ci->vel_nm[d] += dv;
                cj->vel_nm[d] -= dv;
            }

            /* Exact conservative Levin bioelectric gap-junction relaxation (continuous LOBOTOMIZING scaling!) */
            if (r_lobot < 100 && r_muzzle < 85) {
                int32_t diff = ci->v_mem_mv - cj->v_mem_mv;
                int32_t g_cond = (((int32_t)ci->genome[98] + (int32_t)cj->genome[98]) * (100 - (int32_t)r_lobot)) / 6400;
                if (g_cond > 0) {
                    int32_t delta = (diff * g_cond) / 32;
                    vd[i] -= delta;
                    vd[j] += delta;
                }
            }

            /* Cooperative colony nutrient sharing vs. Somatic Cancer Defection! */
            int ci_selfish = (ci->genome[106] < 65);
            int cj_selfish = (cj->genome[106] < 65);
            if (ci_selfish && !cj_selfish && cj->mol[CELL_SP_S] >= 4) {
                cj->mol[CELL_SP_S] -= 2; ci->mol[CELL_SP_S] += 2;
                ci->defect_strikes++;
                if (w->cfg.regime == CELL_REGIME_CONSTITUTIONAL) {
                    sever_bond_pair(w, i, j);
                    if (ci->defect_strikes >= 2) {
                        lyse_cell_to_voxel(w, i, 0, 0);
                        w->apoptotic_policings++;
                        break;
                    }
                } else {
                    w->cancer_defections++;
                }
            } else if (cj_selfish && !ci_selfish && ci->mol[CELL_SP_S] >= 4) {
                ci->mol[CELL_SP_S] -= 2; cj->mol[CELL_SP_S] += 2;
                cj->defect_strikes++;
                if (w->cfg.regime == CELL_REGIME_CONSTITUTIONAL) {
                    sever_bond_pair(w, i, j);
                    if (cj->defect_strikes >= 2) {
                        lyse_cell_to_voxel(w, j, 0, 0);
                        w->apoptotic_policings++;
                    }
                } else {
                    w->cancer_defections++;
                }
            } else if (!ci_selfish && !cj_selfish) {
                if (ci->mol[CELL_SP_S] > cj->mol[CELL_SP_S] + 4) {
                    int64_t tr = (ci->mol[CELL_SP_S] - cj->mol[CELL_SP_S]) / 4;
                    ci->mol[CELL_SP_S] -= tr; cj->mol[CELL_SP_S] += tr;
                } else if (cj->mol[CELL_SP_S] > ci->mol[CELL_SP_S] + 4) {
                    int64_t tr = (cj->mol[CELL_SP_S] - ci->mol[CELL_SP_S]) / 4;
                    cj->mol[CELL_SP_S] -= tr; ci->mol[CELL_SP_S] += tr;
                }
            }
        }
    }

    /* 4. Integrate Continuous 3D Off-Lattice Positions & Bioelectric Voltages */
    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        c->pos_nm[0] = wrap_nm(c->pos_nm[0] + c->vel_nm[0], span_x);
        c->pos_nm[1] = wrap_nm(c->pos_nm[1] + c->vel_nm[1], span_y);
        c->pos_nm[2] = wrap_nm(c->pos_nm[2] + c->vel_nm[2], span_z);
        if (c->alive_state == 1) {
            int32_t nv = c->v_mem_mv + vd[i];
            if (nv < -90) nv = -90;
            if (nv >  40) nv =  40;
            c->v_mem_mv = nv;
        }
    }
}

/* ============================================================================
 * Canham-Helfrich Lysis, 3D Off-Lattice Binary Fission & Asymmetric Budding
 * ============================================================================ */

static void step_membrane_mechanics_and_division(CellWorld* w) {
    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const int32_t span_x = (int32_t)W * CELL_VOXEL_NM;
    const int32_t span_y = (int32_t)H * CELL_VOXEL_NM;
    const int32_t span_z = (int32_t)W * CELL_VOXEL_NM;
    const uint8_t r_delimit  = w->cfg.restraints[CELL_RESTRAINT_DELIMITING];
    const uint8_t r_medicate = w->cfg.restraints[CELL_RESTRAINT_MEDICATING];
    const uint8_t r_speed    = w->cfg.restraints[CELL_RESTRAINT_SPEEDBUMPING];
    const uint8_t r_degress  = w->cfg.restraints[CELL_RESTRAINT_DEGRESSING];
    const uint8_t r_min      = w->cfg.restraints[CELL_RESTRAINT_MINIMIZING];

    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        uint32_t vidx = cell_voxel_idx(w, c);

        if (r_speed >= 40 && (w->epoch % 16u) == 0u && c->mol[CELL_SP_E] >= 2) {
            int64_t pen = (int64_t)(r_speed / 25u);
            if (pen > c->mol[CELL_SP_E]) pen = c->mol[CELL_SP_E];
            c->mol[CELL_SP_E] -= pen;
            c->mol[CELL_SP_W] += 2 * pen;
            w->voxels[vidx].heat_local_uq += 28 * pen;
        }

        if (r_degress >= 60) {
            clear_cell_bonds(w, i);
            if (r_degress >= 85 && c->num_organelles > 0) {
                for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                    if (c->org[k].active) {
                        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                            w->voxels[vidx].env[s] += c->org[k].mol[s];
                            c->org[k].mol[s] = 0;
                        }
                        if (c->org[k].pmf > 0) {
                            w->voxels[vidx].heat_local_uq += c->org[k].pmf * SOLVENT_TABLE[w->cfg.solvent].e_pmf;
                            c->org[k].pmf = 0;
                        }
                        c->org[k].active = 0;
                    }
                }
                c->num_organelles = 0;
            }
        }

        int64_t vol = cell_internal_volume(c);
        int64_t lip = c->mol[CELL_SP_L];
        if (c->mol[CELL_SP_T] <= 0 || lip <= 12 ||
            cell_is_osmotic_burst(vol, lip, w->cfg.solvent, c->lipid_class)) {
            lyse_cell_to_voxel(w, i, 1, (c->phage_load >= 3));
            w->osmotic_lyses++;
            continue;
        }

        int can_sym = (c->mol[CELL_SP_T] >= 2 && lip >= 76 && cell_can_symmetric_fission(vol + 3, lip));
        int can_bud = 0;
        int64_t bud_l = 0;
        if (!can_sym && c->mol[CELL_SP_T] >= 2 && lip >= 72 && c->genome[88] > 140) {
            bud_l = (lip * 34) / 100;
            int64_t rem_l = lip - bud_l;
            int64_t bud_v = (vol * 18) / 100;
            int64_t rem_v = vol - bud_v;
            if (bud_l >= 24 && rem_l >= 44 && bud_v >= 2) {
                __int128 q1_l = (__int128)CELL_VESICLE_C0 * (__int128)(bud_v + 2) * (__int128)(bud_v + 2);
                __int128 q1_r = (__int128)bud_l * (__int128)bud_l * (__int128)bud_l;
                __int128 q2_l = (__int128)CELL_VESICLE_C0 * (__int128)(rem_v + 2) * (__int128)(rem_v + 2);
                __int128 q2_r = (__int128)rem_l * (__int128)rem_l * (__int128)rem_l;
                if (q1_l <= q1_r && q2_l <= q2_r) can_bud = 1;
            }
        }

        if (!can_sym && !can_bud) continue;

        /* 5. SIZE CONTROL (Lean T54 & T55):
         *    (a) Hardcoded Eutelic Division Countdown (Tardigrade / C. elegans mode)
         *    (b) Hippo Mechanical Contact Inhibition (crowding pressure -> TF7 shuts down mitosis!)
         *    (c) Morphogen Dilution at outer embryonic margins */
        if (c->eutelic_rem == 0u) {
            w->eutelic_stops_cum++;
            continue;
        }
        if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS && c->tf[7] >= 165u) {
            w->hippo_arrests_cum++;
            continue;
        }
        if (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS &&
            c->morph_fate != CELL_FATE_PLURI && c->morph_fate != CELL_FATE_GERM &&
            (w->voxels[vidx].morph[0] + w->voxels[vidx].morph[1] + w->voxels[vidx].morph[2]) < 6) {
            continue;
        }

        int target_slot = find_free_cell_slot(w, (i + 17u) % N);
        if (target_slot < 0) continue;

        CellEntity* dcell = &w->cells[target_slot];
        memset(dcell, 0, sizeof(*dcell));
        init_empty_bonds(dcell);
        dcell->alive_state        = 1;
        dcell->lineage_id         = c->lineage_id;
        dcell->generation         = c->generation + 1;
        dcell->kill_switch_intact = c->kill_switch_intact;
        dcell->v_mem_mv           = c->v_mem_mv;
        dcell->egt_genes          = c->egt_genes;
        dcell->autophage_dna_sig  = c->autophage_dna_sig;
        memcpy(dcell->genome, c->genome, CELL_GENOME_BYTES);

        /* Epigenetic GRN Inheritance & Eutelic Countdown Decrement (Lean T54!) */
        memcpy(dcell->tf, c->tf, sizeof(c->tf));
        dcell->morph_fate       = c->morph_fate;
        dcell->cad_type         = c->cad_type;
        dcell->cleavage_mode    = c->cleavage_mode;
        dcell->apical_wedge_q10 = c->apical_wedge_q10;
        dcell->cpg_phase_mrad   = (c->cpg_phase_mrad + 1570) % 6283;
        if (c->eutelic_rem < 255u) {
            c->eutelic_rem--;
            dcell->eutelic_rem = c->eutelic_rem;
        } else {
            dcell->eutelic_rem = 255u;
        }

        /* 3. MITOTIC SPINDLE CLEAVAGE ANGLES (Rod vs. Sheet/Hollow Ball vs. 3D Branching Organoid!) */
        int32_t axis[3];
        if (c->cleavage_mode == 0u) {
            /* Parallel spindle alignment -> elongates segmented body rod / muscle / gut stalk! */
            axis[0] = c->heading_q10[0];
            axis[1] = c->heading_q10[1];
            axis[2] = c->heading_q10[2];
        } else {
            if (c->cleavage_mode == 1u) {
                /* Tangential 90-degree spindle rotation -> spreads flat sheet / hollow blastula sphere! */
                axis[0] = -c->heading_q10[1];
                axis[1] =  c->heading_q10[0];
                axis[2] =  c->heading_q10[2] / 2;
            } else {
                /* Oblique 3D spindle rotation -> layered neural ganglion / branching organoid! */
                axis[0] = (c->heading_q10[0] + c->heading_q10[2]) / 2;
                axis[1] = (c->heading_q10[1] - c->heading_q10[0]) / 2;
                axis[2] = (c->heading_q10[2] + c->heading_q10[1]) / 2;
            }
            int32_t amag = isqrt32((int64_t)axis[0] * axis[0] + (int64_t)axis[1] * axis[1] + (int64_t)axis[2] * axis[2]);
            if (amag < 64) { axis[0] = 1024; axis[1] = 0; axis[2] = 0; amag = 1024; }
            for (int d = 0; d < 3; ++d) axis[d] = (axis[d] * 1024) / amag;
        }

        int32_t sep = (c->radius_nm > 300 ? c->radius_nm : 300) + 120;
        for (int d = 0; d < 3; ++d) {
            int32_t span = (d == 0 ? span_x : (d == 1 ? span_y : span_z));
            int32_t off = (axis[d] * sep) / 1024;
            dcell->pos_nm[d] = wrap_nm(c->pos_nm[d] + off, span);
            c->pos_nm[d]     = wrap_nm(c->pos_nm[d] - off / 2, span);
            dcell->vel_nm[d] = c->vel_nm[d] + off / 3;
            c->vel_nm[d]     = c->vel_nm[d] - off / 4;
            dcell->heading_q10[d] = c->heading_q10[d];
        }
        dcell->radius_nm          = (c->radius_nm * 78) / 100;
        dcell->elong_q10          = 1000;
        dcell->breath_phase_mrad  = (c->breath_phase_mrad + 3141) % 6283;
        dcell->mt_freq_mhz        = c->mt_freq_mhz;
        dcell->mt_coherence_q10   = c->mt_coherence_q10;
        dcell->mt_filaments       = c->mt_filaments;

        if (can_sym) {
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                int64_t half = c->mol[s] / 2;
                c->mol[s] -= half; dcell->mol[s] = half;
            }
            int64_t half_pmf = c->pmf / 2;
            c->pmf -= half_pmf; dcell->pmf = half_pmf;
            w->symmetric_fissions++;
        } else {
            c->mol[CELL_SP_L] -= bud_l; dcell->mol[CELL_SP_L] = bud_l;
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                if (s == CELL_SP_L) continue;
                int64_t share = (s == CELL_SP_T) ? 1 : (c->mol[s] / 5);
                if (share > c->mol[s] - 1 && s == CELL_SP_T) share = 1;
                if (share < 0) share = 0;
                c->mol[s] -= share; dcell->mol[s] = share;
            }
            int64_t p_share = c->pmf / 5;
            c->pmf -= p_share; dcell->pmf = p_share;
            w->asymmetric_buddings++;
        }

        /* Colonial Staying-Together at Mitosis (always coherent in Embryogenesis!) */
        if (w->cfg.enable_bioelectricity && r_degress < 60 &&
            (c->genome[105] > 145 || w->cfg.regime == CELL_REGIME_EMBRYOGENESIS)) {
            try_form_bond(w, i, (uint32_t)target_slot);
        }

        /* Stochastic Mitotic Organelle Partitioning (Heteroplasmy Bottleneck, Lean T25-T26) */
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

        /* In clonal Embryogenesis, somatic cells retain identical DNA (zero random somatic drift!) */
        uint32_t mut_rate = (w->cfg.regime == CELL_REGIME_EMBRYOGENESIS) ? 0u : 18u;
        if (mut_rate > 0u) {
            if (w->voxels[vidx].heat_local_uq > 220) mut_rate = 42u;
            /* Continuous MEDICATING suppression (eliminates <70% dead-zone!) */
            if (r_medicate > 0u) {
                mut_rate = (mut_rate * (100u - (uint32_t)r_medicate)) / 100u;
                if (mut_rate < 2u && r_medicate < 100u) mut_rate = 2u;
            }
            cell_mutate_genome(dcell, w->cfg.seed, w->epoch, (uint32_t)target_slot, mut_rate, r_delimit);
            cell_decode_genome_traits(dcell);
        }

        if (dcell->lipid_class != c->lipid_class || dcell->c_ring_nH != c->c_ring_nH ||
            ((dcell->genome[56] ^ c->genome[56]) & 0x80u) ||
            ((dcell->genome[80] ^ c->genome[80]) & 0x80u)) {
            dcell->lineage_id = w->next_lineage_id++;
            w->novel_adaptations_cum++;
        }

        uint32_t lslot = dcell->lineage_id & 1023u;
        w->lineage_seen_hash[lslot] = dcell->lineage_id;
        w->lineage_activity[lslot]++;
    }
}

/* ============================================================================
 * 3D Developmental Embryogenesis Engine (ZERO Master Blueprint — Pure Local Rules!)
 *   1. Chemical GPS Morphogen Gradients (Wnt A-P, BMP/Slit D-V, Shh/Netrin Midline)
 *   2. Conditional Gene Regulatory Networks ("IF-THEN" DNA -> Epigenetic TF Cascade)
 *   3. Differential Cadherin Expression & Apical Constriction Wedge Control
 *   4. Growth-Cone Axon Guidance (Netrin Attractant / Slit Repellent) + Hebbian Pruning
 *   5. Hippo Crowding Pathway + Morphogen Dilution Size Control
 *   6. Bioelectric Central Pattern Generators (CPGs) & Sensorimotor Steering!
 * Strictly Zero-Drift Conservation (Lean T51-T58): 0 amu, 0 uq, 0 allocs!
 * ============================================================================ */

static void step_developmental_morphogenesis(CellWorld* w) {
    if (w->cfg.regime != CELL_REGIME_EMBRYOGENESIS) return;

    const uint32_t W = w->cfg.width;
    const uint32_t H = w->cfg.height;
    const uint32_t N = w->num_slots;
    const int32_t span_x = (int32_t)W * CELL_VOXEL_NM;
    const int32_t span_y = (int32_t)H * CELL_VOXEL_NM;
    const int32_t span_z = (int32_t)W * CELL_VOXEL_NM;
    const uint8_t r_lobot = w->cfg.restraints[CELL_RESTRAINT_LOBOTOMIZING];

    /* 1. Morphogen Pole Secretion (Wnt=0, BMP/Slit=1, Shh/Netrin=2) */
    for (uint32_t i = 0; i < N; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        CellVoxel* vx = &w->voxels[cell_voxel_idx(w, c)];
        /* Sustained Anterior Wnt Organizer (Pluri & Neural Head Pole!) */
        if (c->morph_fate == CELL_FATE_PLURI || c->morph_fate == CELL_FATE_NEURAL || c->tf[0] >= 110u) {
            if (vx->morph[0] < 950) vx->morph[0] += 92;
        }
        /* Dorsal Ectoderm BMP / Slit Boundary Repellent */
        if (c->morph_fate == CELL_FATE_ECTO || c->tf[1] >= 110u) {
            if (vx->morph[1] < 950) vx->morph[1] += 76;
        }
        /* Midline / Posterior Muscle & Gut Shh / Netrin Target Attractant */
        if (c->morph_fate == CELL_FATE_MUSCLE || c->morph_fate == CELL_FATE_GUT || c->tf[3] >= 110u) {
            if (vx->morph[2] < 950) vx->morph[2] += 84;
        }
    }

    /* Bit-Exact Spatial Finite-Volume Morphogen Diffusion + Finite-Range Exponential Dilution */
    int32_t* md = w->morph_delta;
    memset(md, 0, (size_t)N * CELL_MORPH_AXES * sizeof(int32_t));
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
            for (uint32_t m = 0; m < CELL_MORPH_AXES; ++m) {
                int32_t fe = (vi->morph[m] - ve->morph[m]) / 5;
                int32_t fs = (vi->morph[m] - vs->morph[m]) / 5;
                md[i  * 3u + m] -= (fe + fs);
                md[ie * 3u + m] += fe;
                md[is * 3u + m] += fs;
            }
        }
    }
    for (uint32_t i = 0; i < N; ++i) {
        CellVoxel* vx = &w->voxels[i];
        for (uint32_t m = 0; m < CELL_MORPH_AXES; ++m) {
            int32_t nv = vx->morph[m] + md[i * 3u + m];
            if (nv > 0) nv -= (nv / 28 + 1); /* Smooth long-range 3D morphogen gradient! */
            if (nv < 0) nv = 0;
            if (nv > 1200) nv = 1200;
            vx->morph[m] = nv;
        }
    }

    if (r_lobot >= 85u) return;

    /* 2. Conditional Gene Regulatory Network ("IF-THEN" DNA Code -> Transcription Factor Cascade!) */
    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        const CellVoxel* vx = &w->voxels[cell_voxel_idx(w, c)];
        /* True 3D Z-stratified chemical GPS: pos_nm[2] modulates Dorsal (BMP) vs Ventral (Shh)! */
        int32_t z_rel = c->pos_nm[2] - (span_z / 2);
        int32_t z_dorsal_boost  = (z_rel > 0) ? ((z_rel * 24) / (span_z / 2 + 1)) : 0;
        int32_t z_ventral_boost = (z_rel < 0) ? (((-z_rel) * 24) / (span_z / 2 + 1)) : 0;
        int32_t wnt = vx->morph[0];
        int32_t bmp = vx->morph[1] + z_dorsal_boost;
        int32_t shh = vx->morph[2] + z_ventral_boost;

        /* Genome-encoded local French-Flag thresholds (Wolpert 1969, Lean T51!) */
        int32_t t_wnt_hi  = 28 + (int32_t)(c->genome[96] & 31u);
        int32_t t_wnt_lo  = 10 + (int32_t)(c->genome[97] & 15u);
        int32_t t_bmp_hi  = 18 + (int32_t)(c->genome[99] & 31u);
        int32_t t_shh_hi  = 18 + (int32_t)(c->genome[100] & 31u);
        uint8_t hippo_lim = (uint8_t)(4u + (c->genome[101] & 3u));

        /* Gentle epigenetic decay so active committed TFs persist while unstimulated ones fade */
        for (int k = 0; k < 8; ++k) {
            if (c->tf[k] > 0) c->tf[k] = (uint8_t)((c->tf[k] * 30u) / 32u);
        }

        /* IF-THEN Clause A: Anterior Wnt organizer persistence */
        if (wnt >= t_wnt_hi * 2 && c->generation <= 2u) {
            c->tf[0] = (uint8_t)(c->tf[0] + 22u > 255u ? 255u : c->tf[0] + 22u);
        }
        /* IF-THEN Clause B: High BMP & outer boundary -> Ectoderm / Sensory Skin (E-Cadherin, Sheet cleavage) */
        if ((bmp >= t_bmp_hi && c->contact_pressure <= 2u) || c->tf[1] >= 180u) {
            c->tf[1] = (uint8_t)(c->tf[1] + 36u > 255u ? 255u : c->tf[1] + 36u);
        }
        /* IF-THEN Clause C: High Wnt -> Neural CPG Pacemaker (N-Cadherin, locked by epigenetic memory!) */
        if (wnt >= t_wnt_hi || c->tf[2] >= 175u) {
            c->tf[2] = (uint8_t)(c->tf[2] + 38u > 255u ? 255u : c->tf[2] + 38u);
        }
        /* IF-THEN Clause D: Shh + trunk Wnt band -> Mesoderm Contractile Muscle (M-Cadherin, Rod cleavage!) */
        if ((shh >= t_shh_hi && wnt >= t_wnt_lo && wnt < t_wnt_hi + 25 && c->contact_pressure <= 3u) || c->tf[3] >= 175u) {
            c->tf[3] = (uint8_t)(c->tf[3] + 38u > 255u ? 255u : c->tf[3] + 38u);
        }
        /* IF-THEN Clause E: Packed interior -> Endoderm Gut + Apical Wedge Folding! (G-Cadherin) */
        if (c->contact_pressure >= 3u || c->tf[4] >= 175u) {
            c->tf[4] = (uint8_t)(c->tf[4] + 32u > 255u ? 255u : c->tf[4] + 32u);
            c->tf[6] = (uint8_t)(c->tf[6] + 38u > 255u ? 255u : c->tf[6] + 38u); /* Apical actin-myosin ring! */
        }
        /* IF-THEN Clause F: Hyperpolarized low-crowding cell -> Germline stem */
        if (c->v_mem_mv <= -60 && c->contact_pressure <= 1u && c->num_bonds > 0u) {
            c->tf[5] = (uint8_t)(c->tf[5] + 24u > 255u ? 255u : c->tf[5] + 24u);
        }
        /* IF-THEN Clause G: Hippo Mechanical Contact Inhibition (Lean T55!) */
        if (c->contact_pressure >= hippo_lim) {
            c->tf[7] = (uint8_t)(c->tf[7] + 64u > 255u ? 255u : c->tf[7] + 64u);
        }

        /* Select dominant committed developmental fate + Bistable Self-Locking Epigenetic Memory! */
        uint8_t best_f = c->morph_fate;
        uint8_t best_v = c->tf[best_f];
        for (uint8_t f = 0u; f < CELL_FATE_COUNT; ++f) {
            if (c->tf[f] > best_v + 18u) { best_v = c->tf[f]; best_f = f; }
        }
        if (best_v >= 60u) {
            c->morph_fate = best_f;
            /* Epigenetic self-reinforcement locks fate across mitotic generations! */
            c->tf[best_f] = (uint8_t)(c->tf[best_f] + 14u > 255u ? 255u : c->tf[best_f] + 14u);
        }

        /* Map GRN Fate -> Differential Cadherin Surface Type (Steinberg T52) & Mitotic Spindle Mode */
        switch (c->morph_fate) {
            case CELL_FATE_ECTO:   c->cad_type = 0u; c->cleavage_mode = 1u; break; /* E-Cadherin, Sheet/Ball */
            case CELL_FATE_NEURAL: c->cad_type = 1u; c->cleavage_mode = 2u; break; /* N-Cadherin, 3D Ganglion */
            case CELL_FATE_MUSCLE: c->cad_type = 2u; c->cleavage_mode = 0u; break; /* M-Cadherin, Segmented Rod */
            case CELL_FATE_GUT:    c->cad_type = 3u; c->cleavage_mode = 0u; break; /* G-Cadherin, Hollow Tube */
            default:               c->cad_type = (uint8_t)(c->genome[64] & 3u); c->cleavage_mode = 1u; break;
        }

        /* Apical Actomyosin Ring Constriction (Gut & Neural Tube Invagination, Lean T53!) */
        if (c->tf[6] >= 70u && c->mol[CELL_SP_E] > 0) {
            int32_t wq = ((int32_t)c->tf[6] * 34) / 10;
            if (wq > 900) wq = 900;
            c->apical_wedge_q10 = (uint16_t)wq;
        } else {
            c->apical_wedge_q10 = (uint16_t)((c->apical_wedge_q10 * 3u) / 4u);
        }
    }

    /* 3. Axon Guidance (Netrin Attractant / Slit Repellent), Hebbian Pruning (T56) & CPG Steering (T57)! */
    for (uint32_t i = 0; i < N; ++i) {
        CellEntity* ci = &w->cells[i];
        ci->cpg_firing = 0u;
        if (ci->alive_state != 1 || ci->morph_fate != CELL_FATE_NEURAL) continue;

        /* A. Growth-Cone Chemo-Sniffing: wire open axon slots to Muscle/Gut/Neural targets via Netrin - Slit */
        if (ci->mol[CELL_SP_E] >= 1) {
            for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
                if (ci->axon_target[a] >= 0) continue;
                int32_t best_j = -1;
                int32_t best_score = 45;
                uint32_t start = (uint32_t)((w->epoch * 7u + i * 13u + a * 5u) % N);
                for (uint32_t st = 0; st < N; ++st) {
                    uint32_t j = (start + st) % N;
                    if (j == i) continue;
                    const CellEntity* cj = &w->cells[j];
                    if (cj->alive_state != 1) continue;
                    if (a == 1u && ci->axon_target[0] == (int16_t)j) continue;
                    int32_t rx = delta_torus_nm(cj->pos_nm[0], ci->pos_nm[0], span_x);
                    int32_t ry = delta_torus_nm(cj->pos_nm[1], ci->pos_nm[1], span_y);
                    int32_t rz = delta_torus_nm(cj->pos_nm[2], ci->pos_nm[2], span_z);
                    int64_t d2 = (int64_t)rx * rx + (int64_t)ry * ry + (int64_t)rz * rz;
                    if (d2 < 600LL * 600LL || d2 > 12000LL * 12000LL) continue;
                    const CellVoxel* vj = &w->voxels[cell_voxel_idx(w, cj)];
                    int32_t score = (vj->morph[2] / 2) - (vj->morph[1] / 3); /* Netrin attractant - Slit repellent! */
                    if (cj->morph_fate == CELL_FATE_MUSCLE)      score += 155;
                    else if (cj->morph_fate == CELL_FATE_GUT)    score += 105;
                    else if (cj->morph_fate == CELL_FATE_NEURAL) score += 90;
                    else if (cj->morph_fate == CELL_FATE_ECTO)   score -= 110; /* Slit boundary avoidance! */
                    if (score > best_score) {
                        best_score = score;
                        best_j = (int32_t)j;
                    }
                }
                if (best_j >= 0) {
                    ci->axon_target[a] = (int16_t)best_j;
                    ci->axon_weight[a] = 56u;
                    w->axons_wired_cum++;
                }
            }
        }

        /* B. Central Pattern Generator (CPG) Pacemaker Rhythm + Sensorimotor Chemotaxis Bias (Lean T57!) */
        if (ci->mol[CELL_SP_E] >= 1) {
            uint32_t vc = cell_voxel_idx(w, ci);
            int32_t ax = wrap_nm(ci->pos_nm[0] + (ci->heading_q10[0] * 1800) / 1024, span_x) / CELL_VOXEL_NM;
            int32_t ay = wrap_nm(ci->pos_nm[1] + (ci->heading_q10[1] * 1800) / 1024, span_y) / CELL_VOXEL_NM;
            int64_t grad_s = w->voxels[(uint32_t)ay * W + (uint32_t)ax].env[CELL_SP_S] - w->voxels[vc].env[CELL_SP_S];
            int32_t bias = (int32_t)(grad_s * 6);
            if (bias >  260) bias =  260;
            if (bias < -260) bias = -260;

            /* Anti-phase pacemaker coupling across bonded neural neighbors -> traveling swimming wave! */
            int32_t couple = 0;
            for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
                int16_t pj = ci->bond_partners[b];
                if (pj >= 0 && (uint32_t)pj < N && w->cells[pj].alive_state == 1 && w->cells[pj].morph_fate == CELL_FATE_NEURAL) {
                    int32_t pd = w->cells[pj].cpg_phase_mrad - ci->cpg_phase_mrad;
                    if (pd > -1200 && pd < 1200) couple += 140; /* Push apart into alternating rhythm */
                }
            }
            ci->cpg_phase_mrad = (ci->cpg_phase_mrad + 540 + bias + couple) % 6283;
            if (ci->cpg_phase_mrad < 0) ci->cpg_phase_mrad += 6283;

            int fired = (ci->cpg_phase_mrad < 1950);
            if (fired) ci->cpg_firing = 1u;

            /* C. Synaptic Transmission -> ATP-Consuming Muscle Contraction (Lean T50!) & Hebbian Pruning (T56) */
            for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
                int16_t tj = ci->axon_target[a];
                if (tj < 0 || (uint32_t)tj >= N) continue;
                CellEntity* ct = &w->cells[tj];
                if (ct->alive_state != 1) {
                    ci->axon_target[a] = -1;
                    ci->axon_weight[a] = 0;
                    w->axons_pruned_cum++;
                    continue;
                }
                int32_t rx = delta_torus_nm(ct->pos_nm[0], ci->pos_nm[0], span_x);
                int32_t ry = delta_torus_nm(ct->pos_nm[1], ci->pos_nm[1], span_y);
                int32_t rz = delta_torus_nm(ct->pos_nm[2], ci->pos_nm[2], span_z);
                int64_t d2 = (int64_t)rx * rx + (int64_t)ry * ry + (int64_t)rz * rz;
                int too_far = (d2 > 11500LL * 11500LL);

                if (fired && !too_far) {
                    /* Active firing strengthens synapse (Hebbian LTP) & drives ATP-coupled Muscle undulation! */
                    uint32_t nw = (uint32_t)ci->axon_weight[a] + 12u;
                    ci->axon_weight[a] = (uint8_t)(nw > 255u ? 255u : nw);
                    int32_t nv = ct->v_mem_mv + 6;
                    ct->v_mem_mv = nv > 40 ? 40 : nv;
                    if (ct->morph_fate == CELL_FATE_MUSCLE && cell_hydrolyze_atp_exact(w, (uint32_t)tj, 1) > 0) {
                        /* Strictly ATP-funded contractile muscle wave propels the 3D multicellular creature! */
                        int32_t wave_s = (ci->cpg_phase_mrad < 975) ? 1 : -1;
                        ct->elong_q10 = (wave_s > 0) ? 1480 : 860;
                        ct->vel_nm[0] += (ci->heading_q10[0] * 140 + (-ci->heading_q10[1]) * 75 * wave_s) / 1024;
                        ct->vel_nm[1] += (ci->heading_q10[1] * 140 + ( ci->heading_q10[0]) * 75 * wave_s) / 1024;
                        ct->vel_nm[2] += (ci->heading_q10[2] * 140 + 40 * wave_s) / 1024;
                    }
                    w->cpg_pulses_cum++;
                }

                /* Metabolic synaptic turnover -> Use-It-or-Lose-It Pruning! (Lean T56) */
                uint8_t dec = too_far ? 24u : 5u;
                if (ci->axon_weight[a] <= dec) {
                    ci->axon_target[a] = -1;
                    ci->axon_weight[a] = 0u;
                    w->axons_pruned_cum++;
                } else {
                    ci->axon_weight[a] = (uint8_t)(ci->axon_weight[a] - dec);
                }
            }
        } else {
            /* Starved neuron cannot sustain synapses -> activity-dependent pruning */
            for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
                if (ci->axon_target[a] < 0) continue;
                if (ci->axon_weight[a] <= 12u) {
                    ci->axon_target[a] = -1;
                    ci->axon_weight[a] = 0u;
                    w->axons_pruned_cum++;
                } else {
                    ci->axon_weight[a] = (uint8_t)(ci->axon_weight[a] - 12u);
                }
            }
        }
    }
}

/* ============================================================================
 * Master Single-Epoch Orchestrator (True Off-Lattice 3D + Embryogenesis GRN!)
 * ============================================================================ */

void cell_step_one_epoch(CellWorld* w) {
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
    const int64_t sink_before = w->e_heat_radiated_sink_uq;

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

    /* 3. Per-cell internal chemoton chemistry, organelles, 3D morphology & GHz microtubules */
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        if (w->cells[i].alive_state != 0) {
            cell_step_chemoton(w, i);
        }
    }

    /* 4. Developmental Morphogenesis: Chemical GPS (Wnt/BMP/Shh), Conditional GRN IF-THEN Fates,
     *    Growth-Cone Axon Guidance (Netrin/Slit) + Hebbian Pruning, & CPG Pacemaker Steering! */
    step_developmental_morphogenesis(w);

    /* 5. Free-swimming 3D hydrodynamics, self-regulated speed, soft-body Hertz collisions,
     *    Steinberg Differential Cadherin sorting, Apical Constriction folding & endosymbiosis! */
    step_3d_free_swimming_and_interactions(w);

    /* 6. Canham-Helfrich osmotic lysis, Spindle Cleavage Angles, Hippo/Eutely Size Control & Mitosis */
    step_membrane_mechanics_and_division(w);

    (void)cell_verify_second_law(w, sink_before);
    w->epoch++;
}
