#include "WebSocketClient.hpp"

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;
  WebSocketClient wsc;

  std::string sHostName = "localhost";
  wsc.wscSetHostName(sHostName.c_str(), sHostName.length());

  hr = wsc.wscInitialize();
  if (FAILED(hr)) {
    std::wcout << L"Failed to initialize socket handles\n";
    wsc.wscDeleteHandle();
    return 0;
  }
  std::wcout << L"Successfully initializing socket handles\n";

  hr = wsc.wscPerformHandshake();
  if (FAILED(hr)) {
    std::wcout << L"Hanshaking failed\n ";

    wsc.wscDeleteHandle();
    return 0;
  }
  std::wcout << L"Hanshaking successful. Connection established.\n ";

  hr = wsc.wscPerformDataExchange();
  if (FAILED(hr)) {
    std::wcout << "Failed to iexchange data\n";
    wsc.wscDeleteHandle();
    return 0;
  }
  return 0;
}