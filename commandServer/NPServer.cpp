#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const int MAX_BUFFER_SIZE = 1024;
const int MAX_RETRIES = 60;

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);
    std::wstring pipeName = L"\\\\.\\pipe\\cmdPipe";
    auto pipeHandle = CreateNamedPipe(
        &pipeName[0],
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        0,
        0,
        INFINITE,
        NULL);
    std::wcout << L"Server is ready\n";
    // int clientCnt = 0;
    WCHAR tmpFileName[MAX_PATH];
    CreateDirectoryW(L".\\Temp", NULL);
    auto tmpFileId = GetTempFileNameW(L".\\Temp", L"out", 0, tmpFileName);
    std::wstringstream ss;
    ss << std::hex << tmpFileId;
    auto tmpFilePath = L".\\Temp\\out" + std::wstring(tmpFileName) + ss.str() + L".tmp";
    bool stopFlag = 0;
    while (!stopFlag && pipeHandle != INVALID_HANDLE_VALUE)
    {
        if (ConnectNamedPipe(pipeHandle, NULL))
        {
            ULONG clientPID = 0;
            GetNamedPipeClientProcessId(pipeHandle, &clientPID);
            std::wcout << L"Pipe connecting client process with PID " << clientPID << L" has been connected\n";
            BYTE buffer[MAX_BUFFER_SIZE];
            DWORD dwRead = 0;
            DWORD dwWritten = 0;
            std::wstring cmd = L"";
            while (!stopFlag && ReadFile(pipeHandle, buffer, sizeof(buffer) - 2, &dwRead, NULL))
            {
                LPWSTR str = (LPWSTR)buffer;
                str[dwRead / 2] = L'\0';
                cmd += std::wstring(str);
                std::wcout << L"Client process " << clientPID << L" queried: " << cmd << L"\n";
                if (cmd == L"server_stop")
                {
                    std::wstring ln = L"Server has been shut down successfully\n";
                    WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
                    stopFlag = 1;
                }
                else
                {
                    // std::wstring fullCmd = cmd + L" > " + tmpFilePath;
                    // _wsystem(&fullCmd[0]);
                    // std::wcout << fullCmd << L"\n";
                    auto tmpFileHandle = CreateFile(
                        tmpFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                    );
                    STARTUPINFOW startupInfo = { sizeof(startupInfo) };
                    startupInfo.hStdOutput = tmpFileHandle;
                    startupInfo.hStdError = tmpFileHandle;
                    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                    startupInfo.dwFlags = STARTF_USESTDHANDLES;
                    PROCESS_INFORMATION procInfo;
                    std::wcout << &startupInfo << L"\n";
                    std::wcout << &procInfo << L"\n";
                    CreateProcessW(
                        NULL,
                        &cmd[0],
                        NULL,
                        NULL,
                        TRUE,
                        0,
                        NULL,
                        NULL,
                        &startupInfo,
                        &procInfo
                    );
                    // // std::wcout << GetLastError() << "\n";
                    WaitForSingleObject(&procInfo.hProcess, INFINITE);
                    std::wifstream fi(&tmpFileName[0]);
                    std::wstring ln;
                    while (getline(fi, ln))
                    {
                        ln += L"\n";
                        WriteFile(pipeHandle, &ln[0], (DWORD)ln.size() * 2, &dwWritten, NULL);
                    }
                }
                std::wstring endS = L" ";
                WriteFile(pipeHandle, &endS[0], (DWORD)endS.size() * 2, &dwWritten, NULL);
            }

            std::wcout << L"Pipe connecting client process with PID " << clientPID << L" has been disconnected\n";
            DisconnectNamedPipe(pipeHandle);
        }
    }
    CloseHandle(pipeHandle);
    return 0;
}