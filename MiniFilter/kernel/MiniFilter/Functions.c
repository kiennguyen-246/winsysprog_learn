#include "Functions.h"

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
    UNICODE_STRING usVolumeName;
    WCHAR pwcInitString[1024];
    for (int i = 0; i < 1023; i++) {
      pwcInitString[i] = L' ';
    }
    RtlInitUnicodeString(&usVolumeName, pwcInitString);
    //DbgBreakPoint();
    ULONG ulVolumeNameBufferSize;
    NTSTATUS status = FltGetVolumeName(pFltObj->Volume, &usVolumeName,
                                       &ulVolumeNameBufferSize);
    if (status == STATUS_SUCCESS) {
      // DbgBreakPoint();
      DbgPrint("Open file: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
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
