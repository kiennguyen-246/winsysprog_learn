#include "WebSocketClient.hpp"

#include <cassert>

WebSocketClient::WebSocketClient() {
  sHostName = "";
  wshClient = NULL;
  // wshServer = NULL;
}

WebSocketClient::~WebSocketClient() {
  wshClient = NULL;
  // wshServer = NULL;
}

bool WebSocketClient::wscGetHostName(LPSTR pcHostName, ULONG& uiLength) {
  strcpy(pcHostName, sHostName.c_str());
  uiLength = sHostName.length();
  return 0;
}

bool WebSocketClient::wscSetHostName(LPCSTR pcHostName, const ULONG& uiLength) {
  sHostName = pcHostName;
  return 0;
}

HRESULT WebSocketClient::wscInitialize() {
  HRESULT hr = S_OK;

  WEB_SOCKET_HANDLE wshTempClient;
  // WEB_SOCKET_HANDLE wshTempServer;

  hr = WebSocketCreateClientHandle(NULL, 0, &wshTempClient);
  if (FAILED(hr)) {
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

HRESULT WebSocketClient::wscPerformHandshake() {
  HRESULT hr = S_OK;
  ULONG uiClientAdditionalHeadersCount = 0;
  PWEB_SOCKET_HTTP_HEADER pClientAdditionalHeaders = NULL;
  // ULONG uiServerAdditionalHeadersCount = 0;
  // PWEB_SOCKET_HTTP_HEADER pServerAdditionalHeaders = NULL;
  ULONG uiClientHeadersCount = 0;
  PWEB_SOCKET_HTTP_HEADER pClientHeaders = NULL;

  CHAR pcHostHeaderTag[] = "Host";
  const static WEB_SOCKET_HTTP_HEADER host = {
      pcHostHeaderTag, strlen(pcHostHeaderTag), &sHostName[0],
      sHostName.length()};

  hr = WebSocketBeginClientHandshake(wshClient, NULL, 0, NULL, 0, NULL, 0,
                                     &pClientAdditionalHeaders,
                                     &uiClientAdditionalHeadersCount);
  if (FAILED(hr)) {
    std::wcout << L"Handshake failed\n";
    return hr;
  }

  uiClientHeadersCount = uiClientAdditionalHeadersCount + 1;
  pClientHeaders = new WEB_SOCKET_HTTP_HEADER[uiClientHeadersCount];
  if (pClientHeaders == NULL) {
    hr = E_OUTOFMEMORY;
    std::wcout << L"Handshake failed\n";
    return hr;
  }
  CopyMemory(pClientHeaders, pClientAdditionalHeaders,
             uiClientAdditionalHeadersCount * sizeof(WEB_SOCKET_HTTP_HEADER));
  pClientHeaders[uiClientAdditionalHeadersCount] = host;

  // hr = WebSocketBeginServerHandshake(
  //     wshServer, NULL, NULL, 0, pClientHeaders, uiClientHeadersCount,
  //     &pServerAdditionalHeaders, &uiServerAdditionalHeadersCount);
  // if (FAILED(hr)) {
  //   std::wcout << L"Handshake failed\n";
  //   return hr;
  // }

  std::wcout << L"Handshake initiated successfully\n";
  hr = WebSocketEndClientHandshake(wshClient, pClientAdditionalHeaders,
                                   uiClientAdditionalHeadersCount, NULL, 0,
                                   NULL);
  if (FAILED(hr)) {
    std::wcout << L"Handshake failed\n";
    return hr;
  }

  // hr = WebSocketEndServerHandshake(wshServer);
  // if (FAILED(hr)) {
  //   std::wcout << L"Handshake failed\n";
  //   return hr;
  // }

  std::wcout << L"Handshake ended successfully\n";
  return hr;
}

HRESULT WebSocketClient::wscPerformDataExchange() {
  HRESULT hr = S_OK;
  WCHAR pwcData[] = L"Hello world";
  WEB_SOCKET_BUFFER wsbBuffer;

  wsbBuffer.Data.pbBuffer = (PBYTE)pwcData;
  wsbBuffer.Data.ulBufferLength = wcslen(pwcData) * sizeof(WCHAR);

  std::wcout << "Queuing a send with a buffer\n";
  wprintf(L"Data pending: %ws", pwcData);

  hr = WebSocketSend(wshClient, WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, &wsbBuffer,
                     NULL);
  if (FAILED(hr)) {
    std::wcout << "Failed to send some data\n";
    return hr;
  }

  hr = wscRunGetActionLoop();
  if (FAILED(hr)) {
    std::wcout << "Failed to get actions after sending\n";
    return hr;
  }

  std::wcout << "Queuing a receive\n";

  hr = WebSocketReceive(wshClient, NULL, NULL);
  if (FAILED(hr)) {
    std::wcout << "Failed to get actions after sending\n";
    return hr;
  }
}

HRESULT WebSocketClient::wscRunGetActionLoop() {
  HRESULT hr = S_OK;
  WEB_SOCKET_BUFFER pwsbBuffers[WSC_MAX_NUMBER_OF_BUFFERS] = {0};
  ULONG uiBufferCount = WSC_MAX_NUMBER_OF_BUFFERS;
  ULONG uiByteTransfered = 0;
  WEB_SOCKET_BUFFER_TYPE bufferType;
  WEB_SOCKET_ACTION action;
  PVOID actionContext;

  do {
    uiBufferCount = 1;
    uiByteTransfered = 0;

    hr = WebSocketGetAction(wshClient, WEB_SOCKET_ALL_ACTION_QUEUE, pwsbBuffers,
                            &uiBufferCount, &action, &bufferType, NULL,
                            &actionContext);
    if (FAILED(hr)) {
      WebSocketAbortHandle(wshClient);
    }

    switch (action) {
      case WEB_SOCKET_NO_ACTION:
        break;
      case WEB_SOCKET_RECEIVE_FROM_NETWORK_ACTION:
        std::wcout << (L"Receiving data from a network\n");

        assert(uiBufferCount >= 1);
        for (int i = 0; i < uiBufferCount; i++) {
          hr = tm.tmReadData(pwsbBuffers[i].Data.pbBuffer, &uiByteTransfered);
          if (FAILED(hr)) {
            break;
          }
          wprintf(L"%ws", (LPWSTR)pwsbBuffers[i].Data.pbBuffer);

          if (uiByteTransfered < pwsbBuffers[i].Data.ulBufferLength) {
            break;
          }
        }

        break;

      case WEB_SOCKET_INDICATE_RECEIVE_COMPLETE_ACTION:
        std::wcout << (L"Receiving operation completed with a buffer\n");
        if (uiBufferCount != 1) {
          // assert(0);
          hr = E_FAIL;
          return hr;
        }
        break;

      case WEB_SOCKET_SEND_TO_NETWORK_ACTION:
        std::wcout << L"Sending data to a network\n";
        for (int i = 0; i < uiBufferCount; i++) {
          hr = tm.tmWriteData(pwsbBuffers[i].Data.pbBuffer,
                              &pwsbBuffers[i].Data.ulBufferLength);
          if (FAILED(hr)) {
            break;
          }

          uiByteTransfered += pwsbBuffers[i].Data.ulBufferLength;
        }

      case WEB_SOCKET_INDICATE_SEND_COMPLETE_ACTION:
        std::wcout << L"Send operation complete";

      default:
        hr = E_FAIL;
        break;
    }
  } while (action != WEB_SOCKET_NO_ACTION);
  return hr;
}

HRESULT WebSocketClient::wscDeleteHandle() {
  if (wshClient != NULL) {
    WebSocketDeleteHandle(wshClient);
    wshClient = NULL;
  }
  // if (wshServer != NULL) {
  //   WebSocketDeleteHandle(wshServer);
  //   wshServer = NULL;
  // }
}