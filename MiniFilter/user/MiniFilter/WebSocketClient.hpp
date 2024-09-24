#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <winsock2.h>
#include <websocket.h>

#include <iostream>

#include "ClientHTTPSocket.hpp"

const int WSC_MAX_BUFFER_SIZE = 1024;
const int WSC_MAX_NUMBER_OF_BUFFERS = 2;

class WebSocketClient {
 public:
  WebSocketClient();
  ~WebSocketClient();

  //std::string wscGetHostName();

  //bool wscSetHostName(const std::string& sNewHostName);

  HRESULT init();

  HRESULT handshake(std::wstring wsHost, std::wstring wsPort);

  HRESULT send(std::wstring wsMsg);

  HRESULT cleanup();

  bool isHandshakeSuccessful();

 private:
  std::string sFullHostName;

  WEB_SOCKET_HANDLE wshClient;
  //WEB_SOCKET_HANDLE wshServer;
  
  ClientHTTPSocket chs;

  HRESULT runGetActionLoop();

  bool bIsHandshakeSuccessful;
};

#endif