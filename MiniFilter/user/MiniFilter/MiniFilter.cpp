#include <cassert>
#include <fstream>
#include <future>
#include <iostream>

#include "FilterUser.hpp"
#include "utils.hpp"

const WCHAR FILTER_NAME[] = L"MiniFilter";
const WCHAR FILTER_DRIVER_PATH[] = L"";
const WCHAR FILTER_COM_PORT_NAME[] = L"\\MiniFilterPort";
const WCHAR LOG_FILE[] = L".\\events.log";
const WCHAR SERVER_HOST[] = L"192.168.88.137";
const WCHAR SERVER_PORT[] = L"55555";
const WCHAR INSTRUCTIONS[] =
    L"Type:\n"
    L"\t- 'install' to install the filter,\n"
    L"\t- 'help' to consult instructions,\n "
    L"\t- 'start' to start filtering,\n "
    L"\t- 'stop' to stop filtering\n"
    L"\t- 'quit' to exit\n ";

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;

  FilterUser fuMfltUser(FILTER_NAME, FILTER_COM_PORT_NAME);
  std::future<HRESULT> fMainRoutine;
  bool bHasStarted = false;

  system("cls");

  wprintf(L"MINIFILTER\n-----------------------------\n\n");
  wprintf(INSTRUCTIONS);

  while (1) {
    wprintf(L"MiniFilter>> ");
    std::wstring wsCommand;
    std::wcin >> wsCommand;
    if (wsCommand == L"install") {
      wprintf(L"Command currently disabled\n");
    } else if (wsCommand == L"start") {
      hr = fuMfltUser.loadFilter();
      if (hr) {
        continue;
      }

      hr = fuMfltUser.connectToServer(SERVER_HOST, SERVER_PORT);

      hr = fuMfltUser.attachFilterToVolume(L"E:");
      if (hr) {
        continue;
      }

      fMainRoutine = std::async(std::launch::async, &FilterUser::doMainRoutine,
                                &fuMfltUser);
      
      bHasStarted = true;
    } else if (wsCommand == L"stop") {
      if (!bHasStarted) {
        wprintf(L"The filter has not started\n");
        continue;
      }
      fuMfltUser.setShouldStop();
      hr = fMainRoutine.get();
      if (FAILED(hr)) {
        wprintf(L"An error occured while filtering. Error 0x%08x\n", hr);
        //continue;
      }

      hr = fuMfltUser.detachFilterFromVolume(L"E:");
      if (hr) {
        continue;
      }

      hr = fuMfltUser.unloadFilter();
      if (hr) {
        continue;
      }

      hr = fuMfltUser.disconnectFromServer();
      if (hr) {
        continue;
      }

      bHasStarted = false;
    } else if (wsCommand == L"quit") {
      if (bHasStarted) {
        wprintf(L"Please stop the filter before exiting\n");
        continue;
      }
      break;
    } else if (wsCommand == L"help") {
      wprintf(INSTRUCTIONS);
    } else {
      wprintf(L"Invalid command. Type 'help' to consult instructions.\n");
    }
  }

  return 0;
}
