# Contributing to CustomShellBar

CustomShellBar changes core Windows UI behavior. Safety and reversibility take priority over appearance.

## Before contributing

1. Read `SAFETY.md`, `docs/ARCHITECTURE.md`, and `docs/DEVELOPMENT.md`.
2. Search existing issues.
3. Use a Windows 10 virtual machine or non-critical test account for changes that hide the native taskbar.
4. Keep Task Manager and the recovery procedure accessible.

## Pull-request requirements

Every pull request must:

- explain the user-visible change;
- identify affected safety invariants;
- include tests or a documented manual test procedure;
- avoid unrelated formatting changes;
- preserve Explorer coexistence and recovery paths;
- update documentation when behavior changes.

Changes that hide the native taskbar must include evidence that normal exit, failed initialization, Explorer restart, and forced main-process termination recover correctly.

## Coding baseline

- C++20, x64, Unicode.
- Visual Studio 2022 and the current Windows 10/11 SDK.
- Treat warnings as errors in CI.
- Prefer RAII for handles and COM initialization.
- Check every Win32/COM result that affects recovery.
- Do not perform shell recovery from global destructors alone.
- Keep UI, taskbar-state management, and watchdog logic in separate components.

## Commit style

Use concise imperative subjects, for example:

- `Add transactional taskbar hiding`
- `Restore AppBar work area on shutdown`
- `Document watchdog heartbeat protocol`

## Review blockers

A change must not merge if it:

- replaces Explorer as the shell;
- modifies a Windows system file;
- writes a permanent Winlogon shell value;
- hides the native taskbar before recovery is registered;
- makes startup automatic by default;
- weakens Task Manager or keyboard recovery;
- relies only on in-process cleanup for crash recovery.
