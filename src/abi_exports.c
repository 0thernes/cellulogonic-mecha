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
    out_cfg->photon_flux_uq        = 572u;
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
    if (eff.regime > CELL_REGIME_EMBRYOGENESIS) eff.regime = CELL_REGIME_FERAL_ECOSYSTEM;
    if (eff.solvent >= CELL_SOLVENT_COUNT) eff.solvent = CELL_SOLVENT_TERRAN_H2O;
    if (eff.kernel_mode != CELL_KERNEL_SCALAR_REF) eff.kernel_mode = CELL_KERNEL_SIMD_AVX2;
    for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) {
        if (eff.restraints[r] > 100u) eff.restraints[r] = 100u;
    }
    eff.enable_endosymbiosis  = eff.enable_endosymbiosis  ? 1u : 0u;
    eff.enable_bioelectricity = eff.enable_bioelectricity ? 1u : 0u;
    eff.enable_hgt_phages     = eff.enable_hgt_phages     ? 1u : 0u;
    /* seed == 0 -> True Hardware Intel RDSEED Non-Deterministic Feral Thermal Entropy! */
    if (eff.seed == 0ULL) eff.seed = cell_hw_rdseed_u64();

    CellWorld* w = (CellWorld*)calloc(1, sizeof(CellWorld));
    if (!w) return NULL;
    w->cfg          = eff;
    w->num_slots    = eff.width * eff.height;
    w->cpu_has_avx2 = (uint32_t)cell_runtime_has_avx2();

    /* Preallocate ALL runtime buffers upfront -> ZERO post-warmup allocations (Clean-Slate HW02!) */
    w->voxels       = (CellVoxel*)calloc(w->num_slots, sizeof(CellVoxel));
    w->cells        = (CellEntity*)calloc(w->num_slots, sizeof(CellEntity));
    w->diff_delta   = (int64_t*)calloc((size_t)w->num_slots * CELL_SPECIES_COUNT, sizeof(int64_t));
    w->heat_delta   = (int64_t*)calloc(w->num_slots, sizeof(int64_t));
    w->quorum_delta = (int64_t*)calloc(w->num_slots, sizeof(int64_t));
    w->morph_delta  = (int32_t*)calloc((size_t)w->num_slots * CELL_MORPH_AXES, sizeof(int32_t));
    w->vmem_delta   = (int32_t*)calloc(w->num_slots, sizeof(int32_t));
    w->hash_head    = (int32_t*)calloc(w->num_slots, sizeof(int32_t));
    w->hash_next    = (int32_t*)calloc(w->num_slots, sizeof(int32_t));

    if (!w->voxels || !w->cells || !w->diff_delta || !w->heat_delta ||
        !w->quorum_delta || !w->morph_delta || !w->vmem_delta ||
        !w->hash_head || !w->hash_next) {
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
    free(w->morph_delta);
    free(w->vmem_delta);
    free(w->hash_head);
    free(w->hash_next);
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
        uint32_t vidx = cell_voxel_idx(w, c);
        if (c->pmf > 0) {
            w->voxels[vidx].heat_local_uq += c->pmf * old_sp->e_pmf;
            c->pmf = 0;
        }
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (c->org[k].active && c->org[k].pmf > 0) {
                w->voxels[vidx].heat_local_uq += c->org[k].pmf * old_sp->e_pmf;
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
    if (!w) {
        if (out_raw) memset(out_raw, 0, 32);
        if (out_hex) out_hex[0] = '\0';
        return;
    }
    CellSha256Ctx sha;
    cell_sha256_init(&sha);

    /* World configuration, complete ledger & all mutable counters (excluding kernel_mode for AVX2 == Scalar parity!) */
    sha_u32(&sha, w->cfg.width);
    sha_u32(&sha, w->cfg.height);
    sha_u32(&sha, w->cfg.regime);
    sha_u32(&sha, w->cfg.solvent);
    sha_u32(&sha, w->cfg.photon_flux_uq);
    sha_u64(&sha, w->cfg.seed);
    cell_sha256_update(&sha, w->cfg.restraints, CELL_RESTRAINT_COUNT);
    cell_sha256_update(&sha, &w->cfg.enable_endosymbiosis, 1);
    cell_sha256_update(&sha, &w->cfg.enable_bioelectricity, 1);
    cell_sha256_update(&sha, &w->cfg.enable_hgt_phages, 1);
    sha_u64(&sha, w->epoch);
    sha_u64(&sha, w->total_cell_steps);
    sha_u64(&sha, w->total_reactions);
    sha_u32(&sha, w->next_lineage_id);
    sha_u64(&sha, (uint64_t)w->mass_initial_amu);
    sha_u64(&sha, (uint64_t)w->e_initial_uq);
    sha_u64(&sha, (uint64_t)w->e_inflow_cum_uq);
    sha_u64(&sha, (uint64_t)w->e_export_cum_uq);
    sha_u64(&sha, (uint64_t)w->e_heat_radiated_sink_uq);
    sha_u32(&sha, w->second_law_violations);
    sha_u64(&sha, w->abiogenesis_vesicles);
    sha_u64(&sha, w->symmetric_fissions);
    sha_u64(&sha, w->asymmetric_buddings);
    sha_u64(&sha, w->osmotic_lyses);
    sha_u64(&sha, w->phagocytic_digestions);
    sha_u64(&sha, w->endosymbiosis_captures);
    sha_u64(&sha, w->secondary_captures);
    sha_u64(&sha, w->egt_transfer_events);
    sha_u64(&sha, w->mitophagy_events);
    sha_u64(&sha, w->hgt_uptake_events);
    sha_u64(&sha, w->phage_lysis_events);
    sha_u64(&sha, w->cancer_defections);
    sha_u64(&sha, w->apoptotic_policings);
    sha_u64(&sha, w->backlash_killswitch_deletions);
    sha_u64(&sha, w->backlash_dormancy_spores);
    sha_u64(&sha, w->backlash_hgt_smuggles);
    sha_u64(&sha, w->backlash_metabolic_bypasses);
    sha_u64(&sha, w->novel_adaptations_cum);
    sha_u64(&sha, w->axons_wired_cum);
    sha_u64(&sha, w->axons_pruned_cum);
    sha_u64(&sha, w->hippo_arrests_cum);
    sha_u64(&sha, w->eutelic_stops_cum);
    sha_u64(&sha, w->cpg_pulses_cum);

    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellVoxel* vx = &w->voxels[i];
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)vx->env[s]);
        sha_u64(&sha, (uint64_t)vx->quorum);
        sha_u64(&sha, (uint64_t)vx->heat_local_uq);
        for (int m = 0; m < (int)CELL_MORPH_AXES; ++m) sha_u32(&sha, (uint32_t)vx->morph[m]);
        cell_sha256_update(&sha, &vx->free_plasmid_valid, 1);
        cell_sha256_update(&sha, &vx->free_plasmid_is_phage, 1);
        if (vx->free_plasmid_valid) {
            cell_sha256_update(&sha, vx->free_plasmid, 32);
        }

        const CellEntity* c = &w->cells[i];
        cell_sha256_update(&sha, &c->alive_state, 1);
        if (c->alive_state != 0) {
            cell_sha256_update(&sha, &c->lipid_class, 1);
            cell_sha256_update(&sha, &c->c_ring_nH, 1);
            cell_sha256_update(&sha, &c->num_organelles, 1);
            cell_sha256_update(&sha, &c->bond_mask, 1);
            cell_sha256_update(&sha, &c->diff_role, 1);
            cell_sha256_update(&sha, &c->phage_load, 1);
            cell_sha256_update(&sha, &c->kill_switch_intact, 1);
            sha_u32(&sha, c->lineage_id);
            sha_u32(&sha, c->generation);
            sha_u32(&sha, c->age_epochs);
            sha_u32(&sha, c->egt_genes);
            sha_u32(&sha, (uint32_t)c->v_mem_mv);
            sha_u32(&sha, (uint32_t)c->defect_strikes);
            for (int d = 0; d < 3; ++d) sha_u32(&sha, (uint32_t)c->pos_nm[d]);
            for (int d = 0; d < 3; ++d) sha_u32(&sha, (uint32_t)c->vel_nm[d]);
            for (int d = 0; d < 3; ++d) sha_u32(&sha, (uint32_t)c->heading_q10[d]);
            sha_u32(&sha, (uint32_t)c->radius_nm);
            sha_u32(&sha, (uint32_t)c->elong_q10);
            sha_u32(&sha, (uint32_t)c->ruffle_q10);
            sha_u32(&sha, (uint32_t)c->breath_phase_mrad);
            sha_u32(&sha, (uint32_t)c->breath_amp_q10);
            sha_u32(&sha, c->mt_freq_mhz);
            sha_u32(&sha, (uint32_t)c->mt_coherence_q10);
            cell_sha256_update(&sha, &c->mt_filaments, 1);
            cell_sha256_update(&sha, &c->flagella_q10, 1);
            cell_sha256_update(&sha, &c->num_bonds, 1);
            for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
                sha_u32(&sha, (uint32_t)(int32_t)c->bond_partners[b]);
            }
            cell_sha256_update(&sha, c->tf, 8);
            cell_sha256_update(&sha, &c->morph_fate, 1);
            cell_sha256_update(&sha, &c->cad_type, 1);
            cell_sha256_update(&sha, &c->cleavage_mode, 1);
            cell_sha256_update(&sha, &c->eutelic_rem, 1);
            cell_sha256_update(&sha, &c->contact_pressure, 1);
            cell_sha256_update(&sha, &c->cpg_firing, 1);
            sha_u32(&sha, (uint32_t)c->apical_wedge_q10);
            sha_u32(&sha, (uint32_t)c->cpg_phase_mrad);
            for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
                sha_u32(&sha, (uint32_t)(int32_t)c->axon_target[a]);
                cell_sha256_update(&sha, &c->axon_weight[a], 1);
            }
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)c->mol[s]);
            sha_u64(&sha, (uint64_t)c->pmf);
            sha_u64(&sha, c->autophage_dna_sig);
            cell_sha256_update(&sha, c->genome, CELL_GENOME_BYTES);
            for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                const CellOrganelle* o = &c->org[k];
                cell_sha256_update(&sha, &o->active, 1);
                if (o->active) {
                    cell_sha256_update(&sha, &o->org_type, 1);
                    cell_sha256_update(&sha, &o->nesting_depth, 1);
                    cell_sha256_update(&sha, &o->c_ring_nH, 1);
                    cell_sha256_update(&sha, &o->antiporter_share, 1);
                    cell_sha256_update(&sha, &o->lipid_class, 1);
                    cell_sha256_update(&sha, &o->egt_transferred, 1);
                    sha_u32(&sha, o->lineage_id);
                    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) sha_u64(&sha, (uint64_t)o->mol[s]);
                    sha_u64(&sha, (uint64_t)o->pmf);
                    cell_sha256_update(&sha, o->mt_genome, CELL_MT_GENOME_BYTES);
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
    out->morph_fate           = c->morph_fate;
    out->cad_type             = c->cad_type;
    out->v_mem_mv             = c->v_mem_mv;
    out->pos_nm[0]            = c->pos_nm[0];
    out->pos_nm[1]            = c->pos_nm[1];
    out->pos_nm[2]            = c->pos_nm[2];
    out->vel_nm[0]            = c->vel_nm[0];
    out->vel_nm[1]            = c->vel_nm[1];
    out->vel_nm[2]            = c->vel_nm[2];
    out->radius_nm            = c->radius_nm;
    out->elong_q10            = c->elong_q10;
    out->mt_freq_mhz          = c->mt_freq_mhz;
    out->mt_coherence_q10     = c->mt_coherence_q10;
    memcpy(out->mol, c->mol, sizeof(c->mol));
    out->pmf                  = c->pmf;
    out->total_volume         = cell_internal_volume(c);
    out->reduced_Q            = cell_reduced_Q(out->total_volume, c->mol[CELL_SP_L]);
    out->egt_genes            = c->egt_genes;
    out->age_epochs           = c->age_epochs;
    out->autophage_dna_sig    = c->autophage_dna_sig;
    memcpy(out->genome, c->genome, CELL_GENOME_BYTES);
    uint8_t ax_cnt = 0;
    for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
        if (c->axon_target[a] >= 0 && (uint32_t)c->axon_target[a] < w->num_slots &&
            w->cells[c->axon_target[a]].alive_state == 1) {
            ax_cnt++;
        }
    }
    out->active_axons     = ax_cnt;
    out->flagella_q10     = c->flagella_q10;
    out->apical_wedge_q10 = c->apical_wedge_q10;
    out->ruffle_q10       = c->ruffle_q10;
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
    uint64_t counters[23];
} SnapshotHeader;

_Static_assert(sizeof(SnapshotHeader) == 320, "SnapshotHeader must have zero implicit padding");

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
    hdr.counters[18] = w->axons_wired_cum;
    hdr.counters[19] = w->axons_pruned_cum;
    hdr.counters[20] = w->hippo_arrests_cum;
    hdr.counters[21] = w->eutelic_stops_cum;
    hdr.counters[22] = w->cpg_pulses_cum;

    /* Compute cryptographic SHA-256 integrity trailer over (hdr, voxels, cells) */
    uint8_t digest[32];
    CellSha256Ctx sha;
    cell_sha256_init(&sha);
    cell_sha256_update(&sha, (const uint8_t*)&hdr, sizeof(hdr));
    cell_sha256_update(&sha, (const uint8_t*)w->voxels, (size_t)w->num_slots * sizeof(CellVoxel));
    cell_sha256_update(&sha, (const uint8_t*)w->cells,  (size_t)w->num_slots * sizeof(CellEntity));
    cell_sha256_final(&sha, digest);

    int ok = (fwrite(&hdr, sizeof(hdr), 1, f) == 1) &&
             (fwrite(w->voxels, sizeof(CellVoxel), w->num_slots, f) == w->num_slots) &&
             (fwrite(w->cells,  sizeof(CellEntity), w->num_slots, f) == w->num_slots) &&
             (fwrite(digest, 1, 32, f) == 32);
    int crc = fclose(f);
    return (ok && crc == 0) ? 0 : -3;
}

CELL_API CellWorld* cellulogonic_load_snapshot(const char* path) {
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    SnapshotHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 ||
        hdr.magic != CELL_SNAPSHOT_MAGIC ||
        hdr.abi_version != CELLULOGONIC_ABI_VERSION ||
        hdr.pad != 0u ||
        hdr.cfg.width < 8u || hdr.cfg.width > 64u ||
        hdr.cfg.height < 8u || hdr.cfg.height > 64u ||
        hdr.cfg.regime > CELL_REGIME_EMBRYOGENESIS ||
        hdr.cfg.solvent >= CELL_SOLVENT_COUNT ||
        hdr.cfg.kernel_mode > CELL_KERNEL_SCALAR_REF ||
        hdr.cfg.photon_flux_uq > 100000u ||
        hdr.cfg.seed == 0ULL ||
        hdr.cfg.enable_endosymbiosis > 1u ||
        hdr.cfg.enable_bioelectricity > 1u ||
        hdr.cfg.enable_hgt_phages > 1u ||
        hdr.num_slots != hdr.cfg.width * hdr.cfg.height ||
        hdr.mass_initial_amu < 0 || hdr.e_initial_uq < 0 ||
        hdr.e_inflow_cum_uq < 0 || hdr.e_export_cum_uq < 0 ||
        hdr.e_heat_radiated_sink_uq < 0) {
        fclose(f);
        return NULL;
    }
    for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) {
        if (hdr.cfg.restraints[r] > 100u) {
            fclose(f);
            return NULL;
        }
    }

    CellWorld* w = cellulogonic_create(&hdr.cfg);
    if (!w) { fclose(f); return NULL; }
    uint8_t file_digest[32];
    uint8_t trailing_byte = 0;
    int read_ok = (hdr.num_slots == w->num_slots) &&
                  (fread(w->voxels, sizeof(CellVoxel), w->num_slots, f) == w->num_slots) &&
                  (fread(w->cells,  sizeof(CellEntity), w->num_slots, f) == w->num_slots) &&
                  (fread(file_digest, 1, 32, f) == 32) &&
                  (fread(&trailing_byte, 1, 1, f) == 0) &&
                  (feof(f) != 0) && (ferror(f) == 0);
    fclose(f);
    if (!read_ok) {
        cellulogonic_destroy(w);
        return NULL;
    }

    /* Verify SHA-256 cryptographic integrity before trusting any payload fields! */
    uint8_t calc_digest[32];
    CellSha256Ctx sha;
    cell_sha256_init(&sha);
    cell_sha256_update(&sha, (const uint8_t*)&hdr, sizeof(hdr));
    cell_sha256_update(&sha, (const uint8_t*)w->voxels, (size_t)w->num_slots * sizeof(CellVoxel));
    cell_sha256_update(&sha, (const uint8_t*)w->cells,  (size_t)w->num_slots * sizeof(CellEntity));
    cell_sha256_final(&sha, calc_digest);
    if (memcmp(calc_digest, file_digest, 32) != 0) {
        cellulogonic_destroy(w);
        return NULL;
    }

    /* 10^9 cap guarantees >300x headroom below INT64_MAX even on 64x64 with 8 organelles (C01) */
    const int64_t MAX_MOL = 1000000000LL;
    const int32_t span_x  = (int32_t)w->cfg.width  * CELL_VOXEL_NM;
    const int32_t span_y  = (int32_t)w->cfg.height * CELL_VOXEL_NM;
    const int32_t span_z  = (int32_t)w->cfg.width  * CELL_VOXEL_NM;

    /* Validate EVERY voxel field + explicit padding bytes */
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellVoxel* vx = &w->voxels[i];
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
            if (vx->env[s] < 0 || vx->env[s] > MAX_MOL) { cellulogonic_destroy(w); return NULL; }
        }
        if (vx->quorum < 0 || vx->quorum > 1000000LL ||
            vx->heat_local_uq < 0 || vx->heat_local_uq > MAX_MOL ||
            vx->free_plasmid_valid > 1u || vx->free_plasmid_is_phage > 1u ||
            vx->pad[0] != 0u || vx->pad[1] != 0u) {
            cellulogonic_destroy(w);
            return NULL;
        }
        for (uint32_t m = 0; m < CELL_MORPH_AXES; ++m) {
            if (vx->morph[m] < 0 || vx->morph[m] > 100000) { cellulogonic_destroy(w); return NULL; }
        }
    }

    /* Full structural, domain, bounds, padding & graph-symmetry validation on EVERY cell (C04) */
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state > 3u ||
            c->pad_bonds != 0u || c->pad_cpg != 0u || c->pad_mol != 0u) {
            cellulogonic_destroy(w);
            return NULL;
        }
        if (c->alive_state == 0u) {
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                if (c->mol[s] != 0) { cellulogonic_destroy(w); return NULL; }
            }
            if (c->pmf != 0 || c->num_organelles != 0u || c->num_bonds != 0u || c->bond_mask != 0u) {
                cellulogonic_destroy(w);
                return NULL;
            }
            for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
                if (c->bond_partners[b] != -1) { cellulogonic_destroy(w); return NULL; }
            }
            for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
                if (c->axon_target[a] != -1 || c->axon_weight[a] != 0u) { cellulogonic_destroy(w); return NULL; }
            }
            for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                if (c->org[k].active != 0u || c->org[k].pmf != 0) { cellulogonic_destroy(w); return NULL; }
                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                    if (c->org[k].mol[s] != 0) { cellulogonic_destroy(w); return NULL; }
                }
            }
            continue;
        }

        if (c->lipid_class >= CELL_SOLVENT_COUNT ||
            c->c_ring_nH < 2u || c->c_ring_nH > 5u ||
            c->num_organelles > CELL_MAX_ORGANELLES ||
            c->diff_role >= 4u ||
            c->kill_switch_intact > 1u ||
            c->morph_fate >= CELL_FATE_COUNT ||
            c->cad_type >= 4u ||
            c->cleavage_mode >= 3u ||
            c->cpg_firing > 1u ||
            c->num_bonds > CELL_MAX_BONDS ||
            c->bond_mask != (uint8_t)((1u << c->num_bonds) - 1u) ||
            c->v_mem_mv < -90 || c->v_mem_mv > 40 ||
            c->defect_strikes < 0 || c->defect_strikes > 1000000 ||
            c->pos_nm[0] < 0 || c->pos_nm[0] >= span_x ||
            c->pos_nm[1] < 0 || c->pos_nm[1] >= span_y ||
            c->pos_nm[2] < 0 || c->pos_nm[2] >= span_z ||
            c->vel_nm[0] < -50000 || c->vel_nm[0] > 50000 ||
            c->vel_nm[1] < -50000 || c->vel_nm[1] > 50000 ||
            c->vel_nm[2] < -50000 || c->vel_nm[2] > 50000 ||
            c->heading_q10[0] < -2048 || c->heading_q10[0] > 2048 ||
            c->heading_q10[1] < -2048 || c->heading_q10[1] > 2048 ||
            c->heading_q10[2] < -2048 || c->heading_q10[2] > 2048 ||
            c->radius_nm <= 0 || c->radius_nm > 5000 ||
            c->elong_q10 <= 0 || c->elong_q10 > 4000 ||
            c->ruffle_q10 < 0 || c->ruffle_q10 > 2000 ||
            c->breath_phase_mrad < 0 || c->breath_phase_mrad >= 6283 ||
            c->breath_amp_q10 < 0 || c->breath_amp_q10 > 1000 ||
            c->mt_freq_mhz > 50000u ||
            c->mt_coherence_q10 > 1000u ||
            c->apical_wedge_q10 > 1000u ||
            c->cpg_phase_mrad < 0 || c->cpg_phase_mrad >= 6283 ||
            c->pmf < 0 || c->pmf > MAX_MOL) {
            cellulogonic_destroy(w);
            return NULL;
        }
        for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
            if (c->mol[s] < 0 || c->mol[s] > MAX_MOL) { cellulogonic_destroy(w); return NULL; }
        }

        /* Organelle field, padding & active-count consistency validation */
        uint8_t org_active = 0u;
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            const CellOrganelle* o = &c->org[k];
            if (o->active > 1u || o->pad != 0u || o->pad2 != 0u) { cellulogonic_destroy(w); return NULL; }
            if (o->active == 0u) {
                for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                    if (o->mol[s] != 0) { cellulogonic_destroy(w); return NULL; }
                }
                if (o->pmf != 0) { cellulogonic_destroy(w); return NULL; }
                continue;
            }
            org_active++;
            if (o->org_type > 1u || o->nesting_depth < 1u || o->nesting_depth > 2u ||
                o->c_ring_nH < 2u || o->c_ring_nH > 5u ||
                o->antiporter_share > 100u || o->lipid_class >= CELL_SOLVENT_COUNT ||
                o->pmf < 0 || o->pmf > MAX_MOL) {
                cellulogonic_destroy(w);
                return NULL;
            }
            for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
                if (o->mol[s] < 0 || o->mol[s] > MAX_MOL) { cellulogonic_destroy(w); return NULL; }
            }
        }
        if (org_active != c->num_organelles) { cellulogonic_destroy(w); return NULL; }

        /* Bonds: bounds, no self-bonds, no duplicates, active partner, exact symmetry */
        uint8_t bond_cnt = 0u;
        for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
            int16_t pj = c->bond_partners[b];
            if (pj < -1 || pj >= (int16_t)w->num_slots) { cellulogonic_destroy(w); return NULL; }
            if (pj < 0) continue;
            if (pj == (int16_t)i || w->cells[pj].alive_state == 0u) { cellulogonic_destroy(w); return NULL; }
            for (uint32_t b2 = 0; b2 < b; ++b2) {
                if (c->bond_partners[b2] == pj) { cellulogonic_destroy(w); return NULL; }
            }
            int sym = 0;
            for (uint32_t ob = 0; ob < CELL_MAX_BONDS; ++ob) {
                if (w->cells[pj].bond_partners[ob] == (int16_t)i) { sym = 1; break; }
            }
            if (!sym) { cellulogonic_destroy(w); return NULL; }
            bond_cnt++;
        }
        if (bond_cnt != c->num_bonds) { cellulogonic_destroy(w); return NULL; }

        /* Axons: bounds, no self-axons, no duplicate targets, live target, 0 weight when unwired */
        for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
            int16_t tj = c->axon_target[a];
            if (tj < -1 || tj >= (int16_t)w->num_slots) { cellulogonic_destroy(w); return NULL; }
            if (tj < 0) {
                if (c->axon_weight[a] != 0u) { cellulogonic_destroy(w); return NULL; }
                continue;
            }
            if (tj == (int16_t)i || w->cells[tj].alive_state == 0u) { cellulogonic_destroy(w); return NULL; }
            for (uint32_t a2 = 0; a2 < a; ++a2) {
                if (c->axon_target[a2] == tj) { cellulogonic_destroy(w); return NULL; }
            }
        }
    }

    w->cfg                     = hdr.cfg;
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
    w->axons_wired_cum               = hdr.counters[18];
    w->axons_pruned_cum              = hdr.counters[19];
    w->hippo_arrests_cum             = hdr.counters[20];
    w->eutelic_stops_cum             = hdr.counters[21];
    w->cpg_pulses_cum                = hdr.counters[22];

    /* Verify exact physical mass & First-Law energy closure (0 amu / 0 uq!) before accepting snapshot */
    CellLedgerSnapshot led;
    cell_compute_ledger(w, &led);
    if (led.mass_residual_amu != 0 || led.int_ledger_residual_uq != 0) {
        cellulogonic_destroy(w);
        return NULL;
    }

    w->post_warmup_allocs = 0;
    return w;
}

/* ============================================================================
 * Conservation-Closed Perturbations: Mass Extinction Lysis & Tardigrade Tun!
 * ============================================================================ */

CELL_API uint32_t cellulogonic_apply_shock_lysis(CellWorld* w, uint32_t kill_percent) {
    if (!w) return 0;
    if (kill_percent > 100u) kill_percent = 100u;
    uint32_t killed = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        /* Vitrified TDP bioglass tun cells resist shock lysis by 90%! (Lean T42) */
        uint32_t eff_kill = (c->alive_state == 3) ? (kill_percent / 10u) : kill_percent;
        uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_WORLD_EXOG, w->epoch + 999ULL, i);
        if ((roll % 100u) < eff_kill) {
            cell_lyse_to_voxel(w, i, 0, 0);
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
        uint32_t vidx = cell_voxel_idx(w, c);
        if (c->genome[122] >= 110 && c->mol[CELL_SP_P] >= 2) {
            if (c->pmf > 0) {
                w->voxels[vidx].heat_local_uq += c->pmf * sp->e_pmf;
                c->pmf = 0;
            }
            /* Also dissipate all nested organelle PMF into local heat on tun vitrification (0 uq drift!) */
            for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
                if (c->org[k].active && c->org[k].pmf > 0) {
                    w->voxels[vidx].heat_local_uq += c->org[k].pmf * sp->e_pmf;
                    c->org[k].pmf = 0;
                }
            }
            /* Detach bonds & axons cleanly on tun entry so no stale tissue links remain (C14) */
            cell_clear_bonds_and_axons(w, i);
            c->alive_state = 3;
            c->mt_coherence_q10 = 0;
            c->breath_amp_q10   = 0;
            c->flagella_q10     = 0;
            vitrified++;
        } else {
            uint32_t roll = cell_philox_u32(w->cfg.seed, RNG_DOMAIN_WORLD_EXOG, w->epoch + 777ULL, i);
            if ((roll % 100u) < severity_pct) {
                cell_lyse_to_voxel(w, i, 0, 0);
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
 * Read-Only 3D Off-Lattice Organic Export for Live WebGL2 Microscopy Observatory (P9 Safe)
 * Exports: living cells with exact continuous 3D (x,y,z), velocity, heading, radius,
 * shape elongation, amoeboid ruffling, breathing phase/amp, GHz MT frequency & coherence,
 * flagella vigor, organelles, bioelectric V_m, PLUS exact organic 3D colony bonds!
 * ============================================================================ */

CELL_API uint32_t cellulogonic_export_grid_json(const CellWorld* w, char* out_buf, size_t buf_cap) {
    if (!w || !out_buf || buf_cap < 512) return 0;
    size_t pos = 0;
    int32_t span_x = (int32_t)w->cfg.width  * CELL_VOXEL_NM;
    int32_t span_y = (int32_t)w->cfg.height * CELL_VOXEL_NM;
    int32_t span_z = (int32_t)w->cfg.width  * CELL_VOXEL_NM;
    uint32_t pstep_x = w->cfg.width  >= 32u ? (w->cfg.width  / 16u) : 1u;
    uint32_t pstep_y = w->cfg.height >= 32u ? (w->cfg.height / 16u) : 1u;
    uint32_t plume_w = (w->cfg.width  + pstep_x - 1u) / pstep_x;
    uint32_t plume_h = (w->cfg.height + pstep_y - 1u) / pstep_y;
    int n = snprintf(out_buf + pos, buf_cap - pos,
                     "{\"w\":%u,\"h\":%u,\"span\":%d,\"span_x\":%d,\"span_y\":%d,\"span_z\":%d,"
                     "\"plume_w\":%u,\"plume_h\":%u,\"epoch\":%llu,\"solvent\":%u,\"cells\":[",
                     w->cfg.width, w->cfg.height, span_x, span_x, span_y, span_z,
                     plume_w, plume_h, (unsigned long long)w->epoch, w->cfg.solvent);
    if (n < 0 || (size_t)n >= buf_cap - pos) return 0;
    pos += (size_t)n;

    int first = 1;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        uint8_t sec = 0, mito = 0, chlo = 0;
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (c->org[k].active) {
                if (c->org[k].nesting_depth >= 2) sec = 1;
                if (c->org[k].org_type == 0) mito++; else chlo++;
            }
        }
        int64_t vol = cell_internal_volume(c);
        /* Compact 24-field 3D biophysical + morphogenetic GRN cell tuple */
        uint32_t morph_sig = ((uint32_t)c->genome[3] << 16) | ((uint32_t)c->genome[35] << 8) | (uint32_t)c->genome[67];
        uint32_t grn_pack  = ((uint32_t)(c->morph_fate & 7u)) |
                             ((uint32_t)(c->cad_type & 3u) << 4) |
                             ((uint32_t)(c->cpg_firing & 1u) << 8) |
                             ((uint32_t)((c->apical_wedge_q10 >> 4) & 63u) << 9);
        int m = snprintf(out_buf + pos, buf_cap - pos,
                         "%s[%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%d,%lld,%lld,%u,%u,%u,%u]",
                         (first ? "" : ","),
                         i,
                         (unsigned)c->alive_state,
                         c->pos_nm[0], c->pos_nm[1], c->pos_nm[2],
                         c->heading_q10[0], c->heading_q10[1], c->heading_q10[2],
                         c->radius_nm, c->elong_q10, c->ruffle_q10,
                         c->breath_phase_mrad, c->breath_amp_q10,
                         c->mt_freq_mhz, (unsigned)c->mt_coherence_q10,
                         (unsigned)((mito & 0xFu) | ((chlo & 0xFu) << 4) | ((sec & 1u) << 8) |
                                    ((c->lipid_class & 3u) << 9) | ((c->mt_filaments & 0x1Fu) << 11)),
                         (unsigned)c->flagella_q10,
                         c->v_mem_mv,
                         (long long)vol,
                         (long long)c->mol[CELL_SP_L],
                         c->lineage_id,
                         morph_sig,
                         grn_pack,
                         (unsigned)c->eutelic_rem);
        if (m < 0 || pos + (size_t)m + 128 >= buf_cap) break;
        pos += (size_t)m;
        first = 0;
    }

    int bstart = snprintf(out_buf + pos, buf_cap - pos, "],\"bonds\":[");
    if (bstart < 0 || pos + (size_t)bstart >= buf_cap) return 0;
    pos += (size_t)bstart;

    int bfirst = 1, btrunc = 0;
    for (uint32_t i = 0; i < w->num_slots && !btrunc; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
            int16_t j = c->bond_partners[b];
            if (j >= 0 && (uint32_t)j > i && w->cells[j].alive_state == 1) {
                int bm = snprintf(out_buf + pos, buf_cap - pos, "%s[%u,%d,%u]",
                                  (bfirst ? "" : ","), i, (int)j,
                                  (unsigned)(c->cad_type == w->cells[j].cad_type ? 1u : 0u));
                if (bm < 0 || pos + (size_t)bm + 96 >= buf_cap) { btrunc = 1; break; }
                pos += (size_t)bm;
                bfirst = 0;
            }
        }
    }

    /* Live 3D Wired Axons & Firing CPG Synapses: [src_id, dst_id, weight, firing] */
    int astart = snprintf(out_buf + pos, buf_cap - pos, "],\"axons\":[");
    if (astart < 0 || pos + (size_t)astart >= buf_cap) return 0;
    pos += (size_t)astart;

    int afirst = 1, atrunc = 0;
    for (uint32_t i = 0; i < w->num_slots && !atrunc; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
            int16_t tj = c->axon_target[a];
            if (tj >= 0 && (uint32_t)tj < w->num_slots && w->cells[tj].alive_state == 1) {
                int am = snprintf(out_buf + pos, buf_cap - pos, "%s[%u,%d,%u,%u]",
                                  (afirst ? "" : ","), i, (int)tj,
                                  (unsigned)c->axon_weight[a], (unsigned)c->cpg_firing);
                if (am < 0 || pos + (size_t)am + 64 >= buf_cap) { atrunc = 1; break; }
                pos += (size_t)am;
                afirst = 0;
            }
        }
    }

    /* Downsampled <=16x16 chemical substrate/oxidant plume + 3D Morphogen GPS (Wnt/BMP/Shh) */
    int fstart = snprintf(out_buf + pos, buf_cap - pos, "],\"plumes\":[");
    if (fstart < 0 || pos + (size_t)fstart >= buf_cap) return 0;
    pos += (size_t)fstart;

    int pfirst = 1, ptrunc = 0;
    for (uint32_t y = 0; y < w->cfg.height && !ptrunc; y += pstep_y) {
        for (uint32_t x = 0; x < w->cfg.width; x += pstep_x) {
            const CellVoxel* vx = &w->voxels[y * w->cfg.width + x];
            int pm = snprintf(out_buf + pos, buf_cap - pos, "%s[%lld,%lld,%lld,%d,%d,%d]",
                              (pfirst ? "" : ","),
                              (long long)vx->env[CELL_SP_S],
                              (long long)vx->env[CELL_SP_X],
                              (long long)vx->quorum,
                              vx->morph[0], vx->morph[1], vx->morph[2]);
            if (pm < 0 || pos + (size_t)pm + 16 >= buf_cap) { ptrunc = 1; break; }
            pos += (size_t)pm;
            pfirst = 0;
        }
    }

    int end = snprintf(out_buf + pos, buf_cap - pos, "]}");
    if (end < 0 || pos + (size_t)end >= buf_cap) return 0;
    pos += (size_t)end;
    return (uint32_t)pos;
}
