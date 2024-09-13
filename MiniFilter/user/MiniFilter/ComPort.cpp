#include "ComPort.hpp"

ComPort::ComPort() { hComPort = NULL; }

ComPort::~ComPort() {
  if (hComPort != NULL) {
    //CloseHandle(hComPort);
  }
  hComPort = NULL;
}

HRESULT ComPort::connect(std::wstring sPortName) {
  HRESULT hr = S_OK;
  hr = FilterConnectCommunicationPort(sPortName.c_str(), 0, NULL, 0, NULL,
                                      &hComPort);
  if (hr != S_OK) {
    wprintf(L"Connect to kernel mode failed 0x%08x\n", hr);
    fflush(stdout);
    return hr;
  }
  return hr;
}

HRESULT ComPort::getMsg(std::wstring& wsMsg) {
  HRESULT hr = S_OK;
  COM_MESSAGE msg;
  memset(&msg, 0, sizeof(&msg));
  hr = FilterGetMessage(hComPort, &msg.header,
                        MAX_BUFFER_SIZE + sizeof(FILTER_MESSAGE_HEADER), NULL);
  if (hr != S_OK) {
    wprintf(L"Get message failed 0x%08x\n", hr);
    return hr;
  }
  WCHAR pwcObtainedMsgContent[MAX_BUFFER_SIZE / sizeof(WCHAR)];
  memcpy(pwcObtainedMsgContent, msg.buffer + sizeof(FILTER_MESSAGE_HEADER),
         MAX_BUFFER_SIZE);
  // mtx.lock();
  wsMsg = std::wstring(pwcObtainedMsgContent);
  // fo << std::wstring(pwcObtainedMsgContent);
  // mtx.unlock();
  return hr;
}

HRESULT ComPort::disconnect() {
  CloseHandle(hComPort);
  return S_OK;
}