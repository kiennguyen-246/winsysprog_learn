#ifndef INCLUDE_GUARD
#define INCLUDE_GUARD

#include <wdm.h>

// #define __USE_DIRECT__
#define __USE_BUFFERED__

#ifdef __USE_DIRECT__
#define IO_TYPE DO_DIRECT_IO
#endif

#ifdef __USE_BUFFERED__
#define IO_TYPE DO_BUFFERED_IO
#endif

#ifndef IO_TYPE
#define IO_TYPE 0
#endif

NTSTATUS handleUnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCloseFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCreateFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleDeviceIoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleReadFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleWriteFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

#pragma alloc_text(PAGE, handleUnsupportedFunction)
#pragma alloc_text(PAGE, handleCreateFile)
#pragma alloc_text(PAGE, handleReadFile)
#pragma alloc_text(PAGE, handleWriteFile)
#pragma alloc_text(PAGE, handleDeviceIoControl)
#pragma alloc_text(PAGE, handleCloseFile)

#endif