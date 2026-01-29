@echo off
echo Building XF_CALCULATE test program...

REM Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

REM Compile the test program
cl /EHsc /Fe:test_calculate.exe test_calculate.cpp >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to compile test program
    echo Trying alternative compilation...
    cl /EHsc test_calculate.cpp /link /out:test_calculate.exe
    if errorlevel 1 (
        echo ERROR: Compilation failed
        exit /b 1
    )
)

echo Test program compiled successfully
echo.

REM Copy DLL to current directory for testing
copy /Y x64\Release\GSswmm.dll . >nul 2>&1
copy /Y swmm5.dll . >nul 2>&1

echo Running XF_CALCULATE tests...
echo.
test_calculate.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Test completed with exit code: %TEST_RESULT%
exit /b %TEST_RESULT%
