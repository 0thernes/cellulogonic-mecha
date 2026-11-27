/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue (Independent Researcher, 0thernes)
 *
 * Minimal runnable C11 ABI quickstart for CELLULOGONIC MECHA v1.7.0.
 * Demonstrates creating a deterministic 3D Embryogenesis chamber, stepping
 * 120 epochs, verifying 0-amu mass & 0-uq First-Law energy closure, and
 * projecting the apex specimen to the Asmodensius 112-trait vector.
 */

#if defined(CELLULOGONIC_BARE_WIN64)
#  include "bare_crt.h"
#else
#  include <stdio.h>
#endif
#include "cellulogonic_abi.h"

int main(void) {
    CellWorldConfig cfg;
    cellulogonic_default_config(&cfg);
    cfg.width  = 32u;
    cfg.height = 32u;
    cfg.regime = CELL_REGIME_EMBRYOGENESIS;
    cfg.seed   = 5000ULL;

    CellWorld* world = cellulogonic_create(&cfg);
    if (!world) {
        fprintf(stderr, "Failed to allocate CellWorld\n");
        return 1;
    }

    cellulogonic_step_epochs(world, 120u);

    CellTelemetry tel;
    cellulogonic_get_telemetry(world, &tel);

    printf("CELLULOGONIC MECHA ABI 0x%08x Quickstart\n", cellulogonic_abi_version());
    printf("  Epoch       : %llu\n", (unsigned long long)tel.epoch);
    printf("  Alive cells : %u (Ec=%u N=%u M=%u G=%u Gm=%u)\n",
           tel.alive_cells,
           tel.fate_counts[CELL_FATE_ECTO],
           tel.fate_counts[CELL_FATE_NEURAL],
           tel.fate_counts[CELL_FATE_MUSCLE],
           tel.fate_counts[CELL_FATE_GUT],
           tel.fate_counts[CELL_FATE_GERM]);
    printf("  Conservation: mass_res=%lld amu | energy_res=%lld uq | 2nd_law_viol=%u\n",
           (long long)tel.ledger.mass_residual_amu,
           (long long)tel.ledger.int_ledger_residual_uq,
           tel.ledger.second_law_violations);
    printf("  State SHA256: %s\n", tel.state_sha256);

    CellSpecimen apex;
    if (cellulogonic_inspect_apex(world, &apex) == 0) {
        float traits[112];
        cellulogonic_project_asmodensius_112(&apex, traits);
        printf("  Apex cell   : slot=%u vol=%lld Q=%.3f Vm=%d mV traits[0..3]=%.3f,%.3f,%.3f,%.3f\n",
               apex.slot_index, (long long)apex.total_volume, apex.reduced_Q, apex.v_mem_mv,
               (double)traits[0], (double)traits[1], (double)traits[2], (double)traits[3]);
    }

    cellulogonic_destroy(world);
    return (tel.ledger.mass_residual_amu == 0 &&
            tel.ledger.int_ledger_residual_uq == 0 &&
            tel.ledger.second_law_violations == 0) ? 0 : 1;
}
