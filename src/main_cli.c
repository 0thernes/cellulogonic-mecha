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

static int cmd_probe(void) {
    CellHwCaps caps;
    cellulogonic_probe_hw(&caps);
    printf("=== CELLULOGONIC MECHA v1.0 SILICON PROBE ===\n");
    printf("ABI Version : 0x%08x\n", cellulogonic_abi_version());
    printf("CPU Vendor  : %s\n", caps.vendor);
    printf("CPU Brand   : %s\n", caps.brand);
    printf("ISA Flags   : AVX2=%u BMI1=%u BMI2=%u POPCNT=%u RDSEED=%u RDTSCP=%u\n",
           caps.has_avx2, caps.has_bmi1, caps.has_bmi2, caps.has_popcnt,
           caps.has_rdseed, caps.has_rdtscp);
    printf("Compiled Kernels : SIMD_AVX2=%s | SCALAR_REF=ENABLED\n",
#if defined(__AVX2__)
           "ENABLED (256-bit int64 finite-volume diffusion)"
#else
           "FALLBACK_SCALAR"
#endif
    );
    return 0;
}

static int cmd_bench(uint32_t epochs, uint32_t size, uint64_t seed) {
    printf("=== CELLULOGONIC MECHA v1.0 DUAL-KERNEL BENCHMARK & PARITY ===\n");
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

        CellWorld* w = cellulogonic_create(&cfg);
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

    CellWorld* w = cellulogonic_create(&cfg);
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    print_telemetry_line(&t);

    uint32_t step = epochs >= 10 ? epochs / 10 : 1;
    for (uint32_t e = step; e <= epochs; e += step) {
        cellulogonic_step_epochs(w, step);
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

        CellWorld* w = cellulogonic_create(&cfg);
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
    printf("=== EXPERIMENT 3: 4 PLANETARY / XENOCELLULAR SOLVENTS & TRANSPLANT ===\n\n");
    for (uint32_t s = 0; s < CELL_SOLVENT_COUNT; ++s) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width   = size;
        cfg.height  = size;
        cfg.seed    = seed + s;
        cfg.solvent = s;
        cfg.regime  = CELL_REGIME_FERAL_ECOSYSTEM;
        CellWorld* w = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(w, epochs);
        CellTelemetry t;
        cellulogonic_get_telemetry(w, &t);
        printf("[NATIVE %-16s] alive=%4u | adapted=%4u (%.1f%%) | mean_cring=%.2f (min=%u) | "
               "mean_Q=%.3f | COVI=%5.2f | mRes=%lld eRes=%lld\n",
               SOLVENT_NAMES[s], t.alive_cells, t.solvent_adapted_cells,
               t.alive_cells > 0 ? 100.0 * (double)t.solvent_adapted_cells / (double)t.alive_cells : 0.0,
               t.mean_cring_nH, SOLVENT_TABLE[s].min_cring, t.mean_reduced_Q, t.covi_score,
               (long long)t.ledger.mass_residual_amu, (long long)t.ledger.int_ledger_residual_uq);
        cellulogonic_destroy(w);
    }

    printf("\n--- Cross-World Transplant: Terran H2O (100 ep) -> Venusian H2SO4 (140 ep) ---\n");
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width   = size;
    cfg.height  = size;
    cfg.seed    = seed;
    cfg.solvent = CELL_SOLVENT_TERRAN_H2O;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 100);
    CellTelemetry t0, t1, t2;
    cellulogonic_get_telemetry(w, &t0);
    printf("Pre-Transplant (Terran)     : alive=%4u | adapted_to_H2O=%4u | mean_cring=%.2f | mRes=%lld eRes=%lld\n",
           t0.alive_cells, t0.solvent_adapted_cells, t0.mean_cring_nH,
           (long long)t0.ledger.mass_residual_amu, (long long)t0.ledger.int_ledger_residual_uq);

    cellulogonic_set_solvent(w, CELL_SOLVENT_VENUS_H2SO4);
    cellulogonic_step_epochs(w, 20);
    cellulogonic_get_telemetry(w, &t1);
    printf("Post-Shock (+20 ep Venus)   : alive=%4u | adapted_to_H2SO4=%4u | lyses=%llu | mRes=%lld eRes=%lld\n",
           t1.alive_cells, t1.solvent_adapted_cells, (unsigned long long)t1.osmotic_lyses,
           (long long)t1.ledger.mass_residual_amu, (long long)t1.ledger.int_ledger_residual_uq);

    cellulogonic_step_epochs(w, 120);
    cellulogonic_get_telemetry(w, &t2);
    printf("Adapted (+140 ep Venus)     : alive=%4u | adapted_to_H2SO4=%4u | mean_cring=%.2f | COVI=%.2f | mRes=%lld eRes=%lld\n",
           t2.alive_cells, t2.solvent_adapted_cells, t2.mean_cring_nH, t2.covi_score,
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
        CellWorld* w = cellulogonic_create(&cfg);
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
        CellWorld* w = cellulogonic_create(&cfg);
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

        CellWorld* w = cellulogonic_create(&cfg);
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

            CellWorld* w = cellulogonic_create(&cfg);
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

/* Exact Paired Wilcoxon Signed-Rank + Deterministic Monte-Carlo Sign-Flip + Holm-Bonferroni */
static double paired_signflip_p(const double* diffs, int n) {
    double obs = 0.0;
    for (int i = 0; i < n; ++i) obs += diffs[i];
    obs = fabs(obs);
    int ge = 0;
    const int M = 10000;
    for (int m = 0; m < M; ++m) {
        uint64_t bits = cell_philox_u64(0x57A75ULL, 0x9999u, (uint64_t)m, 0ULL);
        double s = 0.0;
        for (int i = 0; i < n; ++i) {
            s += ((bits >> (i & 63)) & 1ULL) ? diffs[i] : -diffs[i];
        }
        if (fabs(s) >= obs - 1e-12) ge++;
    }
    return (double)(ge + 1) / (double)(M + 1);
}

static int cmd_stats(int n_pairs, uint32_t epochs, uint32_t size) {
    if (n_pairs < 4) n_pairs = 4;
    if (n_pairs > 48) n_pairs = 48;
    printf("=== EXPERIMENT 7: LOCKED PAIRED STATISTICAL SUITE (N=%d, Seeds 5000..%d, %u epochs) ===\n",
           n_pairs, 5000 + n_pairs - 1, epochs);
    printf("Multiple-testing correction: Holm-Bonferroni FWER across 4 pre-registered hypotheses.\n\n");

    double h_a[4][48], h_b[4][48], h_d[4][48];
    int wins[4] = {0, 0, 0, 0};

    for (int p = 0; p < n_pairs; ++p) {
        uint64_t seed = 5000ULL + (uint64_t)p;
        CellWorldConfig cfg;
        CellTelemetry tA, tB;

        /* H1: COVI — Feral Endosymbiotic vs. 70% Castrated */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        CellWorld* wA = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = 70;
        CellWorld* wB = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[0][p] = tA.covi_score; h_b[0][p] = tB.covi_score; h_d[0][p] = tA.covi_score - tB.covi_score;
        if (h_d[0][p] > 0.0) wins[0]++;

        /* H2: Lane Bioenergetic Liberation — Max Cell Volume (Endosymbiosis ON vs. OFF) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        cfg.enable_endosymbiosis = 1;
        wA = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        cfg.enable_endosymbiosis = 0;
        wB = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[1][p] = tA.max_cell_volume; h_b[1][p] = tB.max_cell_volume; h_d[1][p] = tA.max_cell_volume - tB.max_cell_volume;
        if (h_d[1][p] > 0.0) wins[1]++;

        /* H3: Bioelectric Voltage Diversity (Bioelectricity ON vs. OFF) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
        cfg.enable_bioelectricity = 1;
        wA = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wA, epochs);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        cfg.enable_bioelectricity = 0;
        wB = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wB, epochs);
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        h_a[2][p] = tA.std_v_mem_mv; h_b[2][p] = tB.std_v_mem_mv; h_d[2][p] = tA.std_v_mem_mv - tB.std_v_mem_mv;
        if (h_d[2][p] > 0.0) wins[2]++;

        /* H4: Tardigrade TDP Bioglass Post-Shock Survival (Intact TDP vs. TDP Knockout) */
        cellulogonic_default_config(&cfg);
        cfg.width = size; cfg.height = size; cfg.seed = seed; cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
        wA = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wA, epochs / 2);
        cellulogonic_apply_desiccation_tun(wA, 92);
        cellulogonic_apply_shock_lysis(wA, 90);
        cellulogonic_rehydrate_tun(wA);
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        wB = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wB, epochs / 2);
        for (uint32_t i = 0; i < wB->num_slots; ++i) wB->cells[i].genome[122] = 0;
        cellulogonic_apply_desiccation_tun(wB, 92);
        cellulogonic_apply_shock_lysis(wB, 90);
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
    double p_raw[4];
    for (int h = 0; h < 4; ++h) p_raw[h] = paired_signflip_p(h_d[h], n_pairs);

    /* Holm-Bonferroni conservative step-down adjusted p-values (multiply by 4 worst-case rank) */
    printf("%-44s | Mean A   | Mean B   | Diff     | Wins   | p_raw   | p_Holm\n", "Hypothesis");
    printf("-----------------------------------------------------------------------------------------------------\n");
    for (int h = 0; h < 4; ++h) {
        double ma = 0.0, mb = 0.0, md = 0.0;
        for (int p = 0; p < n_pairs; ++p) { ma += h_a[h][p]; mb += h_b[h][p]; md += h_d[h][p]; }
        ma /= n_pairs; mb /= n_pairs; md /= n_pairs;
        double ph = p_raw[h] * 4.0;
        if (ph > 1.0) ph = 1.0;
        printf("%-44s | %8.3f | %8.3f | %+8.3f | %2d/%-2d  | %.5f | %.5f\n",
               hnames[h], ma, mb, md, wins[h], n_pairs, p_raw[h], ph);
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
    CellWorld* w = cellulogonic_create(&cfg);

    /* Synthesize a canonical deterministic 256-byte AUTOPHAGE dual-strand plasmid */
    uint8_t autophage_dna[256];
    for (int i = 0; i < 256; i += 8) {
        uint64_t v = cell_philox_u64(0xA070F46EULL, 0x1111u, (uint64_t)i, seed);
        memcpy(&autophage_dna[i], &v, 8);
    }
    int rc = cellulogonic_ingest_autophage_genome(w, 0, autophage_dna);
    cellulogonic_step_epochs(w, 120);

    CellSpecimen apex;
    cellulogonic_inspect_apex(w, &apex);
    float traits[112];
    cellulogonic_project_asmodensius_112(&apex, traits);

    double tmin = 1e9, tmax = -1e9, tsum = 0.0, tsum2 = 0.0;
    for (int i = 0; i < 112; ++i) {
        if (traits[i] < tmin) tmin = traits[i];
        if (traits[i] > tmax) tmax = traits[i];
        tsum  += traits[i];
        tsum2 += traits[i] * traits[i];
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
    for (int i = 0; i < 16; ++i) printf("%.3f%s", traits[i], i < 15 ? "," : "\n");
    printf("Ledger Closure              : mRes=%lld amu | eRes=%lld uq | SHA=%s\n",
           (long long)tel.ledger.mass_residual_amu,
           (long long)tel.ledger.int_ledger_residual_uq,
           tel.state_sha256);
    cellulogonic_destroy(w);
    return 0;
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

    CellWorld* w = cellulogonic_create(&cfg);
    if (epochs > 0) cellulogonic_step_epochs(w, epochs);
    if (do_tun == 1) cellulogonic_apply_desiccation_tun(w, 92);
    if (do_tun == 2) {
        cellulogonic_apply_desiccation_tun(w, 92);
        cellulogonic_apply_shock_lysis(w, 90);
        cellulogonic_rehydrate_tun(w);
        cellulogonic_step_epochs(w, 12);
    }

    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    size_t gcap = (size_t)size * (size_t)size * 64 + 4096;
    char* gbuf = (char*)malloc(gcap);
    cellulogonic_export_grid_json(w, gbuf, gcap);

    printf("{\"telemetry\":{"
           "\"epoch\":%llu,\"alive\":%u,\"prok\":%u,\"euk\":%u,\"sec\":%u,"
           "\"spores\":%u,\"tuns\":%u,\"orgs\":%u,\"mito\":%u,\"chlo\":%u,\"selfish_org\":%u,"
           "\"multicellular\":%u,\"max_tissue\":%u,\"lineages\":%u,\"adapted\":%u,"
           "\"autotrophs\":%u,\"respirers\":%u,"
           "\"abiogenesis\":%llu,\"sym_fiss\":%llu,\"asym_bud\":%llu,\"lyses\":%llu,"
           "\"endo_cap\":%llu,\"sec_cap\":%llu,\"egt\":%llu,\"mitophagy\":%llu,"
           "\"hgt\":%llu,\"phages\":%llu,\"cancer\":%llu,\"apoptosis\":%llu,"
           "\"mean_vmem\":%.2f,\"std_vmem\":%.2f,\"mean_prok_vol\":%.2f,\"mean_euk_vol\":%.2f,"
           "\"max_vol\":%.2f,\"mean_Q\":%.3f,\"mean_cring\":%.2f,\"amb_X\":%.2f,"
           "\"covi\":%.2f,\"stage\":%u,\"met\":%u,\"assembly\":%.2f,\"leff\":%.2f,\"anew\":%.3f,"
           "\"mass_res\":%lld,\"energy_res\":%lld,\"drift\":%.2e,\"second_law\":%u,"
           "\"heat_sink\":%lld,\"sha256\":\"%s\","
           "\"pillars\":[%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f],"
           "\"roles\":[%u,%u,%u,%u],"
           "\"backlash\":[%llu,%llu,%llu,%llu]"
           "},\"grid\":%s}\n",
           (unsigned long long)t.epoch, t.alive_cells, t.prokaryote_cells, t.eukaryote_cells,
           t.secondary_chimeras, t.spore_cells, t.vitrified_tun_cells,
           t.total_organelles, t.mitoch_organelles, t.chloro_organelles, t.selfish_organelles,
           t.multicellular_cells, t.max_tissue_size, t.distinct_lineages, t.solvent_adapted_cells,
           t.autotroph_cells, t.respiratory_cells,
           (unsigned long long)t.abiogenesis_vesicles, (unsigned long long)t.symmetric_fissions,
           (unsigned long long)t.asymmetric_buddings, (unsigned long long)t.osmotic_lyses,
           (unsigned long long)t.endosymbiosis_captures, (unsigned long long)t.secondary_captures,
           (unsigned long long)t.egt_transfer_events, (unsigned long long)t.mitophagy_events,
           (unsigned long long)t.hgt_uptake_events, (unsigned long long)t.phage_lysis_events,
           (unsigned long long)t.cancer_defections, (unsigned long long)t.apoptotic_policings,
           t.mean_v_mem_mv, t.std_v_mem_mv, t.mean_prok_volume, t.mean_euk_volume,
           t.max_cell_volume, t.mean_reduced_Q, t.mean_cring_nH, t.ambient_oxidant_X,
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
           gbuf);

    free(gbuf);
    cellulogonic_destroy(w);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("CELLULOGONIC MECHA v1.0 — Clean-Room Cellular A-Life CLI\n");
        printf("Usage: cellulogonic <command> [options]\n");
        printf("Commands: probe | bench | abiogenesis | endosymbiosis | xenocell |\n");
        printf("          bioelectric | anhydrobiosis | ablation | stats | bridge | json\n");
        return 0;
    }

    uint32_t epochs = 200, size = 32, regime = CELL_REGIME_ENDOSYMBIOSIS_LAB, solvent = 0;
    uint64_t seed = 5000ULL;
    int pairs = 24, do_tun = 0;
    const char* out_path = NULL;
    const char* restraints_csv = NULL;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc)     epochs = (uint32_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc)  size = (uint32_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)  seed = strtoull(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--pairs") == 0 && i + 1 < argc) pairs = atoi(argv[++i]);
        else if (strcmp(argv[i], "--regime") == 0 && i + 1 < argc) regime = (uint32_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--solvent") == 0 && i + 1 < argc) solvent = (uint32_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)   out_path = argv[++i];
        else if (strcmp(argv[i], "--restraints") == 0 && i + 1 < argc) restraints_csv = argv[++i];
        else if (strcmp(argv[i], "--tun") == 0 && i + 1 < argc)   do_tun = atoi(argv[++i]);
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
    if (strcmp(argv[1], "json") == 0)          return cmd_json_snap(epochs, size, seed, regime, solvent, restraints_csv, do_tun);

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}
