@echo off
REM ============================================================================
REM verify_release.bat
REM 
REM Pre-release verification script for GSswmm Bridge
REM Checks that all necessary files are ready for GitHub release
REM ============================================================================

echo ========================================
echo GSswmm Bridge v4.1 Release Verification
echo ========================================
echo.

set ERROR_COUNT=0

REM Check version in source code
echo [1/10] Checking version in SwmmGoldSimBridge.cpp...
findstr /C:"#define DLL_VERSION 4.1" SwmmGoldSimBridge.cpp >nul
if %ERRORLEVEL% EQU 0 (
    echo   [OK] Version 4.1 found in source code
) else (
    echo   [ERROR] Version 4.1 NOT found in source code
    set /A ERROR_COUNT+=1
)
echo.

REM Check README version
echo [2/10] Checking version in README.md...
findstr /C:"DLL Version**: 4.1" README.md >nul
if %ERRORLEVEL% EQU 0 (
    echo   [OK] Version 4.1 found in README
) else (
    echo   [ERROR] Version 4.1 NOT found in README
    set /A ERROR_COUNT+=1
)
echo.

REM Check for Known Limitations section
echo [3/10] Checking for Known Limitations section in README...
findstr /C:"Known Limitations" README.md >nul
if %ERRORLEVEL% EQU 0 (
    echo   [OK] Known Limitations section exists
) else (
    echo   [ERROR] Known Limitations section NOT found
    set /A ERROR_COUNT+=1
)
echo.

REM Check for release notes
echo [4/10] Checking for release notes file...
if exist "RELEASE_NOTES_v4.1.md" (
    echo   [OK] RELEASE_NOTES_v4.1.md exists
) else (
    echo   [ERROR] RELEASE_NOTES_v4.1.md NOT found
    set /A ERROR_COUNT+=1
)
echo.

REM Check for changelog
echo [5/10] Checking for CHANGELOG.md...
if exist "CHANGELOG.md" (
    echo   [OK] CHANGELOG.md exists
) else (
    echo   [ERROR] CHANGELOG.md NOT found
    set /A ERROR_COUNT+=1
)
echo.

REM Check for .gitignore
echo [6/10] Checking for .gitignore...
if exist ".gitignore" (
    echo   [OK] .gitignore exists
) else (
    echo   [WARNING] .gitignore NOT found (optional)
)
echo.

REM Check for release DLL
echo [7/10] Checking for release DLL...
if exist "x64\Release\GSswmm.dll" (
    echo   [OK] x64\Release\GSswmm.dll exists
) else (
    echo   [ERROR] x64\Release\GSswmm.dll NOT found - Build required!
    set /A ERROR_COUNT+=1
)
echo.

REM Check for SWMM dependency
echo [8/10] Checking for SWMM5 DLL...
if exist "swmm5.dll" (
    echo   [OK] swmm5.dll exists
) else (
    echo   [ERROR] swmm5.dll NOT found
    set /A ERROR_COUNT+=1
)
echo.

REM Check for essential documentation
echo [9/10] Checking for essential documentation...
set DOC_OK=1
if not exist "README.md" (
    echo   [ERROR] README.md NOT found
    set /A ERROR_COUNT+=1
    set DOC_OK=0
)
if not exist "BUILD_CONFIGURATION.md" (
    echo   [WARNING] BUILD_CONFIGURATION.md NOT found (optional)
)
if not exist "GOLDSIM_INTERFACE_SETUP.md" (
    echo   [WARNING] GOLDSIM_INTERFACE_SETUP.md NOT found (optional)
)
if %DOC_OK% EQU 1 (
    echo   [OK] Essential documentation exists
)
echo.

REM Check for test files
echo [10/10] Checking for test infrastructure...
if exist "tests\run_all_tests.bat" (
    echo   [OK] Test infrastructure exists
) else (
    echo   [WARNING] Test infrastructure NOT found (optional)
)
echo.

REM Summary
echo ========================================
echo Verification Summary
echo ========================================
if %ERROR_COUNT% EQU 0 (
    echo.
    echo   *** ALL CHECKS PASSED ***
    echo.
    echo   Ready for release! Next steps:
    echo   1. Run tests: cd tests ^&^& run_all_tests.bat
    echo   2. Commit changes: git add . ^&^& git commit -m "Release v4.1"
    echo   3. Tag release: git tag -a v4.1 -m "Version 4.1"
    echo   4. Push: git push origin main ^&^& git push origin v4.1
    echo   5. Create GitHub release using GITHUB_RELEASE_CHECKLIST.md
    echo.
) else (
    echo.
    echo   *** ERRORS FOUND ***
    echo   Error count: %ERROR_COUNT%
    echo.
    echo   Please fix the errors above before releasing.
    echo.
)

pause
