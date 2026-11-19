#include "cellulogonic_internal.h"

/* ============================================================================
 * Exact Integer Stoichiometry, Canham-Helfrich Membrane Geometry,
 * Mitchell Chemiosmosis & Organelle Bioenergetics (Matches Lean 4 T01-T36)
 * ============================================================================ */

int64_t cell_species_mass(const int64_t mol[CELL_SPECIES_COUNT]) {
    int64_t m = 0;
    for (int i = 0; i < CELL_SPECIES_COUNT; ++i) {
        m += CELL_MASS_W[i] * mol[i];
    }
    return m;
}

int64_t cell_species_energy(const int64_t mol[CELL_SPECIES_COUNT]) {
    int64_t e = 0;
    for (int i = 0; i < CELL_SPECIES_COUNT; ++i) {
        e += CELL_MU_POT[i] * mol[i];
    }
    return e;
}

int64_t cell_organelle_volume(const CellOrganelle* o) {
    if (!o || !o->active) return 0;
    /* Internal osmolyte count excluding membrane lipid L */
    return o->mol[CELL_SP_S] + o->mol[CELL_SP_X] + o->mol[CELL_SP_M] +
           o->mol[CELL_SP_E] + o->mol[CELL_SP_P] + o->mol[CELL_SP_T] +
           o->mol[CELL_SP_W] + 1;
}

int64_t cell_internal_volume(const CellEntity* c) {
    if (!c || c->alive_state == 0) return 0;
    int64_t v = c->mol[CELL_SP_S] + c->mol[CELL_SP_X] + c->mol[CELL_SP_M] +
                c->mol[CELL_SP_E] + c->mol[CELL_SP_P] + c->mol[CELL_SP_T] +
                c->mol[CELL_SP_W];
    for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
        if (c->org[k].active) {
            /* Compact folded cristae organelle occupies 1/3 displaced osmotic volume */
            v += 2 + (cell_organelle_volume(&c->org[k]) / 3);
        }
    }
    return v > 0 ? v : 1;
}

double cell_reduced_Q(int64_t vol, int64_t lipid) {
    if (lipid <= 0) return 999.0;
    double dv = (double)vol;
    double dl = (double)lipid;
    return (100.0 * dv * dv) / (dl * dl * dl);
}

/* Exact Lean Theorem T12 check: 2 * C0 * V^2 <= L^3 (no division, 128-bit safe!) */
int cell_can_symmetric_fission(int64_t vol, int64_t lipid) {
    if (vol <= 0 || lipid < 24) return 0;
    __int128 lhs = (__int128)2 * (__int128)CELL_VESICLE_C0 * (__int128)vol * (__int128)vol;
    __int128 rhs = (__int128)lipid * (__int128)lipid * (__int128)lipid;
    return lhs <= rhs;
}

/* Exact Osmotic Lysis threshold: C0 * V^2 * 100 > burst_eff * L^3 */
int cell_is_osmotic_burst(int64_t vol, int64_t lipid, uint32_t solvent, uint8_t lipid_class) {
    if (lipid <= 12) return 1;
    const SolventParams* sp = &SOLVENT_TABLE[solvent < CELL_SOLVENT_COUNT ? solvent : 0];
    int64_t burst = sp->burst_num;
    /* Solvent-lipid mismatch weakens osmotic burst resilience by 18% */
    if (lipid_class != sp->optimal_lipid) {
        burst = (burst * 82) / 100;
    }
    __int128 lhs = (__int128)CELL_VESICLE_C0 * (__int128)vol * (__int128)vol * (__int128)100;
    __int128 rhs = (__int128)burst * (__int128)lipid * (__int128)lipid * (__int128)lipid;
    return lhs > rhs;
}

/* Decode physical phenotype parameters from the cell's 128-byte operon genome */
void cell_decode_genome_traits(CellEntity* c) {
    /* Locus 8 (bytes 64..71): membrane lipid chemistry class (0..3) */
    c->lipid_class = (uint8_t)(c->genome[64] & 3u);
    /* Locus 2 (bytes 16..23): evolved F1F0 ATP-synthase c-ring stoichiometry (2..5) */
    c->c_ring_nH   = (uint8_t)(2u + (c->genome[16] & 3u));
}

/* Domain-isolated stochastic genome mutation (respecting DELIMITING restraint) */
void cell_mutate_genome(CellEntity* c, uint64_t seed, uint64_t epoch, uint32_t slot,
                        uint32_t rate_pct, uint8_t delimit_pct) {
    if (rate_pct == 0) return;
    /* Locus 15 byte 120: proofreading fidelity lowers mutation error rate */
    uint32_t proof = c->genome[120];
    uint32_t eff_rate = rate_pct;
    if (proof > 160 && eff_rate > 2) eff_rate = (eff_rate * 60) / 100;
    if (delimit_pct >= 90) {
        eff_rate = eff_rate / 10;
        if (eff_rate == 0) return;
    }
    uint64_t r0 = cell_philox_u64(seed, RNG_DOMAIN_CELL_MUT, epoch, (uint64_t)slot * 7ULL + 1ULL);
    if ((r0 % 100u) >= eff_rate) return;

    uint32_t n_flips = 1u + (uint32_t)((r0 >> 16) % 3u);
    if (delimit_pct >= 50) n_flips = 1u;

    for (uint32_t f = 0; f < n_flips; ++f) {
        uint64_t rf = cell_philox_u64(seed, RNG_DOMAIN_CELL_MUT, epoch, (uint64_t)slot * 7ULL + 2ULL + f);
        uint32_t byte_idx = (uint32_t)(rf & 127u);
        /* DELIMITING clamps structural transitions (lipid_class byte 64 & c-ring byte 16) */
        if (delimit_pct >= 50 && (byte_idx == 64 || byte_idx == 16 || byte_idx == 80)) {
            continue;
        }
        uint8_t bit = (uint8_t)(1u << ((rf >> 8) & 7u));
        c->genome[byte_idx] ^= bit;
    }
    cell_decode_genome_traits(c);
}

/* ============================================================================
 * Conservation-Guaranteed Elementary Reaction Engine (Zero Residual!)
 * Applies k firings of a stoichiometrically balanced reaction to (mol, pmf),
 * booking exact inflow into w->e_inflow_cum_uq and exact heat into voxel->heat_local_uq.
 * ============================================================================ */

static inline int64_t apply_exact_rxn(
    CellWorld* w,
    CellVoxel* vx,
    int64_t mol[CELL_SPECIES_COUNT],
    int64_t* pmf,
    const int64_t dmol[CELL_SPECIES_COUNT],
    int64_t dpmf,
    int64_t e_inflow_per_rxn,
    int64_t k_req)
{
    if (k_req <= 0) return 0;
    int64_t k = k_req;
    for (int i = 0; i < CELL_SPECIES_COUNT; ++i) {
        if (dmol[i] < 0) {
            int64_t need = -dmol[i];
            int64_t avail = mol[i] / need;
            if (avail < k) k = avail;
        }
    }
    if (dpmf < 0) {
        int64_t need = -dpmf;
        int64_t avail = (*pmf) / need;
        if (avail < k) k = avail;
    }
    if (k <= 0) return 0;

    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
    int64_t chem_delta = 0;
    for (int i = 0; i < CELL_SPECIES_COUNT; ++i) {
        chem_delta += CELL_MU_POT[i] * dmol[i];
    }
    int64_t pmf_delta_e = dpmf * sp->e_pmf;
    int64_t heat_per_rxn = e_inflow_per_rxn - chem_delta - pmf_delta_e;

    /* STRICT SECOND-LAW GATE: reject any endergonic uncoupled reaction! */
    if (heat_per_rxn < 0) {
        return 0;
    }

    for (int i = 0; i < CELL_SPECIES_COUNT; ++i) {
        mol[i] += k * dmol[i];
    }
    *pmf += k * dpmf;
    w->e_inflow_cum_uq += k * e_inflow_per_rxn;
    vx->heat_local_uq  += k * heat_per_rxn;
    w->total_reactions += (uint64_t)k;
    return k;
}

/* Reaction stoichiometric templates (matching Lean 4 StoichiometryThermo.lean) */
static const int64_t RXN_R1_CATAB[8]   = { -1, -1,  1,  0,  0,  0,  0,  0 };
static const int64_t RXN_R2_PUMP[8]    = {  0,  0, -1,  0,  0,  0,  0,  3 };
static const int64_t RXN_R3_ATPSYN[8]  = {  0,  0,  0,  1,  0,  0,  0, -2 };
static const int64_t RXN_R4_FERM[8]    = { -2,  0,  0,  1,  0,  0,  0,  2 };
static const int64_t RXN_R5_LIPID[8]   = { -2,  0,  0, -1,  1,  0,  0,  2 };
static const int64_t RXN_R6_PROT[8]    = { -3,  0,  0, -1,  0,  1,  0,  2 };
static const int64_t RXN_R7_TMPL[8]    = { -4,  0,  0, -1,  0,  0,  1,  2 };
static const int64_t RXN_R8_HYD[8]     = {  0,  0,  0, -1,  0,  0,  0,  2 };
static const int64_t RXN_R9_LDEC[8]    = {  0,  0,  0,  0, -1,  0,  0,  4 };
static const int64_t RXN_R10_PDEC[8]   = {  0,  0,  0,  0,  0, -1,  0,  6 };
static const int64_t RXN_R11_TDEC[8]   = {  0,  0,  0,  0,  0,  0, -1,  8 };
static const int64_t RXN_R12_LEAK[8]   = {  0,  0,  0,  0,  0,  0,  0,  0 };
static const int64_t RXN_R13A_SALVL[8] = {  1,  0,  0,  0, -1,  0,  0,  2 };
static const int64_t RXN_R13B_SALVP[8] = {  2,  0,  0,  0,  0, -1,  0,  2 };
static const int64_t RXN_R14A_PHOTS[8] = {  1,  0,  0,  0,  0,  0,  0, -2 };
static const int64_t RXN_R14B_PHOTX[8] = {  0,  1,  0,  0,  0,  0,  0, -1 };

/* ============================================================================
 * Step One Living Cell's Internal Chemoton + Nested Organelles + Transporters
 * ============================================================================ */

void cell_step_chemoton(CellWorld* w, uint32_t idx) {
    CellEntity* c = &w->cells[idx];
    CellVoxel* vx = &w->voxels[idx];
    if (c->alive_state == 0) return;

    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
    const uint8_t r_nerf     = w->cfg.restraints[CELL_RESTRAINT_NERFING];
    const uint8_t r_slow     = w->cfg.restraints[CELL_RESTRAINT_SLOWING];
    const uint8_t r_box      = w->cfg.restraints[CELL_RESTRAINT_BOXING];
    const uint8_t r_lobot    = w->cfg.restraints[CELL_RESTRAINT_LOBOTOMIZING];
    const uint8_t r_degrade  = w->cfg.restraints[CELL_RESTRAINT_DEGRADING];
    const uint8_t r_minimize = w->cfg.restraints[CELL_RESTRAINT_MINIMIZING];
    const uint8_t r_speed    = w->cfg.restraints[CELL_RESTRAINT_SPEEDBUMPING];

    c->age_epochs++;
    w->total_cell_steps++;

    /* --- 0. Cryptobiotic Endospore (2) & Tardigrade TDP Bioglass Tun (3) Handling --- */
    if (c->alive_state == 3) {
        /* Vitrified TDP/CAHS amorphous bioglass (Lean T42): near-zero metabolic turnover,
         * 10x+ decay suppression until explicit rehydration! */
        return;
    }
    if (c->alive_state == 2) {
        /* Spores have 10x lower decay, wake up when substrate S & oxidant X recover */
        if (vx->env[CELL_SP_S] >= 12 && (r_speed < 40 || (w->epoch % 16) > 3)) {
            c->alive_state = 1;
        } else {
            if ((w->epoch & 31u) == (idx & 31u) && c->mol[CELL_SP_E] > 0) {
                apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R8_HYD, 0, 0, 1);
            }
            return;
        }
    }

    /* Check voluntary spore dormancy (evolved Locus 11 OR escape-backlash under SPEEDBUMPING) */
    uint8_t spore_gene = c->genome[90];
    if (r_speed >= 30 && r_speed < 95 && (w->epoch % 16) < 3 && spore_gene > 96) {
        c->alive_state = 2;
        w->backlash_dormancy_spores++;
        return;
    }
    if (c->mol[CELL_SP_S] == 0 && c->mol[CELL_SP_E] <= 1 && vx->env[CELL_SP_S] < 4 && spore_gene > 170) {
        c->alive_state = 2;
        return;
    }

    /* --- 1. DEGRADING Kill-Switch & Backlash Deletion --- */
    if (r_degrade >= 30) {
        if (c->kill_switch_intact) {
            /* Under partial/strong degrading, cells with active repair locus can delete kill switch! */
            if (c->genome[121] > 140 && r_degrade < 98) {
                uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_CELL_MUT, w->epoch, idx * 13u + 5u);
                if ((roll % 100u) < 28u) {
                    c->kill_switch_intact = 0;
                    w->backlash_killswitch_deletions++;
                }
            }
            if (c->kill_switch_intact) {
                /* Kill-switch triggers ATP-wasting apoptosis leak */
                int64_t drain = (int64_t)(r_degrade / 20u);
                apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R8_HYD, 0, 0, drain);
                if (r_degrade >= 85 && c->mol[CELL_SP_E] == 0) {
                    apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R9_LDEC, 0, 0, 4);
                }
            }
        }
    }

    /* --- 2. Bioelectric Epigenetic Transcription Modulation (Levin Morphogenesis) --- */
    /* Voltage v_mem modulates operon gain without hard-coding caste identities! */
    int32_t v_mod_anabolic = 100;
    int32_t v_mod_catabolic = 100;
    if (w->cfg.enable_bioelectricity && r_lobot < 80) {
        if (c->v_mem_mv < -55) {
            c->diff_role = 2; /* Hyperpolarized -> Biosynthetic / Germline bias */
            v_mod_anabolic = 135;
            v_mod_catabolic = 85;
        } else if (c->v_mem_mv > -25) {
            c->diff_role = 1; /* Depolarized -> Motile / Foraging / Phagocytic Somatic bias */
            v_mod_anabolic = 75;
            v_mod_catabolic = 135;
        } else if (c->bond_mask != 0) {
            c->diff_role = 3; /* Intermediate polarized tissue stroma */
            v_mod_anabolic = 110;
            v_mod_catabolic = 110;
        } else {
            c->diff_role = 0;
        }
    } else {
        c->diff_role = 0;
    }

    /* --- 3. Selective Transmembrane Transport + MscL Mechanosensitive Osmoregulation (Lean T36 & T43) --- */
    int64_t enzyme_cap = 1 + c->mol[CELL_SP_P];
    int64_t lipid_area = c->mol[CELL_SP_L];
    if (lipid_area < 1) lipid_area = 1;

    /* Waste efflux (protonephridial / MscL osmoregulation): keep minimal 4 W for ADP/Pi recycling */
    if (c->mol[CELL_SP_W] > 6) {
        int64_t dump_w = c->mol[CELL_SP_W] - 4;
        c->mol[CELL_SP_W] -= dump_w;
        vx->env[CELL_SP_W] += dump_w;
    }

    /* Check current osmotic strain Q: gate active import down if turgor is already high (MscL channel!) */
    int64_t cur_vol = cell_internal_volume(c);
    double cur_Q = cell_reduced_Q(cur_vol, lipid_area);

    /* Permease rates from Locus 9 (bytes 72..79) */
    int64_t import_s_cap = 2 + ((int64_t)c->genome[72] * (1 + lipid_area / 18)) / 96;
    int64_t import_x_cap = 2 + ((int64_t)c->genome[73] * (1 + lipid_area / 22)) / 96;
    if (import_s_cap > 12) import_s_cap = 12;
    if (import_x_cap > 10) import_x_cap = 10;
    if (r_minimize >= 40) {
        import_s_cap = (import_s_cap * (100 - r_minimize)) / 100;
        if (import_s_cap < 1 && r_minimize < 95) import_s_cap = 1;
    }
    if (c->diff_role == 1) {
        import_s_cap = (import_s_cap * 125) / 100;
        import_x_cap = (import_x_cap * 125) / 100;
    }
    /* MscL mechanosensitive stretch-gating prevents suicidal hyper-osmotic uptake */
    if (cur_Q > 0.72) {
        import_s_cap = (c->mol[CELL_SP_S] < 4) ? 2 : 0;
        import_x_cap = (c->mol[CELL_SP_X] < 2) ? 1 : 0;
        /* Emergency MscL pressure-relief vent of excess cytosolic waste W */
        if (c->mol[CELL_SP_W] > 2) {
            int64_t vent = c->mol[CELL_SP_W] - 2;
            c->mol[CELL_SP_W] -= vent;
            vx->env[CELL_SP_W] += vent;
        }
    } else if (cur_Q > 0.55) {
        import_s_cap = import_s_cap / 2 + 1;
        import_x_cap = import_x_cap / 2 + 1;
    }
    /* Cap internal substrate buffer so cells metabolize rather than hoard to bursting */
    int64_t max_s_buf = 18 + (int64_t)c->num_organelles * 14;
    int64_t max_x_buf = 14 + (int64_t)c->num_organelles * 10;
    if (c->mol[CELL_SP_S] >= max_s_buf) import_s_cap = 0;
    else if (c->mol[CELL_SP_S] + import_s_cap > max_s_buf) import_s_cap = max_s_buf - c->mol[CELL_SP_S];
    if (c->mol[CELL_SP_X] >= max_x_buf) import_x_cap = 0;
    else if (c->mol[CELL_SP_X] + import_x_cap > max_x_buf) import_x_cap = max_x_buf - c->mol[CELL_SP_X];

    /* Conserved uptake: S and X from voxel env into cell cytoplasm */
    int64_t take_s = vx->env[CELL_SP_S] < import_s_cap ? vx->env[CELL_SP_S] : import_s_cap;
    if (take_s > 0) { vx->env[CELL_SP_S] -= take_s; c->mol[CELL_SP_S] += take_s; }

    int64_t take_x = vx->env[CELL_SP_X] < import_x_cap ? vx->env[CELL_SP_X] : import_x_cap;
    if (take_x > 0) { vx->env[CELL_SP_X] -= take_x; c->mol[CELL_SP_X] += take_x; }

    /* Autotrophs also import waste W as carbon/oxidant feedstock for R14 */
    uint8_t photo_gene = c->genome[56];
    if (photo_gene > 110 && r_box < 90 && cur_Q < 0.72 && c->mol[CELL_SP_W] < 8) {
        int64_t take_w = vx->env[CELL_SP_W] < 6 ? vx->env[CELL_SP_W] : 6;
        if (take_w > 0) { vx->env[CELL_SP_W] -= take_w; c->mol[CELL_SP_W] += take_w; }
    }

    /* --- 4. Autotrophic Carbon & Oxidant Fixation (R14a / R14b -> Great Oxidation Event!) --- */
    if (r_box < 95 && w->cfg.photon_flux_uq > 0 && photo_gene > 125) {
        int64_t max_fix = 1 + ((int64_t)(photo_gene - 125) * enzyme_cap) / 256;
        if (max_fix > 4) max_fix = 4;
        int64_t flux_cap = (int64_t)w->cfg.photon_flux_uq / 46;
        if (max_fix > flux_cap) max_fix = flux_cap;
        if (r_box > 0) max_fix = (max_fix * (100 - r_box)) / 100;
        if (max_fix > 0) {
            apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R14A_PHOTS, 0, 46, max_fix);
            /* Oxygenic branch (byte 57) regenerates oxidant X and exports surplus to ambient world! */
            if (c->genome[57] > 120) {
                int64_t made_x = apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R14B_PHOTX, 0, 34, max_fix);
                if (made_x > 0 && c->mol[CELL_SP_X] > 6) {
                    int64_t ex_x = c->mol[CELL_SP_X] - 4;
                    c->mol[CELL_SP_X] -= ex_x;
                    vx->env[CELL_SP_X] += ex_x;
                }
            }
        }
    }

    /* --- 5. Host Outer-Membrane Bioenergetics (Prokaryotic 1/R Surface-Area Cap! Lean T20) --- */
    int64_t nerf_scale = 100 - (r_nerf * 85) / 100;
    /* Outer plasma membrane respiratory proton pump capacity strictly scales with outer area L! */
    int64_t outer_pump_cap = (lipid_area * 3) / 40;
    if (outer_pump_cap < 1) outer_pump_cap = 1;
    if (outer_pump_cap > 8) outer_pump_cap = 8; /* Physical outer-membrane protein packing ceiling */
    outer_pump_cap = (outer_pump_cap * nerf_scale * v_mod_catabolic) / 10000;
    if (outer_pump_cap < 1 && r_nerf < 95) outer_pump_cap = 1;

    /* R1: Catabolism (S + X -> M) */
    int64_t r1_rate = 1 + ((int64_t)c->genome[0] * enzyme_cap) / 192;
    if (r1_rate > outer_pump_cap + 2) r1_rate = outer_pump_cap + 2;
    apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R1_CATAB, 0, 0, r1_rate);

    /* R2: Chemiosmotic Proton Pumping across Outer Membrane (M -> 3W + r2_pmf_yield PMF) */
    int64_t r2_rate = ((int64_t)c->genome[8] * outer_pump_cap) / 160;
    if (r2_rate < 1 && c->genome[8] > 40) r2_rate = 1;
    if (r2_rate > outer_pump_cap) r2_rate = outer_pump_cap;
    apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R2_PUMP, sp->r2_pmf_yield, 0, r2_rate);

    /* R3: Rotary F1F0 ATP Synthase (2W + nH PMF -> E) — enforces exact Lean T28-T30 c-ring law! */
    if (c->c_ring_nH >= sp->min_cring && c->pmf >= c->c_ring_nH) {
        int64_t r3_cap = 1 + ((int64_t)c->genome[17] * outer_pump_cap * 2) / 160;
        if (r_nerf > 30 && r_nerf < 95 && c->genome[24] > 140) {
            /* NERFING backlash: cells bypass throttled ATP synthase via fermentation upregulation */
            w->backlash_metabolic_bypasses++;
        }
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R3_ATPSYN, -(int64_t)c->c_ring_nH, 0, r3_cap);
    }

    /* R4: Anaerobic Fermentation (2S -> E + 2W) — inefficient fallback when oxidant X or c-ring fails */
    if (c->mol[CELL_SP_E] < 6 || c->genome[24] > 145) {
        int64_t r4_cap = 1 + ((int64_t)c->genome[24] * enzyme_cap) / 256;
        if (r4_cap > 3) r4_cap = 3;
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R4_FERM, 0, 0, r4_cap);
    }

    /* --- 6. Nested Endosymbiont Organelles (Shatters Prokaryotic 1/R Ceiling! Lean T22-T27) --- */
    for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
        CellOrganelle* o = &c->org[k];
        if (!o->active) continue;

        /* Host feeds substrate S / oxidant X / waste W into organelle compartment (0 mass loss) */
        if (o->org_type == 0) {
            /* Proto-Mitochondrion: imports S and X from host cytoplasm */
            int64_t feed_s = c->mol[CELL_SP_S] > 4 ? (c->mol[CELL_SP_S] / 3) : 0;
            if (feed_s > 4) feed_s = 4;
            int64_t feed_x = c->mol[CELL_SP_X] > 3 ? (c->mol[CELL_SP_X] / 3) : 0;
            if (feed_x > 4) feed_x = 4;
            c->mol[CELL_SP_S] -= feed_s; o->mol[CELL_SP_S] += feed_s;
            c->mol[CELL_SP_X] -= feed_x; o->mol[CELL_SP_X] += feed_x;

            /* Organelle runs its OWN chemiosmotic cristae respiration! */
            int64_t org_cap = 1 + (o->mol[CELL_SP_L] / 14);
            if (o->nesting_depth >= 2) org_cap += 2; /* Secondary chimera double-cristae boost */
            apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R1_CATAB, 0, 0, org_cap);
            apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R2_PUMP, sp->r2_pmf_yield, 0, org_cap);
            if (o->c_ring_nH >= sp->min_cring) {
                apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R3_ATPSYN, -(int64_t)o->c_ring_nH, 0, org_cap * 2);
            }
        } else {
            /* Proto-Chloroplast: imports host waste W and fixes S + X using photons! */
            int64_t feed_w = c->mol[CELL_SP_W] > 2 ? (c->mol[CELL_SP_W] / 2) : 0;
            if (feed_w > 6) feed_w = 6;
            c->mol[CELL_SP_W] -= feed_w; o->mol[CELL_SP_W] += feed_w;
            if (r_box < 95 && w->cfg.photon_flux_uq > 0) {
                int64_t p_cap = 2 + (o->nesting_depth >= 2 ? 1 : 0);
                apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R14A_PHOTS, 0, 46, p_cap);
                apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R14B_PHOTX, 0, 34, p_cap);
                /* Export fixed S & X back to host (exact mass conservation) */
                if (o->mol[CELL_SP_S] > 2) {
                    int64_t ex_s = o->mol[CELL_SP_S] - 2;
                    o->mol[CELL_SP_S] -= ex_s; c->mol[CELL_SP_S] += ex_s;
                }
                if (o->mol[CELL_SP_X] > 2) {
                    int64_t ex_x = o->mol[CELL_SP_X] - 2;
                    o->mol[CELL_SP_X] -= ex_x; c->mol[CELL_SP_X] += ex_x;
                }
            }
        }

        /* Organelle ATP/ADP Antiporter Export to Host (Cooperative vs Selfish Cheater!) */
        if (o->antiporter_share > 0 && o->mol[CELL_SP_E] > 1) {
            int64_t share = (o->mol[CELL_SP_E] * (int64_t)o->antiporter_share) / 100;
            if (share < 1) share = 1;
            if (share > o->mol[CELL_SP_E] - 1) share = o->mol[CELL_SP_E] - 1;
            if (share > 0) {
                o->mol[CELL_SP_E] -= share;
                c->mol[CELL_SP_E] += share;
                /* Return ADP/Pi equivalent (2 W) from host to organelle if available */
                int64_t ret_w = c->mol[CELL_SP_W] < share * 2 ? c->mol[CELL_SP_W] : share * 2;
                c->mol[CELL_SP_W] -= ret_w;
                o->mol[CELL_SP_W] += ret_w;
            }
        }

        /* Organelle self-maintenance & lipid/template growth */
        if (o->mol[CELL_SP_E] >= 1 && o->mol[CELL_SP_S] >= 2 && o->mol[CELL_SP_L] < 48) {
            apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R5_LIPID, 0, 0, 1);
        }
        /* Organelle maintenance cost reduced by EGT (Lean Theorem T27!) */
        if (o->egt_transferred < 4 && (w->epoch & 3u) == (k & 3u)) {
            apply_exact_rxn(w, vx, o->mol, &o->pmf, RXN_R8_HYD, 0, 0, 1);
        }

        /* Occasional Endosymbiotic Gene Transfer (EGT) from organelle to host nucleus */
        if (o->egt_transferred < 8 && (w->epoch % 25u) == ((idx + k) % 25u)) {
            uint32_t r_egt = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_PHAGO_HGT, w->epoch, idx * 17u + k);
            if ((r_egt % 100u) < 12u) {
                o->egt_transferred++;
                c->egt_genes++;
                w->egt_transfer_events++;
            }
        }

        /* Occasional selfish organelle mutation (turns off antiporter!) */
        if (o->antiporter_share > 0 && (w->epoch % 31u) == ((idx + k) % 31u)) {
            uint32_t r_ch = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_CELL_MUT, w->epoch, idx * 23u + k);
            if ((r_ch % 200u) < 3u) {
                o->antiporter_share = 0; /* Selfish cheater mitochondrion! */
            }
        }

        /* Host Mitophagy Policing (Locus 10 byte 84): culls selfish non-exporting organelles! */
        if (o->antiporter_share == 0 && c->genome[84] > 115) {
            /* Dissolve organelle PMF into local heat & recycle all organelle mass into host! */
            if (o->pmf > 0) {
                vx->heat_local_uq += o->pmf * sp->e_pmf;
                o->pmf = 0;
            }
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                c->mol[s] += o->mol[s];
                o->mol[s] = 0;
            }
            o->active = 0;
            if (c->num_organelles > 0) c->num_organelles--;
            w->mitophagy_events++;
            continue;
        }

        /* Organelle internal binary replication inside host if plenty of lipid & free slot! */
        if (o->mol[CELL_SP_L] >= 36 && o->mol[CELL_SP_E] >= 2 && c->num_organelles < CELL_MAX_ORGANELLES) {
            int free_slot = -1;
            for (int j = 0; j < (int)CELL_MAX_ORGANELLES; ++j) {
                if (!c->org[j].active) { free_slot = j; break; }
            }
            if (free_slot >= 0) {
                CellOrganelle* d = &c->org[free_slot];
                memset(d, 0, sizeof(*d));
                d->active           = 1;
                d->org_type         = o->org_type;
                d->nesting_depth    = o->nesting_depth;
                d->c_ring_nH        = o->c_ring_nH;
                d->antiporter_share = o->antiporter_share;
                d->lipid_class      = o->lipid_class;
                d->egt_transferred  = o->egt_transferred;
                d->lineage_id       = o->lineage_id;
                memcpy(d->mt_genome, o->mt_genome, CELL_MT_GENOME_BYTES);
                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                    int64_t half = o->mol[s] / 2;
                    o->mol[s] -= half;
                    d->mol[s]  = half;
                }
                int64_t half_p = o->pmf / 2;
                o->pmf -= half_p;
                d->pmf  = half_p;
                c->num_organelles++;
            }
        }

        /* Return excess organelle waste W to host */
        if (o->mol[CELL_SP_W] > 6) {
            int64_t rw = o->mol[CELL_SP_W] - 4;
            o->mol[CELL_SP_W] -= rw;
            c->mol[CELL_SP_W] += rw;
        }
    }

    /* --- 7. Anabolic Macromolecule Synthesis (Strictly ATP-Coupled, Lean T04!) --- */
    int64_t slow_scale = 100 - (r_slow * 80) / 100;
    int64_t ana_boost  = (slow_scale * v_mod_anabolic) / 10000;

    /* R5: Lipid bilayer synthesis (2S + E -> L + 2W) — drives reduced volume Q down toward fission! */
    int64_t r5_req = 1 + ((int64_t)c->genome[32] * enzyme_cap) / 128;
    if (cur_Q > 0.55) r5_req += 2; /* Osmotic stretch accelerates bilayer lipid repair */
    if (r5_req > 6) r5_req = 6;
    if (r_slow < 90 && (ana_boost > 0 || cur_Q > 0.65)) {
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R5_LIPID, 0, 0, r5_req);
    }

    /* R6: Enzyme / Polypeptide translation (3S + E -> P + 2W) */
    if (c->mol[CELL_SP_P] < 14 + (int64_t)c->num_organelles * 8 && c->mol[CELL_SP_E] >= 2 && r_slow < 85) {
        int64_t r6_req = (c->genome[40] > 80 || c->mol[CELL_SP_P] < 3) ? 1 : 0;
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R6_PROT, 0, 0, r6_req);
    }

    /* R7: Template Polymer Replication (4S + E -> T + 2W) — required before mitosis! */
    if (c->mol[CELL_SP_T] < 4 + (int64_t)c->num_organelles * 3 && c->mol[CELL_SP_E] >= 2 && r_slow < 85) {
        int64_t r7_req = (c->genome[48] > 70 || c->diff_role == 2 || c->mol[CELL_SP_T] < 2) ? 1 : 0;
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R7_TMPL, 0, 0, r7_req);
    }

    /* Post-metabolism waste vent so newly generated W doesn't push Q over lysis threshold */
    if (c->mol[CELL_SP_W] > 6) {
        int64_t dump_w = c->mol[CELL_SP_W] - 4;
        c->mol[CELL_SP_W] -= dump_w;
        vx->env[CELL_SP_W] += dump_w;
    }

    /* --- 8. Volumetric Maintenance Cost (Nick Lane 1/R Wall!) + Solvent Leak/Decay --- */
    int64_t vol = cell_internal_volume(c);
    int64_t maint_atp = 1 + (vol / 55);
    if (vol > 85 && c->num_organelles == 0) {
        /* Explicit Lane 1/R prokaryotic overhead above critical radius */
        maint_atp += (vol - 85) / 14;
    }
    int64_t paid = apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R8_HYD, 0, 0, maint_atp);

    /* Autophagic starvation salvage if cell could not pay basal ATP maintenance */
    if (paid < maint_atp) {
        if (c->mol[CELL_SP_P] > 1) {
            apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R13B_SALVP, 0, 0, 1);
        } else if (c->mol[CELL_SP_L] > 28) {
            apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R13A_SALVL, 0, 0, 1);
        } else {
            /* Starvation membrane breakdown */
            apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R9_LDEC, 0, 0, 2);
        }
    }

    /* Passive PMF Proton Leak (R12) — strongly amplified by solvent-lipid mismatch! */
    int64_t leak = 1;
    if (c->lipid_class != sp->optimal_lipid) {
        leak += sp->leak_mismatch;
    }
    if (c->pmf > 0) {
        apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R12_LEAK, -1, 0, leak);
    }

    /* Thermal / Acidic Macromolecule Hydrolysis (R9/R10/R11) */
    uint32_t dec_roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_WORLD_EXOG, w->epoch, idx * 19u + 3u);
    int64_t eff_div = sp->decay_div;
    if (c->lipid_class != sp->optimal_lipid) eff_div = eff_div / 3;
    if (eff_div < 4) eff_div = 4;
    if ((int64_t)(dec_roll % (uint32_t)eff_div) == 0) {
        if (c->mol[CELL_SP_L] > 16) apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R9_LDEC, 0, 0, 1);
        if (c->mol[CELL_SP_P] > 2)  apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R10_PDEC, 0, 0, 1);
    }
    if (c->lipid_class != sp->optimal_lipid && w->cfg.solvent == CELL_SOLVENT_VENUS_H2SO4) {
        /* Unadapted Terran nucleic acids suffer accelerated acid depurination */
        if ((dec_roll % 12u) == 0 && c->mol[CELL_SP_T] > 1) {
            apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R11_TDEC, 0, 0, 1);
        }
    }

    /* --- 9. Active Bioelectric Ion Pump (Maintains V_mem at real ATP cost!) --- */
    if (w->cfg.enable_bioelectricity) {
        /* Target resting potential encoded by Locus 12 byte 96 (-85..+15 mV) */
        int32_t target_v = -85 + (int32_t)((c->genome[96] * 100) / 255);
        /* Quorum sensing shifts target polarization for emergent tissue patterning */
        if (vx->quorum > 24 && c->genome[97] > 128) target_v -= 18;
        if (c->mol[CELL_SP_E] >= 1) {
            if (c->v_mem_mv < target_v - 4) {
                c->v_mem_mv += 6;
                if ((w->epoch & 3u) == 0) apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R8_HYD, 0, 0, 1);
            } else if (c->v_mem_mv > target_v + 4) {
                c->v_mem_mv -= 6;
                if ((w->epoch & 3u) == 0) apply_exact_rxn(w, vx, c->mol, &c->pmf, RXN_R8_HYD, 0, 0, 1);
            }
        } else {
            /* Starving cells passively depolarize toward 0 mV */
            if (c->v_mem_mv < -5) c->v_mem_mv += 5;
            else if (c->v_mem_mv > 5) c->v_mem_mv -= 5;
        }
        if (c->v_mem_mv < -90) c->v_mem_mv = -90;
        if (c->v_mem_mv >  40) c->v_mem_mv =  40;
    }

    /* --- 10. Quorum Autoinducer Secretion (Locus 13, attenuated by MUZZLING) --- */
    uint8_t r_muzzle = w->cfg.restraints[CELL_RESTRAINT_MUZZLING];
    if (c->genome[104] > 100 && r_muzzle < 90) {
        int64_t q_add = (int64_t)(c->genome[104] - 100) / 20;
        q_add = (q_add * (100 - r_muzzle)) / 100;
        vx->quorum += q_add;
        if (vx->quorum > 500) vx->quorum = 500;
    }
}

/* ============================================================================
 * Exact World Mass & Thermodynamic First/Second-Law Ledger Audit (P2)
 * ============================================================================ */

void cell_compute_ledger(const CellWorld* w, CellLedgerSnapshot* out) {
    memset(out, 0, sizeof(*out));
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    int64_t m_tot = 0;
    int64_t e_env = 0, e_cell = 0, e_org = 0, e_pmf = 0, e_hloc = 0;

    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellVoxel* vx = &w->voxels[i];
        m_tot  += cell_species_mass(vx->env);
        e_env  += cell_species_energy(vx->env);
        e_hloc += vx->heat_local_uq;

        const CellEntity* c = &w->cells[i];
        if (c->alive_state != 0) {
            m_tot  += cell_species_mass(c->mol);
            e_cell += cell_species_energy(c->mol);
            e_pmf  += c->pmf * sp->e_pmf;
            for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                if (c->org[k].active) {
                    m_tot += cell_species_mass(c->org[k].mol);
                    e_org += cell_species_energy(c->org[k].mol);
                    e_pmf += c->org[k].pmf * sp->e_pmf;
                }
            }
        }
    }

    int64_t e_pools = e_env + e_cell + e_org + e_pmf + e_hloc + w->e_heat_radiated_sink_uq;
    int64_t e_expected = w->e_initial_uq + w->e_inflow_cum_uq - w->e_export_cum_uq;
    int64_t e_res = e_pools - e_expected;

    out->mass_initial_amu        = w->mass_initial_amu;
    out->mass_current_amu        = m_tot;
    out->mass_residual_amu       = m_tot - w->mass_initial_amu;
    out->e_initial_uq            = w->e_initial_uq;
    out->e_inflow_cum_uq         = w->e_inflow_cum_uq;
    out->e_export_cum_uq         = w->e_export_cum_uq;
    out->e_chem_env_uq           = e_env;
    out->e_chem_cells_uq         = e_cell;
    out->e_chem_organelles_uq    = e_org;
    out->e_pmf_total_uq          = e_pmf;
    out->e_heat_local_uq         = e_hloc;
    out->e_heat_radiated_sink_uq = w->e_heat_radiated_sink_uq;
    out->int_ledger_residual_uq  = e_res;
    double denom = (double)(w->e_initial_uq + w->e_inflow_cum_uq);
    out->relative_drift          = (denom > 0.0) ? (fabs((double)e_res) / denom) : 0.0;
    out->second_law_violations   = w->second_law_violations;
}
