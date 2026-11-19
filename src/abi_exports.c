#include "cellulogonic_internal.h"

/* ============================================================================
 * Flat C11 Exported Public ABI, Canonical Observer-Independent SHA-256 (P9),
 * Binary Snapshot .cmbin Save/Resume (P4), Shock/Anhydrobiosis & UI Export
 * ============================================================================ */

CELL_API uint32_t cellulogonic_abi_version(void) {
    return CELLULOGONIC_ABI_VERSION;
}

CELL_API void cellulogonic_default_config(CellWorldConfig* out_cfg) {
    if (!out_cfg) return;
    memset(out_cfg, 0, sizeof(*out_cfg));
    out_cfg->width                 = 32u;
    out_cfg->height                = 32u;
    out_cfg->regime                = CELL_REGIME_FERAL_ECOSYSTEM;
    out_cfg->solvent               = CELL_SOLVENT_TERRAN_H2O;
    out_cfg->kernel_mode           = CELL_KERNEL_SIMD_AVX2;
    out_cfg->photon_flux_uq        = 184u;
    out_cfg->seed                  = 5000ULL;
    out_cfg->enable_endosymbiosis  = 1u;
    out_cfg->enable_bioelectricity = 1u;
    out_cfg->enable_hgt_phages     = 1u;
}

CELL_API CellWorld* cellulogonic_create(const CellWorldConfig* cfg) {
    CellWorldConfig eff;
    if (cfg) eff = *cfg;
    else     cellulogonic_default_config(&eff);

    if (eff.width  < 8u)  eff.width  = 16u;
    if (eff.height < 8u)  eff.height = 16u;
    if (eff.width  > 64u) eff.width  = 64u;
    if (eff.height > 64u) eff.height = 64u;
    if (eff.solvent >= CELL_SOLVENT_COUNT) eff.solvent = CELL_SOLVENT_TERRAN_H2O;

    CellWorld* w = (CellWorld*)calloc(1, sizeof(CellWorld));
    if (!w) return NULL;
    w->cfg       = eff;
    w->num_slots = eff.width * eff.height;

    /* Preallocate ALL runtime buffers upfront -> ZERO post-warmup allocations (Clean-Slate HW02!) */
    w->voxels       = (CellVoxel*)calloc(w->num_slots, sizeof(CellVoxel));
    w->cells        = (CellEntity*)calloc(w->num_slots, sizeof(CellEntity));
    w->diff_delta   = (int64_t*)calloc((size_t)w->num_slots * CELL_SPECIES_COUNT, sizeof(int64_t));
    w->heat_delta   = (int64_t*)calloc(w->num_slots, sizeof(int64_t));
    w->quorum_delta = (int64_t*)calloc(w->num_slots, sizeof(int64_t));
    w->vmem_delta   = (int32_t*)calloc(w->num_slots, sizeof(int32_t));
    w->bfs_queue    = (uint32_t*)calloc(w->num_slots, sizeof(uint32_t));
    w->bfs_vis      = (uint8_t*)calloc(w->num_slots, sizeof(uint8_t));

    if (!w->voxels || !w->cells || !w->diff_delta || !w->heat_delta ||
        !w->quorum_delta || !w->vmem_delta || !w->bfs_queue || !w->bfs_vis) {
        cellulogonic_destroy(w);
        return NULL;
    }

    cell_init_world_state(w);
    w->post_warmup_allocs = 0;
    return w;
}

CELL_API void cellulogonic_destroy(CellWorld* w) {
    if (!w) return;
    free(w->voxels);
    free(w->cells);
    free(w->diff_delta);
    free(w->heat_delta);
    free(w->quorum_delta);
    free(w->vmem_delta);
    free(w->bfs_queue);
    free(w->bfs_vis);
    free(w);
}

CELL_API void cellulogonic_step_epochs(CellWorld* w, uint32_t epochs) {
    if (!w) return;
    for (uint32_t e = 0; e < epochs; ++e) {
        cell_step_one_epoch(w);
    }
}

CELL_API void cellulogonic_set_restraint(CellWorld* w, uint32_t restraint_id, uint8_t pct) {
    if (!w || restraint_id >= CELL_RESTRAINT_COUNT) return;
    w->cfg.restraints[restraint_id] = pct > 100u ? 100u : pct;
}

/* Conservation-exact planetary solvent transplant:
 * Dissipates pre-existing transmembrane PMF at old e_pmf into local heat (0 uq drift!)
 * before switching to the new planetary solvent regime. */
CELL_API void cellulogonic_set_solvent(CellWorld* w, uint32_t new_solvent) {
    if (!w || new_solvent >= CELL_SOLVENT_COUNT) return;
    if (w->cfg.solvent == new_solvent) return;
    const SolventParams* old_sp = &SOLVENT_TABLE[w->cfg.solvent];
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        if (c->pmf > 0) {
            w->voxels[i].heat_local_uq += c->pmf * old_sp->e_pmf;
            c->pmf = 0;
        }
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (c->org[k].active && c->org[k].pmf > 0) {
                w->voxels[i].heat_local_uq += c->org[k].pmf * old_sp->e_pmf;
                c->org[k].pmf = 0;
            }
        }
    }
    w->cfg.solvent = new_solvent;
}

CELL_API void cellulogonic_set_kernel_mode(CellWorld* w, uint32_t kernel_mode) {
    if (!w) return;
    w->cfg.kernel_mode = (kernel_mode == CELL_KERNEL_SCALAR_REF)
                         ? CELL_KERNEL_SCALAR_REF
                         : CELL_KERNEL_SIMD_AVX2;
}

/* ============================================================================
 * Canonical Little-Endian SHA-256 State Hasher (Observer-Independent P9!)
 * Hashes exact simulation state across voxels, cells, organelles & ledger.
 * ============================================================================ */

static inline void sha_u32(CellSha256Ctx* ctx, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)(v & 0xFFu), (uint8_t)((v >> 8) & 0xFFu),
        (uint8_t)((v >> 16) & 0xFFu), (uint8_t)((v >> 24) & 0xFFu)
    };
    cell_sha256_update(ctx, b, 4);
}

static inline void sha_u64(CellSha256Ctx* ctx, uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; ++i) b[i] = (uint8_t)((v >> (i * 8)) & 0xFFu);
    cell_sha256_update(ctx, b, 8);
}

CELL_API void cellulogonic_state_sha256(const CellWorld* w, uint8_t out_raw[32], char out_hex[65]) {
    if (!w) return;
    CellSha256Ctx sha;
    cell_sha256_init(&sha);

    sha_u32(&sha, w->cfg.width);
    sha_u32(&sha, w->cfg.height);
    sha_u32(&sha, w->cfg.regime);
    sha_u32(&sha, w->cfg.solvent);
    sha_u64(&sha, w->epoch);
    sha_u64(&sha, (uint64_t)w->mass_initial_amu);
    sha_u64(&sha, (uint64_t)w->e_initial_uq);
    sha_u64(&sha, (uint64_t)w->e_inflow_cum_uq);
    sha_u64(&sha, (uint64_t)w->e_heat_radiated_sink_uq);

    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellVoxel* vx = &w->voxels[i];
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)vx->env[s]);
        sha_u64(&sha, (uint64_t)vx->quorum);
        sha_u64(&sha, (uint64_t)vx->heat_local_uq);
        cell_sha256_update(&sha, &vx->free_plasmid_valid, 1);

        const CellEntity* c = &w->cells[i];
        cell_sha256_update(&sha, &c->alive_state, 1);
        if (c->alive_state != 0) {
            sha_u32(&sha, c->lineage_id);
            sha_u32(&sha, c->generation);
            sha_u32(&sha, (uint32_t)c->v_mem_mv);
            cell_sha256_update(&sha, &c->lipid_class, 1);
            cell_sha256_update(&sha, &c->c_ring_nH, 1);
            cell_sha256_update(&sha, &c->num_organelles, 1);
            cell_sha256_update(&sha, &c->bond_mask, 1);
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)c->mol[s]);
            sha_u64(&sha, (uint64_t)c->pmf);
            cell_sha256_update(&sha, c->genome, CELL_GENOME_BYTES);
            for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                const CellOrganelle* o = &c->org[k];
                cell_sha256_update(&sha, &o->active, 1);
                if (o->active) {
                    cell_sha256_update(&sha, &o->org_type, 1);
                    cell_sha256_update(&sha, &o->nesting_depth, 1);
                    cell_sha256_update(&sha, &o->antiporter_share, 1);
                    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)o->mol[s]);
                    sha_u64(&sha, (uint64_t)o->pmf);
                }
            }
        }
    }

    uint8_t raw[32];
    cell_sha256_final(&sha, raw);
    if (out_raw) memcpy(out_raw, raw, 32);
    if (out_hex) cell_sha256_to_hex(raw, out_hex);
}

CELL_API void cellulogonic_get_telemetry(const CellWorld* w, CellTelemetry* out_tel) {
    if (!w || !out_tel) return;
    cell_compute_telemetry(w, out_tel);
}

static void fill_specimen(const CellWorld* w, uint32_t slot, CellSpecimen* out) {
    memset(out, 0, sizeof(*out));
    const CellEntity* c = &w->cells[slot];
    out->slot_index           = slot;
    out->x                    = slot % w->cfg.width;
    out->y                    = slot / w->cfg.width;
    out->lineage_id           = c->lineage_id;
    out->generation           = c->generation;
    out->alive_state          = c->alive_state;
    out->lipid_class          = c->lipid_class;
    out->c_ring_nH            = c->c_ring_nH;
    out->num_organelles       = c->num_organelles;
    out->bond_mask            = c->bond_mask;
    out->differentiation_role = c->diff_role;
    out->v_mem_mv             = c->v_mem_mv;
    memcpy(out->mol, c->mol, sizeof(c->mol));
    out->pmf                  = c->pmf;
    out->total_volume         = cell_internal_volume(c);
    out->reduced_Q            = cell_reduced_Q(out->total_volume, c->mol[CELL_SP_L]);
    out->egt_genes            = c->egt_genes;
    out->age_epochs           = c->age_epochs;
    out->autophage_dna_sig    = c->autophage_dna_sig;
    memcpy(out->genome, c->genome, CELL_GENOME_BYTES);
    uint8_t max_d = 0;
    for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
        if (c->org[k].active && c->org[k].nesting_depth > max_d) {
            max_d = c->org[k].nesting_depth;
        }
    }
    out->max_nesting_depth = max_d;
}

CELL_API int cellulogonic_inspect_slot(const CellWorld* w, uint32_t slot, CellSpecimen* out_spec) {
    if (!w || !out_spec || slot >= w->num_slots) return -1;
    fill_specimen(w, slot, out_spec);
    return w->cells[slot].alive_state != 0 ? 0 : 1;
}

CELL_API int cellulogonic_inspect_apex(const CellWorld* w, CellSpecimen* out_spec) {
    if (!w || !out_spec) return -1;
    int best_slot = -1;
    int64_t best_score = -1;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        int64_t score = cell_internal_volume(c) + (int64_t)c->num_organelles * 80 +
                        (int64_t)c->egt_genes * 30 + (c->bond_mask ? 40 : 0);
        if (score > best_score) {
            best_score = score;
            best_slot  = (int)i;
        }
    }
    if (best_slot < 0) {
        memset(out_spec, 0, sizeof(*out_spec));
        return -1;
    }
    fill_specimen(w, (uint32_t)best_slot, out_spec);
    return 0;
}

/* ============================================================================
 * Binary Snapshot Save / Load (.cmbin, Clean-Slate P4 Bit-Exact Resume!)
 * ============================================================================ */

typedef struct SnapshotHeader {
    uint32_t magic;
    uint32_t abi_version;
    CellWorldConfig cfg;
    uint32_t num_slots;
    uint32_t next_lineage_id;
    uint64_t epoch;
    uint64_t total_cell_steps;
    uint64_t total_reactions;
    int64_t  mass_initial_amu;
    int64_t  e_initial_uq;
    int64_t  e_inflow_cum_uq;
    int64_t  e_export_cum_uq;
    int64_t  e_heat_radiated_sink_uq;
    uint32_t second_law_violations;
    uint32_t pad;
    uint64_t counters[18];
} SnapshotHeader;

CELL_API int cellulogonic_save_snapshot(const CellWorld* w, const char* path) {
    if (!w || !path) return -1;
    FILE* f = fopen(path, "wb");
    if (!f) return -2;

    SnapshotHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic                   = CELL_SNAPSHOT_MAGIC;
    hdr.abi_version             = CELLULOGONIC_ABI_VERSION;
    hdr.cfg                     = w->cfg;
    hdr.num_slots               = w->num_slots;
    hdr.next_lineage_id         = w->next_lineage_id;
    hdr.epoch                   = w->epoch;
    hdr.total_cell_steps        = w->total_cell_steps;
    hdr.total_reactions         = w->total_reactions;
    hdr.mass_initial_amu        = w->mass_initial_amu;
    hdr.e_initial_uq            = w->e_initial_uq;
    hdr.e_inflow_cum_uq         = w->e_inflow_cum_uq;
    hdr.e_export_cum_uq         = w->e_export_cum_uq;
    hdr.e_heat_radiated_sink_uq = w->e_heat_radiated_sink_uq;
    hdr.second_law_violations   = w->second_law_violations;
    hdr.counters[0]  = w->abiogenesis_vesicles;
    hdr.counters[1]  = w->symmetric_fissions;
    hdr.counters[2]  = w->asymmetric_buddings;
    hdr.counters[3]  = w->osmotic_lyses;
    hdr.counters[4]  = w->phagocytic_digestions;
    hdr.counters[5]  = w->endosymbiosis_captures;
    hdr.counters[6]  = w->secondary_captures;
    hdr.counters[7]  = w->egt_transfer_events;
    hdr.counters[8]  = w->mitophagy_events;
    hdr.counters[9]  = w->hgt_uptake_events;
    hdr.counters[10] = w->phage_lysis_events;
    hdr.counters[11] = w->cancer_defections;
    hdr.counters[12] = w->apoptotic_policings;
    hdr.counters[13] = w->backlash_killswitch_deletions;
    hdr.counters[14] = w->backlash_dormancy_spores;
    hdr.counters[15] = w->backlash_hgt_smuggles;
    hdr.counters[16] = w->backlash_metabolic_bypasses;
    hdr.counters[17] = w->novel_adaptations_cum;

    int ok = (fwrite(&hdr, sizeof(hdr), 1, f) == 1) &&
             (fwrite(w->voxels, sizeof(CellVoxel), w->num_slots, f) == w->num_slots) &&
             (fwrite(w->cells,  sizeof(CellEntity), w->num_slots, f) == w->num_slots);
    fclose(f);
    return ok ? 0 : -3;
}

CELL_API CellWorld* cellulogonic_load_snapshot(const char* path) {
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    SnapshotHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 ||
        hdr.magic != CELL_SNAPSHOT_MAGIC ||
        hdr.abi_version != CELLULOGONIC_ABI_VERSION) {
        fclose(f);
        return NULL;
    }

    CellWorld* w = cellulogonic_create(&hdr.cfg);
    if (!w) { fclose(f); return NULL; }
    if (hdr.num_slots != w->num_slots ||
        fread(w->voxels, sizeof(CellVoxel), w->num_slots, f) != w->num_slots ||
        fread(w->cells,  sizeof(CellEntity), w->num_slots, f) != w->num_slots) {
        fclose(f);
        cellulogonic_destroy(w);
        return NULL;
    }
    fclose(f);

    w->next_lineage_id         = hdr.next_lineage_id;
    w->epoch                   = hdr.epoch;
    w->total_cell_steps        = hdr.total_cell_steps;
    w->total_reactions         = hdr.total_reactions;
    w->mass_initial_amu        = hdr.mass_initial_amu;
    w->e_initial_uq            = hdr.e_initial_uq;
    w->e_inflow_cum_uq         = hdr.e_inflow_cum_uq;
    w->e_export_cum_uq         = hdr.e_export_cum_uq;
    w->e_heat_radiated_sink_uq = hdr.e_heat_radiated_sink_uq;
    w->second_law_violations   = hdr.second_law_violations;
    w->abiogenesis_vesicles          = hdr.counters[0];
    w->symmetric_fissions            = hdr.counters[1];
    w->asymmetric_buddings           = hdr.counters[2];
    w->osmotic_lyses                 = hdr.counters[3];
    w->phagocytic_digestions         = hdr.counters[4];
    w->endosymbiosis_captures        = hdr.counters[5];
    w->secondary_captures            = hdr.counters[6];
    w->egt_transfer_events           = hdr.counters[7];
    w->mitophagy_events              = hdr.counters[8];
    w->hgt_uptake_events             = hdr.counters[9];
    w->phage_lysis_events            = hdr.counters[10];
    w->cancer_defections             = hdr.counters[11];
    w->apoptotic_policings           = hdr.counters[12];
    w->backlash_killswitch_deletions = hdr.counters[13];
    w->backlash_dormancy_spores      = hdr.counters[14];
    w->backlash_hgt_smuggles         = hdr.counters[15];
    w->backlash_metabolic_bypasses   = hdr.counters[16];
    w->novel_adaptations_cum         = hdr.counters[17];
    w->post_warmup_allocs = 0;
    return w;
}

/* ============================================================================
 * Conservation-Closed Perturbations: Mass Extinction Lysis & Tardigrade Tun!
 * ============================================================================ */

CELL_API uint32_t cellulogonic_apply_shock_lysis(CellWorld* w, uint32_t kill_percent) {
    if (!w) return 0;
    if (kill_percent > 100u) kill_percent = 100u;
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
    uint32_t killed = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        /* Vitrified TDP bioglass tun cells resist shock lysis by 90%! (Lean T42) */
        uint32_t eff_kill = (c->alive_state == 3) ? (kill_percent / 10u) : kill_percent;
        uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_WORLD_EXOG, w->epoch + 999ULL, i);
        if ((roll % 100u) < eff_kill) {
            CellVoxel* vx = &w->voxels[i];
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
            memset(c, 0, sizeof(*c));
            killed++;
        }
    }
    return killed;
}

/* Tardigrade Anhydrobiosis Desiccation Event (Exact Conservation, Lean T42!):
 * Cells expressing TDP/CAHS protectant locus (genome[122] >= 110 & P >= 2) vitrify into
 * bioglass tun state (alive_state == 3); unadapted cells suffer desiccation osmotic lysis! */
CELL_API uint32_t cellulogonic_apply_desiccation_tun(CellWorld* w, uint32_t severity_pct) {
    if (!w) return 0;
    if (severity_pct > 100u) severity_pct = 100u;
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];
    uint32_t vitrified = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        if (c->genome[122] >= 110 && c->mol[CELL_SP_P] >= 2) {
            /* Vitrify into amorphous TDP bioglass tun! Equilibrate PMF to local heat cleanly */
            if (c->pmf > 0) {
                w->voxels[i].heat_local_uq += c->pmf * sp->e_pmf;
                c->pmf = 0;
            }
            c->alive_state = 3;
            vitrified++;
        } else {
            /* Unprotected cell lacks TDP bioglass -> desiccation membrane rupture! */
            uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_WORLD_EXOG, w->epoch + 777ULL, i);
            if ((roll % 100u) < severity_pct) {
                CellVoxel* vx = &w->voxels[i];
                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { vx->env[s] += c->mol[s]; c->mol[s] = 0; }
                if (c->pmf > 0) { vx->heat_local_uq += c->pmf * sp->e_pmf; c->pmf = 0; }
                for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                    if (c->org[k].active) {
                        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) { vx->env[s] += c->org[k].mol[s]; c->org[k].mol[s] = 0; }
                        if (c->org[k].pmf > 0) { vx->heat_local_uq += c->org[k].pmf * sp->e_pmf; c->org[k].pmf = 0; }
                        c->org[k].active = 0;
                    }
                }
                memset(c, 0, sizeof(*c));
                w->osmotic_lyses++;
            }
        }
    }
    return vitrified;
}

CELL_API uint32_t cellulogonic_rehydrate_tun(CellWorld* w) {
    if (!w) return 0;
    uint32_t revived = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        if (w->cells[i].alive_state == 3) {
            w->cells[i].alive_state = 1;
            revived++;
        }
    }
    return revived;
}

/* ============================================================================
 * Read-Only Compact JSON Grid Export for Live Cellular Observatory UI (P9 Safe)
 * ============================================================================ */

CELL_API uint32_t cellulogonic_export_grid_json(const CellWorld* w, char* out_buf, size_t buf_cap) {
    if (!w || !out_buf || buf_cap < 256) return 0;
    size_t pos = 0;
    int n = snprintf(out_buf + pos, buf_cap - pos,
                     "{\"w\":%u,\"h\":%u,\"epoch\":%llu,\"solvent\":%u,\"cells\":[",
                     w->cfg.width, w->cfg.height, (unsigned long long)w->epoch, w->cfg.solvent);
    if (n < 0 || (size_t)n >= buf_cap - pos) return 0;
    pos += (size_t)n;

    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellEntity* c = &w->cells[i];
        const CellVoxel* vx = &w->voxels[i];
        uint8_t sec = 0;
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (c->org[k].active && c->org[k].nesting_depth >= 2) sec = 1;
        }
        int64_t vol = (c->alive_state != 0) ? cell_internal_volume(c) : 0;
        int m = snprintf(out_buf + pos, buf_cap - pos,
                         "%s[%u,%u,%u,%u,%u,%d,%lld,%lld,%lld,%lld]",
                         (i > 0 ? "," : ""),
                         (unsigned)c->alive_state,
                         (unsigned)c->num_organelles,
                         (unsigned)sec,
                         (unsigned)c->bond_mask,
                         (unsigned)c->lipid_class,
                         (int)c->v_mem_mv,
                         (long long)vol,
                         (long long)c->mol[CELL_SP_L],
                         (long long)vx->env[CELL_SP_S],
                         (long long)vx->env[CELL_SP_X]);
        if (m < 0 || pos + (size_t)m + 8 >= buf_cap) break;
        pos += (size_t)m;
    }

    int end = snprintf(out_buf + pos, buf_cap - pos, "]}");
    if (end > 0 && pos + (size_t)end < buf_cap) pos += (size_t)end;
    return (uint32_t)pos;
}
