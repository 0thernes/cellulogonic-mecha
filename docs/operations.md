<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Operations, Backups & Offsite Mirror Runbook (`docs/operations.md`)

This runbook covers local execution of the C11 engine and Observatory server, deterministic backup via `git bundle`, archive registration, DigiCert RFC 3161 timestamping, and offsite mirroring (Build Standard `A35`).

## 1. Local Service Operation (`web/server.mjs`)

- **Start:** `node web/server.mjs` (or `just serve`) — binds strictly to `127.0.0.1:8788` (configurable via `PORT` in `.env`).
- **Health / Readiness Probe:** `GET http://127.0.0.1:8788/healthz` returns HTTP `200` JSON `{status, version, worker_alive, queue_depth}` (`503` if degraded); `GET /api/state` returns live JSON telemetry without mutating world state (`P9` observer independence).
- **State Mutations:** Exclusively via `POST /api/cmd` with `Content-Type: application/json` and same-origin `Sec-Fetch-Site` validation; legacy mutating `GET` routes return HTTP `405`.
- **Failure Recovery:** If the child C11 `stream` process exits unexpectedly, `web/server.mjs` logs the exit code to `stderr`, drains pending FIFO requests cleanly, and respawns on the next `init` command.

## 2. Offline Backup (`git bundle`), RFC 3161 Timestamp & Offsite Mirror Policy (`A35`)

To prevent single-point loss of source code, Lean 4 proofs, empirical logs, and frozen preprints:

1. **Primary GitHub Origin & Pages Portal:** `https://github.com/0thernes/cellulogonic-mecha` (`main` branch + semantic tags `v*` + static 78-theorem Blueprint at `https://0thernes.github.io/cellulogonic-mecha/`).
2. **Master Archive Pin & Mirror:** Pinned as `0TH-BUILD-001` with frozen PDF, submodule commit, and SHA-256 manifest in `0thernes/0thernes-research-archive`.
3. **Self-Contained Offline `git bundle` Backup:**
   ```bash
   git bundle create ../cellulogonic-mecha-v1.7.0.bundle --all
   git bundle verify ../cellulogonic-mecha-v1.7.0.bundle
   ```
   Verify periodically that the bundle restores cleanly into an isolated temporary directory (`git clone ../cellulogonic-mecha-v1.7.0.bundle /tmp/restore-test`).
4. **Dual Independent Cryptographic Timestamps (RFC 3161 + Bitcoin OpenTimestamps):** `scripts/stamp_timestamps.py` anchors LF-normalized `CHECKSUMS.sha256` via BOTH DigiCert's public RFC 3161 TSA (`data/timestamps/CHECKSUMS.sha256.tsr`) AND three Bitcoin OpenTimestamps calendars (`data/timestamps/CHECKSUMS.sha256.ots`), independently verifiable via `python3 scripts/stamp_timestamps.py --verify` and verified on every CI run.
5. **Offsite Archival Mirror (Software Heritage):** Preserved under SWHID `swh:1:snp:8c1696436104a888897ce3b97867eda983d56160`; future updates can be queued via Software Heritage Save Code Now (`https://archive.softwareheritage.org/save/origin/https://github.com/0thernes/cellulogonic-mecha/visit/git/`).
6. **Checksum Verification:** Verify frozen artifacts at any time against [`CHECKSUMS.sha256`](../CHECKSUMS.sha256) and [`data/MANIFEST.sha256`](../data/MANIFEST.sha256):
   ```bash
   sha256sum -c CHECKSUMS.sha256
   sha256sum -c data/MANIFEST.sha256
   ```
