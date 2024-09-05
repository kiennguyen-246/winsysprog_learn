#include "Functions.h"

FLT_PREOP_CALLBACK_STATUS mfltPreClose(PFLT_CALLBACK_DATA pCallbackData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  //DbgBreakPoint();
  DbgPrint("mfltPreClose called\n");

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostClose(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  //DbgBreakPoint();
  DbgPrint("mfltPostClose called\n");

  //UNICODE_STRING usVolumeName;
  //FltGetVolumeName(pFltObj->Volume, &usVolumeName, NULL);
  //DbgBreakPoint();
   DbgPrint("File Closed: %wZ\n",
           pFltObj->FileObject->FileName);

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS mfltPreCreate(PFLT_CALLBACK_DATA pCallbackData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  //DbgBreakPoint();
  DbgPrint("mfltPreCreate called\n");

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostCreate(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  //DbgBreakPoint();
  DbgPrint("mfltPostCreate called\n");

  //UNICODE_STRING usVolumeName;
  //FltGetVolumeName(pFltObj->Volume, &usVolumeName, NULL);
  //DbgBreakPoint();
   DbgPrint("File opened: %wZ\n",
           pFltObj->FileObject->FileName);

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS mfltPreRead(PFLT_CALLBACK_DATA pCallbackData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  //DbgBreakPoint();
  DbgPrint("mfltPreRead called\n");

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostRead(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  //DbgBreakPoint();
  DbgPrint("mfltPostRead called\n");

  //UNICODE_STRING usVolumeName;
  //FltGetVolumeName(pFltObj->Volume, &usVolumeName, NULL);
  //DbgBreakPoint();
   DbgPrint("File Read: %wZ\n",
           pFltObj->FileObject->FileName);

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS mfltPreWrite(PFLT_CALLBACK_DATA pCallbackData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  //DbgBreakPoint();
  DbgPrint("mfltPreWrite called\n");

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostWrite(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  //DbgBreakPoint();
  DbgPrint("mfltPostWrite called\n");

  //UNICODE_STRING usVolumeName;
  //FltGetVolumeName(pFltObj->Volume, &usVolumeName, NULL);
  //DbgBreakPoint();
   DbgPrint("File Written: %wZ\n",
           pFltObj->FileObject->FileName);

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}
