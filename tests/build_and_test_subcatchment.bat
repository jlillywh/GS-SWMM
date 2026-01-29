@echo off
echo ========================================
echo Building Subcatchment Validation Test
echo ========================================
echo.

REM Compile the test program
cl /EHsc /I.. test_subcatchment_validation.cpp /link /LIBPATH:.. swmm5.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Compilation failed
    exit /b 1
)

echo.
echo ========================================
echo Running Subcatchment Validation Test
echo ========================================
echo.

REM Run the test
test_subcatchment_validation.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Tests failed
    exit /b 1
)

echo.
echo ========================================
echo All tests passed successfully!
echo ========================================
