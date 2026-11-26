#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Live Negative Lean 4.35.0 Kernel Rejection Test (Build Standard B06 / Playbook v1.3).

Verifies that the live Lean 4.35.0 axiom audit gate genuinely CATCHES and REJECTS:
  1. A planted `sorry` (`sorryAx`)
  2. A planted custom `axiom` (`BadExploit.bogus`)
  3. A planted `native_decide` (`_native` / `ofReduceBool`)

Must run with `lake` on PATH (skips cleanly only if `--allow-no-lake` is passed).
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

CASES = [
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
    parser = argparse.ArgumentParser(description="Live negative Lean 4.35.0 kernel rejection audit.")
    parser.add_argument("--allow-no-lake", action="store_true", help="Exit 0 with SKIP notice if lake is unavailable")
    args = parser.parse_args()

    lake = shutil.which("lake")
    if not lake:
        if args.allow_no_lake:
            print("SKIP: lake binary not found on PATH (--allow-no-lake).")
            return
        print("FAIL: lake binary not found on PATH.", file=sys.stderr)
        sys.exit(1)

    with tempfile.TemporaryDirectory(prefix="cm_lean_neg_") as tmp:
        tmp_dir = Path(tmp)
        for name, code, expected_marker in CASES:
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
            print(f"  [CAUGHT] {name:<24} -> rejected ({reason})")

    print("PASS: All 3/3 live negative Lean 4.35.0 kernel exploits (sorry, custom axiom, native_decide) were rejected!")


if __name__ == "__main__":
    main()
