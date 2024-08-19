#include "NPServer.hpp"

void processQuery(std::wstring &cmd, const std::wstring &outputFilePath, std::wstring &response)
{
    // std::wstring fullCmd = cmd + L" > " + tmpFilePath;
    // _wsystem(&fullCmd[0]);
    // std::wcout << fullCmd << L"\n";
    SECURITY_ATTRIBUTES tempSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    auto tmpFileHandle = CreateFile(
        &outputFilePath[0],
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &tempSA,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY,
        NULL);
    STARTUPINFOW startupInfo;
    GetStartupInfo(&startupInfo);
    startupInfo.hStdOutput = tmpFileHandle;
    startupInfo.hStdError = tmpFileHandle;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    PROCESS_INFORMATION procInfo;
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
        std::wcout << L"Failed to create process. Error: ";
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
            std::wcout << L"File not found\n";
            response = L"Command not found\n";
            break;
        case ERROR_ACCESS_DENIED:
            std::wcout << L"Access denied\n";
            response = L"Failed to execute command\n";
            break;
        case ERROR_INVALID_PARAMETER:
            std::wcout << L"Invalid parameter\n";
            response = L"Command parameters are invalid\n";
            break;

        // ... other error cases ...
        default:
            std::wcout << L"Unknown error code\n";
            response = L"Failed to execute command\n";
            break;
        };
    }
    else
    {
        std::wcout << L"Process created with PID " << procInfo.dwProcessId << L"\n";
        WaitForSingleObject(procInfo.hProcess, INFINITE);
    }
    CloseHandle(tmpFileHandle);
    CloseHandle(procInfo.hProcess);
}

bool connectNamedPipeThreadFunc(HANDLE __pipeHandle)
{
    auto ret = ConnectNamedPipe(__pipeHandle, NULL);
    return (ret != 0);
}

bool serverThreadFunc(HANDLE pipeHandle, const std::wstring &tmpFileName)
{
    while (!stopFlag)
    {
        // auto connectFuture = std::async([](HANDLE __pipeHandle) -> bool
        //                                 {
        //     ConnectNamedPipe(__pipeHandle, NULL);
        //     return 1; }, pipeHandle);
        BYTE buffer[MAX_BUFFER_SIZE];
        DWORD dwRead = 0;
        DWORD dwWritten = 0;
        auto connectFuture = std::async(connectNamedPipeThreadFunc, pipeHandle);
        ULONG clientPID = 0;
        auto ok = connectFuture.get();
        GetNamedPipeClientProcessId(pipeHandle, &clientPID);
        if (!ok || stopFlag)
        {
            continue;
        }
        std::wcout << "Successfully connected with process with PID " << clientPID << "\n";
        while (!stopFlag && ReadFile(pipeHandle, buffer, sizeof(buffer) - 2, &dwRead, NULL))
        {
            std::wstring cmd = L"";
            LPWSTR str = (LPWSTR)buffer;
            str[dwRead / 2] = L'\0';
            cmd += std::wstring(str);
            std::wcout << L"Client process " << clientPID << L" queried: " << cmd << L"\n";
            if (cmd == L"quit")
            {
                std::wcout << L"Client process " << clientPID << L" is leaving\n";
                // WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
                // continue;
            }
            if (stopFlag == 1 || cmd == L"server_stop")
            {
                std::wstring ln = L"Server is shutting down and will no longer answer queries\nType \"quit\" to exit\n";
                WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
                stopFlag = 1;
                continue;
            }
            if (!stopFlag)
            {
                std::wstring response;
                if (cmd != L"quit")
                {
                    processQuery(cmd, std::wstring(tmpFileName), response);
                }

                if (response != L"")
                {
                    WriteFile(pipeHandle, &response[0], (DWORD)response.size() * 2, &dwWritten, NULL);
                }
                else
                {
                    std::wifstream fi(&tmpFileName[0]);
                    std::wstring ln;
                    while (getline(fi, ln))
                    {
                        ln += L"\n";
                        WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
                    }
                    fi.close();
                }
            }

            std::wstring endS = L" ";
            WriteFile(pipeHandle, &endS[0], (DWORD)endS.size() * 2, &dwWritten, NULL);
            FlushFileBuffers(pipeHandle);
        }
        std::wcout << "Successfully disconnected with process with PID " << clientPID << "\n";
        FlushFileBuffers(pipeHandle);
        DisconnectNamedPipe(pipeHandle);
    }
    return 1;
}

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);
    std::wstring pipeName = L"\\\\.\\pipe\\cmdPipe";
    std::future<bool> serverFutures[MAX_CLIENTS];
    HANDLE threadPipeHandles[MAX_CLIENTS];
    WCHAR tmpFileName[MAX_CLIENTS][MAX_PATH];
    CreateDirectoryW(L".\\Temp", NULL);
    std::wcout << L"Server is ready\n";
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        GetTempFileNameW(L".\\Temp", L"out", 0, tmpFileName[i]);
        threadPipeHandles[i] = CreateNamedPipe(
            &pipeName[0],
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            MAX_CLIENTS,
            0,
            0,
            INFINITE,
            NULL);
        serverFutures[i] = std::async(serverThreadFunc, threadPipeHandles[i], &tmpFileName[i][0]);
    }
    stopFlag = 0;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        serverFutures[i].get();
    }
    std::wcout << "Server has shut down";

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        DeleteFileW(tmpFileName[i]);
        CloseHandle(threadPipeHandles[i]);
    }
    return 0;
}