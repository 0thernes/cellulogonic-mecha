/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue (Independent Researcher, 0thernes)
 */
#include "cellulogonic_internal.h"

/* ============================================================================
 * Cellular Omega Vitality Index (COVI), 8-Pillar OVBS-52 Metrology,
 * High-Copy Assembly Index, Knockout Active Length L_eff, Heteroplasmy Variance,
 * AUTOPHAGE DNA Ingestion Bridge & Asmodensius 112-Trait Bridge
 *
 * STRICTLY READ-ONLY / OBSERVER-INDEPENDENT (Clean-Slate P9)!
 * ============================================================================ */

static void compute_colony_stats(const CellWorld* w, uint32_t* out_max_sz,
                                 uint32_t* out_fewcell, uint32_t* out_multicell) {
    /* Strictly const/read-only (Clean-Slate P9): local stack arrays up to 4096 cells, 0 alloc! */
    const uint32_t N = w->num_slots;
    *out_max_sz = 0; *out_fewcell = 0; *out_multicell = 0;
    if (N > 4096u) return;

    uint8_t  vis[4096];
    uint32_t q[4096];
    memset(vis, 0, N);

    for (uint32_t i = 0; i < N; ++i) {
        if (vis[i] || w->cells[i].alive_state != 1 || w->cells[i].num_bonds == 0) continue;
        uint32_t head = 0, tail = 0;
        vis[i] = 1;
        q[tail++] = i;
        while (head < tail) {
            uint32_t u = q[head++];
            const CellEntity* cu = &w->cells[u];
            for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
                int16_t pj = cu->bond_partners[b];
                if (pj >= 0 && (uint32_t)pj < N) {
                    uint32_t v = (uint32_t)pj;
                    if (!vis[v] && w->cells[v].alive_state == 1) {
                        vis[v] = 1;
                        q[tail++] = v;
                    }
                }
            }
        }
        if (tail > *out_max_sz) *out_max_sz = tail;
        if (tail >= 2u && tail <= 16u) (*out_fewcell)++;
        else if (tail > 16u)           (*out_multicell)++;
    }
}

/* Estimate functional operon active length L_eff (0..16 loci gated on active living catalysis,
 * excluding dead vesicles, template-free micelles, and metabolically arrested spores/tuns) */
static uint32_t cell_active_loci_count(const CellEntity* c) {
    if (c->alive_state != 1 || c->mol[CELL_SP_P] == 0 || c->mol[CELL_SP_T] == 0) return 0u;
    int64_t met_flux = c->mol[CELL_SP_S] + c->mol[CELL_SP_M] + c->mol[CELL_SP_E];
    uint32_t active = 0;
    if (c->genome[0]  > 80  && met_flux > 0) active++; /* R1 catabolism */
    if (c->genome[8]  > 80  && (c->mol[CELL_SP_M] > 0 || c->pmf > 0 || c->mol[CELL_SP_E] > 0)) active++; /* R2 proton pump */
    if (c->genome[17] > 80  && (c->pmf > 0 || c->mol[CELL_SP_E] > 0)) active++; /* R3 ATP synthase */
    if (c->genome[24] > 90  && met_flux > 0) active++; /* R4 fermentation */
    if (c->genome[32] > 80  && c->mol[CELL_SP_L] > 0) active++; /* R5 lipid synthesis */
    if (c->genome[40] > 90  && c->mol[CELL_SP_P] > 0) active++; /* R6 translation */
    if (c->genome[48] > 80  && c->mol[CELL_SP_T] > 0) active++; /* R7 template replication */
    if (c->genome[56] > 125 && c->mol[CELL_SP_P] > 0) active++; /* R14 autotrophy */
    if (c->genome[57] > 120 && c->mol[CELL_SP_P] > 0) active++; /* R14b oxygenesis */
    if (c->genome[72] > 90  && c->mol[CELL_SP_L] > 0) active++; /* S permease */
    if (c->genome[80] > 150 && c->mol[CELL_SP_P] > 0) active++; /* phagocytosis */
    if (c->genome[84] > 115 && c->num_organelles > 0) active++; /* mitophagy */
    if (c->genome[88] > 140 && c->mol[CELL_SP_L] > 0) active++; /* asymmetric budding */
    if (c->genome[98] > 64  && c->num_bonds > 0)      active++; /* gap-junction connexin */
    if (c->genome[105]> 115 && c->num_bonds > 0)      active++; /* tissue adhesion */
    if (c->genome[122]> 110 && c->mol[CELL_SP_P] >= 2)active++; /* TDP/CAHS bioglass protectant */
    return active;
}

/* Genuine non-overlapping LZ77 motif reuse (rejecting trivial constant homopolymers!)
 * + nested organelle compositional Assembly Index A */
static double cell_assembly_index(const CellEntity* c) {
    uint32_t reuse_hits = 0u, covered = 0u;
    size_t i = 0u;
    while (i < CELL_GENOME_BYTES) {
        size_t best_len = 0u;
        for (size_t prev = 0u; prev < i; ++prev) {
            size_t m = 0u;
            while (prev + m < i && i + m < CELL_GENOME_BYTES && m < 16u &&
                   c->genome[prev + m] == c->genome[i + m]) {
                m++;
            }
            if (m >= 2u) {
                /* Reject trivial homopolymer repeats (e.g., 0x00, 0x00, ...): require >= 2 distinct bytes */
                int diverse = 0;
                uint8_t b0 = c->genome[i];
                for (size_t u = 1u; u < m; ++u) {
                    if (c->genome[i + u] != b0) { diverse = 1; break; }
                }
                if (diverse && m > best_len) best_len = m;
            }
        }
        if (best_len >= 2u) {
            reuse_hits++;
            covered += (uint32_t)best_len;
            i += best_len;
        } else {
            i++;
        }
    }
    uint32_t loci = cell_active_loci_count(c);
    double a = (double)loci * 0.90 + (double)reuse_hits * 0.35 + (double)covered * 0.08;
    if (c->num_organelles > 0) a += 4.0 + (double)c->num_organelles * 1.5;
    for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
        if (c->org[k].active && c->org[k].nesting_depth >= 2) {
            a += 5.0; /* Secondary chimeric endosymbiont adds compositional hierarchy */
            break;
        }
    }
    if (c->bond_mask != 0) a += 2.5;
    if (c->egt_genes > 0)  a += 1.5;
    return a;
}

static uint32_t lineage_table_find_or_insert(uint32_t hashes[4096], uint32_t counts[4096],
                                             uint32_t lineage_id, uint32_t* distinct_out) {
    uint32_t slot = lineage_id & 4095u;
    for (uint32_t probe = 0; probe < 4096u; ++probe) {
        uint32_t idx = (slot + probe) & 4095u;
        if (counts[idx] == 0u) {
            hashes[idx] = lineage_id;
            counts[idx] = 1u;
            if (distinct_out) (*distinct_out)++;
            return idx;
        }
        if (hashes[idx] == lineage_id) {
            counts[idx]++;
            return idx;
        }
    }
    return slot;
}

static uint32_t lineage_table_lookup(const uint32_t hashes[4096], const uint32_t counts[4096],
                                     uint32_t lineage_id) {
    uint32_t slot = lineage_id & 4095u;
    for (uint32_t probe = 0; probe < 4096u; ++probe) {
        uint32_t idx = (slot + probe) & 4095u;
        if (counts[idx] == 0u) return 0u;
        if (hashes[idx] == lineage_id) return counts[idx];
    }
    return 0u;
}

static double clamp100(double x) {
    if (x < 0.0) return 0.0;
    if (x > 100.0) return 100.0;
    return x;
}

void cell_compute_telemetry(const CellWorld* w, CellTelemetry* out) {
    memset(out, 0, sizeof(*out));
    out->epoch              = w->epoch;
    out->total_cell_steps   = w->total_cell_steps;
    out->total_reactions    = w->total_reactions;
    out->post_warmup_allocs = w->post_warmup_allocs;

    cell_compute_ledger(w, &out->ledger);

    const uint32_t N = w->num_slots;
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    uint32_t lineage_hashes[4096];
    uint32_t lineage_counts[4096];
    memset(lineage_hashes, 0, sizeof(lineage_hashes));
    memset(lineage_counts, 0, sizeof(lineage_counts));

    double sum_v = 0.0, sum_v2 = 0.0;
    double sum_prok_vol = 0.0, sum_euk_vol = 0.0;
    double max_vol = 0.0, sum_Q = 0.0, sum_cring = 0.0;
    double sum_leff = 0.0, max_A = 0.0;
    double sum_amb_x = 0.0;
    double sum_mt_ghz = 0.0, sum_mt_coh = 0.0, sum_speed = 0.0;
    double sum_apical = 0.0, sum_wnt = 0.0, sum_bmp = 0.0, sum_shh = 0.0;
    uint32_t total_bond_edges = 0, homo_bond_edges = 0;
    double sum_coop_frac = 0.0, sum_coop_frac2 = 0.0;
    uint32_t euk_with_org = 0;
    uint32_t active_catalysts = 0;
    uint32_t template_carriers = 0;

    for (uint32_t i = 0; i < N; ++i) {
        sum_amb_x += (double)w->voxels[i].env[CELL_SP_X];
        sum_wnt   += (double)w->voxels[i].morph[0];
        sum_bmp   += (double)w->voxels[i].morph[1];
        sum_shh   += (double)w->voxels[i].morph[2];
        const CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;

        if (c->alive_state == 2) out->spore_cells++;
        if (c->alive_state == 3) out->vitrified_tun_cells++;
        out->alive_cells++;

        (void)lineage_table_find_or_insert(lineage_hashes, lineage_counts, c->lineage_id, &out->distinct_lineages);

        int64_t vol = cell_internal_volume(c);
        double dvol = (double)vol;
        if (dvol > max_vol) max_vol = dvol;
        sum_Q += cell_reduced_Q(vol, c->mol[CELL_SP_L]);
        sum_cring += (double)c->c_ring_nH;
        sum_v  += (double)c->v_mem_mv;
        sum_v2 += (double)c->v_mem_mv * (double)c->v_mem_mv;
        sum_mt_ghz += (double)c->mt_freq_mhz / 1000.0;
        sum_mt_coh += (double)c->mt_coherence_q10 / 1000.0;
        double spd = sqrt((double)c->vel_nm[0] * c->vel_nm[0] +
                          (double)c->vel_nm[1] * c->vel_nm[1] +
                          (double)c->vel_nm[2] * c->vel_nm[2]) / 100.0;
        sum_speed += spd;

        uint32_t leff = cell_active_loci_count(c);
        sum_leff += (double)leff;
        if (c->alive_state == 1 && c->mol[CELL_SP_T] > 0) template_carriers++;
        if (c->mol[CELL_SP_P] > 0 && leff >= 4) active_catalysts++;

        if (c->lipid_class == sp->optimal_lipid) out->solvent_adapted_cells++;
        if (c->alive_state == 1 && c->genome[56] > 125) out->autotroph_cells++;
        if (c->alive_state == 1 && c->c_ring_nH >= sp->min_cring && c->genome[8] > 80) out->respiratory_cells++;

        /* Organelle & Eukaryogenesis Accounting */
        uint32_t org_here = 0, coop_here = 0, sec_here = 0;
        for (uint32_t k = 0; k < CELL_MAX_ORGANELLES; ++k) {
            if (!c->org[k].active) continue;
            org_here++;
            out->total_organelles++;
            if (c->org[k].org_type == 0) out->mitoch_organelles++;
            else                         out->chloro_organelles++;
            if (c->org[k].antiporter_share == 0) out->selfish_organelles++;
            else                                 coop_here++;
            if (c->org[k].nesting_depth >= 2) sec_here = 1;
        }
        if (org_here == 0) {
            out->prokaryote_cells++;
            sum_prok_vol += dvol;
        } else {
            out->eukaryote_cells++;
            sum_euk_vol += dvol;
            if (sec_here) out->secondary_chimeras++;
            double cf = (double)coop_here / (double)org_here;
            sum_coop_frac  += cf;
            sum_coop_frac2 += cf * cf;
            euk_with_org++;
        }

        /* Emergent Multicellular Tissue Topology, Bioelectric Roles & Developmental GRN Fates */
        if (c->morph_fate < CELL_FATE_COUNT) out->fate_counts[c->morph_fate]++;
        sum_apical += (double)c->apical_wedge_q10 / 1000.0;
        for (uint32_t a = 0; a < CELL_MAX_AXONS; ++a) {
            if (c->axon_target[a] >= 0 && (uint32_t)c->axon_target[a] < N &&
                w->cells[c->axon_target[a]].alive_state == 1) {
                out->active_axons++;
            }
        }
        if (c->alive_state == 1 && c->num_bonds > 0) {
            out->multicellular_cells++;
            if (c->num_bonds < 4u) out->role_boundary_epi++;
            for (uint32_t b = 0; b < CELL_MAX_BONDS; ++b) {
                int16_t pj = c->bond_partners[b];
                if (pj >= 0 && (uint32_t)pj < N && (uint32_t)pj > i && w->cells[pj].alive_state == 1) {
                    total_bond_edges++;
                    if (c->cad_type == w->cells[pj].cad_type) homo_bond_edges++;
                }
            }
        }
        if (c->diff_role == 1)      out->role_soma_forage++;
        else if (c->diff_role == 2) out->role_germ_repro++;
        else if (c->diff_role == 3) out->role_stroma_interior++;
    }

    /* High-copy (N >= 4) Assembly Index to filter single-copy random noise */
    for (uint32_t i = 0; i < N; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state == 0) continue;
        if (lineage_table_lookup(lineage_hashes, lineage_counts, c->lineage_id) >= 4u) {
            double a = cell_assembly_index(c);
            if (a > max_A) max_A = a;
        }
    }

    compute_colony_stats(w, &out->max_tissue_size, &out->fewcell_colonies, &out->multicell_colonies);
    out->abiogenesis_vesicles          = w->abiogenesis_vesicles;
    out->symmetric_fissions            = w->symmetric_fissions;
    out->asymmetric_buddings           = w->asymmetric_buddings;
    out->osmotic_lyses                 = w->osmotic_lyses;
    out->phagocytic_digestions         = w->phagocytic_digestions;
    out->endosymbiosis_captures        = w->endosymbiosis_captures;
    out->secondary_captures            = w->secondary_captures;
    out->egt_transfer_events           = w->egt_transfer_events;
    out->mitophagy_events              = w->mitophagy_events;
    out->hgt_uptake_events             = w->hgt_uptake_events;
    out->phage_lysis_events            = w->phage_lysis_events;
    out->cancer_defections             = w->cancer_defections;
    out->apoptotic_policings           = w->apoptotic_policings;
    out->backlash_killswitch_deletions = w->backlash_killswitch_deletions;
    out->backlash_dormancy_spores      = w->backlash_dormancy_spores;
    out->backlash_hgt_smuggles         = w->backlash_hgt_smuggles;
    out->backlash_metabolic_bypasses   = w->backlash_metabolic_bypasses;
    out->axons_wired_cum               = w->axons_wired_cum;
    out->axons_pruned_cum              = w->axons_pruned_cum;
    out->hippo_arrests_cum             = w->hippo_arrests_cum;
    out->eutelic_stops_cum             = w->eutelic_stops_cum;
    out->cpg_pulses_cum                = w->cpg_pulses_cum;
    out->homophilic_bond_pct           = total_bond_edges > 0
                                         ? (100.0 * (double)homo_bond_edges / (double)total_bond_edges)
                                         : 0.0;
    out->ambient_oxidant_X             = sum_amb_x / (double)(N > 0 ? N : 1);
    out->mean_morph_wnt                = sum_wnt / (double)(N > 0 ? N : 1);
    out->mean_morph_bmp                = sum_bmp / (double)(N > 0 ? N : 1);
    out->mean_morph_shh                = sum_shh / (double)(N > 0 ? N : 1);

    if (out->alive_cells > 0) {
        double na = (double)out->alive_cells;
        out->mean_v_mem_mv         = sum_v / na;
        double var_v               = (sum_v2 / na) - (out->mean_v_mem_mv * out->mean_v_mem_mv);
        out->std_v_mem_mv          = var_v > 0.0 ? sqrt(var_v) : 0.0;
        out->mean_prok_volume      = out->prokaryote_cells > 0 ? (sum_prok_vol / (double)out->prokaryote_cells) : 0.0;
        out->mean_euk_volume       = out->eukaryote_cells  > 0 ? (sum_euk_vol  / (double)out->eukaryote_cells)  : 0.0;
        out->max_cell_volume       = max_vol;
        out->mean_reduced_Q        = sum_Q / na;
        out->mean_cring_nH         = sum_cring / na;
        out->mean_active_loci_leff = sum_leff / na;
        out->mean_mt_freq_ghz      = sum_mt_ghz / na;
        out->mean_mt_coherence     = sum_mt_coh / na;
        out->mean_swim_speed_ums   = sum_speed / na;
        out->mean_apical_wedge     = sum_apical / na;
    }
    if (euk_with_org > 1) {
        double mcf = sum_coop_frac / (double)euk_with_org;
        double vcf = (sum_coop_frac2 / (double)euk_with_org) - mcf * mcf;
        out->heteroplasmy_variance = vcf > 0.0 ? vcf : 0.0;
    }
    out->max_assembly_index = max_A;
    out->bedau_anew         = (double)w->novel_adaptations_cum / (double)(w->epoch > 0 ? w->epoch : 1);

    /* --- 8-Pillar Cellular Omega Vitality Index (COVI) with Anti-Gaming Gates (Lean T34-T35) --- */
    uint32_t alive = out->alive_cells;
    uint64_t total_divs = out->symmetric_fissions + out->asymmetric_buddings;

    /* Pillar 0: Autopoietic Survival & Membrane Homeostasis */
    out->pillar[0] = clamp100(((double)alive / (double)(N / 2u)) * 75.0 +
                              (total_divs > 0 ? 25.0 : 0.0));
    /* Pillar 1: Chemiosmotic Bioenergetics & Solvent Adaptation */
    out->pillar[1] = alive > 0 ? clamp100(((double)out->respiratory_cells / (double)alive) * 55.0 +
                                          ((double)out->solvent_adapted_cells / (double)alive) * 45.0) : 0.0;
    /* Pillar 2: Open-Ended Lineage & Adaptation Diversity (Bedau A_new) */
    out->pillar[2] = clamp100(log2(1.0 + (double)out->distinct_lineages) * 14.0 +
                              log2(1.0 + (double)w->novel_adaptations_cum) * 5.0);
    /* Pillar 3: Eukaryogenesis & Serial Endosymbiosis (Lane Wall Shattered) */
    out->pillar[3] = clamp100((out->eukaryote_cells > 0 ? 40.0 : 0.0) +
                              (out->secondary_chimeras > 0 ? 25.0 : 0.0) +
                              (out->egt_transfer_events > 0 ? 20.0 : 0.0) +
                              (out->mitophagy_events > 0 ? 15.0 : 0.0));
    /* Pillar 4: Bioelectric Morphogenesis & Multicellular Differentiation */
    out->pillar[4] = alive > 0 ? clamp100(((double)out->multicellular_cells / (double)alive) * 55.0 +
                                          out->std_v_mem_mv * 1.6 +
                                          (out->max_tissue_size >= 16 ? 20.0 : (double)out->max_tissue_size)) : 0.0;
    /* Pillar 5: Compositional Assembly Complexity & Active Operon Length */
    out->pillar[5] = clamp100(out->max_assembly_index * 3.8 + out->mean_active_loci_leff * 2.0);
    /* Pillar 6: Trophic Completeness & Horizontal Genetic Ecology */
    out->pillar[6] = clamp100((out->autotroph_cells > 0 ? 35.0 : 0.0) +
                              (out->hgt_uptake_events > 0 ? 30.0 : 0.0) +
                              (out->asymmetric_buddings > 0 ? 20.0 : 0.0) +
                              (out->phagocytic_digestions > 0 ? 15.0 : 0.0));
    /* Pillar 7: Thermodynamic Closure & Stress Resilience (Spore/Tun Plasticity) */
    out->pillar[7] = (out->ledger.mass_residual_amu == 0 &&
                      out->ledger.int_ledger_residual_uq == 0 &&
                      out->ledger.second_law_violations == 0)
                     ? clamp100(80.0 + ((out->spore_cells + out->vitrified_tun_cells > 0) ? 20.0 : 10.0))
                     : 0.0;

    /* Anti-Gaming Gate (Lean T34 & T35): Zero if inert crystal / no catalysis / no template / no reactions */
    double gamma = 1.0;
    if (alive == 0 || active_catalysts == 0 || template_carriers == 0 || w->total_reactions == 0) gamma = 0.0;
    else if (out->mean_active_loci_leff < 3.0) gamma = 0.25;
    out->gamma_anti_gaming = gamma;

    /* Spiegelman Complexity Collapse Gate */
    double theta = (out->mean_active_loci_leff >= 5.0) ? 1.0 : (out->mean_active_loci_leff / 5.0);
    if (theta > 1.0) theta = 1.0;
    out->theta_spiegelman = theta;

    double w_sum = 0.16 * out->pillar[0] + 0.14 * out->pillar[1] +
                   0.14 * out->pillar[2] + 0.15 * out->pillar[3] +
                   0.13 * out->pillar[4] + 0.12 * out->pillar[5] +
                   0.08 * out->pillar[6] + 0.08 * out->pillar[7];
    out->covi_score = clamp100(w_sum * gamma * theta);

    /* Major Evolutionary Transitions Crossed (0..7) */
    uint32_t met = 0;
    if (alive > 0 && active_catalysts > 0)           met++; /* 1. Compartmentalized protocells */
    if (out->respiratory_cells > 0)                  met++; /* 2. Chemiosmotic proton-motive bioenergetics */
    if (out->autotroph_cells > 0)                    met++; /* 3. Photo/chemoautotrophic fixation */
    if (out->eukaryote_cells > 0)                    met++; /* 4. Primary Eukaryogenesis (Endosymbiosis) */
    if (out->secondary_chimeras > 0 || out->egt_transfer_events > 0) met++; /* 5. Secondary Chimeras / EGT */
    if (out->multicellular_cells >= 16)              met++; /* 6. Bioelectric Multicellular Tissue */
    if (out->role_soma_forage > 0 && out->role_germ_repro > 0) met++; /* 7. Soma/Germline Differentiation */
    out->major_transitions_crossed = met;

    if (out->covi_score < 5.0)       out->covi_stage = 0;
    else if (out->covi_score < 25.0) out->covi_stage = 1;
    else if (out->covi_score < 45.0) out->covi_stage = 2;
    else if (out->covi_score < 62.0) out->covi_stage = 3;
    else if (out->covi_score < 76.0) out->covi_stage = 4;
    else if (out->covi_score < 88.0) out->covi_stage = 5;
    else                             out->covi_stage = 6;

    cellulogonic_state_sha256(w, NULL, out->state_sha256);
}

/* ============================================================================
 * Ecosystem Bridge 1: Locality-Preserving AUTOPHAGE 128B / 256B DNA Ingestion
 * Decodes AUTOPHAGE dual-strand DNA smoothly & Lipschitz-continuously into the
 * 16 cellular operons (ZERO cryptographic XOR avalanche on operon bytes!), using
 * SHA-256 strictly for `autophage_dna_sig` provenance while preserving baseline
 * metabolic & solvent viability so transfected AUTOPHAGE lineages can compete & win!
 * ============================================================================ */

static void apply_smooth_autophage_operons(CellWorld* w, CellEntity* c, const uint8_t dna128[128]) {
    const SolventParams* sp = &SOLVENT_TABLE[w->cfg.solvent];

    /* 1-Lipschitz direct operon mapping (zero avalanche scrambling!) */
    for (uint32_t i = 0; i < CELL_GENOME_BYTES; ++i) {
        c->genome[i] = dna128[i];
    }

    /* Smooth 0.5-Lipschitz affine viability envelopes for essential housekeeping loci
     * so transfected AUTOPHAGE DNA preserves catalytic viability without destroying variation */
    c->genome[0]  = (uint8_t)(128u + (dna128[0]  >> 1)); /* R1 catabolism      [128..255] */
    c->genome[8]  = (uint8_t)(128u + (dna128[8]  >> 1)); /* R2 proton pump     [128..255] */
    c->genome[17] = (uint8_t)(128u + (dna128[17] >> 1)); /* R3 ATP synthase    [128..255] */
    c->genome[32] = (uint8_t)(144u + (dna128[32] >> 2) + (dna128[32] >> 3)); /* R5 lipid [144..239] */
    c->genome[40] = (uint8_t)(120u + (dna128[40] >> 1)); /* R6 translation     [120..247] */
    c->genome[48] = (uint8_t)(120u + (dna128[48] >> 1)); /* R7 replication     [120..247] */
    c->genome[72] = (uint8_t)(128u + (dna128[72] >> 1)); /* S permease         [128..255] */
    c->genome[73] = (uint8_t)(128u + (dna128[73] >> 1)); /* X permease         [128..255] */

    /* Ensure host-solvent baseline viability while retaining full operon individuality */
    cell_decode_genome_traits(c);
    if (c->c_ring_nH < sp->min_cring) {
        cell_encode_cring_nH(c, sp->min_cring);
    }
    if (c->lipid_class != sp->optimal_lipid) {
        cell_encode_lipid_class(c, sp->optimal_lipid);
    }
    cell_decode_genome_traits(c);
}

CELL_API int cellulogonic_ingest_autophage_128b(
    CellWorld* w, const uint8_t dna128[128], uint32_t target_slot)
{
    if (!w || !dna128 || target_slot >= w->num_slots) return -1;
    CellEntity* c = &w->cells[target_slot];
    if (c->alive_state == 0) return -2;

    /* SHA-256 used strictly for cryptographic provenance signature (NEVER XORed into operons!) */
    uint8_t digest[32];
    CellSha256Ctx sha;
    cell_sha256_init(&sha);
    cell_sha256_update(&sha, dna128, 128);
    cell_sha256_final(&sha, digest);

    c->autophage_dna_sig = cell_load_le64(digest);

    apply_smooth_autophage_operons(w, c, dna128);
    return 0;
}

CELL_API int cellulogonic_ingest_autophage_genome(
    CellWorld* w, uint32_t slot, const uint8_t autophage_256b[256])
{
    if (!w || !autophage_256b || slot >= w->num_slots) return -1;
    CellEntity* c = &w->cells[slot];
    if (c->alive_state == 0) return -2;

    /* Sanitize non-deterministic RDTSC bytes at [0xE0..0xE7] if a raw 256B AutophageCell struct is passed */
    uint8_t clean_256b[256];
    memcpy(clean_256b, autophage_256b, 256u);
    memset(&clean_256b[0xE0], 0, 8u);

    /* SHA-256 strictly for provenance signature */
    uint8_t digest[32];
    CellSha256Ctx sha;
    cell_sha256_init(&sha);
    cell_sha256_update(&sha, clean_256b, 256);
    cell_sha256_final(&sha, digest);

    c->autophage_dna_sig = cell_load_le64(digest);

    /* Smooth 0.5-Lipschitz dual-strand arithmetic fold (ZERO XOR avalanche!) */
    uint8_t folded_128b[128];
    for (uint32_t i = 0; i < CELL_GENOME_BYTES; ++i) {
        folded_128b[i] = (uint8_t)(((uint32_t)clean_256b[i] + (uint32_t)clean_256b[128u + i]) >> 1);
    }
    apply_smooth_autophage_operons(w, c, folded_128b);
    return 0;
}

static inline float cell_clamp01f(double v) {
    if (v != v) return 0.0f;
    if (v < 0.0) return 0.0f;
    if (v > 1.0) return 1.0f;
    return (float)v;
}

/* ============================================================================
 * Ecosystem Bridge 2: Asmodensius 112-Trait Deterministic Physiological Bridge
 * Projects a Cellulogonic Mecha v1.5.0 specimen strictly into [0.0, 1.0]^112!
 *   0..7   : Core bioenergetics, reduced Q, endosymbionts, V_mem, solvent/c-ring, role
 *   8..15  : Conserved molecular pools mol[0..7]
 *   16..23 : v1.1–v1.5 cytoskeleton, GHz microtubules, 3D swim speed, GRN fate/cadherin/axons
 *   24..111: Exact smooth operon projection normalized cleanly by / 1020.0
 * ============================================================================ */

CELL_API void cellulogonic_project_asmodensius_112(
    const CellSpecimen* spec, float out_traits_112[112])
{
    if (!spec || !out_traits_112) return;
    /* Traits 0..15: Strictly clamped [0.0f, 1.0f] physiological & bioenergetic observables
     * (all arithmetic promoted to double BEFORE add/sub so arbitrary fuzzed structs never UB!) */
    out_traits_112[0]  = cell_clamp01f((double)spec->total_volume / 250.0);
    out_traits_112[1]  = cell_clamp01f(spec->reduced_Q / 1.5);
    out_traits_112[2]  = cell_clamp01f((double)spec->num_organelles / (double)CELL_MAX_ORGANELLES);
    out_traits_112[3]  = cell_clamp01f((double)spec->max_nesting_depth / 2.0);
    out_traits_112[4]  = cell_clamp01f(((double)spec->v_mem_mv + 90.0) / 130.0);
    out_traits_112[5]  = cell_clamp01f((double)spec->lipid_class / 3.0);
    out_traits_112[6]  = cell_clamp01f(((double)spec->c_ring_nH - 2.0) / 3.0);
    out_traits_112[7]  = cell_clamp01f((double)spec->differentiation_role / 3.0);
    for (int s = 0; s < CELL_SPECIES_COUNT; ++s) {
        out_traits_112[8 + s] = cell_clamp01f((double)spec->mol[s] / 80.0);
    }

    /* Traits 16..23: v1.1–v1.5 cytoskeletal, GHz microtubule, 3D motility & morphogenetic observables */
    double speed_nm = sqrt((double)spec->vel_nm[0] * (double)spec->vel_nm[0] +
                           (double)spec->vel_nm[1] * (double)spec->vel_nm[1] +
                           (double)spec->vel_nm[2] * (double)spec->vel_nm[2]);
    out_traits_112[16] = cell_clamp01f(((double)spec->radius_nm - 220.0) / 1260.0);
    out_traits_112[17] = cell_clamp01f(((double)spec->elong_q10 - 650.0) / 1500.0);
    out_traits_112[18] = cell_clamp01f(((double)spec->mt_freq_mhz - 1200.0) / 13600.0);
    out_traits_112[19] = cell_clamp01f((double)spec->mt_coherence_q10 / 1000.0);
    out_traits_112[20] = cell_clamp01f(speed_nm / 800.0);
    out_traits_112[21] = cell_clamp01f((double)spec->morph_fate / 5.0);
    out_traits_112[22] = cell_clamp01f((double)spec->cad_type / 3.0);
    out_traits_112[23] = cell_clamp01f(((double)spec->active_axons / (double)CELL_MAX_AXONS) * 0.5 +
                                       ((double)spec->apical_wedge_q10 / 1000.0) * 0.5);

    /* Traits 24..111: Exact deterministic projection from the 128-byte catalytic operon (max 255*3+255 = 1020) */
    for (int i = 0; i < 88; ++i) {
        uint8_t b0 = spec->genome[i];
        uint8_t b1 = spec->genome[(i + 32) & 127];
        out_traits_112[24 + i] = cell_clamp01f((double)((uint32_t)b0 * 3u + (uint32_t)b1) / 1020.0);
    }
}
