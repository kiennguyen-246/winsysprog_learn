#ifndef FUNCTIONS_GUARD
#define FUNCTIONS_GUARD

#include "common.h"

FLT_PREOP_CALLBACK_STATUS mfltPreCreate(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID *pCompletionContext);

FLT_POSTOP_CALLBACK_STATUS mfltPostCreate(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID *pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags);


#endif