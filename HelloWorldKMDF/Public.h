/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_HelloWorldKMDF,
    0x732775c2,0xe9d7,0x43d9,0x82,0x3e,0x61,0xc6,0x4f,0x13,0x6a,0x3c);
// {732775c2-e9d7-43d9-823e-61c64f136a3c}
