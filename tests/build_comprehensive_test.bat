@echo off
echo Building Comprehensive Test Runner...

cl /EHsc /W3 /MD /I..\include test_runner_comprehensive.cpp /link /OUT:test_runner_comprehensive.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build completed
    echo.
    echo Running comprehensive tests...
    echo.
    test_runner_comprehensive.exe
) else (
    echo [FAIL] Build failed
    exit /b 1
)
