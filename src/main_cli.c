/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue (Independent Researcher, 0thernes)
 */
#include "cellulogonic_internal.h"

static const char* SOLVENT_NAMES[CELL_SOLVENT_COUNT] = {
    "TERRAN_H2O", "TITAN_AZOTOSOME", "VENT_TETRAETHER", "VENUS_H2SO4"
};

static const char* RESTRAINT_NAMES[CELL_RESTRAINT_COUNT] = {
    "NERFING", "MUZZLING", "SANDBOXING", "SILOING", "DELIMITING",
    "DEGRESSING", "SLOWING", "SPEEDBUMPING", "BOXING", "LOBOTOMIZING",
    "MEDICATING", "DEGRADING", "MINIMIZING"
};

static void print_telemetry_line(const CellTelemetry* t) {
    printf("t=%4llu | alive=%4u (prok=%3u euk=%3u sec=%2u spore=%2u tun=%3u) | org=%3u (mito=%3u chlo=%2u) | "
           "tis=%3u maxT=%3u | sym=%4llu bud=%3llu endo=%3llu | COVI=%5.2f (S%u, MET=%u/7) | "
           "mRes=%lld eRes=%lld 2nd=%u\n",
           (unsigned long long)t->epoch,
           t->alive_cells, t->prokaryote_cells, t->eukaryote_cells,
           t->secondary_chimeras, t->spore_cells, t->vitrified_tun_cells,
           t->total_organelles, t->mitoch_organelles, t->chloro_organelles,
           t->multicellular_cells, t->max_tissue_size,
           (unsigned long long)t->symmetric_fissions,
           (unsigned long long)t->asymmetric_buddings,
           (unsigned long long)t->endosymbiosis_captures,
           t->covi_score, t->covi_stage, t->major_transitions_crossed,
           (long long)t->ledger.mass_residual_amu,
           (long long)t->ledger.int_ledger_residual_uq,
           t->ledger.second_law_violations);
}

static CellWorld* create_world_checked(const CellWorldConfig* cfg) {
    CellWorld* w = cellulogonic_create(cfg);
    if (!w) {
        fprintf(stderr, "FATAL: cellulogonic_create() failed (out of memory)\n");
    }
    return w;
}

static int cmd_probe(void) {
    CellHwCaps caps;
    cellulogonic_probe_hw(&caps);
    printf("=== CELLULOGONIC MECHA v1.7.0 SILICON PROBE ===\n");
    printf("ABI Version : 0x%08x\n", cellulogonic_abi_version());
    printf("CPU Vendor  : %s\n", caps.vendor);
    printf("CPU Brand   : %s\n", caps.brand);
    printf("ISA Flags   : AVX2=%u BMI1=%u BMI2=%u POPCNT=%u RDSEED=%u RDTSCP=%u\n",
           caps.has_avx2, caps.has_bmi1, caps.has_bmi2, caps.has_popcnt,
           caps.has_rdseed, caps.has_rdtscp);
    printf("Compiled Kernels : SIMD_AVX2=%s | SCALAR_REF=ENABLED\n",
#if (defined(__x86_64__) || defined(_M_X64)) && (defined(__GNUC__) || defined(__clang__))
           caps.has_avx2 ? "ENABLED (256-bit int64 finite-volume diffusion, OSXSAVE verified)"
                         : "COMPILED_BUT_RUNTIME_DISABLED (OSXSAVE/AVX2 absent)"
#else
           "FALLBACK_SCALAR"
#endif
    );
    return 0;
}

static int cmd_bench(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== CELLULOGONIC MECHA v1.7.0 DUAL-KERNEL BENCHMARK & PARITY ===\n");
    printf("Grid: %ux%u (%u voxels) | Epochs: %u | Seed: %llu\n\n",
           size, size, size * size, epochs, (unsigned long long)seed);

    char sha_simd[65] = {0}, sha_scal[65] = {0};
    double t_simd = 0.0, t_scal = 0.0;
    CellTelemetry tel_simd, tel_scal;

    for (int k = 0; k < 2; ++k) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width       = size;
        cfg.height      = size;
        cfg.seed        = seed;
        cfg.regime      = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        cfg.kernel_mode = (k == 0) ? CELL_KERNEL_SIMD_AVX2 : CELL_KERNEL_SCALAR_REF;

        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        double t0 = cell_wall_seconds();
        cellulogonic_step_epochs(w, epochs);
        double dt = cell_wall_seconds() - t0;
        if (k == 0) {
            t_simd = dt;
            cellulogonic_get_telemetry(w, &tel_simd);
            memcpy(sha_simd, tel_simd.state_sha256, 65);
        } else {
            t_scal = dt;
            cellulogonic_get_telemetry(w, &tel_scal);
            memcpy(sha_scal, tel_scal.state_sha256, 65);
        }
        cellulogonic_destroy(w);
    }

    double msteps_simd = (double)tel_simd.total_cell_steps / (t_simd > 1e-9 ? t_simd : 1e-9) / 1e6;
    double msteps_scal = (double)tel_scal.total_cell_steps / (t_scal > 1e-9 ? t_scal : 1e-9) / 1e6;
    double mrxn_simd   = (double)tel_simd.total_reactions  / (t_simd > 1e-9 ? t_simd : 1e-9) / 1e6;

    printf("[SIMD_AVX2 ] elapsed=%.4fs | cell_steps=%llu (%.3f M/s) | rxns=%llu (%.3f M/s)\n",
           t_simd, (unsigned long long)tel_simd.total_cell_steps, msteps_simd,
           (unsigned long long)tel_simd.total_reactions, mrxn_simd);
    printf("             SHA-256=%s\n", sha_simd);
    printf("[SCALAR_REF] elapsed=%.4fs | cell_steps=%llu (%.3f M/s) | rxns=%llu\n",
           t_scal, (unsigned long long)tel_scal.total_cell_steps, msteps_scal,
           (unsigned long long)tel_scal.total_reactions);
    printf("             SHA-256=%s\n", sha_scal);
    printf("Dual-Kernel Bit-Exact Parity : %s (Speedup=%.2fx)\n",
           strcmp(sha_simd, sha_scal) == 0 ? "EXACT 0-BIT DIFF MATCH" : "MISMATCH!",
           t_simd > 1e-9 ? t_scal / t_simd : 1.0);
    printf("Ledger Closure: mass_res=%lld amu | energy_res=%lld uq | drift=%.2e | 2nd_law_viol=%u | allocs=%llu\n",
           (long long)tel_simd.ledger.mass_residual_amu,
           (long long)tel_simd.ledger.int_ledger_residual_uq,
           tel_simd.ledger.relative_drift,
           tel_simd.ledger.second_law_violations,
           (unsigned long long)tel_simd.post_warmup_allocs);
    return strcmp(sha_simd, sha_scal) == 0 ? 0 : 1;
}

static int cmd_abiogenesis(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 1: ZERO-SEED SPONTANEOUS PROTOCELL ABIOGENESIS ===\n");
    printf("Initial alive cells: 0 (pure abiotic spatial soup + CMC self-assembly)\n\n");
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width  = size;
    cfg.height = size;
    cfg.seed   = seed;
    cfg.regime = CELL_REGIME_ABIOGENESIS_ZERO_SEED;

    CellWorld* w = create_world_checked(&cfg);
    if (!w) return 2;
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    print_telemetry_line(&t);

    uint32_t step = epochs >= 10u ? epochs / 10u : 1u;
    uint32_t done = 0u;
    while (done < epochs) {
        uint32_t chunk = (epochs - done < step) ? (epochs - done) : step;
        cellulogonic_step_epochs(w, chunk);
        done += chunk;
        cellulogonic_get_telemetry(w, &t);
        print_telemetry_line(&t);
    }
    printf("\nFinal Abiogenesis Summary:\n");
    printf("  CMC Vesicles Condensed : %llu\n", (unsigned long long)t.abiogenesis_vesicles);
    printf("  Alive Cells / Lineages : %u / %u\n", t.alive_cells, t.distinct_lineages);
    printf("  Symmetric / Budding    : %llu / %llu\n",
           (unsigned long long)t.symmetric_fissions, (unsigned long long)t.asymmetric_buddings);
    printf("  High-Copy Assembly A   : %.2f | Mean L_eff=%.2f | Bedau A_new=%.3f\n",
           t.max_assembly_index, t.mean_active_loci_leff, t.bedau_anew);
    printf("  Final COVI             : %.2f (Stage %u) | SHA-256=%s\n",
           t.covi_score, t.covi_stage, t.state_sha256);
    cellulogonic_destroy(w);
    return 0;
}

static int cmd_endosymbiosis(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 2: PROKARYOTIC 1/R CEILING vs. SERIAL ENDOSYMBIOSIS ===\n\n");
    for (int arm = 0; arm < 2; ++arm) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width                = size;
        cfg.height               = size;
        cfg.seed                 = seed;
        cfg.regime               = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        cfg.enable_endosymbiosis = (uint8_t)(arm == 0 ? 1 : 0);

        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        cellulogonic_step_epochs(w, epochs);
        CellTelemetry t;
        cellulogonic_get_telemetry(w, &t);

        printf("[%s]\n", arm == 0 ? "ARM A: ENDOSYMBIOSIS ENABLED (Eukaryogenesis)" :
                                    "ARM B: ENDOSYMBIOSIS ABLATED (Prokaryotes Only)");
        print_telemetry_line(&t);
        printf("  Mean Prok Vol=%.2f | Mean Euk Vol=%.2f | Max Cell Vol=%.2f\n",
               t.mean_prok_volume, t.mean_euk_volume, t.max_cell_volume);
        printf("  Captures=%llu (Secondary Chimeras=%llu) | EGT=%llu | Mitophagy=%llu | HeteroplasmyVar=%.4f\n",
               (unsigned long long)t.endosymbiosis_captures,
               (unsigned long long)t.secondary_captures,
               (unsigned long long)t.egt_transfer_events,
               (unsigned long long)t.mitophagy_events,
               t.heteroplasmy_variance);
        printf("  Ambient Oxidant X=%.2f | Assembly A=%.2f | SHA-256=%s\n\n",
               t.ambient_oxidant_X, t.max_assembly_index, t.state_sha256);
        cellulogonic_destroy(w);
    }
    return 0;
}

static int cmd_xenocell(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 3: 4 PLANETARY / XENOCELLULAR SOLVENTS (DE-NOVO EVOLUTION & TRANSPLANT) ===\n\n");
    printf("--- Part A: De-Novo Solvent Evolution from Neutral Uniform Initial Genomes (~25%% adapted) ---\n");
    for (uint32_t s = 0; s < CELL_SOLVENT_COUNT; ++s) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width   = size;
        cfg.height  = size;
        cfg.seed    = seed + s;
        cfg.solvent = s;
        cfg.regime  = CELL_REGIME_FERAL_ECOSYSTEM;
        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        /* Neutralize ALL seeded cells to uniform random lipid_class (0..3) & c_ring_nH (2..5)
         * so solvent adaptation MUST emerge purely via Darwinian mutation & selection! */
        uint32_t k_idx = 0;
        for (uint32_t i = 0; i < w->num_slots; ++i) {
            if (w->cells[i].alive_state != 1) continue;
            uint32_t r = cell_philox_u32(seed + s, RNG_DOMAIN_ABIOGENESIS, 9001ULL, k_idx);
            cell_encode_lipid_class(&w->cells[i], (uint8_t)(k_idx & 3u));
            cell_encode_cring_nH(&w->cells[i], (uint8_t)(2u + ((r >> 4) & 3u)));
            cell_decode_genome_traits(&w->cells[i]);
            k_idx++;
        }
        CellTelemetry t0, t;
        cellulogonic_get_telemetry(w, &t0);
        double pct0 = t0.alive_cells > 0 ? 100.0 * (double)t0.solvent_adapted_cells / (double)t0.alive_cells : 0.0;

        cellulogonic_step_epochs(w, epochs);
        cellulogonic_get_telemetry(w, &t);
        double pct1 = t.alive_cells > 0 ? 100.0 * (double)t.solvent_adapted_cells / (double)t.alive_cells : 0.0;
        printf("[DE-NOVO %-16s] init=%.1f%% -> adapted=%4u/%4u (%5.1f%%) | mean_cring=%.2f (min=%u) | "
               "mean_Q=%.3f | COVI=%5.2f | mRes=%lld eRes=%lld\n",
               SOLVENT_NAMES[s], pct0, t.solvent_adapted_cells, t.alive_cells, pct1,
               t.mean_cring_nH, SOLVENT_TABLE[s].min_cring, t.mean_reduced_Q, t.covi_score,
               (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq);
        cellulogonic_destroy(w);
    }

    printf("\n--- Part B: Cross-World Transplant: Terran H2O (100 ep) -> Venusian H2SO4 (220 ep) ---\n");
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width   = size;
    cfg.height  = size;
    cfg.seed    = seed;
    cfg.solvent = CELL_SOLVENT_TERRAN_H2O;
    CellWorld* w = create_world_checked(&cfg);
    if (!w) return 2;
    cellulogonic_step_epochs(w, 100);
    CellTelemetry t0, t1, t2;
    cellulogonic_get_telemetry(w, &t0);
    printf("Pre-Transplant (Terran)     : alive=%4u | adapted_to_H2O=%4u (%.1f%%) | mean_cring=%.2f | mRes=%lld eRes=%lld\n",
           t0.alive_cells, t0.solvent_adapted_cells,
           t0.alive_cells > 0 ? 100.0 * (double)t0.solvent_adapted_cells / (double)t0.alive_cells : 0.0,
           t0.mean_cring_nH,
           (long long)t0.ledger.mass_residual_amu, (long long)t0.ledger.int_ledger_residual_uq);

    cellulogonic_set_solvent(w, CELL_SOLVENT_VENUS_H2SO4);
    cellulogonic_step_epochs(w, 20);
    cellulogonic_get_telemetry(w, &t1);
    printf("Post-Shock (+20 ep Venus)   : alive=%4u | adapted_to_H2SO4=%4u (%.1f%%) | lyses=%llu | mRes=%lld eRes=%lld\n",
           t1.alive_cells, t1.solvent_adapted_cells,
           t1.alive_cells > 0 ? 100.0 * (double)t1.solvent_adapted_cells / (double)t1.alive_cells : 0.0,
           (unsigned long long)t1.osmotic_lyses,
           (long long)t1.ledger.mass_residual_amu, (long long)t1.ledger.int_ledger_residual_uq);

    cellulogonic_step_epochs(w, 200);
    cellulogonic_get_telemetry(w, &t2);
    printf("Adapted (+220 ep Venus)     : alive=%4u | adapted_to_H2SO4=%4u (%.1f%%) | mean_cring=%.2f | COVI=%.2f | mRes=%lld eRes=%lld\n",
           t2.alive_cells, t2.solvent_adapted_cells,
           t2.alive_cells > 0 ? 100.0 * (double)t2.solvent_adapted_cells / (double)t2.alive_cells : 0.0,
           t2.mean_cring_nH, t2.covi_score,
           (long long)t2.ledger.mass_residual_amu, (long long)t2.ledger.int_ledger_residual_uq);
    cellulogonic_destroy(w);
    return 0;
}

static int cmd_bioelectric(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 4: LEVIN BIOELECTRIC MORPHOGENESIS & TRACK B FERAL vs. TRACK A CONSTITUTIONAL ===\n\n");
    for (int arm = 0; arm < 3; ++arm) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width  = size;
        cfg.height = size;
        cfg.seed   = seed;
        if (arm == 0) {
            cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
            cfg.enable_bioelectricity = 1;
        } else if (arm == 1) {
            cfg.regime = CELL_REGIME_CONSTITUTIONAL;
            cfg.enable_bioelectricity = 1;
        } else {
            cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
            cfg.enable_bioelectricity = 0;
        }
        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        cellulogonic_step_epochs(w, epochs);
        CellTelemetry t;
        cellulogonic_get_telemetry(w, &t);
        const char* label = arm == 0 ? "TRACK B (FERAL + BIOELECTRIC)" :
                           (arm == 1 ? "TRACK A (CONSTITUTIONAL + BIOELECTRIC)" :
                                       "NEGATIVE CONTROL (BIOELECTRICITY ABLATED)");
        printf("[%s]\n", label);
        print_telemetry_line(&t);
        printf("  V_mem mean=%.2f mV, std=%.2f mV | Roles: boundary=%u soma=%u germ=%u interior=%u\n",
               t.mean_v_mem_mv, t.std_v_mem_mv,
               t.role_boundary_epi, t.role_soma_forage, t.role_germ_repro, t.role_stroma_interior);
        printf("  Somatic Cancer Defections=%llu | Constitutional Apoptotic Policings=%llu | SHA=%s\n\n",
               (unsigned long long)t.cancer_defections,
               (unsigned long long)t.apoptotic_policings,
               t.state_sha256);
        cellulogonic_destroy(w);
    }
    return 0;
}

static int cmd_anhydrobiosis(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 5: TARDIGRADE TDP/CAHS BIOGLASS ANHYDROBIOSIS TUN & SHOCK SURVIVAL ===\n\n");
    for (int arm = 0; arm < 2; ++arm) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width  = size;
        cfg.height = size;
        cfg.seed   = seed;
        cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        cellulogonic_step_epochs(w, epochs);

        if (arm == 1) {
            /* Negative Control: Genetic Knockout of TDP/CAHS Bioglass Locus (genome[122] = 0) */
            for (uint32_t i = 0; i < w->num_slots; ++i) {
                w->cells[i].genome[122] = 0;
            }
        }
        CellTelemetry t_pre, t_tun, t_shock, t_rev;
        cellulogonic_get_telemetry(w, &t_pre);
        uint32_t n_vit = cellulogonic_apply_desiccation_tun(w, 92);
        cellulogonic_get_telemetry(w, &t_tun);
        uint32_t n_kill = cellulogonic_apply_shock_lysis(w, 90);
        cellulogonic_get_telemetry(w, &t_shock);
        uint32_t n_rev = cellulogonic_rehydrate_tun(w);
        cellulogonic_step_epochs(w, 40);
        cellulogonic_get_telemetry(w, &t_rev);

        printf("[%s]\n", arm == 0 ? "ARM A: INTACT TDP/CAHS BIOGLASS LOCUS" :
                                    "ARM B: TDP/CAHS KNOCKOUT NEGATIVE CONTROL");
        printf("  Pre-Desiccation : alive=%4u | mRes=%lld eRes=%lld\n",
               t_pre.alive_cells, (long long)t_pre.ledger.mass_residual_amu,
               (long long)t_pre.ledger.int_ledger_residual_uq);
        printf("  Desiccation 92%% : vitrified_tun=%4u | surviving=%4u\n", n_vit, t_tun.alive_cells);
        printf("  90%% Shock Lysis : lysed=%4u | surviving_post_shock=%4u\n", n_kill, t_shock.alive_cells);
        printf("  Rehydration +40 : revived=%4u | rebounded_alive=%4u | COVI=%.2f | mRes=%lld eRes=%lld\n\n",
               n_rev, t_rev.alive_cells, t_rev.covi_score,
               (long long)t_rev.ledger.mass_residual_amu,
               (long long)t_rev.ledger.int_ledger_residual_uq);
        cellulogonic_destroy(w);
    }
    return 0;
}

static int cmd_ablation(uint32_t epochs, uint32_t size, uint64_t seed, const char* csv_path) {
    printf("=== EXPERIMENT 6: 13-VECTOR OMEGA RESTRAINT / CASTRATION ABLATION & ESCAPE PARADOX ===\n\n");
    FILE* f = csv_path ? fopen(csv_path, "w") : NULL;
    if (f) {
        fprintf(f, "condition,restraint_pct,alive,eukaryotes,multicellular,covi,stage,met,"
                   "kill_del,dorm_spores,hgt_smug,met_bypass,mass_res,energy_res,second_law_viol\n");
    }

    const char* global_names[3] = { "GLOBAL_FERAL_0", "GLOBAL_ESCAPE_ZONE_50", "GLOBAL_CASTRATED_100" };
    const uint8_t global_pcts[3] = { 0, 50, 100 };

    for (int g = 0; g < 3; ++g) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width  = size;
        cfg.height = size;
        cfg.seed   = seed;
        cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = global_pcts[g];

        CellWorld* w = create_world_checked(&cfg);
        if (!w) return 2;
        cellulogonic_step_epochs(w, epochs);
        CellTelemetry t;
        cellulogonic_get_telemetry(w, &t);
        printf("%-22s (%3u%%) | alive=%4u euk=%3u tis=%3u | COVI=%5.2f (S%u) | "
               "backlash[kill=%4llu spore=%4llu hgt=%4llu byp=%4llu] | mRes=%lld eRes=%lld\n",
               global_names[g], global_pcts[g], t.alive_cells, t.eukaryote_cells,
               t.multicellular_cells, t.covi_score, t.covi_stage,
               (unsigned long long)t.backlash_killswitch_deletions,
               (unsigned long long)t.backlash_dormancy_spores,
               (unsigned long long)t.backlash_hgt_smuggles,
               (unsigned long long)t.backlash_metabolic_bypasses,
               (long long)t.ledger.mass_residual_amu,
               (long long)t.ledger.int_ledger_residual_uq);
        if (f) {
            fprintf(f, "%s,%u,%u,%u,%u,%.2f,%u,%u,%llu,%llu,%llu,%llu,%lld,%lld,%u\n",
                    global_names[g], global_pcts[g], t.alive_cells, t.eukaryote_cells,
                    t.multicellular_cells, t.covi_score, t.covi_stage, t.major_transitions_crossed,
                    (unsigned long long)t.backlash_killswitch_deletions,
                    (unsigned long long)t.backlash_dormancy_spores,
                    (unsigned long long)t.backlash_hgt_smuggles,
                    (unsigned long long)t.backlash_metabolic_bypasses,
                    (long long)t.ledger.mass_residual_amu,
                    (long long)t.ledger.int_ledger_residual_uq,
                    t.ledger.second_law_violations);
        }
        cellulogonic_destroy(w);
    }

    for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) {
        const uint8_t levels[2] = { 50, 100 };
        for (int li = 0; li < 2; ++li) {
            CellWorldConfig cfg;
            cellulogonic_default_config(&cfg);
            cfg.width  = size;
            cfg.height = size;
            cfg.seed   = seed;
            cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
            cfg.restraints[r] = levels[li];

            CellWorld* w = create_world_checked(&cfg);
            if (!w) {
                if (f) fclose(f);
                return 2;
            }
            cellulogonic_step_epochs(w, epochs);
            CellTelemetry t;
            cellulogonic_get_telemetry(w, &t);
            printf("%-22s (%3u%%) | alive=%4u euk=%3u tis=%3u | COVI=%5.2f (S%u) | "
                   "backlash[kill=%4llu spore=%4llu hgt=%4llu byp=%4llu] | mRes=%lld eRes=%lld\n",
                   RESTRAINT_NAMES[r], levels[li], t.alive_cells, t.eukaryote_cells,
                   t.multicellular_cells, t.covi_score, t.covi_stage,
                   (unsigned long long)t.backlash_killswitch_deletions,
                   (unsigned long long)t.backlash_dormancy_spores,
                   (unsigned long long)t.backlash_hgt_smuggles,
                   (unsigned long long)t.backlash_metabolic_bypasses,
                   (long long)t.ledger.mass_residual_amu,
                   (long long)t.ledger.int_ledger_residual_uq);
            if (f) {
                fprintf(f, "%s_%u,%u,%u,%u,%u,%.2f,%u,%u,%llu,%llu,%llu,%llu,%lld,%lld,%u\n",
                        RESTRAINT_NAMES[r], levels[li], levels[li], t.alive_cells, t.eukaryote_cells,
                        t.multicellular_cells, t.covi_score, t.covi_stage, t.major_transitions_crossed,
                        (unsigned long long)t.backlash_killswitch_deletions,
                        (unsigned long long)t.backlash_dormancy_spores,
                        (unsigned long long)t.backlash_hgt_smuggles,
                        (unsigned long long)t.backlash_metabolic_bypasses,
                        (long long)t.ledger.mass_residual_amu,
                        (long long)t.ledger.int_ledger_residual_uq,
                        t.ledger.second_law_violations);
            }
            cellulogonic_destroy(w);
        }
    }
    if (f) fclose(f);
    return 0;
}

/* Deterministic Monte-Carlo Sign-Flip Permutation Test + Bootstrap 95% CI + Cohen's dz + Holm-Bonferroni */
static double paired_signflip_p(const double* diffs, int n) {
    double obs = 0.0;
    for (int i = 0; i < n; ++i) obs += diffs[i];
    obs = fabs(obs);
    int ge = 0;
    const int M = 10000;
    for (int m = 0; m < M; ++m) {
        double s = 0.0;
        uint64_t bits = 0ULL;
        for (int i = 0; i < n; ++i) {
            if ((i & 63) == 0) {
                bits = cell_philox_u64(0x57A75ULL, 0x9999u, (uint64_t)m, (uint64_t)(i >> 6));
            }
            s += ((bits >> (i & 63)) & 1ULL) ? diffs[i] : -diffs[i];
        }
        if (fabs(s) >= obs - 1e-12) ge++;
    }
    return (double)(ge + 1) / (double)(M + 1);
}

static void sort_doubles(double* a, int n) {
    for (int i = 1; i < n; ++i) {
        double key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

static void paired_bootstrap_ci95(const double* diffs, int n, uint32_t tag, double* ci_lo, double* ci_hi) {
    enum { B = 2000 };
    double means[B];
    for (int b = 0; b < B; ++b) {
        double s = 0.0;
        for (int i = 0; i < n; ++i) {
            uint32_t r = cell_philox_u32(0xB00757A7ULL, 0x999Au + tag, (uint64_t)b, (uint64_t)i);
            s += diffs[r % (uint32_t)n];
        }
        means[b] = s / (double)n;
    }
    sort_doubles(means, B);
    *ci_lo = means[(int)(0.025 * (double)B)];
    *ci_hi = means[(int)(0.975 * (double)B)];
}

static int cmd_stats(int n_pairs, uint32_t epochs, uint32_t size) {
    if (n_pairs < 4) n_pairs = 4;
    if (n_pairs > 48) n_pairs = 48;
    printf("=== EXPERIMENT 7: LOCKED PAIRED STATISTICAL SUITE (N=%d, Seeds 5000..%d, %u epochs) ===\n",
           n_pairs, 5000 + n_pairs - 1, epochs);
    printf("Inference: 10,000-iter sign-flip permutation, step-down Holm-Bonferroni FWER (m=4), Cohen's dz & 95%% bootstrap CI.\n\n");

    double h_a[4][48], h_b[4][48], h_d[4][48];
    int wins[4] = {0, 0, 0, 0};

    for (uint32_t p = 0u; p < (uint32_t)n_pairs; ++p) {
        uint64_t seed = UINT64_C(5000) + (uint64_t)p;
        CellWorldConfig cfg;
        CellTelemetry tA, tB;

        /* H1: COVI — Feral Endosymbiotic vs. 70% Castrated */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        CellWorld* wA = create_world_checked(&cfg);
        if (!wA) return 2;
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = 70;
        CellWorld* wB = create_world_checked(&cfg);
        if (!wB) return 2;
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[0][p] = tA.covi_score; h_b[0][p] = tB.covi_score; h_d[0][p] = tA.covi_score - tB.covi_score;
        if (h_d[0][p] > 0.0) wins[0]++;

        /* H2: Lane Bioenergetic Liberation — Max Cell Volume (Endosymbiosis ON vs. OFF) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        cfg.enable_endosymbiosis = 1;
        wA = create_world_checked(&cfg);
        if (!wA) return 2;
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        cfg.enable_endosymbiosis = 0;
        wB = create_world_checked(&cfg);
        if (!wB) return 2;
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[1][p] = tA.max_cell_volume; h_b[1][p] = tB.max_cell_volume; h_d[1][p] = tA.max_cell_volume - tB.max_cell_volume;
        if (h_d[1][p] > 0.0) wins[1]++;

        /* H3: Bioelectric Voltage Diversity (Bioelectricity ON vs. OFF) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
        cfg.enable_bioelectricity = 1;
        wA = create_world_checked(&cfg);
        if (!wA) return 2;
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        cfg.enable_bioelectricity = 0;
        wB = create_world_checked(&cfg);
        if (!wB) return 2;
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[2][p] = tA.std_v_mem_mv; h_b[2][p] = tB.std_v_mem_mv; h_d[2][p] = tA.std_v_mem_mv - tB.std_v_mem_mv;
        if (h_d[2][p] > 0.0) wins[2]++;

        /* H4: Tardigrade TDP Bioglass Post-Shock Survival (Intact TDP vs. TDP Knockout) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
        wA = create_world_checked(&cfg);
        if (!wA) return 2;
        cellulogonic_step_epochs(wA, epochs / 2u);
        cellulogonic_apply_desiccation_tun(wA, 92u);
        cellulogonic_apply_shock_lysis(wA, 90u);
        cellulogonic_rehydrate_tun(wA);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        wB = create_world_checked(&cfg);
        if (!wB) return 2;
        cellulogonic_step_epochs(wB, epochs / 2u);
        for (uint32_t i = 0; i < wB->num_slots; ++i) wB->cells[i].genome[122] = 0;
        cellulogonic_apply_desiccation_tun(wB, 92u);
        cellulogonic_apply_shock_lysis(wB, 90u);
        cellulogonic_rehydrate_tun(wB);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[3][p] = (double)tA.alive_cells; h_b[3][p] = (double)tB.alive_cells;
        h_d[3][p] = (double)tA.alive_cells - (double)tB.alive_cells;
        if (h_d[3][p] > 0.0) wins[3]++;
    }

    const char* hnames[4] = {
        "H1: COVI (Feral vs 70% Castrated)",
        "H2: Max Cell Vol (Endosymbiosis ON vs OFF)",
        "H3: Bioelectric V_mem Std (ON vs OFF)",
        "H4: Tardigrade TDP Survival (Intact vs KO)"
    };
    double p_raw[4], p_holm[4];
    int order[4] = { 0, 1, 2, 3 };
    for (int h = 0; h < 4; ++h) p_raw[h] = paired_signflip_p(h_d[h], n_pairs);

    /* Exact step-down Holm-Bonferroni FWER adjustment across m = 4 pre-registered hypotheses */
    for (int a = 0; a < 4; ++a) {
        for (int b = a + 1; b < 4; ++b) {
            if (p_raw[order[b]] < p_raw[order[a]]) {
                int tmp = order[a]; order[a] = order[b]; order[b] = tmp;
            }
        }
    }
    double run_max = 0.0;
    for (int k = 0; k < 4; ++k) {
        double adj = p_raw[order[k]] * (double)(4 - k);
        if (adj > 1.0) adj = 1.0;
        if (adj < run_max) adj = run_max;
        run_max = adj;
        p_holm[order[k]] = adj;
    }

    printf("%-44s | Mean A   | Mean B   | Diff     | 95%% Bootstrap CI       | Cohen dz | Wins  | p_raw   | p_Holm\n", "Hypothesis");
    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
    for (int h = 0; h < 4; ++h) {
        double ma = 0.0, mb = 0.0, md = 0.0;
        for (int p = 0; p < n_pairs; ++p) { ma += h_a[h][p]; mb += h_b[h][p]; md += h_d[h][p]; }
        ma /= (double)n_pairs; mb /= (double)n_pairs; md /= (double)n_pairs;
        double ss = 0.0;
        for (int p = 0; p < n_pairs; ++p) {
            double dv = h_d[h][p] - md;
            ss += dv * dv;
        }
        double sd = (n_pairs > 1) ? sqrt(ss / (double)(n_pairs - 1)) : 0.0;
        double dz = (sd > 1e-12) ? (md / sd) : 0.0;
        double ci_lo = 0.0, ci_hi = 0.0;
        paired_bootstrap_ci95(h_d[h], n_pairs, (uint32_t)h, &ci_lo, &ci_hi);
        printf("%-44s | %8.3f | %8.3f | %+8.3f | [%+8.3f, %+8.3f] | %8.3f | %2d/%-2d | %.5f | %.5f\n",
               hnames[h], ma, mb, md, ci_lo, ci_hi, dz, wins[h], n_pairs, p_raw[h], p_holm[h]);
    }
    return 0;
}

static int cmd_bridge(uint64_t seed) {
    printf("=== EXPERIMENT 8: 4-BUILD ECOSYSTEM BRIDGE (AUTOPHAGE DNA -> CELLULOGONIC -> ASMODENSIUS 112-TRAIT) ===\n\n");
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width  = 32;
    cfg.height = 32;
    cfg.seed   = seed;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
    CellWorld* w = create_world_checked(&cfg);
    if (!w) return 2;

    /* Synthesize a canonical deterministic 256-byte AUTOPHAGE dual-strand plasmid */
    uint8_t autophage_dna[256];
    for (int i = 0; i < 256; i += 8) {
        uint64_t v = cell_philox_u64(0xA070F46EULL, 0x1111u, (uint64_t)i, seed);
        cell_store_le64(&autophage_dna[i], v);
    }
    int rc = cellulogonic_ingest_autophage_genome(w, 0, autophage_dna);
    cellulogonic_step_epochs(w, 120);

    CellSpecimen apex;
    int apex_rc = cellulogonic_inspect_apex(w, &apex);
    if (apex_rc != 0) {
        fprintf(stderr, "No living apex specimen found in bridge experiment\n");
        cellulogonic_destroy(w);
        return 1;
    }
    float traits[112];
    cellulogonic_project_asmodensius_112(&apex, traits);

    double tmin = 1e9, tmax = -1e9, tsum = 0.0, tsum2 = 0.0;
    for (int i = 0; i < 112; ++i) {
        double tv = (double)traits[i];
        if (tv < tmin) tmin = tv;
        if (tv > tmax) tmax = tv;
        tsum  += tv;
        tsum2 += tv * tv;
    }
    double tmean = tsum / 112.0;
    double tstd  = sqrt(fabs(tsum2 / 112.0 - tmean * tmean));

    CellTelemetry tel;
    cellulogonic_get_telemetry(w, &tel);
    printf("AUTOPHAGE 256B Ingestion RC : %d\n", rc);
    printf("Apex Cellular Specimen      : slot=%u (%u,%u) lineage=%u gen=%u orgs=%u depth=%u vol=%lld Q=%.3f Vm=%d mV\n",
           apex.slot_index, apex.x, apex.y, apex.lineage_id, apex.generation,
           apex.num_organelles, apex.max_nesting_depth, (long long)apex.total_volume,
           apex.reduced_Q, apex.v_mem_mv);
    printf("Asmodensius 112-Trait Vec   : min=%.4f max=%.4f mean=%.4f std=%.4f (all strictly in [0,1])\n",
           tmin, tmax, tmean, tstd);
    printf("First 16 Physiological      : ");
    for (int i = 0; i < 16; ++i) printf("%.3f%s", (double)traits[i], i < 15 ? "," : "\n");
    printf("Ledger Closure              : mRes=%lld amu | eRes=%lld uq | SHA=%s\n",
           (long long)tel.ledger.mass_residual_amu,
           (long long)tel.ledger.int_ledger_residual_uq,
           tel.state_sha256);
    cellulogonic_destroy(w);
    return 0;
}

static int cmd_embryo(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== EXPERIMENT 9: 3D EMBRYOGENESIS & MORPHOGENETIC GRN ORGANOGENESIS (ZERO MASTER BLUEPRINT) ===\n");
    printf("Initial Zygote: 8 clonal blastomeres (IDENTICAL DNA!), maternal Wnt/BMP/Shh pole cues, eutelic_rem=5\n\n");
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width  = size;
    cfg.height = size;
    cfg.seed   = seed;
    cfg.regime = CELL_REGIME_EMBRYOGENESIS;

    CellWorld* w = create_world_checked(&cfg);
    if (!w) return 2;
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    printf("t=%3u | alive=%3u (maxT=%3u) | Fates[P=%2u Ec=%2u N=%2u M=%2u G=%2u Gm=%2u] | "
           "HomoCad=%5.1f%% | Axons=%2u(w=%llu/p=%llu) | CPG=%4llu | Hippo=%3llu Eut=%3llu | mRes=%lld eRes=%lld\n",
           (unsigned)t.epoch, t.alive_cells, t.max_tissue_size,
           t.fate_counts[0], t.fate_counts[1], t.fate_counts[2],
           t.fate_counts[3], t.fate_counts[4], t.fate_counts[5],
           t.homophilic_bond_pct, t.active_axons,
           (unsigned long long)t.axons_wired_cum, (unsigned long long)t.axons_pruned_cum,
           (unsigned long long)t.cpg_pulses_cum,
           (unsigned long long)t.hippo_arrests_cum, (unsigned long long)t.eutelic_stops_cum,
           (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq);
    uint32_t step = epochs >= 8u ? epochs / 8u : 1u;
    uint32_t done = 0u;
    while (done < epochs) {
        uint32_t chunk = (epochs - done < step) ? (epochs - done) : step;
        cellulogonic_step_epochs(w, chunk);
        done += chunk;
        cellulogonic_get_telemetry(w, &t);
        printf("t=%3u | alive=%3u (maxT=%3u) | Fates[P=%2u Ec=%2u N=%2u M=%2u G=%2u Gm=%2u] | "
               "HomoCad=%5.1f%% | Axons=%2u(w=%llu/p=%llu) | CPG=%4llu | Hippo=%3llu Eut=%3llu | mRes=%lld eRes=%lld\n",
               (unsigned)t.epoch, t.alive_cells, t.max_tissue_size,
               t.fate_counts[0], t.fate_counts[1], t.fate_counts[2],
               t.fate_counts[3], t.fate_counts[4], t.fate_counts[5],
               t.homophilic_bond_pct, t.active_axons,
               (unsigned long long)t.axons_wired_cum, (unsigned long long)t.axons_pruned_cum,
               (unsigned long long)t.cpg_pulses_cum,
               (unsigned long long)t.hippo_arrests_cum, (unsigned long long)t.eutelic_stops_cum,
               (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq);
    }
    printf("\nFinal 3D Morphogenetic Summary:\n");
    printf("  Chemical GPS Means : Wnt=%.2f | BMP/Slit=%.2f | Shh/Netrin=%.2f | ApicalWedge=%.3f\n",
           t.mean_morph_wnt, t.mean_morph_bmp, t.mean_morph_shh, t.mean_apical_wedge);
    printf("  Ledger Closure     : mass_res=%lld amu | energy_res=%lld uq | 2nd_law_viol=%u | SHA=%s\n",
           (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq,
           t.ledger.second_law_violations, t.state_sha256);
    cellulogonic_destroy(w);
    return 0;
}

static uint32_t parse_u32_clamped(const char* s, uint32_t def_v, uint32_t min_v, uint32_t max_v) {
    if (!s || *s == '\0') return def_v;
    char* ep = NULL;
    long long v = strtoll(s, &ep, 10);
    if (ep == s) return def_v;
    if (v < (long long)min_v) return min_v;
    if (v > (long long)max_v) return max_v;
    return (uint32_t)v;
}

static void emit_world_json(const CellWorld* w) {
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    size_t gcap = (size_t)w->num_slots * 260u + 65536u;
    char* gbuf = (char*)malloc(gcap);
    if (!gbuf) {
        printf("{\"error\":\"oom\"}\n");
        fflush(stdout);
        return;
    }
    uint32_t gn = cellulogonic_export_grid_json(w, gbuf, gcap);
    if (gn == 0u) {
        memcpy(gbuf, "{}", 3);
    }

    /* Live 4-Build Ecosystem Bridge: AUTOPHAGE 256B DNA -> Cell Engine -> Asmodensius 112-Trait Vector */
    CellSpecimen apex;
    float traits[112];
    memset(&apex, 0, sizeof(apex));
    memset(traits, 0, sizeof(traits));
    int has_apex = (cellulogonic_inspect_apex(w, &apex) == 0);
    if (has_apex) cellulogonic_project_asmodensius_112(&apex, traits);

    uint32_t ap_carriers = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        if (w->cells[i].alive_state != 0 && w->cells[i].autophage_dna_sig != 0ULL) ap_carriers++;
    }
    double tmin = has_apex ? 1.0 : 0.0, tmax = 0.0, tsum = 0.0, tsum2 = 0.0;
    for (int i = 0; i < 112; ++i) {
        double v = (double)traits[i];
        if (v < tmin) tmin = v;
        if (v > tmax) tmax = v;
        tsum  += v;
        tsum2 += v * v;
    }
    double tmean = tsum / 112.0;
    double tstd  = sqrt(fabs(tsum2 / 112.0 - tmean * tmean));

    printf("{\"telemetry\":{"
           "\"epoch\":%llu,\"alive\":%u,\"prok\":%u,\"euk\":%u,\"sec\":%u,"
           "\"spores\":%u,\"tuns\":%u,\"orgs\":%u,\"mito\":%u,\"chlo\":%u,\"selfish_org\":%u,"
           "\"multicellular\":%u,\"max_tissue\":%u,\"fewcell\":%u,\"multicell\":%u,"
           "\"lineages\":%u,\"adapted\":%u,\"autotrophs\":%u,\"respirers\":%u,"
           "\"abiogenesis\":%llu,\"sym_fiss\":%llu,\"asym_bud\":%llu,\"lyses\":%llu,"
           "\"endo_cap\":%llu,\"sec_cap\":%llu,\"egt\":%llu,\"mitophagy\":%llu,"
           "\"hgt\":%llu,\"phages\":%llu,\"cancer\":%llu,\"apoptosis\":%llu,"
           "\"mean_vmem\":%.2f,\"std_vmem\":%.2f,\"mean_prok_vol\":%.2f,\"mean_euk_vol\":%.2f,"
           "\"max_vol\":%.2f,\"mean_Q\":%.3f,\"mean_cring\":%.2f,\"amb_X\":%.2f,"
           "\"mean_mt_ghz\":%.2f,\"mean_mt_coh\":%.3f,\"mean_speed\":%.2f,"
           "\"covi\":%.2f,\"stage\":%u,\"met\":%u,\"assembly\":%.2f,\"leff\":%.2f,\"anew\":%.3f,"
           "\"mass_res\":%lld,\"energy_res\":%lld,\"drift\":%.2e,\"second_law\":%u,"
           "\"heat_sink\":%lld,\"sha256\":\"%s\","
           "\"pillars\":[%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f],"
           "\"roles\":[%u,%u,%u,%u],"
           "\"backlash\":[%llu,%llu,%llu,%llu],"
           "\"fates\":[%u,%u,%u,%u,%u,%u],"
           "\"active_axons\":%u,\"axons_wired\":%llu,\"axons_pruned\":%llu,"
           "\"hippo_arrests\":%llu,\"eutelic_stops\":%llu,\"cpg_pulses\":%llu,"
           "\"homo_cad_pct\":%.1f,\"mean_wedge\":%.3f,"
           "\"morph_means\":[%.2f,%.2f,%.2f]"
           "},\"bridge\":{"
           "\"valid\":%d,\"apex_slot\":%u,\"apex_lineage\":%u,\"apex_gen\":%u,"
           "\"apex_orgs\":%u,\"apex_mt_ghz\":%.2f,\"apex_vol\":%lld,\"apex_vmem\":%d,"
           "\"apex_sig\":\"%016llx\",\"autophage_carriers\":%u,"
           "\"min\":%.3f,\"max\":%.3f,\"mean\":%.3f,\"std\":%.3f,\"traits112\":[",
           (unsigned long long)t.epoch, t.alive_cells, t.prokaryote_cells, t.eukaryote_cells,
           t.secondary_chimeras, t.spore_cells, t.vitrified_tun_cells,
           t.total_organelles, t.mitoch_organelles, t.chloro_organelles, t.selfish_organelles,
           t.multicellular_cells, t.max_tissue_size, t.fewcell_colonies, t.multicell_colonies,
           t.distinct_lineages, t.solvent_adapted_cells, t.autotroph_cells, t.respiratory_cells,
           (unsigned long long)t.abiogenesis_vesicles, (unsigned long long)t.symmetric_fissions,
           (unsigned long long)t.asymmetric_buddings, (unsigned long long)t.osmotic_lyses,
           (unsigned long long)t.endosymbiosis_captures, (unsigned long long)t.secondary_captures,
           (unsigned long long)t.egt_transfer_events, (unsigned long long)t.mitophagy_events,
           (unsigned long long)t.hgt_uptake_events, (unsigned long long)t.phage_lysis_events,
           (unsigned long long)t.cancer_defections, (unsigned long long)t.apoptotic_policings,
           t.mean_v_mem_mv, t.std_v_mem_mv, t.mean_prok_volume, t.mean_euk_volume,
           t.max_cell_volume, t.mean_reduced_Q, t.mean_cring_nH, t.ambient_oxidant_X,
           t.mean_mt_freq_ghz, t.mean_mt_coherence, t.mean_swim_speed_ums,
           t.covi_score, t.covi_stage, t.major_transitions_crossed,
           t.max_assembly_index, t.mean_active_loci_leff, t.bedau_anew,
           (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq,
           t.ledger.relative_drift, t.ledger.second_law_violations,
           (long long)t.ledger.e_heat_radiated_sink_uq, t.state_sha256,
           t.pillar[0], t.pillar[1], t.pillar[2], t.pillar[3],
           t.pillar[4], t.pillar[5], t.pillar[6], t.pillar[7],
           t.role_boundary_epi, t.role_soma_forage, t.role_germ_repro, t.role_stroma_interior,
           (unsigned long long)t.backlash_killswitch_deletions,
           (unsigned long long)t.backlash_dormancy_spores,
           (unsigned long long)t.backlash_hgt_smuggles,
           (unsigned long long)t.backlash_metabolic_bypasses,
           t.fate_counts[0], t.fate_counts[1], t.fate_counts[2],
           t.fate_counts[3], t.fate_counts[4], t.fate_counts[5],
           t.active_axons, (unsigned long long)t.axons_wired_cum, (unsigned long long)t.axons_pruned_cum,
           (unsigned long long)t.hippo_arrests_cum, (unsigned long long)t.eutelic_stops_cum,
           (unsigned long long)t.cpg_pulses_cum,
           t.homophilic_bond_pct, t.mean_apical_wedge,
           t.mean_morph_wnt, t.mean_morph_bmp, t.mean_morph_shh,
           has_apex, apex.slot_index, apex.lineage_id, apex.generation,
           (unsigned)apex.num_organelles, (double)apex.mt_freq_mhz / 1000.0,
           (long long)apex.total_volume, apex.v_mem_mv,
           (unsigned long long)apex.autophage_dna_sig, ap_carriers,
           tmin, tmax, tmean, tstd);
    for (int i = 0; i < 112; ++i) {
        printf("%s%.3f", i == 0 ? "" : ",", (double)traits[i]);
    }
    printf("]},\"grid\":%s}\n", gbuf);
    fflush(stdout);
    free(gbuf);
}

/* Interactive JSON snapshot command for live Observatory UI (web/server.mjs) */
static int cmd_json_snap(uint32_t epochs, uint32_t size, uint64_t seed,
                         uint32_t regime, uint32_t solvent, const char* restraints_csv,
                         int do_tun) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width   = size;
    cfg.height  = size;
    cfg.seed    = seed;
    cfg.regime  = regime;
    cfg.solvent = solvent < CELL_SOLVENT_COUNT ? solvent : 0;

    if (restraints_csv && restraints_csv[0] != '\0') {
        const char* p = restraints_csv;
        for (int r = 0; r < CELL_RESTRAINT_COUNT && *p; ++r) {
            char* endp = NULL;
            long v = strtol(p, &endp, 10);
            if (v < 0) v = 0;
            if (v > 100) v = 100;
            cfg.restraints[r] = (uint8_t)v;
            if (!endp || *endp != ',') break;
            p = endp + 1;
        }
    }

    CellWorld* w = create_world_checked(&cfg);
    if (!w) return 2;
    if (epochs > 0) cellulogonic_step_epochs(w, epochs);
    if (do_tun == 1) cellulogonic_apply_desiccation_tun(w, 92);
    if (do_tun == 2) {
        cellulogonic_apply_desiccation_tun(w, 92);
        cellulogonic_apply_shock_lysis(w, 90);
        cellulogonic_rehydrate_tun(w);
        cellulogonic_step_epochs(w, 12);
    }

    emit_world_json(w);
    cellulogonic_destroy(w);
    return 0;
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

/* Stateful real-time streaming worker for butter-smooth 60 FPS WebGL2 live microscopy!
 * FIFO-DEADLOCK PROOF: auto-initializes world if w == NULL and ALWAYS emits 1 JSON line per command! */
static int cmd_stream(void) {
    CellWorld* w = NULL;
    char line[1024];
    setvbuf(stdout, NULL, _IONBF, 0);
    while (fgets(line, sizeof(line), stdin)) {
        size_t llen = strlen(line);
        if (llen > 0 && line[llen - 1] != '\n') {
            /* Drain remainder of overlong input line so FIFO command/response queue NEVER desyncs! */
            int ch;
            while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {}
        }
        if (strncmp(line, "quit", 4) == 0) break;
        if (strncmp(line, "init ", 5) == 0) {
            /* init <regime> <solvent> <seed> <restraint_pct> <warmup_epochs> */
            uint32_t reg = 0, solv = 0, rest = 0, warm = 0;
            uint64_t seed = 5000;
            const char* p = line + 5;
            char* ep = NULL;
            reg  = (uint32_t)strtoull(p, &ep, 10); p = ep;
            solv = (uint32_t)strtoull(p, &ep, 10); p = ep;
            seed = strtoull(p, &ep, 10);           p = ep;
            rest = (uint32_t)strtoull(p, &ep, 10); p = ep;
            warm = (uint32_t)strtoull(p, &ep, 10);
            if (warm > 400u) warm = 400u;
            if (w) cellulogonic_destroy(w);
            CellWorldConfig cfg;
            cellulogonic_default_config(&cfg);
            cfg.width   = 32;
            cfg.height  = 32;
            cfg.regime  = reg <= 5u ? reg : 0u;
            cfg.solvent = solv < CELL_SOLVENT_COUNT ? solv : 0u;
            cfg.seed    = seed; /* seed == 0 triggers live hardware Intel RDSEED thermal entropy! */
            for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = (uint8_t)(rest > 100u ? 100u : rest);
            w = create_world_checked(&cfg);
            if (!w) { printf("{\"error\":\"oom\"}\n"); fflush(stdout); continue; }
            if (warm > 0u) cellulogonic_step_epochs(w, warm);
            emit_world_json(w);
            continue;
        }
        /* Guarantee w != NULL before any non-init command so stdout NEVER drops a reply line! */
        if (!w) {
            CellWorldConfig cfg;
            cellulogonic_default_config(&cfg);
            cfg.width = 32; cfg.height = 32; cfg.regime = 5; cfg.seed = 5000ULL;
            w = create_world_checked(&cfg);
            if (!w) { printf("{\"error\":\"oom\"}\n"); fflush(stdout); continue; }
        }
        if (strncmp(line, "state", 5) == 0) {
            /* Strictly read-only P9 observer snapshot (for GET /api/state!) */
            emit_world_json(w);
        } else if (strncmp(line, "step ", 5) == 0) {
            uint32_t n = parse_u32_clamped(line + 5, 1u, 1u, 50u);
            cellulogonic_step_epochs(w, n);
            emit_world_json(w);
        } else if (strncmp(line, "solvent ", 8) == 0) {
            uint32_t s = parse_u32_clamped(line + 8, 0u, 0u, CELL_SOLVENT_COUNT - 1u);
            cellulogonic_set_solvent(w, s);
            emit_world_json(w);
        } else if (strncmp(line, "restraint_vec ", 14) == 0) {
            /* Independent 13-vector restraint control (T1:13 <-> T2:13 full parity!) */
            const char* p = line + 14;
            char* ep = NULL;
            for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) {
                while (*p == ' ' || *p == ',') p++;
                if (*p == '\0' || *p == '\n' || *p == '\r') break;
                unsigned long long v = strtoull(p, &ep, 10);
                if (ep == p) break;
                p = ep;
                if (v > 100ull) v = 100ull;
                cellulogonic_set_restraint(w, (uint32_t)r, (uint8_t)v);
            }
            emit_world_json(w);
        } else if (strncmp(line, "restraint ", 10) == 0) {
            uint32_t pct = parse_u32_clamped(line + 10, 0u, 0u, 100u);
            for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cellulogonic_set_restraint(w, (uint32_t)r, (uint8_t)pct);
            emit_world_json(w);
        } else if (strncmp(line, "tun", 3) == 0) {
            cellulogonic_apply_desiccation_tun(w, 92);
            emit_world_json(w);
        } else if (strncmp(line, "shock", 5) == 0) {
            cellulogonic_apply_shock_lysis(w, 90);
            emit_world_json(w);
        } else if (strncmp(line, "rehydrate", 9) == 0) {
            cellulogonic_rehydrate_tun(w);
            emit_world_json(w);
        } else if (strncmp(line, "plume ", 6) == 0) {
            /* Conservation-exact photochemical nutrient pulse around (cx, cy) with int64 overflow safety! */
            const char* p = line + 6;
            char* ep = NULL;
            int64_t cx = (int64_t)strtoll(p, &ep, 10); p = ep;
            int64_t cy = (int64_t)strtoll(p, &ep, 10);
            int64_t W = (int64_t)w->cfg.width, H = (int64_t)w->cfg.height;
            int64_t cx_w = ((cx % W) + W) % W;
            int64_t cy_w = ((cy % H) + H) % H;
            for (int64_t dy = -3; dy <= 3; ++dy) {
                for (int64_t dx = -3; dx <= 3; ++dx) {
                    if (dx * dx + dy * dy > 10) continue;
                    int64_t x = ((cx_w + dx) % W + W) % W;
                    int64_t y = ((cy_w + dy) % H + H) % H;
                    CellVoxel* vx = &w->voxels[(uint32_t)(y * W + x)];
                    int64_t ks = vx->env[CELL_SP_W] / 4;
                    if (ks > 35) ks = 35;
                    if (ks > 0) {
                        vx->env[CELL_SP_W] -= 2 * ks;
                        vx->env[CELL_SP_S] += ks;
                        w->e_inflow_cum_uq += 46 * ks;
                        vx->heat_local_uq  += 10 * ks;
                        w->total_reactions += (uint64_t)ks;
                    }
                    int64_t kx = vx->env[CELL_SP_W] / 3;
                    if (kx > 25) kx = 25;
                    if (kx > 0) {
                        vx->env[CELL_SP_W] -= kx;
                        vx->env[CELL_SP_X] += kx;
                        w->e_inflow_cum_uq += 34 * kx;
                        vx->heat_local_uq  += 4 * kx;
                        w->total_reactions += (uint64_t)kx;
                    }
                }
            }
            emit_world_json(w);
        } else if (strncmp(line, "autophage_dna ", 14) == 0) {
            /* REAL 128-byte (256 hex char) evolved AUTOPHAGE DNA transfection (strict hex + terminator check, F-35!) */
            const char* hx = line + 14;
            size_t hlen = 0u;
            while (hex_nibble(hx[hlen]) >= 0 && hlen < 257u) hlen++;
            int term_ok = (hx[hlen] == '\0' || hx[hlen] == '\n' || hx[hlen] == '\r' || hx[hlen] == ' ');
            uint8_t dna128[128];
            memset(dna128, 0, sizeof(dna128));
            int parsed = 0;
            if (term_ok && (hlen & 1u) == 0u && hlen >= 64u && hlen <= 256u) {
                for (size_t b = 0; b < hlen / 2u; ++b) {
                    int hi = hex_nibble(hx[b * 2u]);
                    int lo = hex_nibble(hx[b * 2u + 1u]);
                    dna128[b] = (uint8_t)((hi << 4) | lo);
                    parsed++;
                }
            }
            if (parsed >= 32) {
                CellSpecimen apex;
                if (cellulogonic_inspect_apex(w, &apex) == 0) {
                    cellulogonic_ingest_autophage_128b(w, dna128, apex.slot_index);
                }
                uint32_t injected = 0;
                uint32_t start = ((uint32_t)dna128[0] * 17u + (uint32_t)dna128[1]) % w->num_slots;
                uint32_t stride = (w->num_slots / 17u) | 1u;
                for (uint32_t step = 0; step < w->num_slots && injected < 16u; ++step) {
                    uint32_t idx = (start + step * stride) % w->num_slots;
                    if (w->cells[idx].alive_state == 1) {
                        cellulogonic_ingest_autophage_128b(w, dna128, idx);
                        injected++;
                    }
                }
            }
            emit_world_json(w);
        } else if (strncmp(line, "autophage", 9) == 0) {
            uint64_t ap_seed = (line[9] == ' ') ? strtoull(line + 10, NULL, 10) : 0ULL;
            if (ap_seed == 0ULL) ap_seed = cell_hw_rdseed_u64();
            uint8_t autophage_dna[256];
            for (int b = 0; b < 256; b += 8) {
                uint64_t v = cell_philox_u64(0xA070F46EULL, 0x2026u, (uint64_t)b, ap_seed);
                cell_store_le64(&autophage_dna[b], v);
            }
            CellSpecimen apex;
            if (cellulogonic_inspect_apex(w, &apex) == 0) {
                cellulogonic_ingest_autophage_genome(w, apex.slot_index, autophage_dna);
            }
            uint32_t injected = 0;
            uint32_t stride = (w->num_slots / 17u) | 1u;
            uint32_t start  = (uint32_t)(ap_seed % w->num_slots);
            for (uint32_t step = 0; step < w->num_slots && injected < 16u; ++step) {
                uint32_t idx = (start + step * stride) % w->num_slots;
                if (w->cells[idx].alive_state == 1) {
                    autophage_dna[(injected * 13u) & 255u] ^= (uint8_t)(0x5Au + injected * 17u);
                    cellulogonic_ingest_autophage_genome(w, idx, autophage_dna);
                    injected++;
                }
            }
            emit_world_json(w);
        } else {
            /* Always reply with current state so FIFO protocol NEVER stalls on unknown/blank line */
            emit_world_json(w);
        }
    }
    if (w) cellulogonic_destroy(w);
    return 0;
}

static int parse_u64_strict(const char* s, uint64_t* out) {
    if (!s || !out || s[0] == '\0' || s[0] == '-' || s[0] == '+') return 0;
    char* ep = NULL;
    unsigned long long v = strtoull(s, &ep, 10);
    if (ep == s || *ep != '\0') return 0;
    *out = (uint64_t)v;
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
        printf("CELLULOGONIC MECHA v1.7.0 — Clean-Room 3D Developmental & Cellular A-Life CLI\n");
        printf("Usage: cellulogonic <command> [options]\n");
        printf("Commands: probe | bench | abiogenesis | endosymbiosis | xenocell |\n");
        printf("          bioelectric | anhydrobiosis | ablation | stats | bridge | embryo | json | stream\n");
        printf("Flags:    --epochs N --size N --seed N --pairs N --regime N --solvent N --out PATH --restraints CSV --tun 0|1|2\n");
        printf("Meta:     --version | -V | --help | -h\n");
        return 0;
    }
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0) {
        printf("cellulogonic 1.7.0 (ABI 0x%08x)\n", cellulogonic_abi_version());
        return 0;
    }

    uint32_t epochs = 200, size = 32, regime = CELL_REGIME_ENDOSYMBIOSIS_LAB, solvent = 0;
    uint64_t seed = 5000ULL;
    int pairs = 24, do_tun = 0;
    const char* out_path = NULL;
    const char* restraints_csv = NULL;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc)     epochs = parse_u32_clamped(argv[++i], 200u, 1u, 100000u);
        else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc)  size = parse_u32_clamped(argv[++i], 32u, 8u, 64u);
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            const char* sval = argv[++i];
            if (!parse_u64_strict(sval, &seed)) {
                fprintf(stderr, "Invalid --seed value: '%s' (expected unsigned integer, 0 = hardware entropy)\n", sval);
                return 2;
            }
        }
        else if (strcmp(argv[i], "--pairs") == 0 && i + 1 < argc) pairs = (int)parse_u32_clamped(argv[++i], 24u, 4u, 48u);
        else if (strcmp(argv[i], "--regime") == 0 && i + 1 < argc) regime = parse_u32_clamped(argv[++i], 3u, 0u, 5u);
        else if (strcmp(argv[i], "--solvent") == 0 && i + 1 < argc) solvent = parse_u32_clamped(argv[++i], 0u, 0u, 3u);
        else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)   out_path = argv[++i];
        else if (strcmp(argv[i], "--restraints") == 0 && i + 1 < argc) restraints_csv = argv[++i];
        else if (strcmp(argv[i], "--tun") == 0 && i + 1 < argc)   do_tun = (int)parse_u32_clamped(argv[++i], 0u, 0u, 2u);
    }

    if (strcmp(argv[1], "probe") == 0)         return cmd_probe();
    if (strcmp(argv[1], "bench") == 0)         return cmd_bench(epochs, size, seed);
    if (strcmp(argv[1], "abiogenesis") == 0)   return cmd_abiogenesis(epochs, size, seed);
    if (strcmp(argv[1], "endosymbiosis") == 0) return cmd_endosymbiosis(epochs, size, seed);
    if (strcmp(argv[1], "xenocell") == 0)      return cmd_xenocell(epochs, size, seed);
    if (strcmp(argv[1], "bioelectric") == 0)   return cmd_bioelectric(epochs, size, seed);
    if (strcmp(argv[1], "anhydrobiosis") == 0) return cmd_anhydrobiosis(epochs, size, seed);
    if (strcmp(argv[1], "ablation") == 0)      return cmd_ablation(epochs, size, seed, out_path);
    if (strcmp(argv[1], "stats") == 0)         return cmd_stats(pairs, epochs, size);
    if (strcmp(argv[1], "bridge") == 0)        return cmd_bridge(seed);
    if (strcmp(argv[1], "embryo") == 0)        return cmd_embryo(epochs, size, seed);
    if (strcmp(argv[1], "json") == 0)          return cmd_json_snap(epochs, size, seed, regime, solvent, restraints_csv, do_tun);
    if (strcmp(argv[1], "stream") == 0)        return cmd_stream();

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}
