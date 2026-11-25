# Architecture

## Decision

CustomShellBar will coexist with Windows Explorer rather than replace it.

```text
Windows session
├── explorer.exe
│   ├── Desktop and shell services
│   ├── File Explorer
│   └── Native taskbars, temporarily hidden
├── CustomShellBar.exe
│   ├── AppBar host
│   ├── renderer and input
│   ├── window/task model
│   ├── Start menu
│   ├── settings and diagnostics
│   └── recovery coordinator
└── CustomShellBar.Watchdog.exe
    └── independent liveness monitor and taskbar restoration
```

## Trust boundaries

### Explorer

Explorer remains the Windows shell. The project does not patch its files, replace its executable, or permanently change Winlogon shell configuration.

### Main process

`CustomShellBar.exe` owns custom windows, rendering, AppBar registration, task enumeration, the Start menu, user settings, and orderly cleanup. It must run without administrator rights.

### Watchdog

`CustomShellBar.Watchdog.exe` starts before native taskbars are hidden. It monitors a process handle and a small recovery contract. If the main process exits unexpectedly, the watchdog restores native taskbar visibility and exits. It does not inject into Explorer and does not restart the main process in a loop.

### Optional Windhawk bridge

A future Windhawk mod may expose unsupported shell metadata, such as notification icon information. It must remain optional and cannot own the only recovery path.

## Startup transaction

1. Apply per-monitor DPI awareness.
2. Acquire a single-instance mutex.
3. Discover primary and secondary Explorer taskbars.
4. Capture original taskbar, auto-hide, topmost, monitor, and work-area state.
5. Start the watchdog and establish its recovery contract.
6. Register the emergency hotkey.
7. Create and render custom bars.
8. Register AppBars and verify resulting work areas.
9. Confirm an exit command is reachable.
10. Hide native taskbars.
11. Commit startup state.

Any failure before step 10 leaves native taskbars visible. A failure at or after step 10 invokes idempotent rollback.

## Shutdown transaction

1. Mark shutdown as intentional for the watchdog.
2. Stop accepting new UI actions.
3. Close menus and flyouts.
4. Show native taskbars and restore captured state.
5. Remove AppBar registrations and restore work areas.
6. Unregister global hotkeys and event hooks.
7. Flush settings atomically.
8. Signal the watchdog to exit.
9. Destroy custom windows and terminate.

## Core components

- `TaskbarStateManager`: discovery, capture, hide, show, and restoration.
- `AppBarManager`: `SHAppBarMessage` registration and desktop work-area coordination.
- `WindowTracker`: top-level app enumeration, foreground changes, and icon resolution.
- `Renderer`: Direct2D/DirectWrite drawing; DirectComposition may be added for animation.
- `InputRouter`: mouse, keyboard, accessibility, and context-menu behavior.
- `StartMenu`: app indexing and launch UI, isolated from taskbar recovery.
- `SettingsStore`: validated, atomic JSON writes under the portable folder or `%LOCALAPPDATA%`.
- `Diagnostics`: bounded logs without private window titles by default.

## Non-goals for the first milestone

- replacing Explorer;
- implementing third-party notification icons;
- automatic startup;
- elevation;
- modifying shell registry values;
- a Windhawk dependency;
- per-application audio mixing;
- Wi-Fi connection management.

## Failure model

The design assumes the main UI can crash, hang, or be force-terminated. Therefore recovery cannot depend solely on `WM_DESTROY`, destructors, `atexit`, or structured exception handling. Power loss cannot be cleaned up at runtime, so the next launch and documented manual recovery must tolerate stale settings and state files.
