<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Security Policy (`SECURITY.md`)

## Supported Versions

| Version | Supported |
|---|---|
| `1.6.x` (`main`) | Yes |
| `< 1.6.0` | No (please upgrade to `v1.6.0`) |

## Reporting a Vulnerability

Please report security issues **privately** rather than opening a public GitHub issue:
- **GitHub Private Security Advisory:** [https://github.com/0thernes/cellulogonic-mecha/security/advisories/new](https://github.com/0thernes/cellulogonic-mecha/security/advisories/new)
- **Direct Email:** `0_0@0thernes.art` (Alexander Stephen Donahue)

Include reproduction steps, affected component (`src/abi_exports.c` `.cmbin` parser, `web/server.mjs` HTTP bridge, CLI, or build scripts), and expected vs. actual behavior. Reports receive an initial acknowledgment within 72 hours.

## Security Architecture Summary

- **Zero External Runtime Dependencies:** Eliminates third-party supply-chain package compromises in both the C11 engine and the Node.js WebGL2 Observatory.
- **Authenticated `"CMB5"` `.cmbin` Snapshots:** `cellulogonic_load_snapshot()` verifies magic header (`"CMB5"`), ABI version (`0x00010600`), SHA-256 integrity trailer, accumulator overflow bounds (`<= 1e15`), explicit zeroed padding (`_Static_assert`, including dead organelle slots), genotype/phenotype sync (`lipid_class`, `c_ring_nH`), active-only bond & neural-only axon invariants, no-trailing-EOF bytes, and exact `0 amu / 0 uq` ledger closure; fuzzed via dual-mode `test/fuzz_cmbin_abi.c` (1,000-case standalone + `libFuzzer` CI smoke).
- **Loopback-Only CSRF & DNS-Rebinding Hardened Observatory (`web/server.mjs`):** Binds strictly to `127.0.0.1`, validates exact-port loopback `Host`/`Origin` on ALL static and API routes, exposes `/healthz`, `204 /favicon.ico`, and read-only `GET /api/state`, blocks mutating `GET` requests (`405`), requires `POST /api/cmd` with `Content-Type: application/json`, checks `Sec-Fetch-Site`, enforces strict `CSP` (zero `'unsafe-inline'`), `X-Frame-Options: DENY`, `X-Content-Type-Options: nosniff`, Slowloris timeouts, `413` socket destroy, and queue caps (`429`); verified by `test/test_web_server.mjs` (`13/13` PASS).
- **CI Security Scanning:** SHA-256-verified `gitleaks 8.30.1`, `osv-scanner 2.7.1`, and `cppcheck` run on every push in `.github/workflows/ci.yml`.
- **Full Threat Model:** See [`docs/threat-model.md`](docs/threat-model.md).
