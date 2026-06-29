@echo off
setlocal
title Axiom 1-Hour Validation Test

set "SCRIPT_DIR=%~dp0"
set "SEED=%LOCALAPPDATA%\Axiom\Qualification\earpods-controller-state.json"

echo Axiom 1-Hour Validation Test
echo.
echo This will run the installed Axiom JamesDSP Controller for 1 hour.
echo Route seed: %SEED%
echo.
echo Before continuing:
echo   1. Plug in the USB-C EarPods.
echo   2. Close the Axiom JamesDSP Controller if it is open.
echo   3. Keep the laptop on AC power.
echo   4. Do not unplug audio devices or change Windows audio routes during the run.
echo.
pause

if not exist "%SEED%" (
  echo.
  echo ERROR: Route seed was not found:
  echo %SEED%
  echo.
  echo Open the Axiom JamesDSP Controller, select VB-CABLE to EarPods,
  echo then ask Codex to recreate the qualification route seed.
  pause
  exit /b 1
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%tests\run_overnight_gate.ps1" -DurationMinutes 60 -ConfigReloadCount 12 -PreflightObservationSeconds 60 -StateSeedPath "%SEED%"
set "EXITCODE=%ERRORLEVEL%"

echo.
if "%EXITCODE%"=="0" (
  echo Axiom 1-hour validation finished successfully.
) else (
  echo Axiom 1-hour validation ended with errors. Check the newest folder under:
  echo %LOCALAPPDATA%\Axiom\SoakTests
)
echo.
pause
exit /b %EXITCODE%
