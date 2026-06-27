@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0manual_recovery_session.ps1" %*
exit /b %errorlevel%
