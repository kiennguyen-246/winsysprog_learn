#ifndef ENTRY_GUARD
#define ENTRY_GUARD

#include "common.h"
#include "Functions.h"

MFLT_DATA mfltData;

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pusRegistryPath);
NTSTATUS DriverUnload(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags);
NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);

NTSTATUS comConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                    PVOID pConnectionContext, ULONG uiSizeOfContext,
                    PVOID* pConnectionCookie);
NTSTATUS comDisconnect(PVOID pConnectionCookie);
NTSTATUS comMessage(PVOID pConnectionCookie, PVOID pInputBuffer,
                    ULONG uiInputBufferSize, PVOID pOutputBuffer,
                    ULONG uiOutputBufferSize,
                    PULONG puiReturnOutputBufferLength);

//const FLT_CONTEXT_REGISTRATION[] = {
//        {FLT_INSTANCE_CONTEXT, 0, mfltContextCleanup, CTX_INSTANCE_CONTEXT_SIZE,
//         CTX_INSTANCE_CONTEXT_TAG},
//
//        {FLT_FILE_CONTEXT, 0, CtxContextCleanup, CTX_FILE_CONTEXT_SIZE,
//         CTX_FILE_CONTEXT_TAG},
//
//        {FLT_STREAM_CONTEXT, 0, CtxContextCleanup, CTX_STREAM_CONTEXT_SIZE,
//         CTX_STREAM_CONTEXT_TAG},
//
//        {FLT_STREAMHANDLE_CONTEXT, 0, CtxContextCleanup,
//         CTX_STREAMHANDLE_CONTEXT_SIZE, CTX_STREAMHANDLE_CONTEXT_TAG},
//    {FLT_CONTEXT_END},
//}

const FLT_OPERATION_REGISTRATION fltOperations[] = {
    {IRP_MJ_CREATE, 0, mfltPreCreate, mfltPostCreate},
    {IRP_MJ_WRITE, 0, mfltPreWrite, mfltPostWrite},
    //{IRP_MJ_READ, 0, mfltPreRead, mfltPostRead},
    //{IRP_MJ_NOT_S, 0, mfltPreCleanup, mfltPostCleanup},
    {IRP_MJ_CLOSE, 0, mfltPreClose, mfltPostClose},
    {IRP_MJ_OPERATION_END}};

const FLT_REGISTRATION fltRegistration = {sizeof(FLT_REGISTRATION),
                                          FLT_REGISTRATION_VERSION,
                                          FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,
                                          NULL,
                                          fltOperations,
                                          DriverUnload,
                                          NULL,
                                          DriverQueryTeardown,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL};

#endif  // ENTRY_GUARD