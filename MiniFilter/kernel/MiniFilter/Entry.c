#include "Entry.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pusRegistryPath) {
  DbgPrint("DriverEntry called\n");
  NTSTATUS ntStatus = STATUS_SUCCESS;
  PSECURITY_DESCRIPTOR psd;
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING usMfltPortName;

  mfltData.pDriverObject = pDriverObject;

  // Register the filter
  ntStatus =
      FltRegisterFilter(pDriverObject, &fltRegistration, &mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Register filter failed 0x%08x\n", ntStatus);
    return ntStatus;
  }
  // DbgPrint("Register filter successful\n");

  // DbgBreakPoint();

  // Create a communication port
  ntStatus = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Build security descriptor failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  RtlInitUnicodeString(&usMfltPortName, MFLT_COM_PORT_NAME);

  InitializeObjectAttributes(&oa, &usMfltPortName,
                             OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL,
                             psd);
  ntStatus = FltCreateCommunicationPort(mfltData.pFilter, &mfltData.pServerPort,
                                        &oa, NULL, mfltComConnect,
                                        mfltComDisconnect, NULL, 1);
  FltFreeSecurityDescriptor(psd);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Create COM port failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  // Start filtering (Must unregister immediately if fail)
  ntStatus = FltStartFiltering(mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start filtering failed 0x%08x\n", ntStatus);
    FltCloseCommunicationPort(mfltData.pFilter);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }
  DbgPrint("Start filtering successful\n");

  return ntStatus;
}

NTSTATUS DriverUnload(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags) {
  DbgPrint("DriverUnload called\n");

  FltCloseCommunicationPort(mfltData.pServerPort);

  // Unregister filter
  FltUnregisterFilter(mfltData.pFilter);

  return STATUS_SUCCESS;
}

NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
  DbgPrint("DriverQueryTeardown called\n");
  return STATUS_SUCCESS;
}

NTSTATUS mfltComConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG uiSizeOfContext,
                        PVOID* pConnectionCookie) {
  DbgPrint("mfltComConnect called\n");
  FLT_ASSERT(mfltData.pClientPort == NULL);
  mfltData.pClientPort = pClientPort;
  return STATUS_SUCCESS;
}

NTSTATUS mfltComDisconnect(PVOID pConnectionCookie) {
  DbgPrint("mfltComDisconnect called\n");
  FltCloseClientPort(mfltData.pFilter, mfltData.pClientPort);
  return STATUS_SUCCESS;
}

// NTSTATUS comMessage(PVOID pConnectionCookie, PVOID pInputBuffer,
//                     ULONG uiInputBufferSize, PVOID pOutputBuffer,
//                     ULONG uiOutputBufferSize,
//                     PULONG puiReturnOutputBufferLength) {
//   NTSTATUS ntStatus = STATUS_SUCCESS;
//   if (pInputBuffer != NULL) {
//     if (pOutputBuffer != NULL && uiOutputBufferSize != 0) {
//       PLIST_ENTRY pleCurNode;
//       ULONG ulBytesWritten;
//       NTSTATUS ntStatus = STATUS_SUCCESS;
//       KIRQL oldIrql;
//       BOOLEAN bIsRecordsAvailable = FALSE;
//
//       KeAcquireSpinLock(&mfltData.kslOutputBufferLock, &oldIrql);
//
//       while (!IsListEmpty(&mfltData.leOutputBufferList) &&
//       (uiOutputBufferSize > 0)) {
//         bIsRecordsAvailable = TRUE;
//
//         pleCurNode = RemoveHeadList(&mfltData.leOutputBufferList);
//       }
//
//     } else {
//       ntStatus = STATUS_INVALID_PARAMETER;
//     }
//   }
// }

VOID mfltContextCleanup(PFLT_CONTEXT pFltContext,
                        FLT_CONTEXT_TYPE fltContextType) {}

FLT_PREOP_CALLBACK_STATUS mfltPreClose(PFLT_CALLBACK_DATA pCallbackData,
                                       PCFLT_RELATED_OBJECTS pFltObj,
                                       PVOID* pCompletionContext) {
  // DbgBreakPoint();
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  if (!(pFltObj->FileObject->Flags & FILE_DIRECTORY_FILE)) {
    UNICODE_STRING usVolumeName;
    WCHAR pwcInitString[1024];
    for (int i = 0; i < 1023; i++) {
      pwcInitString[i] = L' ';
    }
    RtlInitUnicodeString(&usVolumeName, pwcInitString);
    // DbgBreakPoint();
    ULONG ulVolumeNameBufferSize;
    NTSTATUS status = FltGetVolumeName(pFltObj->Volume, &usVolumeName,
                                       &ulVolumeNameBufferSize);
    if (status == STATUS_SUCCESS) {
      // DbgBreakPoint();
      DbgPrint("Close file: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
    } else {
      DbgBreakPoint();
    }

    // DbgPrint("Open file: %wZ\n", pFltObj->FileObject->FileName);
  }

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostClose(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  // DbgBreakPoint();
  // DbgPrint("mfltPostClose called\n");

  // UNICODE_STRING usVolumeName;
  // FltGetVolumeName(pFltObj->Volume, &usVolumeName, NULL);
  // DbgBreakPoint();

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS mfltPreCreate(PFLT_CALLBACK_DATA pCallbackData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  // DbgBreakPoint();
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  if (!(pFltObj->FileObject->Flags & FILE_DIRECTORY_FILE)) {
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING usVolumeName;
    WCHAR pwcInitString[1024];
    for (int i = 0; i < 1023; i++) {
      pwcInitString[i] = L' ';
    }
    RtlInitUnicodeString(&usVolumeName, pwcInitString);
    // DbgBreakPoint();
    ULONG ulVolumeNameBufferSize;
    NTSTATUS status = FltGetVolumeName(pFltObj->Volume, &usVolumeName,
                                       &ulVolumeNameBufferSize);
    if (status == STATUS_SUCCESS) {
      // DbgBreakPoint();
      DbgPrint("Open file: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
      if (mfltData.pClientPort != NULL) {
        WCHAR wcComMsg[1024] = L"Open file: ";
        wcscat(wcComMsg, usVolumeName.Buffer);
        wcscat(wcComMsg, pFltObj->FileObject->FileName.Buffer);
        wcscat(wcComMsg, L"\n");
        DbgBreakPoint();
        ntStatus = FltSendMessage(mfltData.pFilter, mfltData.pClientPort,
                                  wcComMsg, 1024, NULL, 0, NULL);
        if (ntStatus != STATUS_SUCCESS) {
          DbgPrint("Send message failed 0x%08x\n", ntStatus);
        }
      }
    } else {
      DbgBreakPoint();
    }

    // DbgPrint("Open file: %wZ\n", pFltObj->FileObject->FileName);
  }

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostCreate(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  // DbgBreakPoint();
  // DbgPrint("mfltPostCreate called\n");

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS mfltPreRead(PFLT_CALLBACK_DATA pCallbackData,
                                      PCFLT_RELATED_OBJECTS pFltObj,
                                      PVOID* pCompletionContext) {
  // DbgBreakPoint();
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  if (!(pFltObj->FileObject->Flags & FILE_DIRECTORY_FILE)) {
    UNICODE_STRING usVolumeName;
    WCHAR pwcInitString[1024];
    for (int i = 0; i < 1023; i++) {
      pwcInitString[i] = L' ';
    }
    RtlInitUnicodeString(&usVolumeName, pwcInitString);
    // DbgBreakPoint();
    ULONG ulVolumeNameBufferSize;
    NTSTATUS status = FltGetVolumeName(pFltObj->Volume, &usVolumeName,
                                       &ulVolumeNameBufferSize);
    if (status == STATUS_SUCCESS) {
      // DbgBreakPoint();
      DbgPrint("Read on file: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
    } else {
      DbgBreakPoint();
    }

    // DbgPrint("Open file: %wZ\n", pFltObj->FileObject->FileName);
  }

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostRead(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  // DbgBreakPoint();
  // DbgPrint("mfltPostRead called\n");

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS mfltPreWrite(PFLT_CALLBACK_DATA pCallbackData,
                                       PCFLT_RELATED_OBJECTS pFltObj,
                                       PVOID* pCompletionContext) {
  // DbgBreakPoint();
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  if (!(pFltObj->FileObject->Flags & FILE_DIRECTORY_FILE)) {
    UNICODE_STRING usVolumeName;
    WCHAR pwcInitString[1024];
    for (int i = 0; i < 1023; i++) {
      pwcInitString[i] = L' ';
    }
    RtlInitUnicodeString(&usVolumeName, pwcInitString);
    // DbgBreakPoint();
    ULONG ulVolumeNameBufferSize;
    NTSTATUS status = FltGetVolumeName(pFltObj->Volume, &usVolumeName,
                                       &ulVolumeNameBufferSize);
    if (status == STATUS_SUCCESS) {
      // DbgBreakPoint();
      DbgPrint("Write to file: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
    } else {
      DbgBreakPoint();
    }

    // DbgPrint("Open file: %wZ\n", pFltObj->FileObject->FileName);
  }

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostWrite(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  // DbgBreakPoint();
  // DbgPrint("mfltPostWrite called\n");

  return FLT_POSTOP_FINISHED_PROCESSING;
}
