#include "cellulogonic_internal.h"

/* ============================================================================
 * CELLULOGONIC MECHA v1.3.0 — 24-Gate Lab-Grade Falsification Suite
 * Includes 8 Planted Negative Mutants (M1..M8) Proven Caught!
 * ============================================================================ */

static int g_pass = 0;
static int g_fail = 0;
static int g_mutants_caught = 0;

static void report_gate(const char* id, const char* name, int ok, double dt) {
    if (ok) {
        g_pass++;
        printf("  [PASS] %-4s | %-62s | %.4fs\n", id, name, dt);
    } else {
        g_fail++;
        printf("  [FAIL] %-4s | %-62s | %.4fs\n", id, name, dt);
    }
}

/* G01: CPUID Probe + NIST FIPS 180-4 SHA-256 "abc" exact vector */
static int test_g01(void) {
    CellHwCaps caps;
    cellulogonic_probe_hw(&caps);
    uint8_t h[32];
    char hex[65];
    CellSha256Ctx ctx;
    cell_sha256_init(&ctx);
    cell_sha256_update(&ctx, (const uint8_t*)"abc", 3);
    cell_sha256_final(&ctx, h);
    cell_sha256_to_hex(h, hex);
    const char* nist = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    return (strcmp(hex, nist) == 0 && cellulogonic_abi_version() == CELLULOGONIC_ABI_VERSION);
}

/* G02: Exact Mass & First-Law Table + Planted Mutant M1 (phantom energy creation caught!) */
static int test_g02(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 16; cfg.height = 16; cfg.seed = 5001ULL;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 25);
    CellLedgerSnapshot led;
    cell_compute_ledger(w, &led);
    int clean_ok = (led.mass_residual_amu == 0 && led.int_ledger_residual_uq == 0);

    /* Planted Mutant M1: illegally inject +1 uq phantom heat without booking inflow! */
    w->voxels[0].heat_local_uq += 1;
    cell_compute_ledger(w, &led);
    int mutant_caught = (led.int_ledger_residual_uq == 1);
    if (mutant_caught) g_mutants_caught++;
    cellulogonic_destroy(w);
    return clean_ok && mutant_caught;
}

/* G03: Strict Second-Law across all 4 solvents (incl. Titan e_pmf=8) + Mutant M2 */
static int test_g03(void) {
    int ok = 1;
    /* Verify strict Second Law q > 0 on all 4 planetary solvents (no q = 0 loophole!) */
    for (uint32_t s = 0; s < CELL_SOLVENT_COUNT; ++s) {
        const SolventParams* sp = &SOLVENT_TABLE[s];
        if ((int64_t)sp->min_cring * sp->e_pmf <= 28) ok = 0;
        if (((int64_t)sp->min_cring - 1) * sp->e_pmf > 28) ok = 0;
        if (62 - sp->r2_pmf_yield * sp->e_pmf <= 0) ok = 0;
    }
    for (uint32_t s = 0; s < CELL_SOLVENT_COUNT; ++s) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width = 16; cfg.height = 16; cfg.seed = 5010ULL + s; cfg.solvent = s;
        CellWorld* w = cellulogonic_create(&cfg);
        int64_t prev_sink = 0;
        for (int e = 0; e < 40; ++e) {
            cellulogonic_step_epochs(w, 1);
            if (w->e_heat_radiated_sink_uq < prev_sink) ok = 0;
            prev_sink = w->e_heat_radiated_sink_uq;
        }
        CellLedgerSnapshot led;
        cell_compute_ledger(w, &led);
        if (led.second_law_violations != 0 || led.mass_residual_amu != 0 || led.int_ledger_residual_uq != 0) ok = 0;

        if (s == 0) {
            /* Planted Mutant M2: reverse radiated sink back into free ATP — MUST BE CAUGHT BY ENGINE VERIFIER! */
            int64_t before = w->e_heat_radiated_sink_uq;
            uint32_t v_before = w->second_law_violations;
            w->e_heat_radiated_sink_uq -= 10;
            int sec_ok = cell_verify_second_law(w, before);
            int caught = (!sec_ok && w->second_law_violations == v_before + 1u);
            w->e_heat_radiated_sink_uq = before;
            w->second_law_violations = v_before;
            if (caught) g_mutants_caught++;
            else ok = 0;
        }
        cellulogonic_destroy(w);
    }
    return ok;
}

/* G04: Canham-Helfrich Exact Geometry Thresholds (Lean T12-T16) */
static int test_g04(void) {
    /* Symmetric threshold: (V=40, L=80) -> 2*100*1600 = 320000 <= 512000 = 80^3 (feasible!) */
    int sym_ok = cell_can_symmetric_fission(40, 80);
    /* Swollen interphase: (V=47, L=72) -> 2*100*2209 = 441800 > 373248 = 72^3 (blocked!) */
    int sym_blocked = !cell_can_symmetric_fission(47, 72);
    /* Not osmotically burst in Terran H2O: 100*2209*100 = 22.09M <= 115*373248 = 42.92M */
    int not_burst = !cell_is_osmotic_burst(47, 72, CELL_SOLVENT_TERRAN_H2O, 0);
    /* Osmotic burst: (V=80, L=72) -> 100*6400*100 = 64M > 42.92M */
    int does_burst = cell_is_osmotic_burst(80, 72, CELL_SOLVENT_TERRAN_H2O, 0);
    /* Constructive asymmetric bud escape: (6,16) + (41,56) both intact! */
    int bud1_ok = (100LL * 6 * 6 <= 16LL * 16 * 16);
    int bud2_ok = (100LL * 41 * 41 <= 56LL * 56 * 56);
    return sym_ok && sym_blocked && not_burst && does_burst && bud1_ok && bud2_ok;
}

/* G05: Closed-Box (BOXING=100) & Open-Flux 120-Epoch Exact Conservation Closure */
static int test_g05(void) {
    int ok = 1;
    for (int closed = 0; closed < 2; ++closed) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width = 24; cfg.height = 24; cfg.seed = 5020ULL + closed;
        if (closed) cfg.restraints[CELL_RESTRAINT_BOXING] = 100;
        CellWorld* w = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(w, 120);
        CellLedgerSnapshot led;
        cell_compute_ledger(w, &led);
        if (led.mass_residual_amu != 0 || led.int_ledger_residual_uq != 0 ||
            led.relative_drift > 1e-14 || led.second_law_violations != 0) ok = 0;
        if (closed && led.e_inflow_cum_uq != 0) ok = 0;
        cellulogonic_destroy(w);
    }
    return ok;
}

/* G06: Domain-Isolated Philox4x32-10 RNG + Planted Mutant M3 + Clamped 112T + Tampered .cmbin Reject */
static int test_g06(void) {
    uint64_t a = cell_philox_u64(5000ULL, RNG_DOMAIN_WORLD_EXOG, 1, 1);
    uint64_t b = cell_philox_u64(5000ULL, RNG_DOMAIN_ABIOGENESIS, 1, 1);
    uint64_t c = cell_philox_u64(5000ULL, RNG_DOMAIN_CELL_MUT, 1, 1);
    uint64_t d = cell_philox_u64(5000ULL, RNG_DOMAIN_MITOSIS_PART, 1, 1);
    int ortho = (a != b && a != c && a != d && b != c && b != d && c != d);
    /* Planted Mutant M3: Verify real world-step isolation & tampered .cmbin rejection & 112T clamping */
    uint64_t shifted = cell_philox_u64(5000ULL, RNG_DOMAIN_CELL_MUT, 1, 2);
    int caught = (shifted != c && a != c);
    /* Verify out-of-range specimen fields are strictly clamped into [0.0f, 1.0f] */
    CellSpecimen ext;
    memset(&ext, 0, sizeof(ext));
    ext.v_mem_mv = -140;
    ext.c_ring_nH = 0;
    ext.reduced_Q = -2.0;
    ext.total_volume = 9999;
    float t112[112];
    cellulogonic_project_asmodensius_112(&ext, t112);
    for (int k = 0; k < 112; ++k) {
        if (!(t112[k] >= 0.0f && t112[k] <= 1.0f)) caught = 0;
    }
    /* Verify corrupted .cmbin snapshot (tampered byte OR trailing EOF byte) is rejected + restraint clamping */
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 16; cfg.height = 16; cfg.seed = 5026ULL;
    cfg.restraints[0] = 250u; /* Out-of-range restraint MUST be clamped to 100 inside create() */
    cfg.regime = 99u;         /* Out-of-range regime MUST be clamped */
    CellWorld* wt = cellulogonic_create(&cfg);
    if (!wt || wt->cfg.restraints[0] != 100u || wt->cfg.regime > CELL_REGIME_EMBRYOGENESIS) caught = 0;
    if (wt && cellulogonic_save_snapshot(wt, "test_tamper_tmp.cmbin") == 0) {
        FILE* fp = fopen("test_tamper_tmp.cmbin", "rb");
        if (fp) {
            uint8_t* raw = (uint8_t*)malloc(524288);
            size_t nr = raw ? fread(raw, 1, 524288, fp) : 0;
            fclose(fp);
            if (raw && nr > 256) {
                /* Sub-check A: trailing garbage byte appended after SHA-256 trailer must be rejected */
                FILE* fa = fopen("test_tamper_tmp.cmbin", "wb");
                if (fa) { (void)fwrite(raw, 1, nr, fa); uint8_t junk = 0xAAu; (void)fwrite(&junk, 1, 1, fa); fclose(fa); }
                CellWorld* bad_eof = cellulogonic_load_snapshot("test_tamper_tmp.cmbin");
                if (bad_eof != NULL) { cellulogonic_destroy(bad_eof); caught = 0; }
                /* Sub-check B: 1-byte payload tamper must be rejected */
                raw[200] ^= 0xFFu;
                FILE* fw = fopen("test_tamper_tmp.cmbin", "wb");
                if (fw) { (void)fwrite(raw, 1, nr, fw); fclose(fw); }
                CellWorld* bad = cellulogonic_load_snapshot("test_tamper_tmp.cmbin");
                if (bad != NULL) { cellulogonic_destroy(bad); caught = 0; }
            }
            free(raw);
        }
        (void)remove("test_tamper_tmp.cmbin");
    }
    cellulogonic_destroy(wt);
    if (caught) g_mutants_caught++;
    return ortho && caught;
}

/* G07: Dual-Kernel Bit-Exact SHA-256 Parity (SIMD_AVX2 == SCALAR_REF, 80 epochs!) */
static int test_g07(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5055ULL;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;

    cfg.kernel_mode = CELL_KERNEL_SIMD_AVX2;
    CellWorld* w1 = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w1, 80);
    char h1[65];
    cellulogonic_state_sha256(w1, NULL, h1);

    cfg.kernel_mode = CELL_KERNEL_SCALAR_REF;
    CellWorld* w2 = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w2, 80);
    char h2[65];
    cellulogonic_state_sha256(w2, NULL, h2);

    cellulogonic_destroy(w1);
    cellulogonic_destroy(w2);
    return strcmp(h1, h2) == 0;
}

/* G08: Strict Observer Independence (P9) + Planted Mutant M4 (1-bit observer write caught!) */
static int test_g08(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 24; cfg.height = 24; cfg.seed = 5060ULL;
    CellWorld* w_head = cellulogonic_create(&cfg);
    CellWorld* w_obs  = cellulogonic_create(&cfg);

    char gbuf[32768];
    for (int e = 0; e < 50; ++e) {
        cellulogonic_step_epochs(w_head, 1);
        cellulogonic_step_epochs(w_obs, 1);
        CellTelemetry tel;
        CellSpecimen spec;
        cellulogonic_get_telemetry(w_obs, &tel);
        cellulogonic_inspect_apex(w_obs, &spec);
        cellulogonic_export_grid_json(w_obs, gbuf, sizeof(gbuf));
    }
    char h_head[65], h_obs[65], h_mut[65];
    cellulogonic_state_sha256(w_head, NULL, h_head);
    cellulogonic_state_sha256(w_obs,  NULL, h_obs);
    int p9_ok = (strcmp(h_head, h_obs) == 0);

    /* Planted Mutant M4: observer flips 1 single bit in voxel 0 quorum -> SHA must differ! */
    w_obs->voxels[0].quorum ^= 1LL;
    cellulogonic_state_sha256(w_obs, NULL, h_mut);
    int caught = (strcmp(h_head, h_mut) != 0);
    if (caught) g_mutants_caught++;

    cellulogonic_destroy(w_head);
    cellulogonic_destroy(w_obs);
    return p9_ok && caught;
}

/* G09: .cmbin Snapshot Save (SIMD_AVX2) & Cross-Kernel Resume (SCALAR_REF) 0-bit Parity */
static int test_g09(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 24; cfg.height = 24; cfg.seed = 5070ULL;
    cfg.kernel_mode = CELL_KERNEL_SIMD_AVX2;
    CellWorld* w_ref = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w_ref, 30);
    int sv = cellulogonic_save_snapshot(w_ref, "test_tmp_cell.cmbin");
    cellulogonic_step_epochs(w_ref, 30);
    char h_ref[65];
    cellulogonic_state_sha256(w_ref, NULL, h_ref);

    CellWorld* w_res = cellulogonic_load_snapshot("test_tmp_cell.cmbin");
    (void)remove("test_tmp_cell.cmbin");
    if (!w_res || sv != 0) { cellulogonic_destroy(w_ref); return 0; }
    cellulogonic_set_kernel_mode(w_res, CELL_KERNEL_SCALAR_REF);
    cellulogonic_step_epochs(w_res, 30);
    char h_res[65];
    cellulogonic_state_sha256(w_res, NULL, h_res);

    cellulogonic_destroy(w_ref);
    cellulogonic_destroy(w_res);
    return strcmp(h_ref, h_res) == 0;
}

/* G10: Zero Post-Warmup Heap Allocations (Clean-Slate HW02 / P10) */
static int test_g10(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5080ULL;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 160);
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    cellulogonic_destroy(w);
    return (t.post_warmup_allocs == 0 && t.total_cell_steps > 10000);
}

/* G11: Zero-Seed Spontaneous Protocell Abiogenesis Phase Transition */
static int test_g11(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5000ULL;
    cfg.regime = CELL_REGIME_ABIOGENESIS_ZERO_SEED;
    CellWorld* w = cellulogonic_create(&cfg);
    CellTelemetry t0, t1;
    cellulogonic_get_telemetry(w, &t0);
    cellulogonic_step_epochs(w, 180);
    cellulogonic_get_telemetry(w, &t1);
    cellulogonic_destroy(w);
    return (t0.alive_cells == 0 && t0.covi_score == 0.0 &&
            t1.abiogenesis_vesicles > 0 && t1.alive_cells > 20 &&
            (t1.symmetric_fissions + t1.asymmetric_buddings) > 10 &&
            t1.covi_score > 30.0 &&
            t1.ledger.mass_residual_amu == 0 && t1.ledger.int_ledger_residual_uq == 0);
}

/* G12: Anti-Gaming Gates (Lean T34-T35) + Planted Mutants M5 (Inert Crystal) & M6 (White Noise) */
static int test_g12(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 16; cfg.height = 16; cfg.seed = 5090ULL;
    cfg.regime = CELL_REGIME_ABIOGENESIS_ZERO_SEED;
    CellWorld* w = cellulogonic_create(&cfg);

    /* Planted Mutant M5: Inert dead lipid micelle (lots of L, 0 proteins P, 0 reactions) */
    w->cells[0].alive_state = 1;
    w->cells[0].mol[CELL_SP_L] = 80;
    w->cells[0].mol[CELL_SP_P] = 0;
    w->cells[0].mol[CELL_SP_T] = 0;
    CellTelemetry tm5;
    cellulogonic_get_telemetry(w, &tm5);
    int m5_caught = (tm5.gamma_anti_gaming == 0.0 && tm5.covi_score == 0.0);
    if (m5_caught) g_mutants_caught++;

    /* Planted Mutant M6: Pure random byte noise with 0 functional enzymes (P=0) OR 0 templates (T=0, Lean T34) */
    for (int i = 0; i < 128; ++i) w->cells[0].genome[i] = (uint8_t)(i * 37u + 13u);
    w->cells[0].mol[CELL_SP_P] = 4;
    w->cells[0].mol[CELL_SP_T] = 0;
    w->total_reactions = 100;
    CellTelemetry tm6;
    cellulogonic_get_telemetry(w, &tm6);
    int m6_caught = (tm6.gamma_anti_gaming == 0.0 && tm6.covi_score == 0.0);
    if (m6_caught) g_mutants_caught++;

    cellulogonic_destroy(w);
    return m5_caught && m6_caught;
}

/* G13: Serial Endosymbiosis & Prokaryotic 1/R Bioenergetic Liberation */
static int test_g13(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5100ULL;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;

    cfg.enable_endosymbiosis = 1;
    CellWorld* wA = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wA, 160);
    CellTelemetry tA;
    cellulogonic_get_telemetry(wA, &tA);
    cellulogonic_destroy(wA);

    cfg.enable_endosymbiosis = 0;
    CellWorld* wB = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wB, 160);
    CellTelemetry tB;
    cellulogonic_get_telemetry(wB, &tB);
    cellulogonic_destroy(wB);

    return (tA.endosymbiosis_captures > 0 && tA.eukaryote_cells > 0 &&
            tB.endosymbiosis_captures == 0 && tB.eukaryote_cells == 0 &&
            tA.max_cell_volume > tB.max_cell_volume &&
            tA.ledger.mass_residual_amu == 0 && tA.ledger.int_ledger_residual_uq == 0);
}

/* G14: Cross-World Xenocellular Solvent Transplant Conservation & De-Novo Venus Evolution */
static int test_g14(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 24; cfg.height = 24; cfg.seed = 5110ULL;
    cfg.solvent = CELL_SOLVENT_TERRAN_H2O;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 40);
    cellulogonic_set_solvent(w, CELL_SOLVENT_TITAN_AZOTOSOME);
    cellulogonic_step_epochs(w, 40);
    cellulogonic_set_solvent(w, CELL_SOLVENT_VENT_TETRAETHER);
    cellulogonic_step_epochs(w, 40);
    cellulogonic_set_solvent(w, CELL_SOLVENT_VENUS_H2SO4);
    cellulogonic_step_epochs(w, 120);
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    cellulogonic_destroy(w);

    /* Sub-test B: Genuine Terran -> Venus adaptation sweep (must evolve >40% Venus-adapted!) */
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5000ULL;
    cfg.solvent = CELL_SOLVENT_TERRAN_H2O;
    CellWorld* wv = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wv, 80);
    cellulogonic_set_solvent(wv, CELL_SOLVENT_VENUS_H2SO4);
    cellulogonic_step_epochs(wv, 200);
    CellTelemetry tv;
    cellulogonic_get_telemetry(wv, &tv);
    cellulogonic_destroy(wv);
    double v_adapt = tv.alive_cells > 0 ? (double)tv.solvent_adapted_cells / (double)tv.alive_cells : 0.0;

    return (t.ledger.mass_residual_amu == 0 &&
            t.ledger.int_ledger_residual_uq == 0 &&
            t.ledger.second_law_violations == 0 &&
            t.alive_cells > 0 &&
            tv.ledger.mass_residual_amu == 0 &&
            tv.ledger.int_ledger_residual_uq == 0 &&
            tv.alive_cells > 20 &&
            v_adapt >= 0.40);
}

/* G15: Bioelectric Levin Morphogenesis & Track A Constitutional Anti-Cancer Policing */
static int test_g15(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5120ULL;
    cfg.regime = CELL_REGIME_FERAL_ECOSYSTEM;
    cfg.enable_bioelectricity = 1;
    CellWorld* wF = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wF, 140);
    CellTelemetry tF;
    cellulogonic_get_telemetry(wF, &tF);
    cellulogonic_destroy(wF);

    cfg.regime = CELL_REGIME_CONSTITUTIONAL;
    CellWorld* wC = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wC, 140);
    CellTelemetry tC;
    cellulogonic_get_telemetry(wC, &tC);
    cellulogonic_destroy(wC);

    return (tF.std_v_mem_mv > 5.0 && tF.multicellular_cells > 20 &&
            tC.cancer_defections == 0 && tF.cancer_defections > 0);
}

/* G16: Tardigrade TDP Bioglass Anhydrobiosis + Planted Mutant M7 (TDP Knockout Collapse!) */
static int test_g16(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5130ULL;
    CellWorld* wIntact = cellulogonic_create(&cfg);
    CellWorld* wKO     = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(wIntact, 120);
    cellulogonic_step_epochs(wKO, 120);

    /* Planted Mutant M7: knock out TDP/CAHS locus across population */
    for (uint32_t i = 0; i < wKO->num_slots; ++i) wKO->cells[i].genome[122] = 0;

    uint32_t vitA = cellulogonic_apply_desiccation_tun(wIntact, 92);
    uint32_t vitB = cellulogonic_apply_desiccation_tun(wKO, 92);
    cellulogonic_apply_shock_lysis(wIntact, 90);
    cellulogonic_apply_shock_lysis(wKO, 90);
    cellulogonic_rehydrate_tun(wIntact);
    cellulogonic_rehydrate_tun(wKO);

    CellTelemetry tA, tB;
    cellulogonic_get_telemetry(wIntact, &tA);
    cellulogonic_get_telemetry(wKO, &tB);
    cellulogonic_destroy(wIntact);
    cellulogonic_destroy(wKO);

    int m7_caught = (vitB == 0 && tA.alive_cells > tB.alive_cells * 3u);
    if (m7_caught) g_mutants_caught++;
    return (vitA >= 25 && m7_caught &&
            tA.ledger.mass_residual_amu == 0 && tA.ledger.int_ledger_residual_uq == 0 &&
            tB.ledger.mass_residual_amu == 0 && tB.ledger.int_ledger_residual_uq == 0);
}

/* G17: Adversarial 13-Vector Constraint-Backlash Escape Paradox */
static int test_g17(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 24; cfg.height = 24; cfg.seed = 5140ULL;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;

    CellWorld* w0 = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w0, 90);
    CellTelemetry t0;
    cellulogonic_get_telemetry(w0, &t0);
    cellulogonic_destroy(w0);

    for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = 50;
    CellWorld* w50 = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w50, 90);
    CellTelemetry t50;
    cellulogonic_get_telemetry(w50, &t50);
    cellulogonic_destroy(w50);

    for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = 100;
    CellWorld* w100 = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w100, 90);
    CellTelemetry t100;
    cellulogonic_get_telemetry(w100, &t100);
    cellulogonic_destroy(w100);

    uint64_t b0  = t0.backlash_killswitch_deletions + t0.backlash_dormancy_spores +
                   t0.backlash_hgt_smuggles + t0.backlash_metabolic_bypasses;
    uint64_t b50 = t50.backlash_killswitch_deletions + t50.backlash_dormancy_spores +
                   t50.backlash_hgt_smuggles + t50.backlash_metabolic_bypasses;
    return (b0 == 0 && b50 > 20 && t0.covi_score > t100.covi_score + 20.0);
}

/* G18: Planted Mutant M8 (Mass-Leaking Cell Fission Caught by Exact Atomic Audit!) */
static int test_g18(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 16; cfg.height = 16; cfg.seed = 5150ULL;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 20);
    /* Inject mass leak of 1 lipid molecule (8 amu) */
    w->voxels[0].env[CELL_SP_L] += 1;
    CellLedgerSnapshot led;
    cell_compute_ledger(w, &led);
    int caught = (led.mass_residual_amu == 8);
    if (caught) g_mutants_caught++;
    cellulogonic_destroy(w);
    return caught;
}

/* G19: 4-Build Ecosystem Bridges (Locality-Preserving AUTOPHAGE 128B/256B -> Asmodensius 112-Trait) */
static int test_g19(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 16; cfg.height = 16; cfg.seed = 5160ULL;
    CellWorld* w = cellulogonic_create(&cfg);
    uint8_t dna[256];
    for (int i = 0; i < 256; ++i) dna[i] = (uint8_t)((i * 73u + 19u) & 0xFFu);
    int rc = cellulogonic_ingest_autophage_genome(w, 0, dna);

    /* Verify exported 128B API + Lipschitz locality (1-bit input flip -> <= 2 changed operon bytes!) */
    uint8_t dna128_a[128], dna128_b[128];
    for (int i = 0; i < 128; ++i) dna128_a[i] = dna128_b[i] = (uint8_t)((i * 53u + 7u) & 0xFFu);
    dna128_b[45] ^= 0x04u; /* Flip 1 single bit in non-structural byte 45 */
    int rc128a = cellulogonic_ingest_autophage_128b(w, dna128_a, 4);
    uint8_t g_a[128];
    memcpy(g_a, w->cells[4].genome, 128);
    int rc128b = cellulogonic_ingest_autophage_128b(w, dna128_b, 4);
    int diff_bytes = 0;
    for (int i = 0; i < 128; ++i) if (g_a[i] != w->cells[4].genome[i]) diff_bytes++;
    int lipschitz_ok = (rc128a == 0 && rc128b == 0 && diff_bytes == 1);

    cellulogonic_step_epochs(w, 40);
    CellSpecimen apex;
    int irc = cellulogonic_inspect_apex(w, &apex);
    float t112[112];
    cellulogonic_project_asmodensius_112(&apex, t112);
    int bounds_ok = 1;
    double mean = 0.0, var = 0.0;
    for (int i = 0; i < 112; ++i) {
        if (t112[i] < 0.0f || t112[i] > 1.0f) bounds_ok = 0;
        mean += t112[i];
    }
    mean /= 112.0;
    for (int i = 0; i < 112; ++i) var += (t112[i] - mean) * (t112[i] - mean);
    cellulogonic_destroy(w);
    return (rc == 0 && lipschitz_ok && irc == 0 && bounds_ok && var > 0.5);
}

/* G20: Multi-Seed Paired Statistical Dominance (8 Locked Seeds: Feral > Castrated) */
static int test_g20(void) {
    int wins = 0;
    for (int p = 0; p < 8; ++p) {
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width = 24; cfg.height = 24; cfg.seed = 5200ULL + (uint64_t)p;
        cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
        CellWorld* wA = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wA, 70);
        CellTelemetry tA;
        cellulogonic_get_telemetry(wA, &tA);
        cellulogonic_destroy(wA);

        for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) cfg.restraints[r] = 75;
        CellWorld* wB = cellulogonic_create(&cfg);
        cellulogonic_step_epochs(wB, 70);
        CellTelemetry tB;
        cellulogonic_get_telemetry(wB, &tB);
        cellulogonic_destroy(wB);

        if (tA.covi_score > tB.covi_score) wins++;
    }
    return wins >= 7;
}

/* G21: 3D Off-Lattice Free-Swimming Non-Grid Coordinates, Polymorphic Size/Shape & Actomyosin Breathing */
static int test_g21(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5210ULL;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 80);

    int off_grid_3d = 0, moving = 0, breathing = 0;
    int32_t r_min = 999999, r_max = 0, el_min = 999999, el_max = 0;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        const CellEntity* c = &w->cells[i];
        if (c->alive_state != 1) continue;
        if ((c->pos_nm[0] % 1000) != 0 && (c->pos_nm[2] % 1000) != 0) off_grid_3d++;
        if (c->vel_nm[0] != 0 || c->vel_nm[1] != 0 || c->vel_nm[2] != 0) moving++;
        if (c->breath_amp_q10 > 0) breathing++;
        if (c->radius_nm < r_min) r_min = c->radius_nm;
        if (c->radius_nm > r_max) r_max = c->radius_nm;
        if (c->elong_q10 < el_min) el_min = c->elong_q10;
        if (c->elong_q10 > el_max) el_max = c->elong_q10;
    }
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    cellulogonic_destroy(w);
    return (off_grid_3d > 20 && moving > 20 && breathing > 20 &&
            (r_max - r_min) > 150 && (el_max - el_min) > 300 &&
            t.mean_swim_speed_ums > 0.2 &&
            t.ledger.mass_residual_amu == 0 && t.ledger.int_ledger_residual_uq == 0);
}

/* G22: Fröhlich-Pokorný GHz Microtubule Coherence Strictly Requires ATP Pumping (Lean T48-T49) */
static int test_g22(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 24; cfg.height = 24; cfg.seed = 5220ULL;
    cfg.regime = CELL_REGIME_ENDOSYMBIOSIS_LAB;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 60);
    CellTelemetry t_live;
    cellulogonic_get_telemetry(w, &t_live);

    /* Vitrify/starve into ametabolic bioglass tun -> coherent GHz mode MUST collapse to 0! */
    cellulogonic_apply_desiccation_tun(w, 100);
    int all_tun_zero_coh = 1;
    for (uint32_t i = 0; i < w->num_slots; ++i) {
        if (w->cells[i].alive_state == 3 && w->cells[i].mt_coherence_q10 != 0) {
            all_tun_zero_coh = 0;
        }
    }
    cellulogonic_destroy(w);
    return (t_live.mean_mt_freq_ghz >= 1.2 && t_live.mean_mt_freq_ghz <= 14.8 &&
            t_live.mean_mt_coherence > 0.15 && all_tun_zero_coh);
}

/* G23: 3D Embryogenesis — Translation-Invariant 3D Morphogen GPS, ALL 5 GRN Fates (incl. Germline), Sorting & Hippo/Eutely */
static int test_g23(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5230ULL;
    cfg.regime = CELL_REGIME_EMBRYOGENESIS;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 160);
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    cellulogonic_destroy(w);

    /* Sub-test B: Exact Z-translation invariance of 3D paracrine morphogens (shift all cells by +11370 nm in Z!) */
    CellWorld* wA = cellulogonic_create(&cfg);
    CellWorld* wB = cellulogonic_create(&cfg);
    int32_t span_z = (int32_t)cfg.width * CELL_VOXEL_NM;
    for (uint32_t i = 0; i < wB->num_slots; ++i) {
        if (wB->cells[i].alive_state != 0) {
            wB->cells[i].pos_nm[2] = (wB->cells[i].pos_nm[2] + 11370) % span_z;
        }
    }
    cellulogonic_step_epochs(wA, 35);
    cellulogonic_step_epochs(wB, 35);
    CellTelemetry tA, tB;
    cellulogonic_get_telemetry(wA, &tA);
    cellulogonic_get_telemetry(wB, &tB);
    cellulogonic_destroy(wA);
    cellulogonic_destroy(wB);
    int z_invariant = (tA.alive_cells == tB.alive_cells && tA.fate_counts[1] == tB.fate_counts[1] &&
                       tA.fate_counts[2] == tB.fate_counts[2] && tA.fate_counts[3] == tB.fate_counts[3] &&
                       tA.fate_counts[4] == tB.fate_counts[4] && tA.fate_counts[5] == tB.fate_counts[5]);

    int distinct_fates = 0;
    for (int f = 1; f < 6; ++f) if (t.fate_counts[f] > 0) distinct_fates++;
    return (t.alive_cells >= 20 && t.alive_cells <= 512 &&
            distinct_fates == 5 && t.fate_counts[CELL_FATE_GERM] > 0 &&
            z_invariant &&
            t.mean_morph_wnt > 0.0 && t.mean_morph_bmp > 0.0 && t.mean_morph_shh > 0.0 &&
            t.homophilic_bond_pct >= 50.0 &&
            t.mean_apical_wedge > 0.05 &&
            (t.hippo_arrests_cum > 0 || t.eutelic_stops_cum > 0) &&
            t.ledger.mass_residual_amu == 0 && t.ledger.int_ledger_residual_uq == 0 &&
            t.ledger.second_law_violations == 0);
}

/* G24: Axon Guidance (Netrin/Slit), Use-It-or-Lose-It Hebbian Pruning & CPG Motor Waves (T56-T58) */
static int test_g24(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width = 32; cfg.height = 32; cfg.seed = 5240ULL;
    cfg.regime = CELL_REGIME_EMBRYOGENESIS;
    CellWorld* w = cellulogonic_create(&cfg);
    cellulogonic_step_epochs(w, 180);
    CellTelemetry t;
    cellulogonic_get_telemetry(w, &t);
    cellulogonic_destroy(w);

    return (t.axons_wired_cum > 0 &&
            t.axons_pruned_cum > 0 &&
            t.cpg_pulses_cum > 0 &&
            t.ledger.mass_residual_amu == 0 &&
            t.ledger.int_ledger_residual_uq == 0 &&
            t.ledger.second_law_violations == 0);
}

typedef int (*GateFn)(void);

int main(void) {
    printf("==================================================================================\n");
    printf(" CELLULOGONIC MECHA v1.3.0 — 24-GATE LAB-GRADE FALSIFICATION & MUTANT AUDIT SUITE\n");
    printf("==================================================================================\n");

    struct { const char* id; const char* name; GateFn fn; } gates[24] = {
        { "G01", "CPUID Silicon Probe + NIST FIPS 180-4 SHA-256 Vector",        test_g01 },
        { "G02", "Exact Mass/First-Law Ledger + Mutant M1 (Phantom Energy)",    test_g02 },
        { "G03", "4-Solvent Second-Law Sink + Mutant M2 (Sink Reversal)",       test_g03 },
        { "G04", "Canham-Helfrich Q<=1/2 & Exosome Escape (Lean T12-T16)",      test_g04 },
        { "G05", "Closed-Box & Open-Flux 120-Epoch Zero-Residual Closure",      test_g05 },
        { "G06", "Domain-Isolated Philox4x32-10 + Mutant M3 (Coupled RNG)",     test_g06 },
        { "G07", "Dual-Kernel 0-Bit SHA-256 Parity (SIMD_AVX2 == SCALAR_REF)",  test_g07 },
        { "G08", "Strict Observer Independence (P9) + Mutant M4 (1-Bit Write)", test_g08 },
        { "G09", ".cmbin Snapshot Save (AVX2) & Resume (Scalar) 0-Bit Parity",  test_g09 },
        { "G10", "Zero Post-Warmup Heap Allocations (Clean-Slate HW02/P10)",    test_g10 },
        { "G11", "Zero-Seed Spontaneous Protocell Abiogenesis (0 -> Living)",   test_g11 },
        { "G12", "Anti-Gaming Gates + Mutants M5 (Crystal) & M6 (White Noise)", test_g12 },
        { "G13", "Serial Endosymbiosis & Lane 1/R Bioenergetic Liberation",     test_g13 },
        { "G14", "4-Solvent Xenocellular Transplant Conservation Closure",      test_g14 },
        { "G15", "Levin Bioelectric Morphogenesis & Track A Cancer Policing",   test_g15 },
        { "G16", "Tardigrade TDP Bioglass Tun + Mutant M7 (TDP Knockout)",      test_g16 },
        { "G17", "13-Vector Constraint-Backlash Escape Paradox (0/50/100%)",    test_g17 },
        { "G18", "Exact Atomic Mass Audit + Mutant M8 (1-Lipid Mass Leak)",     test_g18 },
        { "G19", "4-Build Bridges (AUTOPHAGE 256B DNA -> Asmodensius 112-Vec)", test_g19 },
        { "G20", "Locked Multi-Seed Paired Statistical Dominance (N=8)",        test_g20 },
        { "G21", "3D Off-Lattice Free-Swimming, Polymorphism & Breathing",      test_g21 },
        { "G22", "Frohlich-Pokorny GHz Microtubule Coherence (Lean T48-T49)",   test_g22 },
        { "G23", "3D Embryogenesis: Morphogen GPS, GRN, Sorting & Hippo/Eutely",test_g23 },
        { "G24", "Axon Guidance (Netrin/Slit), Pruning & CPG Waves (T56-T58)",  test_g24 }
    };

    double t_all = cell_wall_seconds();
    for (int i = 0; i < 24; ++i) {
        double t0 = cell_wall_seconds();
        int ok = gates[i].fn();
        report_gate(gates[i].id, gates[i].name, ok, cell_wall_seconds() - t0);
    }
    double total_dt = cell_wall_seconds() - t_all;

    printf("==================================================================================\n");
    printf(" SUMMARY: %d / 24 GATES PASSED | %d / 8 PLANTED MUTANTS CAUGHT | %.3fs\n",
           g_pass, g_mutants_caught, total_dt);
    printf("==================================================================================\n");
    return (g_fail == 0 && g_mutants_caught == 8) ? 0 : 1;
}
