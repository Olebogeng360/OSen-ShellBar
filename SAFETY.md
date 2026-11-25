# CustomShellBar Safety and Recovery Policy

## 1. Purpose

CustomShellBar is an experimental replacement interface for the Windows taskbar and Start menu.

The project does **not** replace the Windows shell. Microsoft Windows Explorer continues running in the background and retains responsibility for the desktop, File Explorer, shell integration and other Windows services.

CustomShellBar temporarily hides selected Explorer interface elements and renders its own interface in a separate process.

The project is designed to be:

- reversible;
- removable;
- testable without installation;
- recoverable after a crash;
- safe to evaluate alongside Windows Explorer.

> [!WARNING]
> CustomShellBar is experimental software. Users should test preview builds on a non-critical Windows installation, virtual machine or separate Windows user account.

---

## 2. Non-destructive design rules

Every official CustomShellBar release must follow these rules.

### 2.1 Windows Explorer must not be replaced

CustomShellBar must not replace, rename, patch or impersonate:

```text
explorer.exe
```

Windows Explorer must remain the active Windows shell.

### 2.2 No permanent shell registry changes

CustomShellBar must not permanently change the Windows shell configuration, including:

```text
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell
```

or:

```text
HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell
```

CustomShellBar must not set itself as the Windows sign-in shell.

### 2.3 No Windows system-file modification

CustomShellBar must not delete, replace or patch Windows system files, including:

- `explorer.exe`;
- `shell32.dll`;
- `user32.dll`;
- `twinui.pcshell.dll`;
- Windows Start menu components;
- Windows taskbar components;
- Windows system resources.

### 2.4 Separate process

The visible custom interface must run in its own process:

```text
CustomShellBar.exe
```

Closing that process must not prevent Windows Explorer from continuing to operate.

### 2.5 Temporary native-taskbar hiding

The native Windows taskbar may only remain hidden while CustomShellBar is active.

The taskbar must be restored when:

- the user exits CustomShellBar;
- Windows shuts down;
- the user signs out;
- the CustomShellBar window closes normally;
- initialization fails;
- an unrecoverable runtime error is detected.

### 2.6 No automatic startup by default

Development and preview builds must not automatically start with Windows.

Startup integration may be offered later as a separate, optional setting that is:

- disabled by default;
- clearly explained;
- easy to disable;
- removed during uninstallation;
- compatible with Safe Mode and Task Manager recovery.

### 2.7 No mandatory installer

Early releases should be distributed as portable builds whenever practical.

A portable release should contain:

```text
CustomShellBar/
├── CustomShellBar.exe
├── settings.json
├── LICENSE
├── README.md
└── SAFETY.md
```

Removing the project should require only restoring the Windows taskbar, exiting the application and deleting the project folder.

---

## 3. Required recovery mechanisms

CustomShellBar must provide multiple independent escape paths.

### 3.1 Visible exit command

The CustomShellBar context menu must contain:

```text
Exit CustomShellBar and restore Windows taskbar
```

This command must:

1. restore every hidden native taskbar;
2. unregister CustomShellBar AppBar windows;
3. remove temporary hooks and event subscriptions;
4. close custom menus and flyouts;
5. save settings safely;
6. terminate CustomShellBar.

The command must not require opening the custom Start menu.

### 3.2 Emergency keyboard shortcut

CustomShellBar must register an emergency shortcut:

```text
Ctrl + Alt + Shift + F12
```

Activating the shortcut must immediately:

1. restore the native Windows taskbar;
2. close CustomShellBar;
3. avoid displaying a confirmation dialog.

The emergency shortcut must continue working when a CustomShellBar menu or flyout has keyboard focus.

If another application already owns the shortcut, CustomShellBar must:

- report the registration failure;
- provide a configurable alternative;
- never claim that emergency recovery is active when registration failed.

### 3.3 Initialization rollback

Startup must follow a transactional sequence:

```text
1. Discover Windows taskbars
2. Register recovery mechanisms
3. Create the custom taskbar
4. Confirm that the custom taskbar is visible
5. Reserve the desktop work area
6. Hide the native taskbar
```

The native taskbar must remain visible if any earlier step fails.

CustomShellBar must never hide the Windows taskbar before confirming that the custom bar and recovery command are operational.

### 3.4 Normal shutdown restoration

The application must restore the taskbar from all normal termination paths, including:

- `WM_CLOSE`;
- `WM_ENDSESSION`;
- `WM_QUERYENDSESSION`;
- menu-based exit;
- emergency shortcut;
- initialization failure;
- internal controlled shutdown.

Restoration must be idempotent. Calling the restoration routine multiple times must not cause errors or progressively change the taskbar state.

### 3.5 Unexpected-termination recovery

An ordinary cleanup handler inside `CustomShellBar.exe` cannot guarantee restoration after:

- forced termination through Task Manager;
- `TerminateProcess`;
- access violations;
- stack corruption;
- process deadlock;
- power loss.

Therefore, crash recovery must not rely only on destructors, `atexit`, structured exception handling or `WM_DESTROY`.

Before enabling automatic startup, the project must implement an independent recovery mechanism, such as a small companion process:

```text
CustomShellBar.Watchdog.exe
```

The watchdog must:

1. start before the native taskbar is hidden;
2. monitor the `CustomShellBar.exe` process handle;
3. restore the native taskbar if the main process exits unexpectedly;
4. terminate after a confirmed normal shutdown;
5. avoid restarting CustomShellBar in a crash loop.

The watchdog must not inject code into Windows Explorer.

---

## 4. Manual recovery instructions

If CustomShellBar becomes unresponsive, use one of the following methods.

### Method A: Emergency shortcut

Press:

```text
Ctrl + Alt + Shift + F12
```

This should restore the Windows taskbar and close CustomShellBar.

### Method B: Task Manager

1. Press **Ctrl + Shift + Esc**.
2. Open the **Processes** or **Details** tab.
3. Locate `CustomShellBar.exe`.
4. Select **End task**.
5. Restart **Windows Explorer** if the native taskbar does not return.

To restart Windows Explorer:

1. Locate **Windows Explorer** in Task Manager.
2. Right-click it.
3. Select **Restart**.

### Method C: Run dialog or Command Prompt

Press **Win + R**, enter:

```cmd
taskkill /f /im CustomShellBar.exe
```

If the native taskbar does not return, restart Explorer:

```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

### Method D: Sign out

If Task Manager cannot be used:

1. Press **Ctrl + Alt + Delete**.
2. Select **Sign out**.
3. Sign back into Windows.

Because CustomShellBar is not the Windows shell, Explorer should start normally after sign-in unless the user separately enabled automatic startup.

### Method E: Disable optional startup

If a future version starts automatically and repeatedly fails:

1. Open Task Manager.
2. Select **Startup** or **Startup apps**.
3. Disable **CustomShellBar**.
4. Sign out and sign back in.

Safe Mode may also be used to remove an invalid startup entry or delete the portable application folder.

---

## 5. Native-taskbar state preservation

CustomShellBar must record the original state of each taskbar before altering it.

For every detected taskbar, the application should preserve:

- window handle;
- monitor association;
- visibility state;
- auto-hide state;
- always-on-top state;
- taskbar edge;
- desktop work area;
- secondary-taskbar presence.

CustomShellBar must restore the saved state rather than assuming that every user originally had the same taskbar configuration.

For example, if the native taskbar was already configured for auto-hide, CustomShellBar must not restore it as permanently visible.

---

## 6. Multi-monitor safety

The primary Windows taskbar normally uses the class:

```text
Shell_TrayWnd
```

Secondary taskbars may use:

```text
Shell_SecondaryTrayWnd
```

CustomShellBar must handle each monitor independently.

If the custom bar fails to initialize on one monitor:

- the native taskbar for that monitor must remain available;
- successfully initialized monitors may continue only if recovery remains clear;
- the project must not hide all taskbars after creating only one custom bar.

Display changes must be handled, including:

- monitor connection and removal;
- resolution changes;
- DPI changes;
- display rotation;
- primary-monitor changes;
- Explorer restarts.

---

## 7. Explorer restart handling

Windows Explorer can restart independently while CustomShellBar remains running. After a restart, taskbar window handles become invalid and new taskbar windows are created.

CustomShellBar must:

1. detect that the original Explorer taskbar windows no longer exist;
2. discard stale window handles;
3. discover the newly created taskbar windows;
4. preserve recovery access during rediscovery;
5. hide the new taskbars only after CustomShellBar confirms that its own interface is operational.

CustomShellBar must never repeatedly hide arbitrary windows based only on previously stored handles.

---

## 8. AppBar and desktop work-area restoration

If CustomShellBar registers an AppBar using `SHAppBarMessage`, it must unregister that AppBar during shutdown.

The application must restore the desktop work area so maximized windows do not leave an empty strip after CustomShellBar exits.

Required cleanup includes:

```cpp
SHAppBarMessage(ABM_REMOVE, &appBarData);
```

An AppBar must not remain registered after the corresponding custom taskbar window has been destroyed.

---

## 9. Windhawk integration rules

If a future release uses a Windhawk companion mod, the mod must be optional.

The main recovery features must not depend entirely on injection into `explorer.exe`.

A Windhawk companion may be used for narrowly defined integration, such as:

- observing Explorer taskbar events;
- forwarding notification-area metadata;
- suppressing specific native visual elements;
- restoring unsupported Windows 10 behavior.

The companion must not:

- replace Explorer;
- patch system files on disk;
- make permanent shell registry changes;
- prevent Explorer from starting;
- disable Task Manager recovery;
- remain necessary to restore the native taskbar.

Disabling or removing the Windhawk mod must return control to the standard Windows interface.

---

## 10. Settings safety

All settings must be stored outside Windows system directories.

Recommended portable location:

```text
CustomShellBar\settings.json
```

If installed per user, the appropriate location is:

```text
%LOCALAPPDATA%\CustomShellBar\
```

Settings must be written safely:

1. write the new configuration to a temporary file;
2. validate the content;
3. replace the previous file;
4. retain usable defaults if loading fails.

Invalid colour, size, opacity or layout values must be clamped to safe ranges instead of causing the bar to disappear.

A future **Reset to defaults** command must not require the custom interface to be visible.

---

## 11. Logging and diagnostics

Debug and preview builds should create a clear diagnostic log containing:

- application version;
- Windows version;
- startup stage;
- monitor discovery;
- taskbar discovery;
- registered recovery mechanisms;
- AppBar registration;
- taskbar hide and restore operations;
- Explorer restart detection;
- shutdown reason;
- errors returned by Windows APIs.

Logs must not contain:

- passwords;
- authentication tokens;
- document names;
- browser history;
- private window titles by default;
- unrelated personal information.

Recommended location:

```text
%LOCALAPPDATA%\CustomShellBar\Logs\
```

The log system should use size limits and rotation to prevent unlimited storage growth.

---

## 12. Release gates

A build must not be published as stable until all of the following tests pass.

### Core recovery

- [ ] Normal exit restores the native taskbar.
- [ ] Emergency shortcut restores the taskbar.
- [ ] Failed initialization leaves the native taskbar visible.
- [ ] Restarting Explorer does not permanently remove the taskbar.
- [ ] Signing out does not leave persistent shell changes.
- [ ] Restarting Windows returns to a usable desktop.
- [ ] Deleting CustomShellBar does not damage Explorer.
- [ ] Resetting settings restores a usable default layout.

### Forced failure

- [ ] Main process crash triggers watchdog restoration.
- [ ] Forced termination triggers watchdog restoration.
- [ ] Custom interface creation failure rolls back safely.
- [ ] Renderer failure does not hide the only usable taskbar.
- [ ] Invalid settings do not make recovery controls inaccessible.

### Multiple displays

- [ ] Primary-monitor layout works.
- [ ] Secondary-monitor layout works.
- [ ] Disconnecting a monitor does not strand windows.
- [ ] Changing the primary monitor rebuilds the bars correctly.
- [ ] Mixed-DPI monitors render and recover correctly.

### Windows integration

- [ ] Maximized windows respect the custom work area.
- [ ] Exiting restores the original work area.
- [ ] Task Manager remains accessible.
- [ ] Ctrl + Alt + Delete remains unaffected.
- [ ] Explorer and File Explorer continue working.
- [ ] Windows desktop icons continue working.
- [ ] Native shell recovery remains available.

---

## 13. Preview-build warning

Every preview release should display the following notice in its README and release description:

> **Preview software:** CustomShellBar temporarily hides parts of the standard Windows interface. Save open work before testing. Keep Task Manager available and review the recovery instructions in `SAFETY.md`. Preview builds should not be used on critical systems.

During early development, releases should be labelled clearly:

```text
Experimental
Alpha
Preview
Beta
Stable
```

An alpha or preview build must not be described as stable merely because it works on the developer's computer.

---

## 14. Issue-report checklist

Users reporting a taskbar recovery problem should include:

- CustomShellBar version;
- Windows edition and build number;
- number of monitors;
- DPI scaling for each monitor;
- whether taskbar auto-hide was enabled;
- whether Windhawk integration was active;
- whether Explorer restarted;
- relevant diagnostic log;
- exact recovery method attempted.

Users should remove private information and unrelated application titles before publishing screenshots or logs.

---

## 15. Project guarantee

CustomShellBar follows this principle:

> **The custom interface may fail, but its failure must not prevent the user from returning to the standard Windows interface.**

Visual appearance, animations and advanced shell integration are secondary to reliable recovery.

Any feature that weakens recovery must remain disabled until a safe fallback exists.

---

## Recommended repository layout

```text
CustomShellBar/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.yml
│   │   └── recovery_problem.yml
│   └── workflows/
├── docs/
│   ├── ARCHITECTURE.md
│   ├── DEVELOPMENT.md
│   └── RECOVERY.md
├── src/
├── tests/
├── LICENSE
├── README.md
├── SAFETY.md
├── SECURITY.md
└── CONTRIBUTING.md
```

## Documentation hierarchy

- `README.md`: project overview, screenshots, installation and basic use.
- `SAFETY.md`: mandatory design rules and release gates.
- `RECOVERY.md`: short emergency instructions written for end users.
- `ARCHITECTURE.md`: Explorer coexistence, AppBar design and component boundaries.
- `SECURITY.md`: vulnerability reporting and supported versions.
- `CONTRIBUTING.md`: coding, review and safety-test requirements.

The project separates **normal cleanup** from **real crash recovery**. A process cannot reliably undo its own UI changes after Task Manager forcibly terminates it, so the architecture requires an independent watchdog before automatic startup or public stable releases are enabled.
