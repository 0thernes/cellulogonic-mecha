#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Live Negative Lean 4.35.0 Kernel & Elaboration Rejection Test (Build Standard B06 / Playbook v1.3).

Verifies that the live Lean 4.35.0 elaborator and axiom audit gate genuinely CATCH and REJECT:
  1. Direct Lean elaboration rejection of a false theorem (`1 = 2 := by omega`)
  2. Live `sorry`-tamper injection against `CellulogonicProofs.Spec` (`T01` mass theorem tampered)
  3. Standalone planted `sorry` (`sorryAx`)
  4. Planted custom `axiom` (`bad_custom_ax`)
  5. Planted `native_decide` (`_native` / `ofReduceBool`)

Must run with `lake` available (skips cleanly only if `--allow-no-lake` is passed).
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LEAN_DIR = ROOT / "lean"
ALLOWED = {"propext", "Classical.choice", "Quot.sound"}
FORBIDDEN_RE = re.compile(r"(sorryAx|ofReduceBool|trustCompiler|_native)", re.I)

ELAB_REJECT_CASES = [
    (
        "false_claim_elaboration",
        "theorem bad_false_claim : (1 : Nat) = 2 := by omega\n",
    ),
    (
        "false_spec_claim_elaboration",
        (
            "import CellulogonicProofs.Spec\n"
            "open Cellulogonic.StoichiometryThermo in\n"
            "theorem bad_titan_cring3 : qHeat .Titan 3 .R3_atp_synth > 0 := by decide\n"
        ),
    ),
]

AXIOM_REJECT_CASES = [
    (
        "live_spec_sorry_tamper",
        (
            "import CellulogonicProofs.Spec\n"
            "open Cellulogonic.StoichiometryThermo in\n"
            "theorem T01_tampered (r : RxnId) : rxnMassDelta r = 0 := by sorry\n"
            "#print axioms T01_tampered\n"
        ),
        "sorryAx",
    ),
    (
        "planted_sorry",
        "theorem bad_sorry : 1 = 2 := by sorry\n#print axioms bad_sorry\n",
        "sorryAx",
    ),
    (
        "planted_custom_axiom",
        "axiom bad_custom_ax : 1 = 2\ntheorem bad_ax_thm : 1 = 2 := bad_custom_ax\n#print axioms bad_ax_thm\n",
        "bad_custom_ax",
    ),
    (
        "planted_native_decide",
        "theorem bad_native : (1 : Nat) + 1 = 2 := by native_decide\n#print axioms bad_native\n",
        "_native",
    ),
]


def find_lake() -> str | None:
    found = shutil.which("lake")
    if found:
        return found
    cand = Path.home() / ".elan" / "bin" / ("lake.exe" if sys.platform == "win32" else "lake")
    if cand.exists():
        return str(cand)
    return None


def audit_accepts(output: str) -> tuple[bool, str]:
    if FORBIDDEN_RE.search(output):
        return False, "forbidden axiom token detected"
    matches = re.findall(
        r"'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])",
        output,
    )
    if not matches:
        return False, "no #print axioms declaration produced"
    for name, _, axs in matches:
        for ax in [a.strip() for a in axs.split(",") if a.strip()]:
            if ax not in ALLOWED:
                return False, f"unpermitted axiom '{ax}' in {name}"
    return True, "accepted"


def main() -> None:
    parser = argparse.ArgumentParser(description="Live negative Lean 4.35.0 elaboration & axiom rejection audit.")
    parser.add_argument("--allow-no-lake", action="store_true", help="Exit 0 with SKIP notice if lake is unavailable")
    args = parser.parse_args()

    lake = find_lake()
    if not lake:
        if args.allow_no_lake:
            print("SKIP: lake binary not found on PATH or ~/.elan/bin (--allow-no-lake).")
            return
        print("FAIL: lake binary not found on PATH or ~/.elan/bin.", file=sys.stderr)
        sys.exit(1)

    with tempfile.TemporaryDirectory(prefix="cm_lean_neg_") as tmp:
        tmp_dir = Path(tmp)

        for name, code in ELAB_REJECT_CASES:
            src = tmp_dir / f"{name}.lean"
            src.write_text(code, encoding="utf-8", newline="\n")
            proc = subprocess.run(
                [lake, "env", "lean", str(src)],
                cwd=LEAN_DIR,
                capture_output=True,
                text=True,
                timeout=60,
            )
            if proc.returncode == 0:
                print(
                    f"FAIL: elaboration-rejection case '{name}' WRONGLY EXITED 0!\n{proc.stdout}\n{proc.stderr}",
                    file=sys.stderr,
                )
                sys.exit(1)
            print(f"  [CAUGHT] {name:<30} -> rejected by Lean elaborator (exit {proc.returncode})")

        for name, code, expected_marker in AXIOM_REJECT_CASES:
            src = tmp_dir / f"{name}.lean"
            src.write_text(code, encoding="utf-8", newline="\n")
            proc = subprocess.run(
                [lake, "env", "lean", str(src)],
                cwd=LEAN_DIR,
                capture_output=True,
                text=True,
                timeout=60,
            )
            combined = (proc.stdout or "") + "\n" + (proc.stderr or "")
            accepted, reason = audit_accepts(combined)
            if accepted:
                print(f"FAIL: negative case '{name}' was WRONGLY ACCEPTED!\n{combined}", file=sys.stderr)
                sys.exit(1)
            if expected_marker not in combined and proc.returncode == 0:
                print(
                    f"FAIL: negative case '{name}' did not emit expected marker '{expected_marker}':\n{combined}",
                    file=sys.stderr,
                )
                sys.exit(1)
            print(f"  [CAUGHT] {name:<30} -> rejected ({reason})")

    total = len(ELAB_REJECT_CASES) + len(AXIOM_REJECT_CASES)
    print(
        f"PASS: All {total}/{total} live negative Lean 4.35.0 elaboration & tamper exploits "
        f"(false claims, live Spec sorry-tamper, standalone sorry, custom axiom, native_decide) were rejected!"
    )


if __name__ == "__main__":
    main()
