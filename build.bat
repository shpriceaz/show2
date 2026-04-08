@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0"
cl show.cpp /EHsc /W3 /Zi /Od /Fe:show.exe psapi.lib user32.lib kernel32.lib
echo Build exit code: %ERRORLEVEL%
