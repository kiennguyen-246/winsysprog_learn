#include <iostream>
#include <ntddk.h>
#include <wdf.h>

/**
 *  The entry point function for the device,
 *  @param pDriverObject Pointer to the DRIVER_OBJECT (DO) representing this driver
 *  @param pRegistryPath A string pointing to the location in the registry where information for the driver was stored
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    return STATUS_SUCCESS;
}