# PowerShell script to build and run the integration test
# This script finds Visual Studio tools and compiles the test

Write-Host "=== GoldSim-SWMM Bridge Integration Test ===" -ForegroundColor Cyan
Write-Host ""

# Find MSBuild
Write-Host "Looking for MSBuild..." -ForegroundColor Yellow
$msbuild = Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Filter "MSBuild.exe" -ErrorAction SilentlyContinue | 
           Select-Object -First 1 -ExpandProperty FullName

if (-not $msbuild) {
    Write-Host "ERROR: MSBuild not found. Please install Visual Studio 2022." -ForegroundColor Red
    exit 1
}

Write-Host "Found MSBuild: $msbuild" -ForegroundColor Green
Write-Host ""

# Find vcvars64.bat
Write-Host "Looking for Visual Studio environment setup..." -ForegroundColor Yellow
$vsInstallPath = Split-Path (Split-Path (Split-Path (Split-Path $msbuild)))
$vcvars = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvars)) {
    Write-Host "ERROR: vcvars64.bat not found at $vcvars" -ForegroundColor Red
    exit 1
}

Write-Host "Found vcvars64.bat: $vcvars" -ForegroundColor Green
Write-Host ""

# Check if DLL exists
Write-Host "Checking for GSswmm.dll..." -ForegroundColor Yellow
if (-not (Test-Path "GSswmm.dll")) {
    Write-Host "ERROR: GSswmm.dll not found in tests directory" -ForegroundColor Red
    Write-Host "Please build the DLL first using Visual Studio or MSBuild" -ForegroundColor Red
    exit 1
}

Write-Host "Found GSswmm.dll" -ForegroundColor Green
Write-Host ""

# Check if treatment train model exists
Write-Host "Checking for treatment train model..." -ForegroundColor Yellow
if (-not (Test-Path "treatment_train_model.inp")) {
    Write-Host "ERROR: treatment_train_model.inp not found" -ForegroundColor Red
    exit 1
}

Write-Host "Found treatment_train_model.inp" -ForegroundColor Green
Write-Host ""

# Copy treatment train model to model.inp
Write-Host "Copying treatment train model to model.inp..." -ForegroundColor Yellow
Copy-Item "treatment_train_model.inp" "model.inp" -Force
Write-Host "Model copied" -ForegroundColor Green
Write-Host ""

# Compile the integration test
Write-Host "Compiling integration test..." -ForegroundColor Yellow
Write-Host ""

# Create a temporary batch file to set up environment and compile
$tempBat = "temp_compile.bat"
@"
@echo off
call "$vcvars" >nul 2>&1
cl /EHsc /I..\include test_integration_treatment_train.cpp /link /OUT:test_integration_treatment_train.exe
exit /b %ERRORLEVEL%
"@ | Out-File -FilePath $tempBat -Encoding ASCII

# Run the compilation
$compileResult = & cmd /c $tempBat
$compileExitCode = $LASTEXITCODE

# Clean up temp file
Remove-Item $tempBat -ErrorAction SilentlyContinue

if ($compileExitCode -ne 0) {
    Write-Host "ERROR: Compilation failed" -ForegroundColor Red
    exit 1
}

Write-Host "Compilation successful" -ForegroundColor Green
Write-Host ""

# Run the test
Write-Host "=== Running Integration Test ===" -ForegroundColor Cyan
Write-Host ""

$testResult = & .\test_integration_treatment_train.exe
$testExitCode = $LASTEXITCODE

Write-Host ""
Write-Host "=== Test Complete ===" -ForegroundColor Cyan

if ($testExitCode -eq 0) {
    Write-Host "ALL TESTS PASSED!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "SOME TESTS FAILED!" -ForegroundColor Red
    exit 1
}
