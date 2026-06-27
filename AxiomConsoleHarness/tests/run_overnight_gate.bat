@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0run_overnight_gate.ps1" %*
exit /b %errorlevel%
