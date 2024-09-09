#include <Windows.h>
#include <fltUser.h>

#include <iostream>

const WCHAR FILTER_NAME[] = L"MiniFilter";

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

  HANDLE hToken = NULL;
  if (!OpenProcessToken(GetCurrentProcess(),
                       TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
    hr = E_FAIL;
    wprintf(L"Get access token failed 0x%08x\n", GetLastError());
    return hr;
  }
  setPrivilege(hToken, SE_LOAD_DRIVER_NAME, TRUE);

  hr = FilterLoad(L"MiniFilter");
  // WCHAR cmd[] = L"fltmc load ";
  // wcscat(cmd, FILTER_NAME);
  // hr = _wsystem(cmd);
  if (FAILED(hr)) {
    wprintf(L"Load filter failed 0x%08x\n", hr);
    return 0;
  }
  hr = FilterAttach(FILTER_NAME, L"C:\\", NULL, 0, NULL);
  if (FAILED(hr)) {
    wprintf(L"Attach filter failed 0x%08x\n", hr);
    return 0;
  }
  // FilterUnload(L"MiniFilter");
  return 0;
}
