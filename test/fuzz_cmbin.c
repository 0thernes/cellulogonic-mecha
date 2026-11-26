/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue
 * libFuzzer target for the "CMB4" binary snapshot parser (cellulogonic_load_state).
 */
#include "cellulogonic_abi.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!data || size == 0 || size > (1u << 22)) return 0;
    const char* tmp = "/tmp/cellulogonic_fuzz_cmb4.cmbin";
    FILE* f = fopen(tmp, "wb");
    if (!f) return 0;
    fwrite(data, 1, size, f);
    fclose(f);

    CellConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.grid_w = 16;
    cfg.grid_h = 16;
    cfg.seed = 5000u;
    CellWorld* w = cellulogonic_create(&cfg);
    if (w) {
        (void)cellulogonic_load_state(w, tmp);
        cellulogonic_destroy(w);
    }
    return 0;
}
