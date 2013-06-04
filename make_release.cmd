@echo off
del Release\Win32\*.exp 2>nul
del Release\Win32\*.lib 2>nul
del Release\x64\*.exp 2>nul
del Release\x64\*.lib 2>nul
7za a T-Clock.zip .\Release\*
pause
