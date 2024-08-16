#include <iostream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

int wmain()
{
    _setmode(_fileno(stdout), _O_WTEXT);
    WCHAR cmd[MAX_PATH];
    wcscpy(cmd, L"add 1 1");
    std::wcout << "Run here\n";
    STARTUPINFOW startupInfo = { sizeof(startupInfo) };
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    PROCESS_INFORMATION procInfo;
    try
    {
        CreateProcessW(
            NULL,
            cmd,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &startupInfo,
            &procInfo);
    }
    catch (std::exception &e)
    {
        std::wcout << e.what();
    }
    std::wcout << L"Process created successfully";
    return 0;
}