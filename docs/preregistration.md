# Pre-Registered Experimental Hypotheses & Statistical Protocol

**Study:** CELLULOGONIC MECHA Multi-Arm Paired Ablation Suite (`src/main_cli.c` -> `cmd_stats`)  
**Seed Range (Locked):** `24` paired deterministic seeds `5000..5023` (`Philox4x32-10`, domain-isolated streams)  
**Horizon:** `180` epochs per arm in the canonical `32×32×32 μm` periodic 3D off-lattice fluid chamber  
**Inference Test:** Exact-style deterministic 10,000-iteration two-sided paired sign-flip permutation test (`p_raw = (count + 1) / 10001` minimum resolution `0.00010`) with step-down **Holm–Bonferroni FWER correction** across all `m = 4` primary hypotheses ($\alpha_{\mathrm{FWER}} = 0.05$).

## Primary Pre-Registered Hypotheses (`H1`–`H4`)

| ID | Arm A (Treatment) | Arm B (Matched Control) | Primary Endpoint | Direction | Falsification Criterion |
|---|---|---|---|---|---|
| **H1** | Feral ecosystem (`0%` restraints) | `70%` global Omega restraint | Final composite `COVI` | `A > B` | `p_Holm >= 0.05` or `Mean A <= Mean B` |
| **H2** | Serial endosymbiosis enabled | Endosymbiosis ablated (prokaryotes only) | Maximum cell volume (`maxVol`) | `A > B` | `p_Holm >= 0.05` or `Mean A <= Mean B` |
| **H3** | Bioelectric gap-junction pumping ON | Bioelectricity ablated (passive uniform `-45 mV`) | Intercellular voltage dispersion $\sigma(V_m)$ | `A > B` | `p_Holm >= 0.05` or $\sigma_A \le \sigma_B$ |
| **H4** | Intact Tardigrade `TDP/CAHS` bioglass locus | `TDP/CAHS` knockout under 92% desiccation + 90% shock lysis | Surviving cell count post-shock | `A > B` | `p_Holm >= 0.05` or `Mean A <= Mean B` |

## Secondary Diagnostic Experiments (Fixed Seed `5000`)

1. **Zero-Seed Abiogenesis (`abiogenesis`, 240 epochs):** Verify spontaneous condensation from `0` initial cells to living autopoietic populations (`COVI > 30`, `0 amu / 0 uq` residual).
2. **De-Novo 4-Solvent Evolution & Terran→Venus Transplant (`xenocell`, 240 epochs):** Verify adaptation from uniform `25.0%` neutral genomes strictly honoring Lean `T28`–`T30` c-ring floors (`Titan >= 4`, `Terran/Vent >= 3`, `Venus >= 2`).
3. **Zero-Blueprint 5-Fate 3D Embryogenesis (`embryo`, 200 epochs):** Verify clonal differentiation of 8 identical-DNA blastomeres into all 5 fates (`Ec, N, M, G, Gm > 0`), Steinberg sorting lift, axon pruning, CPG firing, and exact Z-translation invariance (`G23`).
4. **13-Vector Restraint Backlash Sweep (`ablation`, 180 epochs):** Map `0% / 50% / 100%` profiles across all 13 Omega vectors to test for non-monotonic partial-restraint backlash vs. total collapse.

## Verified Outcome Archive

All four primary hypotheses achieved unanimous `24 / 24` paired wins (`96 / 96` total, `p_raw = 0.00010`, `p_Holm = 0.00040`, recorded verbatim in [`results/09_paired_stats_n24.txt`](../results/09_paired_stats_n24.txt) and re-verified bit-exact under `v1.5.0`).
