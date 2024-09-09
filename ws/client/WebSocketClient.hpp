#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <Windows.h>
#include <websocket.h>

#include <iostream>

#include "TransportManager.hpp"

const int WSC_MAX_BUFFER_SIZE = TM_MAX_BUFFER_SIZE;
const int WSC_MAX_NUMBER_OF_BUFFERS = 2;

class WebSocketClient {
 public:
  WebSocketClient();
  ~WebSocketClient();

  bool wscGetHostName(LPSTR pcHostName, ULONG& uiLength);

  bool wscSetHostName(LPCSTR pcHostName, const ULONG& uiLength);

  HRESULT wscInitialize();

  HRESULT wscPerformHandshake();

  HRESULT wscPerformDataExchange();

  HRESULT wscDeleteHandle();

 private:
  std::string sHostName;

  WEB_SOCKET_HANDLE wshClient;
  // WEB_SOCKET_HANDLE wshServer;

  TransportManager tm;

  HRESULT wscRunGetActionLoop();
};

#endif