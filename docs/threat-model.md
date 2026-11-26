# Threat Model (`docs/threat-model.md`)

STRIDE threat analysis for **CELLULOGONIC MECHA `v1.5.0`** covering the native C11 binary parser, local WebGL2 Observatory HTTP bridge, Lean 4 formal proof pipeline, and supply chain (Build Standard `B36`).

## 1. Trust Boundaries & Attack Surfaces

1. **Untrusted Binary Snapshot Input (`cellulogonic_load_snapshot()` in `src/abi_exports.c`):** Parsing `.cmbin` files from external collaborators or archives.
2. **Local HTTP Bridge (`web/server.mjs` on `127.0.0.1:8788`):** Browser cross-site request forgery (CSRF), oversized payloads, path traversal, or worker queue exhaustion from malicious browser tabs.
3. **Formal Proof Integrity (`lean/`):** Subtle `sorry`, `native_decide`, `autoImplicit` typo-generalization, or `Challenge`/`Solution` definition mismatch sneaking past review.
4. **Supply Chain & Secrets:** Third-party package compromise or accidental credential leaks.

## 2. STRIDE Matrix & Verified Mitigations

| STRIDE Threat | Concrete Scenario | Implemented Mitigation | Verification Artifact |
|---|---|---|---|
| **Tampering** | Crafted or bit-flipped `.cmbin` file injects impossible negative masses, invalid cell/bond indices, or trailing bytes. | `"CMB4"` format validates magic, ABI version, NIST FIPS 180-4 SHA-256 trailer, explicit zeroed padding (`_Static_assert`), slot/bond/axon bounds, bond symmetry, exact EOF, and `0 amu / 0 uq` ledger closure. | `G06` tampered & trailing-EOF rejection in `test/test_falsification.c`; `test/fuzz_cmbin.c` (`libFuzzer`); ASan/UBSan CI job. |
| **Spoofing / CSRF** | Malicious website sends cross-origin `GET` or form `POST` to `http://127.0.0.1:8788` to reset or inject DNA into the local Observatory. | Loopback-only bind (`127.0.0.1`); mutating `GET` blocked with `405`; `POST /api/cmd` requires `Content-Type: application/json`, strict `Sec-Fetch-Site` check, `X-Content-Type-Options: nosniff`, and strict `CSP`. | `web/server.mjs` HTTP handler hardening. |
| **Denial of Service** | Flood of oversized HTTP bodies or concurrent simulation commands exhausts memory or deadlocks the C11 `stream` worker. | Immediate `413` socket destroy above byte limit; strict `400` URL parse guard; serialized FIFO queue with `429` backpressure cap; `0` post-warmup heap allocations in C11 core (`P10`). | `G10` (`post_warmup_allocs == 0`); `web/server.mjs` queue cap. |
| **Information Disclosure** | Static file handler leaks arbitrary host files or `.git`/`.env` via `../` path traversal. | Explicit allowlist of served files (`index.html`, `styles.css`, `app.js`, `renderer3d.js`); all other paths rejected; zero personal data collected (`personal_data: false`). | `web/server.mjs` static allowlist; `gitleaks` CI scan. |
| **Elevation / Formal Proof Spoofing** | A Lean theorem silently uses `sorryAx`, `native_decide`, `autoImplicit`, or alters a shared definition between spec and proof. | `autoImplicit = false`, `relaxedAutoImplicit = false`; pure `Spec.lean` shared by isolated `Challenge.lean` & `Solution.lean`; 6 independent kernels (`lean`, `leanchecker-paranoid`, `lean4lean`, `nanoda_bin` with `unpermitted_axiom_hard_error: true`, `con-leche`, `con-ron`); `check_readme_axioms.py`. | `results/lean4-audit.txt`, `results/lean4-multi-kernel.txt`, `.github/workflows/ci.yml`. |
| **Supply Chain Compromise** | Malicious npm, PyPI, or C library dependency injected during build or execution. | **Zero external runtime dependencies** across C11, Node.js, Python (stdlib only), and Lean 4 (zero Mathlib); SHA-256 manifests (`CHECKSUMS.sha256`, `data/MANIFEST.sha256`) + SLSA `attest-build-provenance` in CI. | `NOTICE`, `THIRD_PARTY_NOTICES.md`, `.github/workflows/ci.yml`. |
