param(
    [ValidateSet("console", "raylib", "all")]
    [string]$Target = "all"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
$SrcDir = "$ProjectRoot\src"
$OutDir = "$ProjectRoot\bin"
$RaylibDir = "$ProjectRoot\raylib"

# Ensure output directory exists
if (!(Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}

function Build-Console {
    Write-Host "Building Console version..." -ForegroundColor Cyan
    g++ -std=c++17 -O2 "$SrcDir\console.cpp" -o "$OutDir\gomoku_console.exe"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Console build success: bin\gomoku_console.exe" -ForegroundColor Green
    }
}

function Build-Raylib {
    Write-Host "Building Raylib version..." -ForegroundColor Cyan
    g++ -std=c++17 -O2 `
        -I "$RaylibDir\include" `
        -L "$RaylibDir\lib" `
        "$SrcDir\game.cpp" `
        -o "$OutDir\gomoku_ui.exe" `
        -lraylib -lopengl32 -lgdi32 -lwinmm
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Raylib build success: bin\gomoku_ui.exe" -ForegroundColor Green
    }
}

switch ($Target) {
    "console" { Build-Console }
    "raylib"  { Build-Raylib }
    "all"     { Build-Console; Build-Raylib }
}
