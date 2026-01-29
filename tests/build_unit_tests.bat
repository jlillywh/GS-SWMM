@echo off
REM Build script for unit tests with Google Test framework

echo ========================================
echo Building Unit Tests
echo ========================================
echo.

REM Compile SWMM mock
echo Compiling SWMM mock...
cl /c /EHsc /W3 /MD swmm_mock.cpp
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile swmm_mock.cpp
    exit /b 1
)
echo [OK] swmm_mock.obj created
echo.

REM Compile and link unit test sample
echo Compiling unit test sample...
cl /EHsc /W3 /MD test_unit_sample.cpp swmm_mock.obj
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile test_unit_sample.cpp
    exit /b 1
)
echo [OK] test_unit_sample.exe created
echo.

echo ========================================
echo Build Complete
echo ========================================
echo.
echo To run the tests, execute: test_unit_sample.exe
echo.
