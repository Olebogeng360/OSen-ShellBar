# CustomShellBar

CustomShellBar is an experimental, reversible Windows 10 taskbar and Start menu interface. It keeps `explorer.exe` running as the Windows shell, temporarily hides the native taskbar, and renders a separate custom interface.

> [!WARNING]
> CustomShellBar is pre-alpha. The image below is a design prototype, not a screenshot of a working release. Safety and recovery infrastructure will be implemented before the native Windows taskbar is hidden.

## Prototype mockup

![CustomShellBar prototype showing a floating application capsule, separate system capsule, and open Start panel](docs/assets/CustomShellBar-prototype-mockup.png)

The prototype establishes the intended direction: independent floating glass surfaces, visible warm-orange borders, a centered application capsule, a separate system capsule, and an accessible Start panel with an **Exit and restore Windows taskbar** action.

See the complete [visual specification](docs/VISUAL_SPECIFICATION.md) for dimensions, design tokens, DPI behavior, accessibility requirements, motion, and fallback rendering.

## Project principles

- Never replace `explorer.exe`.
- Never patch or delete Windows system files.
- Never permanently change the Winlogon shell registry value.
- Hide the native taskbar only after recovery controls are operational.
- Restore the user's original taskbar and work-area state on exit.
- Require an independent watchdog before automatic startup is offered.
- Keep any Windhawk integration optional.

## Planned architecture

- `CustomShellBar.exe`: native x64 Win32 UI and AppBar process.
- `CustomShellBar.Watchdog.exe`: independent crash-recovery process.
- `explorer.exe`: remains active for the desktop, File Explorer, and Windows shell services.
- Optional Windhawk companion: a narrowly scoped bridge for unsupported Explorer integration.

## Documentation

- [Visual specification](docs/VISUAL_SPECIFICATION.md)
- [Safety policy](SAFETY.md)
- [Recovery guide](docs/RECOVERY.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Development guide](docs/DEVELOPMENT.md)
- [Contributing](CONTRIBUTING.md)
- [Security policy](SECURITY.md)

## Current status

**Phase 0: documentation, safety baseline, and visual specification.** Functional shell code must not be merged until the recovery-first project gates are satisfied.

## License

Licensed under the [MIT License](LICENSE).
