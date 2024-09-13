#include "FilterUser.hpp"

FilterUser::FilterUser(std::wstring ws__FilterName,
                       std::wstring ws__ComPortName) {
  wsFilterName = ws__FilterName;
  wsComPortName = ws__ComPortName;
  wsLogFilePath = DEFAULT_LOG_FILE_PATH;
  wfsLog.open(wsLogFilePath);
  bIsFilterLoaded = false;
  bIsComPortConnected = false;
}

FilterUser::~FilterUser() {
  wfsLog.close();
  if (bIsComPortConnected) {
    cp.disconnect();
  }
  if (bIsFilterLoaded) {
    unloadFilter();
  }
}

HRESULT FilterUser::loadFilter() {
  HRESULT hr = S_OK;

  HANDLE hToken = NULL;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
    hr = E_FAIL;
    wprintf(L"Get access token failed 0x%08x\n", GetLastError());
    return hr;
  }
  setPrivilege(hToken, SE_LOAD_DRIVER_NAME, TRUE);

  hr = FilterLoad(wsFilterName.c_str());
  if (FAILED(hr)) {
    if (hr == ERROR_ALREADY_EXISTS) {
      wprintf(L"Driver already loaded.\n");
    } else {
      wprintf(L"Load filter failed 0x%08x\n", hr);
      return hr;
    }
  }
  bIsFilterLoaded = 1;
  wprintf(L"Filter loaded\n");
  fflush(stdout);

  hr = cp.connect(wsComPortName.c_str());
  if (FAILED(hr)) {
    wprintf(L"Connection to kernel mode failed 0x%08x\n", hr);
    return hr;
  }
  bIsComPortConnected = 1;
  wprintf(L"Connection to kernel mode established\n");
  fflush(stdout);
  getchar();
  
  return hr;
}

HRESULT FilterUser::attachFilterToVolume(std::wstring wsVolumeName) {
  HRESULT hr = S_OK;
  hr = FilterAttach(wsFilterName.c_str(), wsVolumeName.c_str(), NULL, 0, NULL);
  if (FAILED(hr)) {
    wprintf(L"Attach filter failed 0x%08x\n", hr);
    return hr;
  }
  wprintf(L"Successfully load and attach filter\n");
  fflush(stdout);
  getchar();
  return hr;
}

HRESULT FilterUser::detachFilterFromVolume(std::wstring wsVolumeName) {
  HRESULT hr = S_OK;
  hr = FilterDetach(wsFilterName.c_str(), wsVolumeName.c_str(), NULL);
  return hr;
}

HRESULT FilterUser::unloadFilter() {
  HRESULT hr;
  hr = FilterUnload(wsFilterName.c_str());
  return hr;
}

HRESULT FilterUser::doMainRoutine() {
  std::wstring wsMsg;
  HRESULT hr = S_OK;
  while (1) {
    hr = cp.getMsg(wsMsg);
    if (FAILED(hr)) {
      return hr;
    }
    wprintf(L"[MiniFilter]%ws", wsMsg.c_str());
  }
  return hr;
}

HRESULT FilterUser::setPrivilege(HANDLE hToken, LPCWSTR pwcPrivilege,
                                 BOOL bIsPrivilegeEnabled) {
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue(NULL, pwcPrivilege, &luid)) {
    HRESULT hrError = GetLastError();
    std::wcout << L"LookupPrivilegeValue error " << hrError << L"\n";
    return hrError;
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
    HRESULT hrError = GetLastError();
    std::wcout << L"AdjustTokenPrivileges error " << hrError << L"\n";
    return hrError;
  }
  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    std::wcout << L"The token does not have the specified privilege. \n";
    return ERROR_NOT_ALL_ASSIGNED;
  }

  return S_OK;
}