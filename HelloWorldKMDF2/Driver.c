#include <ntddk.h>
#include <wdf.h>

//#define __USE_DIRECT__
#define __USE_BUFFERED__

#ifdef __USE_DIRECT__
#define IO_TYPE DO_DIRECT_IO
#define USE_WRITE_FUNCTION  Example_WriteDirectIO
#endif

#ifdef __USE_BUFFERED__
#define IO_TYPE DO_BUFFERED_IO
#define USE_WRITE_FUNCTION  Example_WriteBufferedIO
#endif

#ifndef IO_TYPE
#define IO_TYPE 0
#define USE_WRITE_FUNCTION  Example_WriteNeither
#endif

/**
* Check if the string is terminated (end with a '\0')
* 
* @param pStr Pointer to the string
* @param uiStrLen Length of the string
* 
* @return TRUE if the string is terminated
*/
BOOLEAN isStringTerminated(PCHAR pStr, unsigned uiStrLen) {
	BOOLEAN bStringIsTerminated = FALSE;
	unsigned uiIndex = 0;

	while (uiIndex < uiStrLen && bStringIsTerminated == FALSE)
	{
		if (pStr[uiIndex] == '\0')
		{
			bStringIsTerminated = TRUE;
		}
		else
		{
			uiIndex++;
		}
	}
	return bStringIsTerminated;
}

NTSTATUS unsupportedFunction(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("unsupportedFunction() Called.\r\n");
	return NtStatus;
}

NTSTATUS close(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("close() Called.\r\n");
	return NtStatus;
}

NTSTATUS create(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("create() Called.\r\n");
	return NtStatus;
}

NTSTATUS ioControl(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("ioControl() Called.\r\n");
	return NtStatus;
}

NTSTATUS read(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	DbgPrint("read() Called.\r\n");
	return NtStatus;
}

/**
* Communicating with the user mode CreateFile() function
*
* Supports 3 different input method: direct I/O, buffered I/O, neither
*
* @param pDriverObject Pointer to the current driver object
* @param pIrp Pointer to the IRP structure
*/
NTSTATUS write(PDRIVER_OBJECT pDriverObject, PIRP pIrp) {
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
			if (isStringTerminated(pWriteDataBuffer, pIoStackIrp->Parameters.Write.Length)) {
				DbgPrint(pWriteDataBuffer);
			}
		}
	}
	return NtStatus;
#else

#endif //__USE_BUFFERED__
#endif //__USE_DIRECT__
}

/**
* Function used to unload the driver dynamically
*
* @param pDriverObject Pointer to the current driver object
*/
NTSTATUS unload(PDRIVER_OBJECT pDriverObject) {
	UNICODE_STRING usDosDeviceName;

	DbgPrint("unload() Called\n");

	//Delete the symbolic link created in DriverEntry()
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevice\\Example");
	IoDeleteSymbolicLink(&usDosDeviceName);

	//Delete the device
	IoDeleteDevice(pDriverObject->DeviceObject);
}

/**
* Entry point of the driver
*
* @param pDriverObject Pointer to the current driver object
* @param pUnicodeString String points to the registry location that store the information of the driver
*/
NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT pDriverObject,
	_In_ PUNICODE_STRING pUnicodeString
) {
	NTSTATUS NtStatus = STATUS_SUCCESS;
	unsigned uiIndex = 0;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING usDriverName, usDosDeviceName;

	//Printing to the debug window
	DbgPrint("DriverEntry called\n");

	//Init unicode strings (same as strcat/wcscat)
	//Note that UNICODE_STRINGs do not have \0 at the end
	RtlInitUnicodeString(&usDriverName, L"\\Device\\Example");
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevice\\Example");

	//Create the device driver
	NtStatus = IoCreateDevice(
		pDriverObject,
		0,
		&usDriverName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pDeviceObject
	);

	//Define major functions used to communicate with the user mode application
	for (uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; ++uiIndex) {
		pDriverObject->MajorFunction[uiIndex] = unsupportedFunction;
	}

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = close;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = create;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ioControl;
	pDriverObject->MajorFunction[IRP_MJ_READ] = read;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = write;

	pDriverObject->DriverUnload = unload;

	pDeviceObject->Flags |= IO_TYPE;	//type of IO
	pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);	//remove the flag used to tell the IO manager that the device is initialized

	//Create a symbolic link between DOS device name and NT device name
	IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
}