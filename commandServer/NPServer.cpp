#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <future>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const int MAX_BUFFER_SIZE = 1024;
const int MAX_RETRIES = 60;
const int MAX_CLIENTS = 2;

volatile bool stopFlag;

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

bool serverThread(HANDLE pipeHandle, const std::wstring &tmpFileName)
{
    BYTE buffer[MAX_BUFFER_SIZE];
    DWORD dwRead = 0;
    DWORD dwWritten = 0;
    std::wstring cmd = L"";
    while (!stopFlag)
    {
        auto connectFuture = std::async([](HANDLE __pipeHandle) -> bool
                                        {
            ConnectNamedPipe(__pipeHandle, NULL);
            return 1; }, pipeHandle);
        while (!stopFlag && !connectFuture.valid())
        {
            // do nothing
        };
        LPWSTR str = (LPWSTR)buffer;
        str[dwRead / 2] = L'\0';
        cmd += std::wstring(str);
        // std::wcout << L"Client process " << clientPID << L" queried: " << cmd << L"\n";
        if (cmd == L"server_stop")
        {
            std::wstring ln = L"Server has been shut down successfully\n";
            WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
            stopFlag = 1;
        }
        else
        {
            std::wstring response;
            processQuery(cmd, std::wstring(tmpFileName), response);
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
            }
        }
    }
    // std::wcout << L"Pipe to client process with PID " << clientPID << L" has been disconnected\n";
    DisconnectNamedPipe(pipeHandle);
    return 1;
}

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);
    std::wstring pipeName = L"\\\\.\\pipe\\cmdPipe";
    std::future<bool> clientFutures[MAX_CLIENTS];
    HANDLE threadPipeHandles[MAX_CLIENTS];
    WCHAR tmpFileName[MAX_CLIENTS][MAX_PATH];
    CreateDirectoryW(L".\\Temp", NULL);
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        GetTempFileNameW(L".\\Temp", L"out", 0, tmpFileName[i]);
        auto pipeHandle = CreateNamedPipe(
            &pipeName[0],
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            MAX_CLIENTS,
            0,
            0,
            INFINITE,
            NULL);
        threadPipeHandles[i] = pipeHandle;
        clientFutures[i] = std::async(serverThread, pipeHandle, &tmpFileName[i][0]);
    }
    std::wcout << L"Server is ready\n";
    bool stopFlag = 0;
    while (!stopFlag && pipeHandle != INVALID_HANDLE_VALUE)
    {
        if (ConnectNamedPipe(pipeHandle, NULL))
        {
            ULONG clientPID = 0;
            GetNamedPipeClientProcessId(pipeHandle, &clientPID);
            std::wcout << L"Pipe to client process with PID " << clientPID << L" has been connected\n";
            int threadId = 0;
            while (!threadId)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clientFutures[i].valid())
                    {
                        threadId = i;
                        break;
                    }
                }
            }
            clientFutures[threadId] = std::async(std::launch::async, processClientThread, pipeHandle, std::wstring(tmpFileName[threadId]), clientPID);
            // processClientThread(pipeHandle, std::wstring(tmpFileName[threadId]), clientPID, stopFlag);
            // clientFutures[threadId].get();
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        DeleteFileW(tmpFileName[i]);
    }
    CloseHandle(pipeHandle);
    return 0;
}