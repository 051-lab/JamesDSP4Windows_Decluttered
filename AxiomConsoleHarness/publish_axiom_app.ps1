param(
    [string]$OutputRoot = "",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"
$harness = Split-Path -Parent $MyInvocation.MyCommand.Path
$repo = Split-Path -Parent $harness
$project = Join-Path $harness "AxiomJamesDSPController\AxiomJamesDSPController.csproj"
$console = Join-Path $repo "build-axiom-console\AxiomJamesDSPConsole.exe"
$acceptedEel = Join-Path $repo "JamesDSP-Windows\build-final\assets\Liveprog\axiom_binaural_dsp_v4.1.4.11.eel"

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $harness "dist"
}

$package = Join-Path $OutputRoot "AxiomJamesDSPController-win-x64"
$publish = Join-Path $OutputRoot ".publish"

& (Join-Path $harness "build_axiom_console.bat")
if ($LASTEXITCODE -ne 0) {
    throw "Native processor build failed with exit code $LASTEXITCODE."
}

if (Test-Path $publish) { Remove-Item $publish -Recurse -Force }
if (Test-Path $package) { Remove-Item $package -Recurse -Force }
New-Item $publish -ItemType Directory -Force | Out-Null
New-Item $package -ItemType Directory -Force | Out-Null

dotnet publish $project `
    -c Release `
    -r win-x64 `
    --self-contained true `
    -p:PublishSingleFile=false `
    -p:DebugType=None `
    -p:DebugSymbols=false `
    -o $publish
if ($LASTEXITCODE -ne 0) {
    throw "Controller publish failed with exit code $LASTEXITCODE."
}

Copy-Item (Join-Path $publish "*") $package -Recurse -Force
Copy-Item $console (Join-Path $package "AxiomJamesDSPConsole.exe") -Force
Copy-Item (Join-Path $harness "package-default.ini") (Join-Path $package "axiom-liveprog-test.ini") -Force
Copy-Item (Join-Path $harness "PACKAGE-README.txt") (Join-Path $package "README.txt") -Force

$assets = Join-Path $package "assets\Liveprog"
$runtime = Join-Path $package "runtime"
New-Item $assets -ItemType Directory -Force | Out-Null
New-Item $runtime -ItemType Directory -Force | Out-Null
Copy-Item $acceptedEel (Join-Path $assets "axiom_binaural_dsp_v4.1.4.11.eel") -Force
Copy-Item (Join-Path $harness "runtime\axiom-test-lowcut.eel") $runtime -Force
Copy-Item (Join-Path $harness "runtime\axiom-test-pulse-gate.eel") $runtime -Force

$launcher = @'
@echo off
start "" "%~dp0AxiomJamesDSPController.exe"
'@
Set-Content -Path (Join-Path $package "Launch Axiom.cmd") -Value $launcher -Encoding ASCII

if ($Zip) {
    $zipPath = "$package.zip"
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Compress-Archive -Path (Join-Path $package "*") -DestinationPath $zipPath -CompressionLevel Optimal
    Write-Host "Created package archive: $zipPath"
}

Remove-Item $publish -Recurse -Force
Write-Host "Created package: $package"
