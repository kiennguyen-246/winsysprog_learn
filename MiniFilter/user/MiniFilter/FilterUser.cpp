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
    /* std::vector<MFLT_EVENT_RECORD> vEventRecord;
     vEventRecord.clear();

     hr = cp.getRecord(&vEventRecord);
     if (FAILED(hr)) {
       return hr;
     }

     for (auto& eventRecord : vEventRecord) {
         std::wstring wsMsg;
         hr = composeMessage(&eventRecord, &wsMsg);
         wprintf(L"%ws", wsMsg.c_str());
     }*/

    MFLT_EVENT_RECORD eventRecord;
    ZeroMemory(&eventRecord, sizeof(eventRecord));

    hr = cp.getRecord(&eventRecord);
    if (FAILED(hr)) {
      return hr;
    }

    std::wstring wsMsg;
    hr = composeMessage(&eventRecord, &wsMsg);
    wprintf(wsMsg.c_str());
    // wfsLog << wsMsg;
    // wfsLog.flush();
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

HRESULT FilterUser::composeMessage(PMFLT_EVENT_RECORD pEventRecord,
                                   std::wstring* pwsMsg) {
  FILETIME fileTime = {pEventRecord->uliSysTime.LowPart,
                       pEventRecord->uliSysTime.HighPart};
  SYSTEMTIME sysTime;
  std::wstring wsEventAction = L"";
  std::wstring wsFileObjType = L"File";
  std::wstring wsDescription = L"";
  std::wstring wsDetails = L"";
  pwsMsg->clear();

  FileTimeToSystemTime(&fileTime, &sysTime);

  *pwsMsg =
      std::format(L"[{:02d}/{:02d}/{:04d} {:02d}:{:02d}:{:02d}][{}] ",
                  sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour,
                  sysTime.wMinute, sysTime.wSecond, wsFilterName.c_str());

  switch (pEventRecord->eventType) {
    case MFLT_CLOSE:
    case MFLT_DELETE:
    case MFLT_OPEN:
    case MFLT_WRITE:
      if (pEventRecord->eventType == MFLT_CLOSE) {
        wsEventAction = L"closed";
      }
      if (pEventRecord->eventType == MFLT_DELETE) {
        wsEventAction = L"deleted";
      }
      if (pEventRecord->eventType == MFLT_OPEN) {
        wsEventAction = L"opened";
      }
      if (pEventRecord->eventType == MFLT_WRITE) {
        wsEventAction = L"written";
      }

      if (pEventRecord->objInfo.fileInfo.bIsDirectory) {
        wsFileObjType = L"Directory";
      }

      wsDescription = std::format(
          L"{} {}: {}{}\n", wsFileObjType.c_str(), wsEventAction.c_str(),
          std::wstring(pEventRecord->objInfo.fileInfo.pwcVolumeName)
              .substr(0, pEventRecord->objInfo.fileInfo.uiVolumeNameLength)
              .c_str(),
          std::wstring(pEventRecord->objInfo.fileInfo.pwcFileName)
              .substr(0, pEventRecord->objInfo.fileInfo.uiFileNameLength)
              .c_str());

      *pwsMsg += wsDescription;

      break;
    case MFLT_PROCESS_CREATE:
    case MFLT_PROCESS_TERMINATE:
      if (pEventRecord->eventType == MFLT_PROCESS_CREATE) {
        wsEventAction = L"created";
        wsDetails = std::format(
            L"\tPID\t\t: {}\n"
            L"\tParent PID\t: {}\n"
            L"\tImage name\t: {}\n"
            L"\tCommand Line\t: {}\n",
            pEventRecord->objInfo.procInfo.uiPID,
            pEventRecord->objInfo.procInfo.uiParentPID,
            std::wstring(pEventRecord->objInfo.procInfo.pwcImageName)
                .substr(0, pEventRecord->objInfo.procInfo.uiImageNameLength)
                .c_str(),
            std::wstring(pEventRecord->objInfo.procInfo.pwcCommandLine)
                .substr(0, pEventRecord->objInfo.procInfo.uiCommandLineLength)
                .c_str());
      }
      if (pEventRecord->eventType == MFLT_PROCESS_TERMINATE) {
        wsEventAction = L"terminated";
        wsDetails = std::format(
            L"\tPID\t\t: {}\n"
            L"\tExitcode\t: {}\n",
            pEventRecord->objInfo.procInfo.uiPID,
            pEventRecord->objInfo.procInfo.iExitcode);
      }
      wsFileObjType = L"Process";

      wsDescription =
          std::format(L"{} {}\n", wsFileObjType.c_str(), wsEventAction.c_str());

      *pwsMsg += wsDescription;
      *pwsMsg += wsDetails;
      break;
    default:
      break;
  }
  return S_OK;
}