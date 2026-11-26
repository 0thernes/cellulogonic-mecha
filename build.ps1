# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
param(
    [switch]$SkipTests,
    [switch]$SkipLean,
    [switch]$Native
)

$ErrorActionPreference = "Stop"
$proj  = Split-Path -Parent $MyInvocation.MyCommand.Path
$root  = "$env:USERPROFILE\.elan\toolchains\leanprover--lean4---v4.35.0"
$clang = "$root\bin\clang.exe"
$lake  = "$root\bin\lake.exe"
$ar    = "$root\bin\llvm-ar.exe"
$nm    = "$root\bin\llvm-nm.exe"

if (-not (Test-Path -LiteralPath $clang)) {
    throw "Bundled Clang toolchain not found at $clang"
}

[System.IO.Directory]::CreateDirectory("$proj\bin") | Out-Null
[System.IO.Directory]::CreateDirectory("$proj\results") | Out-Null

$core = @(
    "$proj\src\crypto_rng.c",
    "$proj\src\chemoton_physics.c",
    "$proj\src\cellular_world.c",
    "$proj\src\metrology_covi.c",
    "$proj\src\abi_exports.c"
)

$archFlags = if ($Native) {
    @("-march=native")
} else {
    @("-march=x86-64")
}

$cflags = @(
    "-O3", "-std=c11",
    "-Wall", "-Wextra", "-Wshadow", "-Wstrict-prototypes", "-Wconversion",
    "-Wno-unused-parameter", "-Werror",
    "-ffp-contract=off",
    "--target=x86_64-w64-windows-gnu",
    "-DCELLULOGONIC_BARE_WIN64",
    "-I$proj\include",
    "-isystem", "$root\include\clang"
) + $archFlags

$lto = @("-flto=thin")
$ldflags = @("-L$root\lib", "-fuse-ld=lld")
$libs    = @("-lkernel32", "-lucrtbase", "-lm", "-lmingw32", "-lmingwex")

Write-Host "[1/6] Building bin\cellulogonic.exe (Win64 portable x86-64 + runtime AVX2 dispatch + ThinLTO, -Werror) ..."
& $clang @cflags @lto @core "$proj\src\main_cli.c" -o "$proj\bin\cellulogonic.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[2/6] Building bin\cellulogonic.dll (+ import lib, ThinLTO) ..."
& $clang @cflags @lto -shared -DCELLULOGONIC_BUILD_DLL @core -o "$proj\bin\cellulogonic.dll" "-Wl,--out-implib,$proj\bin\libcellulogonic.dll.a" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[3/6] Building bin\test_falsification.exe (26-gate + 8-mutant suite, ThinLTO) ..."
& $clang @cflags @lto @core "$proj\test\test_falsification.c" -o "$proj\bin\test_falsification.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[4/6] Building bin\fuzz_cmbin_abi.exe & bin\quickstart_c_abi.exe ..."
& $clang @cflags @lto @core "$proj\test\fuzz_cmbin_abi.c" -o "$proj\bin\fuzz_cmbin_abi.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $clang @cflags @lto @core "$proj\examples\quickstart_c_abi.c" -o "$proj\bin\quickstart_c_abi.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (Test-Path -LiteralPath $ar) {
    Write-Host "[5/6] Building static archive bin\libcellulogonic.a & auditing hot-loop zero-heap symbols ..."
    $objs = @()
    foreach ($src in $core) {
        $obj = "$proj\bin\" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
        & $clang -c @cflags $src -o $obj
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        $objs += $obj
    }
    if (Test-Path -LiteralPath $nm) {
        foreach ($hot in @("$proj\bin\chemoton_physics.o", "$proj\bin\cellular_world.o", "$proj\bin\metrology_covi.o")) {
            $syms = (& $nm -u $hot | Out-String)
            if ($syms -match "\b(malloc|calloc|realloc|free)\b") {
                throw "HOT-LOOP HEAP ALLOCATION VIOLATION: $(Split-Path -Leaf $hot) references dynamic allocator symbols!"
            }
        }
        Write-Host "  [PASS] Hot-loop symbol audit: chemoton_physics.o, cellular_world.o & metrology_covi.o have ZERO malloc/calloc/realloc/free imports."
    }
    & $ar rcs "$proj\bin\libcellulogonic.a" @objs
    Remove-Item -Force -LiteralPath $objs
}

if (-not $SkipTests) {
    Write-Host "`n[6/6] Running 26-Gate Falsification Suite + 1000-Case ABI/CMBIN Fuzzer + C11 Quickstart (Win64 PE64)..."
    Push-Location -LiteralPath "$proj\bin"
    & ".\test_falsification.exe"
    $rc = $LASTEXITCODE
    Remove-Item -Force -LiteralPath "test_tmp_cell.cmbin","test_tamper_tmp.cmbin" -ErrorAction SilentlyContinue
    if ($rc -ne 0) { Pop-Location; exit $rc }

    & ".\fuzz_cmbin_abi.exe" 1000
    $frc = $LASTEXITCODE
    if ($frc -ne 0) { Pop-Location; exit $frc }

    & ".\quickstart_c_abi.exe"
    $qrc = $LASTEXITCODE
    Pop-Location
    if ($qrc -ne 0) { exit $qrc }
}

if (-not $SkipLean -and (Test-Path -LiteralPath $lake)) {
    Write-Host "`nVerifying Pure Lean 4.35.0 Proofs (Palomar Solution + Challenge) & Strict Axiom Allowlist..."
    Push-Location -LiteralPath "$proj\lean"
    & $lake build
    $lrc = $LASTEXITCODE
    if ($lrc -ne 0) { Pop-Location; exit $lrc }

    $auditLines = & $lake env lean VerifyAxioms.lean 2>&1
    $arc = $LASTEXITCODE
    Pop-Location
    if ($arc -ne 0) {
        $auditLines | ForEach-Object { Write-Host $_ }
        throw "Lean 4 axiom audit failed with exit code $arc"
    }

    $auditText = ($auditLines -join "`n") + "`n"
    if ($auditText -match "(?i)\b(warning|error)\b|sorryAx|declaration uses 'sorry'|bv_decide|native_decide|ofReduceBool|trustCompiler|Lean\.ofReduceBool|Lean\.trustCompiler") {
        throw "Lean 4 kernel integrity violation detected in VerifyAxioms.lean output!"
    }
    $allowed = @("propext", "Classical.choice", "Quot.sound")
    $theoremMatches = [regex]::Matches($auditText, "'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])")
    if ($theoremMatches.Count -lt 78) {
        throw "Expected >= 78 audited Lean 4 theorems, found $($theoremMatches.Count)"
    }
    foreach ($m in $theoremMatches) {
        if ($m.Groups[3].Success) {
            $axList = $m.Groups[3].Value -split "," | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne "" }
            foreach ($ax in $axList) {
                if ($allowed -notcontains $ax) {
                    throw "FORBIDDEN LEAN AXIOM '$ax' detected in $($m.Groups[1].Value)!"
                }
            }
        }
    }
    $auditPath = "$proj\results\lean4-audit.txt"
    [System.IO.File]::WriteAllText($auditPath, $auditText, [System.Text.UTF8Encoding]::new($false))
    Write-Host "Lean 4.35.0 Formal Verification PASSED ($($theoremMatches.Count) audited theorems, 0 sorryAx, strictly within {propext, Classical.choice, Quot.sound}, 0 warnings)!"
}

Write-Host "`nCELLULOGONIC MECHA v1.6.0 BUILD & VERIFICATION COMPLETE."
