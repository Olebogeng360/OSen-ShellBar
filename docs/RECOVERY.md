# Emergency Recovery

CustomShellBar does not replace Explorer or modify Windows system files. If a preview build fails, use the first available method below.

## 1. Emergency shortcut

Press:

```text
Ctrl + Alt + Shift + F12
```

The application should restore the native taskbar and exit immediately.

## 2. Task Manager

1. Press **Ctrl + Shift + Esc**.
2. End `CustomShellBar.exe`.
3. If the taskbar does not return, select **Windows Explorer** and choose **Restart**.

## 3. Run or Command Prompt

Press **Win + R** and run:

```cmd
taskkill /f /im CustomShellBar.exe
```

If required, restart Explorer:

```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

## 4. Sign out

Press **Ctrl + Alt + Delete**, select **Sign out**, and sign in again.

## 5. Disable future optional startup

Open Task Manager, choose **Startup apps**, disable **CustomShellBar**, and sign out. If Windows remains unstable, use Safe Mode to remove the startup entry or delete the portable application folder.

## What to report

Open a recovery issue with the application version, Windows build, monitor count, DPI scaling, native auto-hide state, exact failure, recovery method used, and a redacted log. Do not publish credentials, tokens, private window titles, or unrelated personal information.
