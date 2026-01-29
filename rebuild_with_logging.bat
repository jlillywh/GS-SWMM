@echo off
echo ========================================
echo Rebuilding GSswmm.dll with Logging
echo ========================================
echo.

REM Set up Visual Studio environment
echo Setting up Visual Studio build environment...
for /f "tokens=*" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath') do set VS_PATH=%%i
if defined VS_PATH (
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    echo Build environment ready.
) else (
    echo ERROR: Visual Studio not found. Please install Visual Studio with C++ tools.
    pause
    exit /b 1
)
echo.

echo Building Release x64 configuration...
echo Using: "%VS_PATH%\Common7\IDE\devenv.exe"
"%VS_PATH%\Common7\IDE\devenv.exe" GSswmm.sln /rebuild "Release|x64" /out build_output.log

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build Successful!
    echo ========================================
    echo.
    echo New DLL location: x64\Release\GSswmm.dll
    echo.
    echo Next steps:
    echo 1. Copy x64\Release\GSswmm.dll to your GoldSim model directory
    echo 2. Run your GoldSim simulation
    echo 3. Check bridge_debug.log in your model directory
    echo.
    echo See LOGGING_INSTRUCTIONS.md for details
    echo.
) else (
    echo.
    echo ========================================
    echo Build Failed!
    echo ========================================
    echo.
    echo Check build_output.log for error details
    echo.
)

pause
    echo.
)

pause
