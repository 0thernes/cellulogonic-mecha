#!/usr/bin/env bash
set -euo pipefail

PROJ="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$PROJ/bin" "$PROJ/results"

CC="${CC:-gcc}"
FLAGS=(-O3 -std=c11 -Wall -Wextra -Wno-unused-parameter -march=native -mavx2 -mbmi -mbmi2 -mpopcnt -I"$PROJ/include")
CORE=(
  "$PROJ/src/crypto_rng.c"
  "$PROJ/src/chemoton_physics.c"
  "$PROJ/src/cellular_world.c"
  "$PROJ/src/metrology_covi.c"
  "$PROJ/src/abi_exports.c"
)

echo "[1/4] Building Linux ELF64 bin/cellulogonic_linux ..."
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/src/main_cli.c" -o "$PROJ/bin/cellulogonic_linux" -lm

echo "[2/4] Building Linux shared library bin/libcellulogonic.so ..."
"$CC" "${FLAGS[@]}" -fPIC -shared "${CORE[@]}" -o "$PROJ/bin/libcellulogonic.so" -lm

echo "[3/4] Building Linux static archive bin/libcellulogonic_linux.a ..."
OBJS=()
for s in "${CORE[@]}"; do
  o="$PROJ/bin/$(basename "$s" .c)_linux.o"
  "$CC" -c -fPIC "${FLAGS[@]}" "$s" -o "$o"
  OBJS+=("$o")
done
ar rcs "$PROJ/bin/libcellulogonic_linux.a" "${OBJS[@]}"
rm -f "${OBJS[@]}"

echo "[4/4] Building & running Linux ELF64 24-gate falsification suite ..."
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/test/test_falsification.c" -o "$PROJ/bin/test_falsification_linux" -lm

cd "$PROJ/bin"
./test_falsification_linux
rm -f test_tmp_cell.cmbin test_tamper_tmp.cmbin

if command -v lake >/dev/null 2>&1; then
  echo "Verifying 62 Pure Lean 4 Mathematical Proofs & Strict Axiom Allowlist..."
  cd "$PROJ/lean"
  lake build
  AUDIT_OUT="$(lake env lean CellulogonicProofs.lean)"
  python3 - "$AUDIT_OUT" <<'PY'
import re, sys
text = sys.argv[1]
allowed = {"propext", "Classical.choice", "Quot.sound"}
forbidden = re.compile(r"(warning|error|sorryAx|declaration uses 'sorry'|bv_decide|native_decide|ofReduceBool|trustCompiler)", re.I)
if forbidden.search(text):
    sys.exit("ERROR: Lean kernel integrity violation detected in audit output!")
matches = re.findall(r"'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])", text)
if len(matches) < 62:
    sys.exit(f"ERROR: expected >= 62 audited theorems, found {len(matches)}")
for name, _, axs in matches:
    for ax in [a.strip() for a in axs.split(",") if a.strip()]:
        if ax not in allowed:
            sys.exit(f"ERROR: forbidden axiom '{ax}' in {name}")
print(f"Lean 4 Strict Axiom Audit PASSED ({len(matches)} theorems, strictly subset of {{propext, Classical.choice, Quot.sound}})")
PY
fi

echo "LINUX ELF64 BUILD & 24-GATE VERIFICATION PASSED!"
