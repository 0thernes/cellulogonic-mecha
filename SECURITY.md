# Security Policy (`SECURITY.md`)

## Supported Versions

| Version | Supported |
|---|---|
| `1.5.x` (`main`) | Yes |
| `< 1.5.0` | No (please upgrade to `v1.5.0`) |

## Reporting a Vulnerability

Please report security issues **privately** rather than opening a public GitHub issue:
- **GitHub Private Security Advisory:** [https://github.com/0thernes/cellulogonic-mecha/security/advisories/new](https://github.com/0thernes/cellulogonic-mecha/security/advisories/new)
- **Direct Email:** `0_0@0thernes.art` (Alexander Stephen Donahue)

Include reproduction steps, affected component (`src/abi_exports.c` `.cmbin` parser, `web/server.mjs` HTTP bridge, CLI, or build scripts), and expected vs. actual behavior. Reports receive an initial acknowledgment within 72 hours.

## Security Architecture Summary

- **Zero External Runtime Dependencies:** Eliminates third-party supply-chain package compromises in both the C11 engine and the Node.js WebGL2 Observatory.
- **Authenticated `"CMB4"` `.cmbin` Snapshots:** `cellulogonic_load_snapshot()` verifies magic header, ABI version, SHA-256 integrity trailer, struct bounds, bond symmetry, no-trailing-EOF bytes, and exact `0 amu / 0 uq` ledger closure; fuzzed via `test/fuzz_cmbin.c`.
- **Loopback-Only CSRF-Hardened Observatory (`web/server.mjs`):** Binds strictly to `127.0.0.1`, blocks mutating `GET` requests (`405`), requires `POST /api/cmd` with `Content-Type: application/json`, checks `Sec-Fetch-Site`, sets `X-Content-Type-Options: nosniff` and strict `CSP`, destroys oversized (`413`) sockets immediately, and enforces worker queue caps (`429`).
- **Full Threat Model:** See [`docs/threat-model.md`](docs/threat-model.md).
