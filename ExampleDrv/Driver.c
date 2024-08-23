#include <wdm.h>

#define DbgPrint(x, ...) \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, x, __VA_ARGS__)

// #define __USE_DIRECT__
#define __USE_BUFFERED__

#ifdef __USE_DIRECT__
#define IO_TYPE DO_DIRECT_IO
#define USE_WRITE_FUNCTION Example_WriteDirectIO
#endif

#ifdef __USE_BUFFERED__
#define IO_TYPE DO_BUFFERED_IO
#define USE_WRITE_FUNCTION Example_WriteBufferedIO
#endif

#ifndef IO_TYPE
#define IO_TYPE 0
#define USE_WRITE_FUNCTION Example_WriteNeither
#endif

NTSTATUS __unsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS __close(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS __create(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS __ioControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS __read(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS __write(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pUnicodeString);
VOID __unload(PDRIVER_OBJECT pDriverObject);

BOOLEAN isStringTerminated(PCHAR pStr, UINT32 uiStrLen);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, __unsupportedFunction)
#pragma alloc_text(PAGE, __create)
#pragma alloc_text(PAGE, __read)
#pragma alloc_text(PAGE, __write)
#pragma alloc_text(PAGE, __ioControl)
#pragma alloc_text(PAGE, __close)
#pragma alloc_text(PAGE, __unload)
#pragma alloc_text(PAGE, isStringTerminated)

/**
 * Check if the string is terminated (end with a '\0')
 *
 * @param pStr Pointer to the string
 * @param uiStrLen Length of the string
 *
 * @return TRUE if the string is terminated
 */
BOOLEAN isStringTerminated(PCHAR pStr, UINT32 uiStrLen) {
  BOOLEAN bStringIsTerminated = FALSE;
  UINT32 uiIndex = 0;

  while (uiIndex < uiStrLen && bStringIsTerminated == FALSE) {
    if (pStr[uiIndex] == '\0') {
      bStringIsTerminated = TRUE;
    } else {
      uiIndex++;
    }
  }
  return bStringIsTerminated;
}

NTSTATUS __unsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
  DbgPrint("unsupportedFunction() Called.\r\n");
  return NtStatus;
}

NTSTATUS __close(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  DbgPrint("close() Called.\r\n");
  return NtStatus;
}

NTSTATUS __create(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  DbgPrint("create() Called.\r\n");
  return NtStatus;
}

NTSTATUS __ioControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  DbgPrint("ioControl() Called.\r\n");
  return NtStatus;
}

/**
 * Communicating with the user mode ReadFile() function
 *
 * Supports 3 different input method: direct I/O, buffered I/O, neither
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 */
NTSTATUS __read(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
#ifdef __USE_DIRECT__

#else
#ifdef __USE_BUFFERED__
  NTSTATUS NtStatus = STATUS_BUFFER_TOO_SMALL;
  PIO_STACK_LOCATION pIoStackIrp = NULL;
  PCHAR pReturnData = "Hello from the kernel";
  UINT32 dwDataSize = sizeof("Hello from the kernel");
  UINT32 dwDataRead = 0;
  PCHAR pReadDataBuffer;

  DbgPrint("read() Called.\r\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp && pIrp->MdlAddress) {
    pReadDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pReadDataBuffer && pIoStackIrp->Parameters.Read.Length >= dwDataSize) {
      RtlCopyMemory(pReadDataBuffer, pReturnData, dwDataSize);
      dwDataRead = dwDataSize;
      NtStatus = STATUS_SUCCESS;
    }
  }

  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Information = dwDataRead;

  // Priority boost to make the thread waiting for this IRP to complete
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Communicating with the user mode WriteFile() function
 *
 * Supports 3 different input method: direct I/O, buffered I/O, neither
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 */
NTSTATUS __write(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
#ifdef __USE_DIRECT__

#else
#ifdef __USE_BUFFERED__
  NTSTATUS NtStatus = STATUS_SUCCESS;
  PIO_STACK_LOCATION pIoStackIrp = NULL;
  PCHAR pWriteDataBuffer;

  DbgPrint("write() Called. Mode: Buffered I/O \r\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    pWriteDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pWriteDataBuffer) {
      if (isStringTerminated(pWriteDataBuffer,
                             pIoStackIrp->Parameters.Write.Length)) {
        DbgPrint(pWriteDataBuffer);
      }
    }
  }
  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Function used to unload the driver dynamically
 *
 * @param pDriverObject Pointer to the current driver object
 */
VOID __unload(PDRIVER_OBJECT pDriverObject) {
  UNICODE_STRING usDosDeviceName;

  DbgPrint("unload() Called\n");

  // Delete the symbolic link created in DriverEntry()
  RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevice\\ExampleDrv");
  IoDeleteSymbolicLink(&usDosDeviceName);

  // Delete the device
  IoDeleteDevice(pDriverObject->DeviceObject);
}

/**
 * Entry point of the driver
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pUnicodeString String points to the registry location that store the
 * information of the driver
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pRegistryPath) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  UINT32 uiIndex = 0;
  PDEVICE_OBJECT pDeviceObject = NULL;
  UNICODE_STRING usDriverName, usDosDeviceName;

  // Printing to the DebugView window
  DbgPrint("DriverEntry called\n");

  // Init unicode strings (same as strcat/wcscat)
  // Note that UNICODE_STRINGs do not have \0 at the end
  RtlInitUnicodeString(&usDriverName, L"\\Device\\ExampleDrv");
  RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\ExampleDrv");

  // Create the device driver
  NtStatus =
      IoCreateDevice(pDriverObject, 0, &usDriverName, FILE_DEVICE_UNKNOWN,
                     FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
  if (NtStatus != STATUS_SUCCESS) {
    DbgPrint("IoCreateDevice failed. Exitcode 0x%08x\n", NtStatus);
    // return NtStatus;
  }

  if (NtStatus == STATUS_SUCCESS) {
    // Define major functions used to communicate with the user mode application
    for (uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; ++uiIndex) {
      pDriverObject->MajorFunction[uiIndex] = __unsupportedFunction;
    }

    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = __close;
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = __create;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = __ioControl;
    pDriverObject->MajorFunction[IRP_MJ_READ] = __read;
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = __write;

    pDriverObject->DriverUnload = __unload;

    pDeviceObject->Flags |= IO_TYPE;  // type of IO
    pDeviceObject->Flags &=
        (~DO_DEVICE_INITIALIZING);  // remove the flag used to tell the IO
                                    // manager that the device is initialized

    // Create a symbolic link between DOS device name and NT device name
    IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
  }

  return NtStatus;
  // return STATUS_SUCCESS;
}