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
  HRESULT hr = S_OK;
  while (1) {
    MFLT_EVENT_RECORD eventRecord;
    ZeroMemory(&eventRecord, sizeof(MFLT_EVENT_RECORD));

    while (1) {
      hr = cp.getRecord(&eventRecord);
      if (FAILED(hr)) {
        return hr;
      }

      FILETIME fileTime = {eventRecord.uliSysTime.LowPart,
                           eventRecord.uliSysTime.HighPart};
      SYSTEMTIME sysTime;
      FileTimeToSystemTime(&fileTime, &sysTime);

      std::wstring wsEventTypeName;
      switch (eventRecord.eventType) {
        case MFLT_OPEN:
          wsEventTypeName = L"opened";
          break;
        case MFLT_CLOSE:
          wsEventTypeName = L"closed";
          break;
        case MFLT_WRITE:
          wsEventTypeName = L"writen";
          break;
        case MFLT_PROCESS_CREATE:
          wsEventTypeName = L"created";
          break;
        case MFLT_PROCESS_TERMINATE:
          wsEventTypeName = L"terminated";
          break;
        default:
          break;
      }

      std::wstring wsFileObjType = L"File";
      if (eventRecord.objInfo.fileInfo.bIsDirectory) {
        std::wstring wsFileObjType = L"Directory";
      }
      if (eventRecord.eventType == MFLT_PROCESS_CREATE ||
          eventRecord.eventType == MFLT_PROCESS_TERMINATE) {
        std::wstring wsFileObjType = L"Process";
      }
      

      wprintf(L"[%02d/%02d/%d %02d:%02d:%02d][%ws] %ws %ws: %ws%ws\n",
              sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour,
              sysTime.wMinute, sysTime.wSecond, wsFilterName.c_str(),
              wsFileObjType.c_str(), wsEventTypeName.c_str(),
              eventRecord.objInfo.fileInfo.pwcVolumeName,
              eventRecord.objInfo.fileInfo.pwcFileName);
      if (eventRecord.eventType == MFLT_PROCESS_CREATE) {
        wprintf(L"\tPID: %d\n", eventRecord.objInfo.procInfo.uiPID);
        wprintf(L"\tParent PID: %d\n", eventRecord.objInfo.procInfo.uiParentPID);
        wprintf(L"\tImage name: %ws\n", eventRecord.objInfo.procInfo.pwcImageName);
        wprintf(L"\tCommand line: %ws\n", eventRecord.objInfo.procInfo.pwcCommandLine);
      }
      if (eventRecord.eventType == MFLT_PROCESS_TERMINATE) {
        wprintf(L"\tPID: %d", eventRecord.objInfo.procInfo.uiPID);
        wprintf(L"\tExitcode: %d", eventRecord.objInfo.procInfo.iExitcode);
      }
      break;
    }
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