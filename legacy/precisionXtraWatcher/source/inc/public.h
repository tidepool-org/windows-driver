#ifndef __PUBLIC_H
#define __PUBLIC_H

// GUID definition are required to be outside of header inclusion pragma
// to avoid error during precompiled headers.
//

//
// Define a WMI GUID to represent device arrival notification WMIEvent class.
//

DEFINE_GUID (IGX_NOTIFY_DEVICE_ARRIVAL_EVENT,
	0x1cdaff1, 0xc901, 0x45b4, 0xb3, 0x59, 0xb5, 0x54, 0x27, 0x25, 0xe2, 0x9c);

#if 0 // for IGX filter devices if present

// RJM: For testing with CP2101
DEFINE_GUID(GUID_DEVINTERFACE_SILABUSBSERIALBRIDGE, 
 	0xa2a39220, 0x39f4, 0x4b88, 0xae, 0xcb, 0x3d, 0x86, 0xa3, 0x5d, 0xc7, 0x48);

DEFINE_GUID(GUID_DEVINTERFACE_IGX, 
	0xf5309dc4, 0x1bb3, 0x4e32, 0x82, 0x5e, 0xa8, 0xcc, 0x72, 0xe1, 0x2a, 0xe2);

#define NTDEVICE_NAME_STRING      L"\\Device\\IgxFilter"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\IgxFilter"

#define IGX_FILTER_FILENAME     L"IgxFilter"
#define FILE_DEVICE_INFINEON      FILE_DEVICE_UNKNOWN

#else

#define GUID_DEVINTERFACE_IGX GUID_DEVINTERFACE_COMPORT

#endif

// used by both driver and exe for init
// #define DEFAULT_BAUDRATE		57600
#define DEFAULT_BAUDRATE		115200

#endif
