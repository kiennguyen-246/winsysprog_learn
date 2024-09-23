#include <Windows.h>
#include <fltUser.h>

#include <cassert>
#include <fstream>
#include <future>
#include <iostream>

#include "FilterUser.hpp"
#include "utils.hpp"

const WCHAR FILTER_NAME[] = L"MiniFilter";
const WCHAR FILTER_COM_PORT_NAME[] = L"\\MiniFilterPort";
const WCHAR LOG_FILE[] = L".\\events.log";

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

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;

  FilterUser fuMfltUser(FILTER_NAME, FILTER_COM_PORT_NAME);

  hr = fuMfltUser.loadFilter();
  if (hr) {
    return hr;
  }

  hr = fuMfltUser.attachFilterToVolume(L"E:\\");
  if (hr) {
    return hr;
  }  

  auto fMainRoutine =
      std::async(std::launch::async, &FilterUser::doMainRoutine, &fuMfltUser);
  hr = fMainRoutine.get();
  if (FAILED(hr)) {
    wprintf(L"An error occured while receiving message. Error 0x%08x\n", hr);
    return 1;
  }
  getchar();

   //FilterUnload(L"MiniFilter");
  return 0;
}
