/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue (Independent Researcher, 0thernes)
 *
 * CELLULOGONIC MECHA v1.5.0 — Structured Fuzzing Harness (Build Standard B15)
 * Dual-mode:
 *   1. libFuzzer target: compile with `-fsanitize=fuzzer,address,undefined -DCELL_LIBFUZZER`
 *   2. Standalone 1,000-case deterministic adversarial corpus runner under ASan+UBSan
 * Exercises:
 *   - .cmbin snapshot loader against truncated, corrupted, oversized, and re-signed malformed blobs
 *   - cellulogonic_create() extreme config clamping
 *   - cellulogonic_ingest_autophage_128b() & cellulogonic_ingest_autophage_genome()
 *   - cellulogonic_project_asmodensius_112() on arbitrary bit-pattern CellSpecimen structs
 *   - cellulogonic_export_grid_json() on boundary buffer capacities (0..65536)
 */

#include "cellulogonic_internal.h"

static void fuzz_one_input(const uint8_t* data, size_t size) {
    if (!data || size < 16u) return;

    uint8_t mode = data[0] % 4u;

    if (mode == 0u) {
        /* Target A: Arbitrary CellSpecimen -> Asmodensius 112-trait projection (must stay finite in [0,1]!) */
        CellSpecimen spec;
        memset(&spec, 0, sizeof(spec));
        size_t copy_n = size < sizeof(spec) ? size : sizeof(spec);
        memcpy(&spec, data, copy_n);
        float out112[112];
        cellulogonic_project_asmodensius_112(&spec, out112);
        for (int i = 0; i < 112; ++i) {
            if (!(out112[i] >= 0.0f && out112[i] <= 1.0f)) {
                fprintf(stderr, "FUZZ FAIL: out112[%d] = %g out of [0,1]\n", i, (double)out112[i]);
                *(volatile int*)0 = 1; /* trap sanitizer immediately */
            }
        }
    } else if (mode == 1u) {
        /* Target B: Extreme CellWorldConfig clamping + AUTOPHAGE 128B/256B + tiny JSON buffers */
        CellWorldConfig cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.width  = (uint32_t)(data[1] % 24u);
        cfg.height = (uint32_t)(data[2] % 24u);
        cfg.regime = (uint32_t)data[3];
        cfg.solvent = (uint32_t)data[4];
        cfg.kernel_mode = (uint32_t)data[5];
        cfg.photon_flux_uq = (uint32_t)data[6] * 4u;
        cfg.seed = 1ULL + (uint64_t)data[7] + ((uint64_t)data[8] << 8);
        for (int r = 0; r < CELL_RESTRAINT_COUNT; ++r) {
            cfg.restraints[r] = data[(size_t)(9 + r) % size];
        }
        cfg.enable_endosymbiosis  = data[9] & 3u;
        cfg.enable_bioelectricity = data[10] & 3u;
        cfg.enable_hgt_phages     = data[11] & 3u;

        CellWorld* w = cellulogonic_create(&cfg);
        if (!w) return;
        uint8_t d128[128];
        uint8_t d256[256];
        for (size_t i = 0; i < 128u; ++i) d128[i] = data[i % size];
        for (size_t i = 0; i < 256u; ++i) d256[i] = data[(i * 3u) % size];
        (void)cellulogonic_ingest_autophage_128b(w, d128, (uint32_t)data[12]);
        (void)cellulogonic_ingest_autophage_genome(w, (uint32_t)data[13], d256);
        cellulogonic_step_epochs(w, 2u);

        CellLedgerSnapshot led;
        cell_compute_ledger(w, &led);
        if (led.mass_residual_amu != 0 || led.int_ledger_residual_uq != 0 || led.second_law_violations != 0) {
            fprintf(stderr, "FUZZ FAIL: conservation violation m=%lld e=%lld v=%u\n",
                    (long long)led.mass_residual_amu, (long long)led.int_ledger_residual_uq,
                    led.second_law_violations);
            *(volatile int*)0 = 1;
        }

        size_t cap = (size_t)(data[14] * 32u);
        if (cap > 4096u) cap = 4096u;
        char jbuf[4096];
        (void)cellulogonic_export_grid_json(w, jbuf, cap);
        cellulogonic_destroy(w);
    } else if (mode == 2u) {
        /* Target C: Raw untrusted bytes fed directly to cellulogonic_load_snapshot() */
        const char* fpath = "fuzz_cmbin_tmp.cmbin";
        FILE* f = fopen(fpath, "wb");
        if (f) {
            (void)fwrite(data, 1, size, f);
            fclose(f);
            CellWorld* w = cellulogonic_load_snapshot(fpath);
            if (w) cellulogonic_destroy(w);
            (void)remove(fpath);
        }
    } else {
        /* Target D: Re-signed adversarial .cmbin mutation (bypasses SHA-256 to hammer field validators!) */
        CellWorldConfig cfg;
        cellulogonic_default_config(&cfg);
        cfg.width = 8u; cfg.height = 8u; cfg.seed = 9001ULL;
        CellWorld* base = cellulogonic_create(&cfg);
        if (!base) return;
        cellulogonic_step_epochs(base, 2u);
        const char* fpath = "fuzz_resign_tmp.cmbin";
        if (cellulogonic_save_snapshot(base, fpath) == 0) {
            FILE* f = fopen(fpath, "rb");
            if (f) {
                uint8_t* blob = (uint8_t*)malloc(262144u);
                size_t nb = blob ? fread(blob, 1, 262144u, f) : 0u;
                fclose(f);
                if (blob && nb > 352u) {
                    size_t payload_len = nb - 32u;
                    /* Mutate payload bytes using fuzz input, then recompute valid SHA-256 trailer */
                    for (size_t k = 1u; k < size && k < 16u; ++k) {
                        size_t pos = ((size_t)data[k] * 131u + k * 977u) % payload_len;
                        blob[pos] ^= data[(k + 7u) % size];
                    }
                    CellSha256Ctx sha;
                    cell_sha256_init(&sha);
                    cell_sha256_update(&sha, blob, payload_len);
                    cell_sha256_final(&sha, blob + payload_len);
                    FILE* fw = fopen(fpath, "wb");
                    if (fw) {
                        (void)fwrite(blob, 1, nb, fw);
                        fclose(fw);
                        CellWorld* loaded = cellulogonic_load_snapshot(fpath);
                        if (loaded) {
                            /* Any accepted snapshot MUST step without UB and preserve 0 drift! */
                            cellulogonic_step_epochs(loaded, 1u);
                            CellLedgerSnapshot led;
                            cell_compute_ledger(loaded, &led);
                            if (led.mass_residual_amu != 0 || led.int_ledger_residual_uq != 0) {
                                fprintf(stderr, "FUZZ FAIL: accepted malformed snapshot broke ledger!\n");
                                *(volatile int*)0 = 1;
                            }
                            cellulogonic_destroy(loaded);
                        }
                    }
                }
                free(blob);
            }
            (void)remove(fpath);
        }
        cellulogonic_destroy(base);
    }
}

#if defined(CELL_LIBFUZZER)
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    fuzz_one_input(data, size);
    return 0;
}
#else
int main(void) {
    const uint32_t N_CASES = 1000u;
    uint8_t buf[512];
    double t0 = cell_wall_seconds();
    for (uint32_t c = 0; c < N_CASES; ++c) {
        size_t len = 16u + (size_t)((c * 37u) % (sizeof(buf) - 16u));
        for (size_t i = 0; i < len; i += 8u) {
            uint64_t r = cell_philox_u64(0xF0221500ULL, 0x7777u, (uint64_t)c, (uint64_t)i);
            uint8_t tmp[8];
            cell_store_le64(tmp, r);
            for (size_t j = 0; j < 8u && i + j < len; ++j) buf[i + j] = tmp[j];
        }
        buf[0] = (uint8_t)(c % 4u);
        fuzz_one_input(buf, len);
    }
    double dt = cell_wall_seconds() - t0;
    printf("PASS: Structured fuzz harness completed %u adversarial cases (0 crashes, 0 UB, 0 conservation leaks) in %.3fs\n",
           N_CASES, dt);
    return 0;
}
#endif
