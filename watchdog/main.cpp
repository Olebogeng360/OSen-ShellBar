#include <windows.h>
#include <shellapi.h>

#include <string>

namespace
{
    void RestoreExplorerTaskbars()
    {
        constexpr const wchar_t* taskbarClasses[]
        {
            L"Shell_TrayWnd",
            L"Shell_SecondaryTrayWnd"
        };

        for (const wchar_t* className : taskbarClasses)
        {
            HWND taskbar = nullptr;

            while (
                (
                    taskbar = FindWindowExW(
                        nullptr,
                        taskbar,
                        className,
                        nullptr
                    )
                ) != nullptr
            )
            {
                ShowWindow(
                    taskbar,
                    SW_SHOW
                );

                SetWindowPos(
                    taskbar,
                    HWND_TOP,
                    0,
                    0,
                    0,
                    0,
                    SWP_NOMOVE |
                        SWP_NOSIZE |
                        SWP_NOACTIVATE |
                        SWP_FRAMECHANGED
                );
            }
        }
    }
}

int WINAPI wWinMain(
    HINSTANCE,
    HINSTANCE,
    PWSTR,
    int
)
{
    DWORD mainProcessId = 0;
    std::wstring cleanExitEventName;

    int argumentCount = 0;

    LPWSTR* arguments =
        CommandLineToArgvW(
            GetCommandLineW(),
            &argumentCount
        );

    if (!arguments)
    {
        return 1;
    }

    for (
        int index = 1;
        index < argumentCount;
        ++index
    )
    {
        if (
            lstrcmpW(
                arguments[index],
                L"--pid"
            ) == 0 &&
            index + 1 < argumentCount
        )
        {
            mainProcessId =
                wcstoul(
                    arguments[++index],
                    nullptr,
                    10
                );
        }
        else if (
            lstrcmpW(
                arguments[index],
                L"--event"
            ) == 0 &&
            index + 1 < argumentCount
        )
        {
            cleanExitEventName =
                arguments[++index];
        }
    }

    LocalFree(arguments);

    if (
        mainProcessId == 0 ||
        cleanExitEventName.empty()
    )
    {
        return 2;
    }

    HANDLE mainProcess =
        OpenProcess(
            SYNCHRONIZE,
            FALSE,
            mainProcessId
        );

    HANDLE cleanExitEvent =
        OpenEventW(
            SYNCHRONIZE,
            FALSE,
            cleanExitEventName.c_str()
        );

    if (
        !mainProcess ||
        !cleanExitEvent
    )
    {
        if (mainProcess)
        {
            CloseHandle(mainProcess);
        }

        if (cleanExitEvent)
        {
            CloseHandle(cleanExitEvent);
        }

        return 3;
    }

    HANDLE waitHandles[]
    {
        cleanExitEvent,
        mainProcess
    };

    const DWORD waitResult =
        WaitForMultipleObjects(
            ARRAYSIZE(waitHandles),
            waitHandles,
            FALSE,
            INFINITE
        );

    // Index zero means the main application signalled a clean exit.
    // Index one means the main application terminated unexpectedly.
    if (
        waitResult ==
        WAIT_OBJECT_0 + 1
    )
    {
        RestoreExplorerTaskbars();
    }

    CloseHandle(cleanExitEvent);
    CloseHandle(mainProcess);

    return 0;
}