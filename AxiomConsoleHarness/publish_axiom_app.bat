@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0publish_axiom_app.ps1" %*
exit /b %errorlevel%
