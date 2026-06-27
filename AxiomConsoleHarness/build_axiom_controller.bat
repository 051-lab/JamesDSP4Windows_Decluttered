@echo off
setlocal

cd /d "%~dp0AxiomJamesDSPController"
dotnet build -c Release
exit /b %errorlevel%
