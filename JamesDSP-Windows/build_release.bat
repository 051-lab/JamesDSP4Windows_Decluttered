@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 exit /b %errorlevel%

REM Configure Release
cmake -S . -B build-final -G Ninja -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b %errorlevel%

REM Build Release
cmake --build build-final --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

REM Deploy Qt Dependencies (Release)
D:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe --release --no-translations --compiler-runtime build-final\JamesDSP-GUI.exe
if %errorlevel% neq 0 exit /b %errorlevel%
