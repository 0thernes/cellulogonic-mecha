name: Proof Challenge / Falsification Audit
description: Challenge a theorem, report a conservation/determinism violation, or audit a scientific claim in CELLULOGONIC MECHA.
title: "[Falsification]: "
labels: ["verification", "falsification"]
body:
  - type: markdown
    attributes:
      value: |
        Thank you for auditing **CELLULOGONIC MECHA**. Every mathematical, biophysical, and evolutionary claim in this repository is meant to be independently falsifiable.
  - type: dropdown
    id: target
    attributes:
      label: Target Claim / Artifact
      options:
        - Theorem I (Stoichiometric Mass & Non-Equilibrium Second Law)
        - Theorem II (Canham-Helfrich Isoperimetric Fission & Budding)
        - Theorem III (Prokaryotic 1/R Bioenergetic Ceiling & Endosymbiotic Liberation)
        - Theorem IV (Chemiosmosis, Bioelectric Laplacian & Anti-Gaming Bounds)
        - Lean 4 Proof Audit (lean/)
        - 20-Gate Falsification Suite / Planted Mutants
        - 4-Way Cross-OS SHA-256 Bit-Exact Determinism
        - Empirical Evolution / Statistical Claim (results/)
    validations:
      required: true
  - type: textarea
    id: reproduction
    attributes:
      label: Exact Reproduction Steps & Seed
      placeholder: Provide OS, compiler/Lean version, exact CLI command, seed, and observed SHA-256 / residual.
    validations:
      required: true
  - type: textarea
    id: expected_vs_actual
    attributes:
      label: Expected vs. Observed Discrepancy
    validations:
      required: true
