#include "WebSocketClient.hpp"

#include <cassert>

WebSocketClient::WebSocketClient() {
  sFullHostName = "";
  bIsHandshakeSuccessful = 0;
  wshClient = NULL;
  // wshServer = NULL;
}

WebSocketClient::~WebSocketClient() {
  if (wshClient != NULL) {
    if (bIsHandshakeSuccessful) {
      WebSocketAbortHandle(wshClient);
    }
    WebSocketDeleteHandle(wshClient);
    wshClient = NULL;
  }
  wshClient = NULL;
  // wshServer = NULL;
}

// std::string WebSocketClient::wscGetHostName() { return sFullHostName; }
//
// bool WebSocketClient::wscSetHostName(const std::string& sNewHostName) {
//   sFullHostName = sNewHostName;
//   return 1;
// }

HRESULT WebSocketClient::init() {
  HRESULT hr = S_OK;

  WEB_SOCKET_HANDLE wshTempClient;
  // WEB_SOCKET_HANDLE wshTempServer;

  chs = ClientHTTPSocket();
  hr = chs.getSocketInitResult();
  if (FAILED(hr)) {
    wprintf(L"WinSock initialization failed 0x%08x\n", hr);
    return hr;
  }

  hr = WebSocketCreateClientHandle(NULL, 0, &wshTempClient);
  if (FAILED(hr)) {
    wprintf(L"Websocket initialization failed 0x%08x\n", hr);
    if (wshTempClient != NULL) {
      WebSocketDeleteHandle(wshTempClient);
      wshTempClient = NULL;
    }
    return hr;
  }

  // hr = WebSocketCreateServerHandle(NULL, 0, &wshTempServer);
  // if (FAILED(hr)) {
  //   if (wshTempServer != NULL) {
  //     WebSocketDeleteHandle(wshTempServer);
  //     wshTempServer = NULL;
  //   }
  //   return hr;
  // }

  wshClient = wshTempClient;
  // wshServer = wshTempServer;
  wshTempClient = NULL;
  // wshTempServer = NULL;

  return hr;
}

HRESULT WebSocketClient::handshake(std::wstring wsHost, std::wstring wsPort) {
  HRESULT hr = S_OK;
  ULONG uiClientAdditionalHeadersCount = 0;
  PWEB_SOCKET_HTTP_HEADER pClientAdditionalHeaders = NULL;
  ULONG uiServerAdditionalHeadersCount = 0;
  PWEB_SOCKET_HTTP_HEADER pServerAdditionalHeaders = NULL;
  ULONG uiClientHeadersCount = 0;
  PWEB_SOCKET_HTTP_HEADER pClientHeaders = NULL;

  chs.connectToHost(wsHost, wsPort);
  auto wsFullHostName = wsHost + L":" + wsPort;
  auto sFullHostName = wstrToStr(wsFullHostName);

  CHAR pcHostHeaderTag[] = "Host";
  const static WEB_SOCKET_HTTP_HEADER host = {
      pcHostHeaderTag, (ULONG)strlen(pcHostHeaderTag), &sFullHostName[0],
      (ULONG)sFullHostName.length()};

  hr = WebSocketBeginClientHandshake(wshClient, NULL, 0, NULL, 0, NULL, 0,
                                     &pClientAdditionalHeaders,
                                     &uiClientAdditionalHeadersCount);
  if (FAILED(hr)) {
    wprintf(L"WebSocketBeginClientHandshake failed 0x%08x\n", hr);
    return hr;
  }

  uiClientHeadersCount = uiClientAdditionalHeadersCount + 1;
  pClientHeaders = new WEB_SOCKET_HTTP_HEADER[uiClientHeadersCount];
  if (pClientHeaders == NULL) {
    hr = E_OUTOFMEMORY;
    std::wcout << L"Handshake failed due to lack of memeory\n";
    return hr;
  }
  CopyMemory(pClientHeaders, pClientAdditionalHeaders,
             uiClientAdditionalHeadersCount * sizeof(WEB_SOCKET_HTTP_HEADER));
  pClientHeaders[uiClientAdditionalHeadersCount] = host;

  for (ULONG uiClientHeaderCurrentId = 0;
       uiClientHeaderCurrentId < uiClientHeadersCount;
       uiClientHeaderCurrentId++) {
    std::string key = pClientHeaders[uiClientHeaderCurrentId].pcName;
    std::string value = pClientHeaders[uiClientHeaderCurrentId].pcValue;
    value =
        value.substr(0, pClientHeaders[uiClientHeaderCurrentId].ulValueLength);
    chs.addRequestHeader(key, value);
  }
  chs.sendGETRequest();
  chs.receiveResponse();

  // hr = WebSocketBeginServerHandshake(
  //     wshServer, NULL, NULL, 0, pClientHeaders, uiClientHeadersCount,
  //     &pServerAdditionalHeaders, &uiServerAdditionalHeadersCount);
  // if (FAILED(hr)) {
  //   std::wcout << L"Handshake failed\n";
  //   return hr;
  // }

  int iHTTPStatus = chs.getResponseHTTPStatus();
  if (iHTTPStatus != 101) {
    wprintf(L"Handshake to the server failed. HTTP Status is %d\n",
            iHTTPStatus);
    return E_FAIL;
  }

  auto umssResponseHeader = chs.getResponseHeaders();
  uiServerAdditionalHeadersCount = (ULONG)umssResponseHeader.size();
  pServerAdditionalHeaders =
      new WEB_SOCKET_HTTP_HEADER[uiServerAdditionalHeadersCount];

  std::pair<std::string, std::string>* ppssKeyValuePairs =
      new std::pair<std::string, std::string>[uiServerAdditionalHeadersCount];
  uiServerAdditionalHeadersCount = 0;
  for (auto pssKeyValuePair : umssResponseHeader) {
    ppssKeyValuePairs[uiServerAdditionalHeadersCount++] = pssKeyValuePair;
  }

  for (ULONG i = 0; i < uiServerAdditionalHeadersCount; i++) {
    pServerAdditionalHeaders[i] = {&ppssKeyValuePairs[i].first[0],
                                   (ULONG)ppssKeyValuePairs[i].first.length(),
                                   &ppssKeyValuePairs[i].second[0],
                                   (ULONG)ppssKeyValuePairs[i].second.length()};
  }

  hr = WebSocketEndClientHandshake(wshClient, pServerAdditionalHeaders,
                                   uiServerAdditionalHeadersCount, NULL, 0,
                                   NULL);
  if (FAILED(hr)) {
    wprintf(L"WebSocketEndClientHandshake failed 0x%08x\n", hr);
    return hr;
  }

  // hr = WebSocketEndServerHandshake(wshServer);
  // if (FAILED(hr)) {
  //   std::wcout << L"Handshake failed\n";
  //   return hr;
  // }

  std::wcout << L"Handshake ended successfully\n";
  bIsHandshakeSuccessful = 1;
  return hr;
}

HRESULT WebSocketClient::send(std::wstring wsMsg) {
  HRESULT hr = S_OK;
  WEB_SOCKET_BUFFER wsbBuffer;

  wsbBuffer.Data.pbBuffer = (PBYTE)wsMsg.c_str();
  wsbBuffer.Data.ulBufferLength = (ULONG)wsMsg.size() * sizeof(WCHAR);

  // std::wcout << "Queuing a send with a buffer\n";
  // wprintf(L"Data pending: %ws\n", wsMsg.c_str());

  hr = WebSocketSend(wshClient, WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, &wsbBuffer,
                     NULL);
  if (FAILED(hr)) {
    wprintf(L"WebSocketSend failed 0x%08x\n", hr);
    return hr;
  }

  hr = runGetActionLoop();
  if (FAILED(hr)) {
    return hr;
  }

  // std::wcout << "Queuing a receive\n";

  //hr = WebSocketReceive(wshClient, NULL, NULL);
  //if (FAILED(hr)) {
  //  wprintf(L"WebSocketReceive failed 0x%08x\n", hr);
  //  return hr;
  //}

  //hr = runGetActionLoop();
  //if (FAILED(hr)) {
  //  return hr;
  //}

  // wprintf(L"Data received: %ws\n", (LPWSTR)wsbBuffer.Data.pbBuffer);

  return hr;
}

bool WebSocketClient::isHandshakeSuccessful() { return bIsHandshakeSuccessful; }

HRESULT WebSocketClient::runGetActionLoop() {
  HRESULT hr = S_OK;
  WEB_SOCKET_BUFFER pwsbBuffers[WSC_MAX_NUMBER_OF_BUFFERS] = {0};
  ULONG uiBufferCount = WSC_MAX_NUMBER_OF_BUFFERS;
  ULONG uiBytesTransfered = 0, uiBytesNeedTransferingToCurrentBuffer = 0;
  WEB_SOCKET_BUFFER_TYPE bufferType;
  WEB_SOCKET_ACTION action;
  PVOID actionContext;
  std::wstring wsMsg;

  do {
    uiBufferCount = ARRAYSIZE(pwsbBuffers);
    uiBytesTransfered = 0;

    hr = WebSocketGetAction(wshClient, WEB_SOCKET_ALL_ACTION_QUEUE, pwsbBuffers,
                            &uiBufferCount, &action, &bufferType, NULL,
                            &actionContext);
    if (FAILED(hr)) {
      wprintf(L"WebSocketGetAction failed 0x%08x\n", hr);
      WebSocketAbortHandle(wshClient);
    }

    switch (action) {
      case WEB_SOCKET_NO_ACTION:
        break;
      case WEB_SOCKET_RECEIVE_FROM_NETWORK_ACTION:
        // std::wcout << (L"Receiving data from a network\n");

        assert(uiBufferCount >= 1);

        //hr = chs.receiveMsg(&wsMsg);
        //if (FAILED(hr)) {
        //  break;
        //}

        //for (ULONG i = 0; i < uiBufferCount; i++) {
        //  uiBytesNeedTransferingToCurrentBuffer =
        //      min(sizeof(WCHAR) * wsMsg.size() - uiBytesTransfered,
        //          pwsbBuffers[i].Data.ulBufferLength);
        //  CopyMemory(pwsbBuffers[i].Data.pbBuffer,
        //             &wsMsg[0] + uiBytesTransfered,
        //             uiBytesNeedTransferingToCurrentBuffer);
        //  // uiBytesTransfered += sizeof(WCHAR) * wsMsg.size();
        //  uiBytesTransfered += uiBytesNeedTransferingToCurrentBuffer;
        //  if (uiBytesNeedTransferingToCurrentBuffer <
        //      pwsbBuffers[i].Data.ulBufferLength) {
        //    break;
        //  }
        //}
        /*wprintf(L"Size of buffer is %d\n", pwsbBuffers[0].Data.ulBufferLength);
        wprintf(L"Number of buffer is %d\n", uiBufferCount);*/

        hr = chs.receiveData((LPSTR)pwsbBuffers[0].Data.pbBuffer,
                             &pwsbBuffers[0].Data.ulBufferLength,
                             &uiBytesTransfered);
        if (FAILED(hr)) {
          break;
        }
        
        uiBytesTransfered = pwsbBuffers[0].Data.ulBufferLength;
        //wprintf(L"Receive through socket OK\n");

        break;

      case WEB_SOCKET_INDICATE_RECEIVE_COMPLETE_ACTION:
        // std::wcout << (L"Receiving operation completed with a buffer\n");
        if (uiBufferCount != 1) {
          //wprintf(L"Number of buffer is %d\n",
          //        uiBufferCount);
          assert(0);
          hr = E_FAIL;
          return hr;
        }
        break;

      case WEB_SOCKET_SEND_TO_NETWORK_ACTION:
        // std::wcout << L"Sending data to a network\n";
        //for (ULONG i = 0; i < uiBufferCount; i++) {
        //  /*hr = tm.tmWriteData(pwsbBuffers[i].Data.pbBuffer,
        //                      &pwsbBuffers[i].Data.ulBufferLength);*/
        //  for (int j = 0; j < pwsbBuffers[i].Data.ulBufferLength; j++) {
        //    std::wcout << (int)pwsbBuffers[i].Data.pbBuffer[j] << " ";
        //  }
        //  std::wcout << "\n";
        //  wsMsg += pwsbBuffers[i].Data.pbBuffer;
        //  uiBytesTransfered += pwsbBuffers[i].Data.ulBufferLength;
        //}
        hr = chs.sendData((LPSTR)pwsbBuffers[0].Data.pbBuffer, &pwsbBuffers[0].Data.ulBufferLength);
        if (FAILED(hr)) {
          break;
        }
        //wprintf(L"Send through socket OK\n");
        uiBytesTransfered = pwsbBuffers[0].Data.ulBufferLength;
        break;

      case WEB_SOCKET_INDICATE_SEND_COMPLETE_ACTION:
        //std::wcout << L"Send operation complete\n";
        break;

      default:
        assert(0);
        hr = E_FAIL;
        break;
    }
    WebSocketCompleteAction(wshClient, actionContext, uiBytesTransfered);
  } while (action != WEB_SOCKET_NO_ACTION);
  return hr;
}

HRESULT WebSocketClient::cleanup() {
  if (wshClient != NULL) {
    if (bIsHandshakeSuccessful) {
      WebSocketAbortHandle(wshClient);
    }
    WebSocketDeleteHandle(wshClient);
    wshClient = NULL;
  }
  chs.disconnectFromCurrentHost();
  bIsHandshakeSuccessful = 0;
  // if (wshServer != NULL) {
  //   WebSocketDeleteHandle(wshServer);
  //   wshServer = NULL;
  // }
  return S_OK;
}