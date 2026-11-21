param(
    [switch]$SkipTests,
    [switch]$SkipLean
)

$ErrorActionPreference = "Stop"
$proj  = Split-Path -Parent $MyInvocation.MyCommand.Path
$root  = "$env:USERPROFILE\.elan\toolchains\leanprover--lean4---v4.35.0"
$clang = "$root\bin\clang.exe"
$lake  = "$root\bin\lake.exe"
$ar    = "$root\bin\llvm-ar.exe"

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

$cflags = @(
    "-O3", "-std=c11", "-Wall", "-Wextra", "-Wno-unused-parameter",
    "-march=native", "-mavx2", "-mbmi", "-mbmi2", "-mpopcnt",
    "--target=x86_64-w64-windows-gnu",
    "-DCELLULOGONIC_BARE_WIN64",
    "-I$proj\include",
    "-isystem", "$root\include\clang"
)
$ldflags = @("-L$root\lib", "-fuse-ld=lld")
$libs    = @("-lkernel32", "-lucrtbase", "-lm", "-lmingw32", "-lmingwex")

Write-Host "[1/4] Building bin\cellulogonic.exe ..."
& $clang @cflags @core "$proj\src\main_cli.c" -o "$proj\bin\cellulogonic.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[2/4] Building bin\cellulogonic.dll (+ import lib) ..."
& $clang @cflags -shared -DCELLULOGONIC_BUILD_DLL @core -o "$proj\bin\cellulogonic.dll" "-Wl,--out-implib,$proj\bin\libcellulogonic.dll.a" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[3/4] Building bin\test_falsification.exe ..."
& $clang @cflags @core "$proj\test\test_falsification.c" -o "$proj\bin\test_falsification.exe" @ldflags @libs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (Test-Path -LiteralPath $ar) {
    Write-Host "[4/4] Building static archive bin\libcellulogonic.a ..."
    $objs = @()
    foreach ($src in $core) {
        $obj = "$proj\bin\" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
        & $clang -c @cflags $src -o $obj
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        $objs += $obj
    }
    & $ar rcs "$proj\bin\libcellulogonic.a" @objs
    Remove-Item -Force -LiteralPath $objs
}

if (-not $SkipTests) {
    Write-Host "`nRunning 24-Gate Lab-Grade Falsification Suite (Win64 PE64)..."
    Push-Location -LiteralPath "$proj\bin"
    & ".\test_falsification.exe"
    $rc = $LASTEXITCODE
    Remove-Item -Force -LiteralPath "test_tmp_cell.cmbin","test_tamper_tmp.cmbin" -ErrorAction SilentlyContinue
    Pop-Location
    if ($rc -ne 0) { exit $rc }
}

if (-not $SkipLean -and (Test-Path -LiteralPath $lake)) {
    Write-Host "`nVerifying 58 Pure Lean 4 Mathematical Proofs & Strict Axiom Allowlist..."
    Push-Location -LiteralPath "$proj\lean"
    & $lake build
    $lrc = $LASTEXITCODE
    if ($lrc -ne 0) { Pop-Location; exit $lrc }

    $auditLines = & $lake env lean CellulogonicProofs.lean 2>&1
    $arc = $LASTEXITCODE
    Pop-Location
    if ($arc -ne 0) {
        $auditLines | ForEach-Object { Write-Host $_ }
        throw "Lean 4 axiom audit failed with exit code $arc"
    }

    $auditText = ($auditLines | Out-String)
    $allowed = @("propext", "Classical.choice", "Quot.sound")
    $theoremMatches = [regex]::Matches($auditText, "'([^']+)'\s+(does not depend on any axioms|depends on axioms:\s*\[([^\]]*)\])")
    if ($theoremMatches.Count -lt 58) {
        throw "Expected >= 58 audited Lean 4 theorems, found $($theoremMatches.Count)"
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
    Write-Host "Lean 4 Formal Verification PASSED ($($theoremMatches.Count) theorems, 0 sorryAx, strictly within {propext, Classical.choice, Quot.sound})!"
}

Write-Host "`nCELLULOGONIC MECHA v1.3.0 BUILD & VERIFICATION COMPLETE."
