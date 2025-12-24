@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 exit /b %errorlevel%
cmake -S . -B build-fix -G Ninja
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build build-fix --config Release
if %errorlevel% neq 0 exit /b %errorlevel%
