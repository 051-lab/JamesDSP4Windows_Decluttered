@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_release_preflight.ps1" %*
exit /b %errorlevel%
