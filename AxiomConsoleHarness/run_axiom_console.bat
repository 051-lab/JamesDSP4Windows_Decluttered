@echo off
cd /d "%~dp0..\build-axiom-console"
AxiomJamesDSPConsole.exe --watch-config -c "%~dp0axiom-liveprog-test.ini"
