@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cl /EHsc test_writable_during_step.cpp /I. lib\swmm5.lib /Fe:test_writable_during_step.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    test_writable_during_step.exe
)
