#include "ComPort.hpp"

ComPort::ComPort() { hComPort = NULL; }

ComPort::~ComPort() {
  if (hComPort != NULL) {
    // CloseHandle(hComPort);
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

HRESULT ComPort::getRecord(PMFLT_EVENT_RECORD pEventRecord) {
  HRESULT hr = S_OK;
  COM_MESSAGE msg;
  memset(&msg, 0, sizeof(&msg));
  hr = FilterGetMessage(
      hComPort, &msg.header,
      sizeof(MFLT_EVENT_RECORD) + sizeof(FILTER_MESSAGE_HEADER), NULL);
  if (hr != S_OK) {
    wprintf(L"Get message failed 0x%08x\n", hr);
    return hr;
  }
  // wprintf(L"Get message OK\n");
  // fflush(stdout);

  // wprintf(L"0x%08x\n", pEventRecord);
  // wprintf(L"%d\n", sizeof(MFLT_EVENT_RECORD));

  CopyMemory(pEventRecord, &msg.eventRecord, sizeof(MFLT_EVENT_RECORD));

  // wprintf(L"Copy memory OK\n");
  // fflush(stdout);

  // mtx.lock();
  // fo << std::wstring(pwcObtainedMsgContent);
  // mtx.unlock();
  return hr;
}

//HRESULT ComPort::getRecord(std::vector<MFLT_EVENT_RECORD>* pvEventRecord) {
//  HRESULT hr = S_OK;
//  COM_MESSAGE msg;
//  memset(&msg, 0, sizeof(&msg));
//  hr = FilterGetMessage(
//      hComPort, &msg.header,
//      sizeof(MFLT_SEND_MESSAGE) + sizeof(FILTER_MESSAGE_HEADER), NULL);
//  if (hr != S_OK) {
//    wprintf(L"Get message failed 0x%08x\n", hr);
//    return hr;
//  }
//  // wprintf(L"Get message OK\n");
//  // fflush(stdout);
//
//  // wprintf(L"0x%08x\n", pEventRecord);
//  // wprintf(L"%d\n", sizeof(MFLT_EVENT_RECORD));
//
//  for (ULONG uiCurrentRecordId = 0;
//       uiCurrentRecordId < msg.sendMessage.uiEventRecordCount;
//       uiCurrentRecordId++) {
//    pvEventRecord->push_back(
//        msg.sendMessage.pEventRecordBuffer[uiCurrentRecordId]);
//  }
//
//  // wprintf(L"Copy memory OK\n");
//  // fflush(stdout);
//
//  // mtx.lock();
//  // fo << std::wstring(pwcObtainedMsgContent);
//  // mtx.unlock();
//  return hr;
//}

HRESULT ComPort::disconnect() {
  CloseHandle(hComPort);
  return S_OK;
}