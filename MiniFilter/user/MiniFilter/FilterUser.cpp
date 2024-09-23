#include "FilterUser.hpp"

FilterUser::FilterUser(std::wstring ws__FilterName,
                       std::wstring ws__ComPortName) {
  wsFilterName = ws__FilterName;
  wsComPortName = ws__ComPortName;
  wsLogFilePath = DEFAULT_LOG_FILE_PATH;
  wfsLog.open(wsLogFilePath);
  bIsFilterLoaded = false;
  bIsComPortConnected = false;
  umDosDevices.clear();
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

HRESULT FilterUser::attachFilterToVolume(std::wstring wsVolumeDosName) {
  HRESULT hr = S_OK;
  if (umDosDevices.find(wsVolumeDosName) == umDosDevices.end()) {
    WCHAR pwcVolumeName[MAX_PATH];
    DWORD uiVolumeNameLength = 0;
    uiVolumeNameLength =
        QueryDosDeviceW(wsVolumeDosName.c_str(), pwcVolumeName, MAX_PATH);
    if (uiVolumeNameLength == 0) {
      hr = GetLastError();
      wprintf(L"Cannot find volume %ws, error 0x%08x", wsVolumeDosName.c_str(),
              hr);
      return hr;
    }
    umDosDevices.insert({std::wstring(pwcVolumeName), wsVolumeDosName});
    std::wcout << std::wstring(pwcVolumeName) << " " << wsVolumeDosName << "\n";
  }

  hr = FilterAttach(wsFilterName.c_str(), wsVolumeDosName.c_str(), NULL, 0,
                    NULL);
  if (FAILED(hr)) {
    wprintf(L"Attach filter failed 0x%08x\n", hr);
    return hr;
  }
  wprintf(L"Successfully load and attach filter\n");
  fflush(stdout);
  return hr;
}

HRESULT FilterUser::detachFilterFromVolume(std::wstring wsVolumeDosName) {
  HRESULT hr = S_OK;
  hr = FilterDetach(wsFilterName.c_str(), wsVolumeDosName.c_str(), NULL);
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
    fflush(stdout);
    wfsLog << wsMsg;
    wfsLog.flush();
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
  JSONObj jsObj;

  pwsMsg->clear();

  FileTimeToSystemTime(&fileTime, &sysTime);

  jsObj.addSingleObj(
      L"time", std::format(L"{:02d}/{:02d}/{:04d} {:02d}:{:02d}:{:02d}",
                           sysTime.wDay, sysTime.wMonth, sysTime.wYear,
                           sysTime.wHour, sysTime.wMinute, sysTime.wSecond));
  jsObj.addSingleObj(L"filterName", wsFilterName);

  //*pwsMsg =
  //    std::format(L"[{:02d}/{:02d}/{:04d} {:02d}:{:02d}:{:02d}][{}] ",
  //                sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour,
  //                sysTime.wMinute, sysTime.wSecond, wsFilterName.c_str());

  switch (pEventRecord->eventType) {
    case MFLT_CREATE:
    case MFLT_CLOSE:
    case MFLT_DELETE:
    case MFLT_OPEN:
    case MFLT_WRITE:
      jsObj.addSingleObj(L"eventObjType", L"file");

      if (pEventRecord->eventType == MFLT_CREATE) {
        wsEventAction = L"create";
      }
      if (pEventRecord->eventType == MFLT_CLOSE) {
        wsEventAction = L"close";
      }
      if (pEventRecord->eventType == MFLT_DELETE) {
        wsEventAction = L"delete";
      }
      if (pEventRecord->eventType == MFLT_OPEN) {
        wsEventAction = L"open";
      }
      if (pEventRecord->eventType == MFLT_WRITE) {
        wsEventAction = L"write";
      }
      jsObj.addSingleObj(L"action", wsEventAction);

      if (pEventRecord->objInfo.fileInfo.bIsDirectory) {
        wsFileObjType = L"directory";
      }
      if (pEventRecord->eventType == MFLT_CLOSE) {
        wsFileObjType = L"handle";
      }
      jsObj.addSingleObj(L"fileHandleType", wsFileObjType);

      jsObj.addSingleObj(
          L"fileHandleVolumeDosName",
          umDosDevices[std::wstring(
                           pEventRecord->objInfo.fileInfo.pwcVolumeName)
                           .substr(0, pEventRecord->objInfo.fileInfo
                                          .uiVolumeNameLength)]);

      jsObj.addSingleObj(
          L"fileHandleDirectory",
          std::wstring(pEventRecord->objInfo.fileInfo.pwcFileName)
              .substr(0, pEventRecord->objInfo.fileInfo.uiFileNameLength));

      jsObj.addSingleObj(
          L"operationStatus",
          std::format(L"0x{:08x}",
                      pEventRecord->objInfo.fileInfo.iOperationStatus));

      if (pEventRecord->eventType == MFLT_CREATE ||
          pEventRecord->eventType == MFLT_OPEN) {
        jsObj.addSingleObj(L"isAttemptingOverwrite",
                           std::format(L"{}", pEventRecord->objInfo.fileInfo.bIsOverwritten));
      }

      break;
    case MFLT_PROCESS_CREATE:
    case MFLT_PROCESS_TERMINATE:
      if (pEventRecord->eventType == MFLT_PROCESS_CREATE) {
        jsObj.addSingleObj(L"eventObjType", L"process");
        jsObj.addSingleObj(L"action", L"create");
        jsObj.addSingleObj(L"pid", std::format(L"{}", pEventRecord->objInfo.procInfo.uiPID));
        jsObj.addSingleObj(
            L"parentPid",
            std::format(L"{}", pEventRecord->objInfo.procInfo.uiParentPID));
        jsObj.addSingleObj(
            L"imageFileDir", std::wstring(pEventRecord->objInfo.procInfo.pwcImageName)
                .substr(0, pEventRecord->objInfo.procInfo.uiImageNameLength));
        jsObj.addSingleObj(
            L"commandLine",
            std::wstring(pEventRecord->objInfo.procInfo.pwcCommandLine)
                .substr(0, pEventRecord->objInfo.procInfo.uiCommandLineLength));
      }
      if (pEventRecord->eventType == MFLT_PROCESS_TERMINATE) {
        jsObj.addSingleObj(L"eventObjType", L"process");
        jsObj.addSingleObj(L"action", L"create");
        jsObj.addSingleObj(
            L"pid", std::format(L"{}", pEventRecord->objInfo.procInfo.uiPID));
        jsObj.addSingleObj(
            L"exitcode",
            std::format(L"{}", pEventRecord->objInfo.procInfo.iExitcode));
      }

      break;
    default:
      break;
  }

  *pwsMsg = jsObj.toString();
  return S_OK;
}