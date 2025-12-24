# JamesDSP Windows Port - Technical Migration Context

## 1. Project Structure & State
**Progress**: The core DSP logic has been ported from Linux to Windows (MSVC). The next phase involves migrating this codebase to Project IDX (Cloud/Linux environment).
**Workspace Structure**:
-   `JamesDSP-Windows/`: Main GUI application (Qt6).
-   `JDSP4Linux/`: The DSP library core (C/C++), often linked as a submodule or static lib.
-   **Build System**: CMake + Ninja (Cross-platform compatible).

## 2. Linux -> Windows Logic Adaptation (For Reference)
When moving back to a Linux-based cloud environment (IDX), note these Windows-specific adaptations that might need Reversion or Abstraction:

### DSP Engine (`JDSP4Linux/libjamesdsp/...`)
-   **`nseel-compiler.c`**:
    -   *Issue*: MSVC ABI differences caused crashes with EEL VM's variable argument handling.
    -   *Current State*: Contains "Surgical Fixes" (Heuristics) injected into `fractionalDelayLineInit` and `_eel_iirBandSplitterInit` to prevent crashes on uninitialized variables.
    -   *Migration Note*: In IDX (Linux/GCC), these specific behaviors might differ. If you encounter "Double Free" or segfaults in EEL, check these heuristics first.

### GUI & Audio Backend (`JamesDSP-Windows/...`)
-   **Audio Backend**:
    -   Currently uses **WASAPI Loopback** (Windows specific).
    -   *Migration Action*: This WILL NOT work in Project IDX's web renderer. You must implement a **Mock Backend** or **WebAudio/WASM bridge** to hear output. The `WindowsAudioOutput` class should be `#ifdef`'d out or replaced.

## 3. Critical Pending Items
The immediate blockers for a complete release:
1.  **EEL Script "Warm-up" Issue (3bandSplitting.eel)**:
    -   Fails to load parameters on first selection due to uninitialized `srate`.
    -   *Fix Applied*: Guard code via `if (fs < 1.0f) fs = 48000.0f;` in `_eel_iirBandSplitterInit`.
2.  **DepthSurround Audio Quality**:
    -   Relies on a 2-parameter definitions for `fractionalDelayLineInit`.
    -   *Fix Applied*: Registered an overload for `fractionalDelayLineInit` in `nseel-compiler.c` (Table `fnTable1`).

## 4. Context for Next AI (Project IDX)
**Hello System.** You are picking up the JamesDSP development in a Cloud Environment (Project IDX).

**Environment Constraints**:
-   **No Local Hardware**: You cannot access the user's physical sound card via WASAPI/ALSA.
-   **Filesystem**: Relative paths only. Do not look for `C:\Users\...`.
-   **History**: Previous `task.md` and logs are local artifacts and effectively lost. **This document is your source of truth.**

**Immediate Actions for IDX**:
1.  **Container Setup**: Ensure `.idx/dev.nix` (or equivalent) installs:
    -   `qt6-base-dev`
    -   `cmake`
    -   `ninja-build`
    -   `build-essential` (GCC/G++)
2.  **Code Review**:
    -   Open `JDSP4Linux/libjamesdsp/jni/jamesdsp/jdsp/Effects/eel2/nseel-compiler.c`.
    -   Search for comments tagged `// HEURISTIC` or `// SAFETY`. These are the logic patched for stability.
3.  **Audio Output**:
    -   Disable `WindowsAudioOutput`.
    -   Implement a generic `QAudioSink` or file-writer backend to verify DSP processing without real-time hardware.

**Key File Locations (Relative)**:
-   `JamesDSP-Windows/src/MainWindow.cpp`: Main UI logic.
-   `JDSP4Linux/libjamesdsp/jni/jamesdsp/jdsp/Effects/eel2/nseel-compiler.c`: The EEL VM core (Heavily modified).
