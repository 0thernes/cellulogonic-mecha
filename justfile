# CELLULOGONIC MECHA v1.5.0 — One-Word Task Runner (Build Standard A31, B01)
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue

set shell := ["bash", "-euo", "pipefail", "-c"]
set windows-shell := ["pwsh", "-NoProfile", "-Command"]

# Print available recipes
default:
    @just --list

# Build native C11 binaries and run 26-gate / 8-mutant falsification + 1,000-case ABI fuzz suite
[unix]
build:
    ./build_linux.sh

[windows]
build:
    .\build.ps1

# Run native 26-gate falsification suite + 1,000-case ABI fuzz + 2,700 randomized property checks
[unix]
test:
    ./bin/test_falsification_linux
    ./bin/fuzz_cmbin_abi_linux 1000
    python3 scripts/prop_check.py

[windows]
test:
    .\bin\test_falsification.exe
    .\bin\fuzz_cmbin_abi.exe 1000
    python3 scripts/prop_check.py

# Compile and run C11 falsification suite with ASan + UBSan sanitizers (Linux/macOS, B16)
[unix]
sanitize:
    gcc -O1 -g -std=c11 -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude src/crypto_rng.c src/chemoton_physics.c src/cellular_world.c src/metrology_covi.c src/abi_exports.c test/test_falsification.c -o bin/test_falsification_asan -lm
    ./bin/test_falsification_asan
    rm -f test_tmp_cell.cmbin test_tamper_tmp.cmbin bin/test_falsification_asan

# Build libFuzzer harness for binary "CMB4" .cmbin snapshot parser (Clang Linux, B15)
[unix]
fuzz:
    clang -O1 -g -std=c11 -fsanitize=fuzzer,address,undefined -Iinclude src/crypto_rng.c src/chemoton_physics.c src/cellular_world.c src/metrology_covi.c src/abi_exports.c test/fuzz_cmbin.c -o bin/fuzz_cmbin -lm
    ./bin/fuzz_cmbin -max_total_time=15

# Report-only static analysis and linter pass (B17)
[unix]
lint:
    command -v cppcheck >/dev/null && cppcheck --enable=warning,style --quiet -Iinclude src/ test/ || true
    command -v clang-tidy >/dev/null && clang-tidy src/*.c -- -std=c11 -Iinclude || true
    command -v ruff >/dev/null && ruff check scripts/ examples/ || true

# Build Pure Lean 4.35.0 suite (Solution + Challenge) and audit verbatim axioms
lean:
    cd lean && lake build && lake build Challenge && lake env lean VerifyAxioms.lean > ../results/lean4-audit.txt
    python3 scripts/check_readme_axioms.py

# Run full 6-kernel Palomar comparator (Linux with bubblewrap)
[unix]
lean-multi:
    cd lean && lake env leanchecker --fresh Solution && lake env leanchecker-paranoid --fresh Solution && lake check --paranoid && lake comparator --config comparator.json --paranoid

# Verify live DOIs in paper/refs.bib
citations:
    python3 scripts/check_citations.py

# Compile AMS-LaTeX white paper, check 0 overfull hboxes, package & verify clean-room arXiv tarball
paper:
    python3 scripts/build_paper.py

# Run Build Standard v1.2.0 self-audit
[unix]
audit:
    bash scripts/audit-repo.sh .

[windows]
audit:
    pwsh -NoProfile -File scripts/Audit-Repo.ps1

# Refresh SHA-256 manifests
checksums:
    python3 -c "import hashlib, pathlib; root=pathlib.Path('.'); targets=sorted(list(root.glob('dist/*')) + list(root.glob('results/*')) + list(root.glob('paper/main.tex')) + list(root.glob('paper/refs.bib')) + list(root.glob('lean/comparator.json'))); lines=[f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.as_posix()}' for p in targets if p.is_file()]; (root/'CHECKSUMS.sha256').write_text('\n'.join(lines)+'\n', encoding='utf-8'); data_t=sorted(p for p in root.glob('results/*') if p.is_file()); (root/'data'/'MANIFEST.sha256').write_text('\n'.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.as_posix()}' for p in data_t)+'\n', encoding='utf-8'); print(f'Wrote {len(lines)} root checksums and {len(data_t)} data checksums.')"

# Launch CSRF-hardened 60 FPS WebGL2 3D Observatory on 127.0.0.1:8788
serve:
    node web/server.mjs

# Full end-to-end clean-clone reproduction pipeline (B01)
reproduce: build test lean citations paper audit
