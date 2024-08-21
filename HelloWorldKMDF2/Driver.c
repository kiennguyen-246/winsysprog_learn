#include <winioctl.h>
#include <ntddk.h>
#include <wdf.h>

#define IO_TYPE 0

NTSTATUS exampleUnsupportedFunction(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleClose(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleCreate(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleIoControl(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleRead(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleWrite(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

NTSTATUS exampleUnload(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

}

/**
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
		pDriverObject->MajorFunction[uiIndex] = exampleUnsupportedFunction;
	}

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = exampleClose;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = exampleCreate;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = exampleIoControl;
	pDriverObject->MajorFunction[IRP_MJ_READ] = exampleRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = exampleWrite;

	pDriverObject->DriverUnload = exampleUnload;

	pDeviceObject->Flags |= IO_TYPE;	//type of IO
	pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);	//remove the flag used to tell the IO manager that the device is initialized

	//Create a symbolic link between DOS device name and NT device name
	IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
}