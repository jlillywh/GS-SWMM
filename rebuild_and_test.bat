@echo off
REM Master build and test script for GSswmm project
REM Handles all compilation, file copying, and test execution

setlocal enabledelayedexpansion

echo ============================================================
echo GSswmm: Full Rebuild and Test Suite
echo ============================================================
echo.

REM Define paths
set PROJECT_ROOT=C:\Users\JasonLillywhite\OneDrive - GoldSim\Python\GSswmm
set TESTS_DIR=%PROJECT_ROOT%\tests
set RELEASE_DIR=%PROJECT_ROOT%\x64\Release
set VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community
set MSBUILD=%VS_PATH%\MSBuild\Current\Bin\MSBuild.exe
set VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat

REM Step 1: Rebuild the DLL
echo.
echo [1/6] Rebuilding GSswmm.dll...
call "%VCVARS%" > nul 2>&1
"%MSBUILD%" "%PROJECT_ROOT%\GSswmm.sln" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /v:minimal
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: DLL rebuild failed
    exit /b 1
)
echo [OK] DLL rebuilt successfully
echo.

REM Step 2: Copy DLL to tests directory
echo [2/6] Copying GSswmm.dll to tests directory...
copy "%RELEASE_DIR%\GSswmm.dll" "%TESTS_DIR%\GSswmm.dll" > nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to copy DLL
    exit /b 1
)
echo [OK] DLL copied
echo.

REM Step 3: Copy model.inp to tests directory
echo [3/6] Copying model.inp to tests directory...
copy "%PROJECT_ROOT%\model.inp" "%TESTS_DIR%\model.inp" > nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to copy model.inp
    exit /b 1
)
echo [OK] model.inp copied
echo.

REM Step 4: Build tests
echo [4/6] Building test suite...
cd /d "%TESTS_DIR%"
call "%VCVARS%" > nul 2>&1
echo Compiling swmm_mock...
cl /c /EHsc /W3 /MD swmm_mock.cpp > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile swmm_mock.cpp
    exit /b 1
)
echo Compiling test_unit_sample...
cl /EHsc /W3 /MD test_unit_sample.cpp swmm_mock.obj > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile test_unit_sample.cpp
    exit /b 1
)
echo Compiling test_lifecycle...
cl /EHsc /W3 /MD test_lifecycle.cpp /link "%RELEASE_DIR%\GSswmm.lib" > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile test_lifecycle.cpp
    exit /b 1
)
echo [OK] All tests compiled
echo.

REM Step 5: Run tests
echo [5/6] Running tests...
echo.
echo --- Test: test_unit_sample.exe ---
call test_unit_sample.exe
echo.
echo --- Test: test_lifecycle.exe ---
call test_lifecycle.exe
echo.

REM Step 6: Summary
echo [6/6] Build complete!
echo ============================================================
echo Build Summary:
echo  [OK] DLL rebuilt and deployed
echo  [OK] Model file deployed
echo  [OK] All tests compiled
echo  [OK] Test suite executed
echo ============================================================
echo.
echo Next steps:
echo  - Review test output above
echo  - Edit source files as needed
echo  - Run this script again: rebuild_and_test.bat
echo.
