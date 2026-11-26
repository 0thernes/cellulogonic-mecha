# Operations, Backups & Offsite Mirror Runbook (`docs/operations.md`)

This runbook covers local execution of the C11 engine and Observatory server, deterministic backup via `git bundle`, archive registration, and offsite mirroring (Build Standard `A35`).

## 1. Local Service Operation (`web/server.mjs`)

- **Start:** `node web/server.mjs` (or `just serve`) — binds strictly to `127.0.0.1:8788` (configurable via `PORT` in `.env`).
- **Health / Read-Only Probe:** `GET http://127.0.0.1:8788/api/state` returns live JSON telemetry without mutating world state (`P9` observer independence).
- **State Mutations:** Exclusively via `POST /api/cmd` with `Content-Type: application/json` and same-origin `Sec-Fetch-Site` validation; legacy mutating `GET` routes return HTTP `405`.
- **Failure Recovery:** If the child C11 `stream` process exits unexpectedly, `web/server.mjs` logs the exit code to `stderr`, drains pending FIFO requests cleanly, and respawns on the next `init` command.

## 2. Offline Backup (`git bundle`) & Offsite Mirror Policy (`A35`)

To prevent single-point loss of source code, Lean 4 proofs, empirical logs, and frozen preprints:

1. **Primary GitHub Origin:** `https://github.com/0thernes/cellulogonic-mecha` (`main` branch + semantic tags `v*`).
2. **Master Archive Pin & Mirror:** Pinned as a submodule/entry with frozen PDF and SHA-256 manifest in `0thernes/0thernes-research-archive` (`Z:\[Vibe Coded (AI)]\ANTIGRAVITY\0thernes-research-archive`).
3. **Self-Contained Offline `git bundle` Backup:**
   ```bash
   git bundle create ../cellulogonic-mecha-v1.5.0.bundle --all
   git bundle verify ../cellulogonic-mecha-v1.5.0.bundle
   ```
   Verify periodically that the bundle restores cleanly into an isolated temporary directory (`git clone ../cellulogonic-mecha-v1.5.0.bundle /tmp/restore-test`).
4. **Offsite Archival Mirror (Software Heritage):** Public `main` commits and release tags are preserved via Software Heritage (`https://archive.softwareheritage.org/`) Save Code Now snapshots so both source and formal proofs remain permanently retrievable independently of GitHub.
5. **Checksum Verification:** Verify frozen artifacts at any time against [`CHECKSUMS.sha256`](../CHECKSUMS.sha256) and [`data/MANIFEST.sha256`](../data/MANIFEST.sha256):
   ```bash
   sha256sum -c CHECKSUMS.sha256
   sha256sum -c data/MANIFEST.sha256
   ```
