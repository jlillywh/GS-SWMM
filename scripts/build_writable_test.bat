@echo off
echo Building writable properties test...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cl /EHsc test_all_writable_properties.cpp /I. lib\swmm5.lib /Fe:test_all_writable_properties.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Running test...
    echo.
    test_all_writable_properties.exe
) else (
    echo Build failed!
)
