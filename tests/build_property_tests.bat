@echo off
REM Build script for property-based tests with RapidCheck framework

echo ========================================
echo Building Property-Based Tests
echo ========================================
echo.

REM Compile SWMM mock if not already compiled
if not exist swmm_mock.obj (
    echo Compiling SWMM mock...
    cl /c /EHsc /W3 /MD swmm_mock.cpp
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to compile swmm_mock.cpp
        exit /b 1
    )
    echo [OK] swmm_mock.obj created
    echo.
)

REM Compile and link property test sample
echo Compiling property test sample...
cl /EHsc /W3 /MD test_property_sample.cpp swmm_mock.obj
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile test_property_sample.cpp
    exit /b 1
)
echo [OK] test_property_sample.exe created
echo.

echo ========================================
echo Build Complete
echo ========================================
echo.
echo To run the tests, execute: test_property_sample.exe
echo.
echo Property tests will run with minimum 100 iterations per property
echo.
