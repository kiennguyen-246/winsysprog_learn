#include <iostream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const std::wstring DRIVER_NAME = L"HelloWorldKMDF2";
const std::wstring DRIVER_PATH = L"C:\\DriverTest\\Drivers\\" + DRIVER_NAME + L".sys";

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);

    auto hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCM)
    {
        std::wcout << L"SCM successfully loaded\n";
        auto hSvc = CreateService(
            hSCM,
            DRIVER_NAME.c_str(),
            (DRIVER_NAME + L"Driver").c_str(),
            SERVICE_START | DELETE | SERVICE_STOP,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            DRIVER_PATH.c_str(),
            NULL, NULL, NULL, NULL, NULL);
        if (!hSvc)
        {
            hSvc = OpenService(hSCM, DRIVER_NAME.c_str(), SERVICE_START | DELETE | SERVICE_STOP);
        }
        if (hSvc)
        {
            StartService(hSvc, 0, NULL);
            std::wcout << L"Successfully started the service\n";
            std::wcout << L"Press Enter to stop the service\n";
            getchar();
            SERVICE_STATUS ss;
            ControlService(hSvc, SERVICE_CONTROL_STOP, &ss);
            DeleteService(hSvc);
            CloseServiceHandle(hSvc);
        }
        CloseServiceHandle(hSCM);
    }

    return 0;
}