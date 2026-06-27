Axiom JamesDSP Controller

Run "Axiom JamesDSP Controller.exe".

First run:
1. Install VB-CABLE from https://vb-audio.com/Cable/ and reboot Windows.
2. Open Routing and choose the physical device you will hear.
3. Select "Use VB-CABLE -> Output".
4. Set VB-CABLE as the Windows source/default when prompted.
5. Start the processor and confirm audio reaches the physical output.
6. Open Profiles and save a named listening profile.

Routing:
- Windows applications play to VB-CABLE.
- Axiom captures VB-CABLE, processes audio through JamesDSP, and renders to the
  selected physical output.
- "Restore Previous Default" returns Windows to the output that was active
  before Axiom took route ownership.
- Setup & System can restore the previous output automatically when Axiom exits.

Profiles:
- Profiles can be created, updated, loaded, duplicated, renamed, deleted,
  imported, and exported.
- The Qualification profile is protected and restores the fixed comparison
  baseline.

Diagnostics:
- Persistent telemetry is stored in:
  %LOCALAPPDATA%\Axiom\JamesDSPController\diagnostics\health-history.jsonl
- Diagnostics can open or clear this history and export a diagnostic report.
- Warnings identify new dropped frames, conversion failures, discontinuities,
  starvation, render errors, and unusually long DSP calls.

Lifecycle options under Setup & System:
- Start with Windows
- Start the processor automatically when the saved route is valid
- Close or minimize to the notification area
- Restore the previous Windows output on exit

All mutable settings, profiles, runtime scripts, and diagnostics are stored
under:
%LOCALAPPDATA%\Axiom\JamesDSPController

Uninstall preserves that Local AppData directory. VB-CABLE is an external
Windows driver and is not redistributed with Axiom.
