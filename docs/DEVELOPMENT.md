# Development Guide

## Baseline toolchain

- Windows 10 test environment, preferably a Hyper-V, VMware Workstation, or VirtualBox virtual machine.
- Visual Studio 2022 with **Desktop development with C++**.
- C++20 and x64.
- Windows 10 or Windows 11 SDK.
- Git and CMake 3.25 or newer when implementation begins.

## Phase 0 gate: documentation

Before shell implementation starts:

- [x] safety policy exists;
- [x] architecture and recovery model exist;
- [x] contribution and security policies exist;
- [x] bug and recovery issue templates exist;
- [x] CI documentation checks exist;
- [ ] repository owner reviews the MIT license choice;
- [ ] GitHub private vulnerability reporting is enabled;
- [ ] branch protection requires CI and pull-request review.

## Phase 1 gate: recovery prototype

Build recovery before polished rendering:

1. Detect `Shell_TrayWnd` and `Shell_SecondaryTrayWnd`.
2. Capture native visibility, auto-hide, topmost, monitor, and work-area state.
3. Implement idempotent restore.
4. Register `Ctrl+Alt+Shift+F12`; fail safely if registration fails.
5. Implement the watchdog process.
6. Prove recovery after normal exit and forced main-process termination.
7. Only then create a placeholder AppBar.

## Phase 2 gate: visual bar

- Direct2D rounded capsules and borders.
- DirectWrite clock.
- Per-monitor DPI and display-change handling.
- Correct AppBar work-area reservation.
- Accessible exit command and keyboard navigation.

## Phase 3 gate: task model

- Enumerate eligible top-level windows.
- Resolve icons without retaining stale `HICON` handles.
- Activate, minimize, and restore windows.
- Handle cloaked windows, Explorer restart, and app closure.

## Build policy

The initial repository intentionally contains no functional shell executable. When implementation begins, add a root `CMakeLists.txt` and compile with warnings treated as errors. CI must target x64 and must not run UI integration tests on a shared hosted runner.

Suggested local configure commands after CMake files exist:

```powershell
cmake -S . -B out/build -A x64
cmake --build out/build --config Debug
ctest --test-dir out/build -C Debug --output-on-failure
```

## Testing rules

- Run taskbar-hiding tests in a VM first.
- Take a VM snapshot before watchdog or startup experiments.
- Never test automatic startup until forced-termination recovery passes.
- Keep `Ctrl+Shift+Esc` and `Ctrl+Alt+Delete` functional.
- Test 100%, 125%, 150%, and 200% DPI where available.
- Test native auto-hide both enabled and disabled.
- Test Explorer restart and monitor connect/disconnect.

## Definition of done

A feature is done only when code, error handling, tests, documentation, and rollback behavior are complete. A screenshot is not sufficient evidence for a shell-safety change.
