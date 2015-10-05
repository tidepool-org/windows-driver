/*++
Copyright (c) 2015 -  Intelligraphics, Inc.

Module Name:

    igx_service.h

--*/

#ifndef __IGX_SERVICE_H
#define __IGX_SERVICE_H

typedef struct _DEVICE_INFO
{
   HANDLE       hDevice; // file handle
   HDEVNOTIFY   hHandleNotification;    // notification handle
   TCHAR        DeviceName[MAX_PATH];   // friendly name of device description
   TCHAR        DevicePath[MAX_PATH]; 
   char 		SystemDir[MAX_PATH];   // system dir with backslash appended
   ULONG        SerialNo;               // Serial number of the device.
   HANDLE       hNewDeviceArrivalEvent;
   HWND         hWndList;
} DEVICE_INFO, *PDEVICE_INFO;


typedef enum {

    PLUGIN = 1,
    UNPLUG,
    EJECT

} USER_ACTION_TYPE;

typedef struct _DIALOG_RESULT
{
    ULONG   SerialNo;
    PWCHAR  DeviceId;
} DIALOG_RESULT, *PDIALOG_RESULT;

#define ID_EDIT 1

#define  IDM_OPEN       100
#define  IDM_CLOSE      101
#define  IDM_EXIT       102
#define  IDM_HIDE       103
#define  IDM_PLUGIN     104
#define  IDM_UNPLUG     105
#define  IDM_EJECT      106
#define  IDM_ENABLE     107
#define  IDM_DISABLE    108
#define  IDM_CLEAR      109
#define  IDM_IOCTL      110
#define  IDM_VERBOSE    111
#define  IDM_PAUSE      112
#define  IDM_RESUME     113

#define IDD_DIALOG                     115
#define IDD_DIALOG1                    116
#define IDD_DIALOG2                    117
#define ID_OK                          118
#define ID_CANCEL                      119
#define IDC_SERIALNO                   1000
#define IDC_DEVICEID                   1001
#define IDC_STATIC                      -1

#define IDI_CLASS_ICON                 200

LRESULT FAR PASCAL
WndProc (
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOLEAN EnumExistingDevices(
    HWND   hWnd
    );

BOOL HandleDeviceInterfaceChange(
    HWND hwnd,
    DWORD evtype,
    PDEV_BROADCAST_DEVICEINTERFACE dip
    );

BOOL HandleDeviceChange(
    HWND hwnd,
    DWORD evtype,
    PDEV_BROADCAST_HANDLE dhp
    );

BOOL
GetDeviceDescription(
    __in LPTSTR DevPath,
    __out_bcount_full(OutBufferLen) LPTSTR OutBuffer,
    __in ULONG OutBufferLen,
    __in PULONG SerialNo
    );

#if DBG
VOID
Display(
    __in HWND hWnd,
    __in LPWSTR pstrFormat,  // @parm A printf style format string
    ...                 // @parm | ... | Variable paramters based on <p pstrFormat>
    );


VOID
DisplayV(
    __in HWND hWnd,
    __in LPWSTR pstrFormat,  // @parm A printf style format string
    ...                 // @parm | ... | Variable paramters based on <p pstrFormat>
    );

VOID
SvcDebugOut(
	LPCTSTR String,
	DWORD Status
	); 
#else
#define Display
#define DisplayV
#define SvcDebugOut(A,B)
#endif

#endif


