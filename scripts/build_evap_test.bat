@echo off
echo Building evaporation test...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /EHsc test_evap_setting.cpp /I. lib\swmm5.lib /Fe:test_evap_setting.exe
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Running test...
    echo.
    test_evap_setting.exe
) else (
    echo Build failed!
)
pause
