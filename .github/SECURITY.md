# Security Policy

**CELLULOGONIC MECHA** is a clean-room, deterministic C11 cellular biophysics simulator and local-only Observatory UI.

- The native C11 engine performs zero network I/O and zero post-warmup dynamic heap allocations.
- The optional Observatory server (`web/server.mjs`) binds strictly to `127.0.0.1` (loopback only).
- Please report memory-safety bugs, parser issues in `.cmbin` snapshot loading, or local server security issues privately to `0_0@0thernes.art`.
