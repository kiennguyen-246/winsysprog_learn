#include "NPServerSvc.hpp"

void WINAPI svcMain(int argc, LPWSTR argv[])
{
    svcLogEvent(L"Entering svcMain()");

    svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    svcStatus.dwCurrentState = SERVICE_START_PENDING;
    svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    svcStatus.dwWin32ExitCode = NO_ERROR;
    svcStatus.dwServiceSpecificExitCode = 0;
    svcStatus.dwCheckPoint = 0;
    svcStatus.dwWaitHint = 2 * UPDATE_TIME;

    svcStatusHandle = RegisterServiceCtrlHandler(&svcName[0], svcCtrlHandler);

    if (!svcStatusHandle)
    {
        svcLogEvent(L"Cannot register handle\n", EVENTLOG_ERROR_TYPE);
        svcStatus.dwCurrentState = SERVICE_STOPPED;
        svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        svcStatus.dwServiceSpecificExitCode = 1;
        svcStatusUpdate(SERVICE_STOPPED, -1);
        return;
    }

    svcLogEvent(L"Control handler registered");
    SetServiceStatus(svcStatusHandle, &svcStatus);
    svcLogEvent(L"Status SERVICE_START_PENDING");

    svcSpecific(argc, argv);

    svcLogEvent(L"Server threads shut down");
    svcLogEvent(L"Set SERVICE_STOPPED status");
    svcStatusUpdate(SERVICE_STOPPED, 0);
    svcLogEvent(L"Status set to SERVICE_STOPPED");
    return;
}

int svcSpecific(int argc, LPWSTR argv[])
{
    svcStatusUpdate(-1, -1);

    STARTUPINFOW startupInfo;
    GetStartupInfoW(&startupInfo);
    PROCESS_INFORMATION procInfo;
    std::wstring cmd = L"NPServer";
    if (!CreateProcessW(
            NULL,
            &cmd[0],
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &startupInfo,
            &procInfo))
    {
        auto error = GetLastError();
        std::wcout << L"Failed to create process for NPServer. Error: ";
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
            std::wcout << L"File not found\n";
            break;
        case ERROR_ACCESS_DENIED:
            std::wcout << L"Access denied\n";
            break;
        case ERROR_INVALID_PARAMETER:
            std::wcout << L"Invalid parameter\n";
            break;

        // ... other error cases ...
        default:
            std::wcout << L"Unknown error code\n";
            break;
        };
    }
    svcStatusUpdate(SERVICE_RUNNING, -1);
    svcLogEvent(L"Service is running");

    while (WaitForSingleObject(procInfo.hProcess, 1000) != WAIT_OBJECT_0)
    {
        svcStatusUpdate(-1, -1);
        svcLogEvent(L"Service is running");
    }

    svcLogEvent(L"Service process has shut down.");
    return 0;
}

void WINAPI svcCtrlHandler(DWORD dwControl)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        shutdownFlag = true;
        svcStatusUpdate(SERVICE_STOP_PENDING, -1);
        break;
    case SERVICE_CONTROL_PAUSE:
        pauseFlag = true;
        break;
    case SERVICE_CONTROL_CONTINUE:
        pauseFlag = false;
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        break;
    }
    svcStatusUpdate(-1, 1);
    return;
}

bool svcStatusUpdate(const int &newStatus, const int &checkPointMod)
{
    if (checkPointMod < 0)
    {
        svcStatus.dwCheckPoint++;
    }
    else
    {
        svcStatus.dwCheckPoint = checkPointMod;
    }
    if (newStatus >= 0)
    {
        svcStatus.dwCurrentState = newStatus;
    }
    if (!SetServiceStatus(svcStatusHandle, &svcStatus))
    {
        svcLogEvent(L"Cannot set status", EVENTLOG_ERROR_TYPE);
        svcStatus.dwCurrentState = SERVICE_STOPPED;
        svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        svcStatus.dwServiceSpecificExitCode = 1;
        svcStatusUpdate(SERVICE_STOPPED, -1);
        return 0;
    }
    else
    {
        svcLogEvent(L"Service status updated");
    }
    return 1;
}

bool svcLogEvent(const std::wstring &msg, const int &eventType)
{
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    fo << L"[" << std::ctime(&currentTime) << L"]";
    switch (eventType)
    {
    case EVENTLOG_SUCCESS:
    case EVENTLOG_INFORMATION_TYPE:
        fo << L"(Information)";
        break;
    case EVENTLOG_ERROR_TYPE:
        fo << L"(Error)      ";
        break;
    case EVENTLOG_WARNING_TYPE:
        fo << L"(Warning)    ";
        break;
    default:
        fo << L"(Unknown)    ";
        break;
    }
    fo << msg << L"\n";
    return 1;
}

bool svcLogEvent(const std::wstring &msg)
{
    return svcLogEvent(msg, EVENTLOG_SUCCESS);
}

bool svcLogInit()
{
    std::wofstream fo(&logFileName[0]);
    svcLogEvent(L"Logging successfully initialized");
    return fo.is_open();
}

bool svcLogClose()
{
    fo.close();
    svcLogEvent(L"Logging successfully closed");
    return 1;
}

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);
    SERVICE_TABLE_ENTRYW dispatchTable[] = {
        {&svcName[0], (LPSERVICE_MAIN_FUNCTIONW)svcMain},
        {NULL, NULL},
    };

    svcLogInit();
    svcLogEvent(L"Starting dispatcher");
    StartServiceCtrlDispatcherW(dispatchTable);
    svcLogClose();
    return 0;
}