#include "Entry.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pusRegistryPath) {
  DbgPrint("DriverEntry called\n");
  NTSTATUS ntStatus = STATUS_SUCCESS;

  mfltData.pDriverObject = pDriverObject;

  // Register the filter
  ntStatus =
      FltRegisterFilter(pDriverObject, &fltRegistration, &mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Register filter failed\n");
    return ntStatus;
  }
  DbgPrint("Register filter successful\n");



  //DbgBreakPoint();

  // Start filtering (Must unregister immediately if fail)
  ntStatus = FltStartFiltering(mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start filtering failed\n");
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }
  DbgPrint("Start filtering successful\n");

  return ntStatus;
}

NTSTATUS DriverUnload(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags) {
  DbgPrint("DriverUnload called\n");

  // Unregister filter
  FltUnregisterFilter(mfltData.pFilter);

  return STATUS_SUCCESS;
}

NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
  DbgPrint("DriverQueryTeardown called\n");
  return STATUS_SUCCESS;
}

NTSTATUS comConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
    PVOID pConnectionContext, ULONG uiSizeOfContext,
    PVOID* pConnectionCookie) {
  FLT_ASSERT(mfltData.pClientPort == NULL);
  mfltData.pClientPort = pClientPort;
  return STATUS_SUCCESS;
}

NTSTATUS comDisconnect(PVOID pConnectionCookie) {
  FltCloseClientPort(mfltData.pFilter, mfltData.pClientPort);
  return STATUS_SUCCESS;
}

NTSTATUS comMessage(PVOID pConnectionCookie, PVOID pInputBuffer,
    ULONG uiInputBufferSize, PVOID pOutputBuffer,
    ULONG uiOutputBufferSize,
    PULONG puiReturnOutputBufferLength) {

}