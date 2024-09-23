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

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;

  FilterUser fuMfltUser(FILTER_NAME, FILTER_COM_PORT_NAME);

  hr = fuMfltUser.loadFilter();
  if (hr) {
    return hr;
  }

  hr = fuMfltUser.attachFilterToVolume(L"E:");
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
