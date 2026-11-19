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

echo "[4/4] Building & running Linux ELF64 20-gate falsification suite ..."
"$CC" "${FLAGS[@]}" "${CORE[@]}" "$PROJ/test/test_falsification.c" -o "$PROJ/bin/test_falsification_linux" -lm

cd "$PROJ/bin"
./test_falsification_linux
rm -f test_tmp_cell.cmbin
echo "LINUX ELF64 BUILD & 20-GATE VERIFICATION PASSED!"
