@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_smoke_tests.ps1" %*
exit /b %errorlevel%
