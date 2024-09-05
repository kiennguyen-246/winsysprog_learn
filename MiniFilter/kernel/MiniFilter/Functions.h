#ifndef FUNCTIONS_GUARD
#define FUNCTIONS_GUARD

#include "common.h"

FLT_PREOP_CALLBACK_STATUS mfltPreClose(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID *pCompletionContext);

FLT_POSTOP_CALLBACK_STATUS mfltPostClose(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID *pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags);

FLT_PREOP_CALLBACK_STATUS mfltPreCreate(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID *pCompletionContext);

FLT_POSTOP_CALLBACK_STATUS mfltPostCreate(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID *pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags);

FLT_PREOP_CALLBACK_STATUS mfltPreRead(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID *pCompletionContext);

FLT_POSTOP_CALLBACK_STATUS mfltPostRead(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID *pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags);

FLT_PREOP_CALLBACK_STATUS mfltPreWrite(PFLT_CALLBACK_DATA pCallbckData,
                                        PCFLT_RELATED_OBJECTS pFltObj,
                                        PVOID *pCompletionContext);

FLT_POSTOP_CALLBACK_STATUS mfltPostWrite(
    PFLT_CALLBACK_DATA pCallbckData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID *pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags);


#endif