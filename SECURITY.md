# Security Policy

## Supported versions

Until the first stable release, only the latest published preview is eligible for security fixes. Preview builds are experimental and must not be used on critical systems.

## Reporting a vulnerability

Do not publish exploitable details in a public issue. Use GitHub private vulnerability reporting when enabled for the repository. If private reporting is unavailable, contact the repository owner privately before disclosure.

Include:

- affected release or commit;
- Windows edition, version, and build;
- reproducible steps;
- impact and required privileges;
- whether Explorer, a Windhawk component, or the watchdog is involved;
- a minimal proof of concept if safe to provide.

## Security boundaries

The project must not:

- require administrator rights for ordinary operation;
- inject into `explorer.exe` from the main UI process;
- download or execute updates silently;
- modify Windows system files;
- disable Task Manager, Ctrl+Alt+Delete, Safe Mode, or Windows recovery paths;
- collect window titles, file names, browsing activity, credentials, or tokens by default.

A future optional Windhawk bridge must be separately documented, narrowly scoped, removable, and unnecessary for restoring the native taskbar.

## Update integrity

Release artifacts should be built by GitHub Actions, checksummed with SHA-256, and signed with Authenticode when a trusted code-signing certificate becomes available. Never publish unsigned artifacts under a filename suggesting that they are signed.

## Disclosure target

The project aims to acknowledge a valid report within 7 days and publish a remediation plan within 30 days. These are project targets, not guarantees.
