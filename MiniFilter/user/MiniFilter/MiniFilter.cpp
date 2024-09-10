#include <Windows.h>
#include <fltUser.h>

#include <future>
#include <iostream>
#include <cassert>

const WCHAR FILTER_NAME[] = L"MiniFilter";
const WCHAR FILTER_COM_PORT_NAME[] = L"\\MfltPort";
const WCHAR LOG_FILE[] = L".\\events.log";
const int MAX_BUFFER_SIZE = 1024;

typedef union __COM_MESSAGE {
  FILTER_MESSAGE_HEADER header;
  BYTE buffer[MAX_BUFFER_SIZE];
}COM_MESSAGE, *PCOM_MESSAGE;

BOOL setPrivilege(HANDLE hToken, LPCWSTR pwcPrivilege,
                  BOOL bIsPrivilegeEnabled) {
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue(NULL, pwcPrivilege, &luid)) {
    std::wcout << L"LookupPrivilegeValue error " << GetLastError() << L"\n";
    return 0;
  };

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  if (bIsPrivilegeEnabled) {
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  } else {
    tp.Privileges[0].Attributes = 0;
  }

  if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                             (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
    std::wcout << L"AdjustTokenPrivileges error " << GetLastError() << L"\n";
    return 0;
  }
  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    std::wcout << L"The token does not have the specified privilege. \n";
    return 0;
  }
}

BOOL getComMsg() {}

class ComPort {
 public:
  ComPort() { hComPort = NULL; }

  ~ComPort() { hComPort = NULL; }

  HRESULT connect(std::wstring sPortName) {
    HRESULT hr = S_OK;
    hr = FilterConnectCommunicationPort(sPortName.c_str(), 0, NULL, 0, NULL,
                                        &hComPort);
    if (hr != S_OK) {
      wprintf(L"Connect to kernel mode failed 0x%08x\n", hr);
      fflush(stdout);
      return hr;
    }
    return hr;
  }

  HRESULT getMsg() {
    HRESULT hr = S_OK;
    while (1) {
      COM_MESSAGE msg;
      memset(&msg, 0, sizeof(&msg));
      hr = FilterGetMessage(hComPort, &msg.header, MAX_BUFFER_SIZE * sizeof(BYTE), NULL);
      if (hr != S_OK) {
        wprintf(L"Get message failed 0x%08x\n", hr);
        return hr;
      }
      WCHAR pwcMsgContent[MAX_BUFFER_SIZE / sizeof(WCHAR)];
      memcpy(pwcMsgContent, msg.buffer + sizeof(FILTER_MESSAGE_HEADER),
             MAX_BUFFER_SIZE);
      wprintf(L"[MiniFilter]%ws", pwcMsgContent);
    }
    return hr;
  }

 private:
  HANDLE hComPort;
};

int wmain(int argc, LPWSTR argv[]) {
  assert(0);

  HRESULT hr = S_OK;

  HANDLE hToken = NULL;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
    hr = E_FAIL;
    wprintf(L"Get access token failed 0x%08x\n", GetLastError());
    return hr;
  }
  setPrivilege(hToken, SE_LOAD_DRIVER_NAME, TRUE);

  hr = FilterLoad(FILTER_NAME);
  // WCHAR cmd[] = L"fltmc load ";
  // wcscat(cmd, FILTER_NAME);
  // hr = _wsystem(cmd);
  if (FAILED(hr)) {
    wprintf(L"Load filter failed 0x%08x\n", hr);
    return 1;
  }
  wprintf(L"Successfully attach filter\n");
  hr = FilterAttach(FILTER_NAME, L"C:\\", NULL, 0, NULL);
  if (FAILED(hr)) {
    wprintf(L"Attach filter failed 0x%08x\n", hr);
    return 1;
  }
  wprintf(L"Successfully attach filter\n");
  fflush(stdout);
  getchar();

  ComPort cp;
  hr = cp.connect(FILTER_COM_PORT_NAME);
  if (FAILED(hr)) {
    wprintf(L"Connection to kernel mode failed 0x%08x\n", hr);
    return 1;
  }

  wprintf(L"Connection to kernel mode established\n");
  fflush(stdout);
  getchar();

  std::future<HRESULT> fGetComMsg =
      std::async(std::launch::async, &ComPort::getMsg, &cp);
  hr = fGetComMsg.get();
  if (FAILED(hr)) {
    wprintf(L"An error occured while receiving message. Error 0x%08x\n", hr);
    return 1;
  }
  getchar();

  // FilterUnload(L"MiniFilter");
  return 0;
}
