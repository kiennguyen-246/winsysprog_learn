#ifndef MFLT_USER_HPP
#define MFLT_USER_HPP

#include <format>
#include <fstream>
#include <future>
#include <map>

#include "ComPort.hpp"
#include "utils.hpp"
#include "JSONObj.hpp"

const WCHAR DEFAULT_LOG_FILE_PATH[] = L".\\events.log";
const int MAX_SAVED_LOG_LENGTH = 1024;

class FilterUser {
 public:
  FilterUser(std::wstring ws__FilterName, std::wstring ws__ComPortName);
  ~FilterUser();

  HRESULT loadFilter();

  HRESULT attachFilterToVolume(std::wstring wsVolumeName);

  HRESULT detachFilterFromVolume(std::wstring wsVolumeName);

  HRESULT unloadFilter();

  HRESULT doMainRoutine();

 private:
  ComPort cp;

  std::wstring wsFilterName;

  std::wstring wsComPortName;

  std::wstring wsLogFilePath;

  std::wofstream wfsLog;

  std::wstring wsSavedLog;

  std::unordered_map<std::wstring, std::wstring> umDosDevices;

  std::mutex mtx;

  bool bIsFilterLoaded;

  bool bIsComPortConnected;

  HRESULT setPrivilege(HANDLE hToken, LPCWSTR pwcPrivilege,
                       BOOL bIsPrivilegeEnabled);

  HRESULT composeMessage(PMFLT_EVENT_RECORD pEventRecord, std::wstring* pwsMsg);
};

#endif