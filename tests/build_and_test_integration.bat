@echo off
echo ========================================
echo Building Integration Test
echo ========================================

REM Try to find and call vcvarsall.bat to set up the build environment
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALLDIR=%%i"
    )
)

if defined VSINSTALLDIR (
    call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else (
    REM Fallback to common paths
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    if errorlevel 1 (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    )
)

REM Compile the integration test
echo Compiling test_integration_treatment_train.cpp...
cl /EHsc /I..\include test_integration_treatment_train.cpp /link /OUT:test_integration_treatment_train.exe >nul 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo Build successful!
echo.
echo ========================================
echo Running Integration Test
echo ========================================
echo.

REM Copy the treatment train model to the expected location
copy treatment_train_model.inp model.inp

REM Run the test
test_integration_treatment_train.exe

set TEST_RESULT=%ERRORLEVEL%

echo.
echo ========================================
echo Test completed with exit code: %TEST_RESULT%
echo ========================================

exit /b %TEST_RESULT%
