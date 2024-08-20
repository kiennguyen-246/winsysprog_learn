/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    driver and application

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_HelloWorldUMDF,
    0x12e7572a,0x01bc,0x493b,0x80,0x25,0xfa,0x81,0xc4,0x51,0xc6,0x1a);
// {12e7572a-01bc-493b-8025-fa81c451c61a}
