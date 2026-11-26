#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
set -euo pipefail

PROJ="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$PROJ/bin" "$PROJ/results"

CC="${CC:-cc}"
if [ "$CC" = "cc" ] && command -v gcc >/dev/null 2>&1 && [ "$(uname -s)" = "Linux" ]; then
  CC="gcc"
fi
HOST_OS="$(uname -s)"
HOST_ARCH="$(uname -m)"
ARCH_FLAGS=()
if [ "${NATIVE:-0}" = "1" ]; then
  ARCH_FLAGS=(-march=native)
elif [ "$HOST_ARCH" = "x86_64" ] || [ "$HOST_ARCH" = "amd64" ]; then
  # Portable x86-64 baseline (AVX2 is isolated via __attribute__((target("avx2"))) & runtime OSXSAVE guard, F-02)
  ARCH_FLAGS=(-march=x86-64)
fi

SHLIB_EXT="so"
if [ "$HOST_OS" = "Darwin" ]; then
  SHLIB_EXT="dylib"
fi

FLAGS=(
  -O3 -std=c11
  -Wall -Wextra -Wshadow -Wstrict-prototypes -Wconversion
  -Wno-unused-parameter -Werror
  -ffp-contract=off
  "${ARCH_FLAGS[@]}"
  -I"$PROJ/include"
)

CORE=(
  "$PROJ/src/crypto_rng.c"
  "$PROJ/src/chemoton_physics.c"
  "$PROJ/src/cellular_world.c"
  "$PROJ/src/metrology_covi.c"
  "$PROJ/src/abi_exports.c"
)

echo "[1/6] Building POSIX bin/cellulogonic_linux (${HOST_OS}/${HOST_ARCH}, -Werror) ..."
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/src/main_cli.c" -o "$PROJ/bin/cellulogonic_linux" -lm

echo "[2/6] Building POSIX shared library bin/libcellulogonic.${SHLIB_EXT} ..."
"$CC" "${FLAGS[@]}" -fPIC -shared "${CORE[@]}" -o "$PROJ/bin/libcellulogonic.${SHLIB_EXT}" -lm

echo "[3/6] Building POSIX static archive bin/libcellulogonic_linux.a & auditing zero-heap hot-loop symbols ..."
OBJS=()
for s in "${CORE[@]}"; do
  o="$PROJ/bin/$(basename "$s" .c)_linux.o"
  "$CC" -c -fPIC "${FLAGS[@]}" "$s" -o "$o"
  OBJS+=("$o")
done
if command -v nm >/dev/null 2>&1; then
  for hot in "$PROJ/bin/chemoton_physics_linux.o" "$PROJ/bin/cellular_world_linux.o" "$PROJ/bin/metrology_covi_linux.o"; do
    if nm -u "$hot" | grep -Eq '(^|[[:space:]_])(malloc|calloc|realloc|free)($|[[:space:]@])'; then
      echo "ERROR: HOT-LOOP HEAP ALLOCATION VIOLATION in $hot" >&2
      exit 1
    fi
  done
  echo "  [PASS] Hot-loop symbol audit: chemoton_physics, cellular_world & metrology_covi have ZERO malloc/calloc/realloc/free imports."
fi
ar rcs "$PROJ/bin/libcellulogonic_linux.a" "${OBJS[@]}"
rm -f "${OBJS[@]}"

echo "[4/6] Building POSIX 26-gate falsification suite, ABI fuzzer, and C11 quickstart ..."
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/test/test_falsification.c" -o "$PROJ/bin/test_falsification_linux" -lm
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/test/fuzz_cmbin_abi.c" -o "$PROJ/bin/fuzz_cmbin_abi_linux" -lm
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/examples/quickstart_c_abi.c" -o "$PROJ/bin/quickstart_c_abi_linux" -lm

echo "[5/6] Running POSIX 26-gate falsification suite + 1000-case fuzzer + quickstart ..."
(
  cd "$PROJ/bin"
  ./test_falsification_linux
  rm -f test_tmp_cell.cmbin test_tamper_tmp.cmbin
  ./fuzz_cmbin_abi_linux 1000
  ./quickstart_c_abi_linux
)

if [ "${SKIP_LEAN:-0}" != "1" ] && command -v lake >/dev/null 2>&1; then
  echo "[6/6] Verifying Pure Lean 4.35.0 Proofs & Strict Axiom Allowlist..."
  cd "$PROJ/lean"
  lake build
  AUDIT_OUT="$(lake env lean VerifyAxioms.lean 2>&1)"
  python3 - "$AUDIT_OUT" <<'PY'
import re, sys
text = sys.argv[1]
allowed = {"propext", "Classical.choice", "Quot.sound"}
forbidden = re.compile(r"(\bwarning\b|\berror\b|sorryAx|declaration uses 'sorry'|bv_decide|native_decide|ofReduceBool|trustCompiler|Lean\.ofReduceBool|Lean\.trustCompiler)", re.I)
if forbidden.search(text):
    sys.exit("ERROR: Lean kernel integrity violation detected in audit output!\n" + text)
matches = re.findall(r"'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])", text)
if len(matches) < 78:
    sys.exit(f"ERROR: expected >= 78 audited theorems, found {len(matches)}")
for name, _, axs in matches:
    for ax in [a.strip() for a in axs.split(",") if a.strip()]:
        if ax not in allowed:
            sys.exit(f"ERROR: forbidden axiom '{ax}' in {name}")
print(f"Lean 4.35.0 Strict Axiom Audit PASSED ({len(matches)} theorems, strictly subset of {{propext, Classical.choice, Quot.sound}})")
PY
fi

echo "CELLULOGONIC MECHA v1.6.0 POSIX (${HOST_OS}/${HOST_ARCH}) BUILD & 26-GATE VERIFICATION PASSED!"
