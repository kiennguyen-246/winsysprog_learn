#include "Functions.h"

FLT_PREOP_CALLBACK_STATUS mfltPreCreate(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID* pCompletionContext) {
  DbgPrint("mfltPreCreate called\n");
  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostCreate(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  DbgPrint("mfltPostCreate called\n");
  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}