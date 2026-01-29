@echo off
echo ========================================
echo Building File Validation Test
echo ========================================
echo.

REM Compile the test
cl /EHsc /W4 /I.. test_file_validation.cpp /Fe:test_file_validation.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo BUILD FAILED!
    exit /b 1
)

echo.
echo ========================================
echo Build Successful - Running Test
echo ========================================
echo.

REM Run the test
test_file_validation.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo ALL TESTS PASSED!
    echo ========================================
) else (
    echo.
    echo ========================================
    echo TESTS FAILED!
    echo ========================================
)

exit /b %ERRORLEVEL%
