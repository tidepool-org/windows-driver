/******************************************************************************
 *  Copyright (c) 2015 - Intelligraphics, Inc.
 *  ALL RIGHTS RESERVED
 *
 *****************************************************************************/

/*++

Module Name: igx_service.c

--*/

#define UNICODE
#define _UNICODE
#define INITGUID

#include "StdAfx.h"
#include "igxver.h"
#ifdef DDKBUILDENV// for DDK compile
#include <dontuse.h>
#endif
#include <assert.h>

//
// Global variables
//
HINSTANCE   hInst;
HWND        hWndMain = NULL;
HWND        hWndList;
TCHAR       szTitle[]=TEXT("IGX Service Application");
TCHAR       OutText[500];
UINT        ListBoxIndex = 0;
GUID        InterfaceGuid; // GUID_DEVINTERFACE_IGX;
UINT        NumDevicesOpened=0;

#define CLOSE_TIMEOUT 20 // ms

PDEVICE_INFO       deviceInfo = NULL;

// SHOW EVERYTHING
#ifdef SHOW_WINDOW
BOOLEAN     Verbose= TRUE; // verbose prints on in debug mode
#else //NDEBUG
BOOLEAN     Verbose= FALSE; // verbose prints off in RELEASE mode
#endif

#ifdef IGX_SERVICE_APPLICATION
SERVICE_STATUS          IgxServiceStatus; 
SERVICE_STATUS_HANDLE   IgxServiceStatusHandle; 

VOID  WINAPI IgxServiceCtrlHandler (DWORD opcode); 
VOID  WINAPI IgxServiceStart (DWORD argc, LPTSTR *argv); 
DWORD IgxServiceInitialization (DWORD argc, LPTSTR *argv, DWORD *specificError); 
#endif // IGX_SERVICE_APPLICATION


#if DBG
//*****************************************************************************
//
//  Function: Display
// 
//  Description: Nonverbose display routine.
//
//*****************************************************************************

VOID
Display(
    __in HWND hWnd,
    __in LPWSTR pstrFormat,  // @parm A printf style format string
    ...                 // @parm | ... | Variable paramters based on <p pstrFormat>
    )
{
    HRESULT hr;
    va_list va;

    va_start(va, pstrFormat);

    //
    // Truncation is acceptable.
    //
    hr =
		StringCbVPrintf(
			OutText,
			sizeof(OutText)-sizeof(WCHAR),
			pstrFormat,
			va);

    va_end(va);

    if(FAILED(hr))
    {
        return;
    }

#ifdef IGX_SERVICE_APPLICATION
	OutputDebugString((LPCWSTR)OutText); 
	OutputDebugString((LPCWSTR)TEXT("\n")); 
#else
    SendMessage(hWnd, LB_INSERTSTRING, ListBoxIndex, (LPARAM)OutText);
    SendMessage(hWnd, LB_SETCURSEL, ListBoxIndex, 0);
    ListBoxIndex++;
#endif
}


//*****************************************************************************
//
//  Function: DisplayV
// 
//  Description: Verbose message display routine.
//
//*****************************************************************************

VOID
DisplayV(
    __in HWND hWnd,
    __in LPWSTR pstrFormat,  // @parm A printf style format string
    ...                 // @parm | ... | Variable paramters based on <p pstrFormat>
    )
{
    HRESULT hr;
    va_list va;

	if (Verbose)
    {
		va_start(va, pstrFormat);

		//
		// Truncation is acceptable.
		//
		hr =
			StringCbVPrintf(
				OutText,
				sizeof(OutText)-sizeof(WCHAR),
				pstrFormat,
				va);

		va_end(va);

		if(FAILED(hr))
		{
			return;
		}

#ifdef IGX_SERVICE_APPLICATION
		OutputDebugString((LPCWSTR)OutText); 
		OutputDebugString((LPCWSTR)TEXT("\n")); 
#else
		SendMessage(hWnd, LB_INSERTSTRING, ListBoxIndex, (LPARAM)OutText);
		SendMessage(hWnd, LB_SETCURSEL, ListBoxIndex, 0);
		ListBoxIndex++;
#endif
	}
}


VOID SvcDebugOut(LPCTSTR String, DWORD Status) 
{
	TCHAR  Buffer[1024]; 
	HRESULT hr =
		StringCbPrintf( (LPTSTR)Buffer, 1024*sizeof(TCHAR), String, Status);

	OutputDebugString((LPCWSTR)Buffer); 
}
#endif


void EnablePnPNotification(HWND hWnd)
{
	DEV_BROADCAST_DEVICEINTERFACE filter;
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = InterfaceGuid;

	Display(hWndList, TEXT("REGISTER for device notification"));

	deviceInfo->hHandleNotification =
		RegisterDeviceNotification(hWnd, &filter, 0);

	if (!deviceInfo->hHandleNotification)
	{
		Display(hWndList, TEXT("Failed to register for device notification"));
	}
}

void DisablePnPNotification()
{
	Display(hWndList, TEXT("UNREGISTER for device notification"));

	if (deviceInfo->hHandleNotification)
	{
		UnregisterDeviceNotification(deviceInfo->hHandleNotification);
		deviceInfo->hHandleNotification = NULL;
	}
	else
	{
		Display(hWndList, TEXT("ERROR: no dev notificaiton handle"));
	}
}

BOOL OpenDevice(BOOL init)
{
    HRESULT hr;

#ifdef INIT_COM_FROM_SERVICE
	if (init)
	{
		TCHAR portNumChar;
		TCHAR portBaseChar = (TCHAR)'0';
		unsigned portNum;
		TCHAR tmpbus[4];
		TCHAR tmpdash[2];
		TCHAR manuName[7];
		TCHAR *testString;
		TCHAR delim[3] = TEXT("()");
		TCHAR *tmpstring, *portName = NULL;

		testString = deviceInfo->DeviceName;

		DisplayV(
			hWndList,
			TEXT("Checking PORT: %ws"),
			testString);

		swscanf(testString, TEXT("%4s %2s %7s"), tmpbus, tmpdash, manuName);
		if (wcscmp(manuName, TEXT("Abbott")) != 0)
			return FALSE;

		// parse for second string which will be "COMxx"
		tmpstring = wcstok(testString, delim);
		if (tmpstring)
			portName = wcstok(NULL, delim);

		if (portName == NULL)
		{
			DisplayV(hWndList, TEXT("Error: COM port string not found"));
			return FALSE;
		}

		if (wcslen(portName) < 4)
		{
			DisplayV(hWndList, TEXT("Error: COM port string not long enough"));
			return FALSE;
		}

		DisplayV(
			hWndList,
			TEXT("Found Abbott device on: %ws"),
			portName
			);

		// portNumChar = deviceInfo->DeviceName[42];
		// this would only support 1-9
		
		portNumChar = portName[3];

#if 0
		DisplayV(
			hWndList,
			TEXT("Found Abbott device on COM PORT: %wc"),
			portNumChar
			);
#endif

		portNum = (unsigned)portNumChar - (unsigned)(portBaseChar);

#if 0
		DisplayV(
			hWndList,
			TEXT("ComInitIdx: %d"),
			portNum);
#endif

		ComInitIdx(portNum, DEFAULT_BAUDRATE);
        ++NumDevicesOpened;
	}
#endif

	return TRUE;
}


void CloseDevices()
{
	// nothing to do in this version where we open/fix/close
}


BOOL CreateEvents()
{
    HRESULT hr;

	assert(deviceInfo->hNewDeviceArrivalEvent == NULL);
	deviceInfo->hNewDeviceArrivalEvent =
		 CreateEvent( 
                NULL,   // default security attributes
                FALSE,  // auto-reset event object
                FALSE,  // initial state is nonsignaled
                NULL);  // unnamed object
	if (deviceInfo->hNewDeviceArrivalEvent == NULL) 
   	{ 
   		DisplayV(deviceInfo->hWndList,
			TEXT("hNewDeviceArrivalEvent error: %d"), GetLastError() ); 
		return FALSE;
	}

	return TRUE;
}

BOOL Cleanup()
{
	DisablePnPNotification();
	CloseDevices();

    return TRUE;
}

DWORD
CreateHandler (HWND hWnd)
{
	// DisplayV(hWndList, TEXT("CreateHandler")); 
	DisplayV(hWndList, TEXT("Abbott Serial Fix Test App\n")); 

   	deviceInfo = (PDEVICE_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DEVICE_INFO));

   	if (deviceInfo == NULL)
   	{
   		DWORD dwError = GetLastError();
   		return dwError;
   	}

	deviceInfo->hDevice = INVALID_HANDLE_VALUE;
	deviceInfo->hWndList = hWndList;
		
   	deviceInfo->hNewDeviceArrivalEvent = NULL;

	if (!CreateEvents())
	{
   		DWORD dwError = GetLastError();
   		DisplayV(deviceInfo->hWndList,
			 TEXT("Create events error: %d"), dwError); 
   		return dwError;
	}
		
	EnablePnPNotification(hWnd);

	// need to do this at start just in case driver is already started
	EnumExistingDevices(hWnd);

   	return 0;
}

void
WaitForAllHandlesClosed ()
{
	BOOL bError = FALSE;

   	if(NULL != deviceInfo->hHandleNotification)
   		bError = TRUE;
   	if(INVALID_HANDLE_VALUE != deviceInfo->hDevice)
   		bError = TRUE;

	if (bError)
		Sleep(CLOSE_TIMEOUT);
}

void CloseHandler()
{
	long dummyPreviousCount = 0;

	DisplayV(hWndList, TEXT("\nCloseHandler")); 

   	// Verbose = FALSE;
	
	Cleanup();

	deviceInfo->hWndList = (HWND)INVALID_HANDLE_VALUE;

	HeapFree (GetProcessHeap(), 0, deviceInfo);
	deviceInfo = (PDEVICE_INFO)NULL;
}


#ifdef IGX_SERVICE_APPLICATION // (RELEASE BUILD) service application
//*****************************************************************************
//
//  Function: IgxServiceCtrlHandler
// 
//  Description: Control handler for service application.
//
//*****************************************************************************

VOID WINAPI IgxServiceCtrlHandler (DWORD Opcode) 
{ 
	DWORD status; 

	switch(Opcode) 
	{ 
		case SERVICE_CONTROL_PAUSE: 
			// Do whatever it takes to pause here. 
			Display(hWndList,TEXT("IgxServiceCtrlHandler: PAUSE")); 

			DisablePnPNotification();
			CloseDevices();

			// give time for all calls to return
   			Sleep(100);

			IgxServiceStatus.dwCurrentState = SERVICE_PAUSED; 
			break; 

		case SERVICE_CONTROL_CONTINUE: 
			// Do whatever it takes to continue here.
			Display(hWndList,TEXT("IgxServiceCtrlHandler: RESUME")); 

			// DON'T REOPEN device here - let PNP notification callback
			// handle this or we will have all kinds of race conditions
			EnablePnPNotification(hWndMain);
			
            IgxServiceStatus.dwCurrentState = SERVICE_RUNNING;
			break; 

		case SERVICE_CONTROL_STOP: 
			// Do whatever it takes to stop here. 
			Display(hWndList, TEXT("Stopping Igx Service... "));

			// IgxServiceStatus.dwWin32ExitCode = 0;

            // Multithread Example Stop
			IgxServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
			// IgxServiceStatus.dwCheckPoint    = 1; 
			// IgxServiceStatus.dwWaitHint      = CLOSE_TIMEOUT + 10;

			if (!SetServiceStatus (
				IgxServiceStatusHandle, &IgxServiceStatus))
			{ 
				status = GetLastError(); 
				Display(
					hWndList,
					TEXT("IgxServiceCtrlHandler: stop error %ld"),
					status); 
			}

			CloseHandler();

			IgxServiceStatus.dwCurrentState = SERVICE_STOPPED; 

			// Send current status. 
			if (!SetServiceStatus (
					IgxServiceStatusHandle,
					&IgxServiceStatus)) 
			{ 
				status = GetLastError(); 
				Display(
					hWndList,
					TEXT("IgxServiceCtrlHandler: stop error %ld"),
					status); 
			} 

			Display(hWndList, TEXT("Igx Service STOPPED"));
			return; 

		case SERVICE_CONTROL_INTERROGATE: 
			// Fall through to send current status. 
			Display(hWndList, TEXT("Interrogate Igx Service"));
			break; 

		default: 
			Display(hWndList, TEXT("Unkonwn Service Control Code"));
			break;
	} 

	// Send current status. 
	if (!SetServiceStatus (
		IgxServiceStatusHandle,  &IgxServiceStatus)) 
	{ 
		status = GetLastError(); 
		Display(
			hWndList,
			TEXT("SetServiceStatus error %ld"), status); 
	} 

	return; 
}

//*****************************************************************************
//
//  Function: IgxServiceInitialization
// 
//  Description: Initialization routine for service application.
//
//*****************************************************************************

DWORD IgxServiceInitialization(DWORD   argc, LPTSTR  *argv, DWORD *specificError) 
{
    *specificError = 0;

    return(0); 
}


//*****************************************************************************
//
//  Function: IgxServiceStart
// 
//  Description: Main service routine
//  commonly refered to as ServiceMain in SDK documentation.
//
//*****************************************************************************

void WINAPI IgxServiceStart (DWORD argc, LPTSTR *argv) 
{ 
    DWORD status; 
    DWORD specificError;
    HMODULE hInstance = (HMODULE)INVALID_HANDLE_VALUE;

	Display(hWndList, TEXT("Starting Igx Service... "));

	// RJM:HERE: enable this if you need to debug the service application
    // DebugBreak();

	// RJM:HERE: enable to print out lots of info but fail with UART COMM
   	// Verbose = TRUE;

    IgxServiceStatus.dwServiceType        = SERVICE_WIN32; 
    IgxServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    IgxServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    IgxServiceStatus.dwWin32ExitCode      = 0; 
    IgxServiceStatus.dwServiceSpecificExitCode = 0; 
    IgxServiceStatus.dwCheckPoint         = 0; 
    IgxServiceStatus.dwWaitHint           = 0; 
 
    IgxServiceStatusHandle = RegisterServiceCtrlHandler( 
        TEXT("Igx Service"), 
        IgxServiceCtrlHandler); 
 
    if (IgxServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
		SvcDebugOut(TEXT("IgxServiceStart: RegisterServiceCtrlHandler failed %d\n"), GetLastError()); 
        return; 
    } 
 
    // Initialization code goes here. 
    status = IgxServiceInitialization(argc, argv, &specificError); 
 
    // Handle error condition 
    if (status != NO_ERROR) 
    { 
        IgxServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
        IgxServiceStatus.dwCheckPoint         = 0; 
        IgxServiceStatus.dwWaitHint           = 0; 
        IgxServiceStatus.dwWin32ExitCode      = status; 
        IgxServiceStatus.dwServiceSpecificExitCode = specificError; 
 
        SetServiceStatus (IgxServiceStatusHandle, &IgxServiceStatus); 
        return; 
    } 
 
    // Initialization complete - report running status. 
    IgxServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    IgxServiceStatus.dwCheckPoint         = 0; 
    IgxServiceStatus.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (IgxServiceStatusHandle, &IgxServiceStatus)) 
    { 
        status = GetLastError(); 
        SvcDebugOut(TEXT("IgxServiceStart:SetServiceStatus error %ld\n"),status);
        return;
    } 
 
	Display(hWndList, TEXT("Igx Service RUNNING"));

	hInstance = GetModuleHandle( NULL );
	
// SHOW EVERYTHING
#ifdef SHOW_WINDOW
	status = WinMain ( hInstance, NULL /*hPrevInstance*/, NULL /*lpCmdLine*/, SW_SHOW);
#else //NDEBUG
	status = WinMain ( hInstance, NULL /*hPrevInstance*/, NULL /*lpCmdLine*/, SW_HIDE);
	EnumExistingDevices(hWndList);
#endif


    return; 
} 
 
//*****************************************************************************
//
//  Function: main
// 
//  Description: entry point for service application
//
//*****************************************************************************

int __cdecl main(int argc, char* argv[])
{ 
	SERVICE_TABLE_ENTRY		DispatchTable[] =
	{
		{TEXT("Igx Service"), (LPSERVICE_MAIN_FUNCTION)IgxServiceStart},
		{NULL, NULL}
	};

	if ( !StartServiceCtrlDispatcher(DispatchTable) ) 
	{
		DWORD dwError = GetLastError();
		SvcDebugOut(TEXT("main:StartServiceCtrlDispatcher failed 0x%08x\n"), dwError);
        return dwError;
	}
    return 0;
} 

#endif


//*****************************************************************************
//
//  Function: WinMain
// 
//  Description: entry point for windows application
//
//*****************************************************************************

int PASCAL
WinMain (
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in_opt LPSTR lpCmdLine,
    __in int nShowCmd
    )
{
    static    TCHAR szAppName[]=TEXT("Igx Service");
    HWND      hWnd;
    MSG       msg;
    WNDCLASS  wndclass;

    InterfaceGuid = GUID_DEVINTERFACE_IGX;

    hInst=hInstance;

    if (!hPrevInstance)
    {
         wndclass.style        =  CS_HREDRAW | CS_VREDRAW;
         wndclass.lpfnWndProc  =  WndProc;
         wndclass.cbClsExtra   =  0;
         wndclass.cbWndExtra   =  0;
         wndclass.hInstance    =  hInstance;
         wndclass.hIcon        =  LoadIcon (NULL, IDI_APPLICATION);
         wndclass.hCursor      =  LoadCursor(NULL, IDC_ARROW);
         wndclass.hbrBackground=  (HBRUSH)GetStockObject(WHITE_BRUSH);
         wndclass.lpszMenuName =  TEXT("GenericMenu");
         wndclass.lpszClassName=  szAppName;

         RegisterClass(&wndclass);
    }

    hWndMain = hWnd = CreateWindow (szAppName,
                         szTitle,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

	if (!hWndMain)
	{
		DWORD dwError = GetLastError();
        return dwError;
	}

	// Could use AnimateWindow() for special effects.
    ShowWindow (hWnd, nShowCmd);

	UpdateWindow(hWnd);

    while (GetMessage (&msg, NULL, 0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (0);
}

//*****************************************************************************
//
//  Function: WndProc
// 
//  Description: Windows message processing.
//
//*****************************************************************************

LRESULT
FAR PASCAL
WndProc (
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD nEventType = (DWORD)wParam;
    PDEV_BROADCAST_HDR p = (PDEV_BROADCAST_HDR) lParam;

    switch (message)
    {
        case WM_COMMAND:
            // HandleCommands(hWnd, message, wParam, lParam);
            return 0;

        case WM_CREATE:
        {
		    //
            // Load and set the icon of the program
            //
            // LoadImage has superceded LoadIcon
            if (!SetClassLongPtr(
				hWnd,
				(int)GCLP_HICON,
				(LONG_PTR) LoadImage(
					(HINSTANCE) lParam,
					(LPCTSTR) MAKEINTRESOURCE(IDI_CLASS_ICON),
					(UINT) IMAGE_ICON,
					(int) 0,
					(int) 0,
					(UINT) 0 ) ))
            {
                DWORD dwError = GetLastError();
                return dwError;
            }

            hWndList = CreateWindow (TEXT("listbox"),
                         NULL,
                         WS_CHILD|WS_VISIBLE|LBS_NOTIFY |
                         WS_VSCROLL | WS_BORDER,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         hWnd,
                         (HMENU)ID_EDIT,
                         hInst,
                         NULL);

		    // initialize check on verbose accordingly
		    if(Verbose)
		    {
			    CheckMenuItem(GetMenu(hWnd), (UINT)IDM_VERBOSE, MF_CHECKED);
		    }
		    else
		    {
			    CheckMenuItem(GetMenu(hWnd), (UINT)IDM_VERBOSE, MF_UNCHECKED);
		    }

			return CreateHandler(hWnd);


        case WM_SIZE:
            MoveWindow(hWndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            return 0;

        case WM_SETFOCUS:
            SetFocus(hWndList);
            return 0;

        case WM_DEVICECHANGE:
            //
            // The DBT_DEVNODES_CHANGED broadcast message is sent
            // everytime a device is added or removed. This message
            // is typically handled by Device Manager kind of apps,
            // which uses it to refresh window whenever something changes.
            // The lParam is always NULL in this case.
            //
            if(DBT_DEVNODES_CHANGED == wParam)
            {
                // DisplayV(hWndList, TEXT("Received DBT_DEVNODES_CHANGED broadcast message"));
                return 0;
            }

#if 0 // RJM:HERE: this may be too strict
            //
            // All the events we're interested in come with lParam pointing to
            // a structure headed by a DEV_BROADCAST_HDR.  This is denoted by
            // bit 15 of wParam being set, and bit 14 being clear.
            //
            if((wParam & 0xC000) != 0x8000)
				return 0;
#endif

			if (!p)
			{
				return 0;
			}

			if (p->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				HandleDeviceInterfaceChange(
					hWnd, nEventType, (PDEV_BROADCAST_DEVICEINTERFACE) p);
			}
			else if (p->dbch_devicetype == DBT_DEVTYP_HANDLE)
			{
				HandleDeviceChange(
					hWnd, nEventType, (PDEV_BROADCAST_HANDLE) p);
			}
            return 0;

        case WM_POWERBROADCAST:
            // HandlePowerBroadcast(hWnd, wParam, lParam);
            return 0;

        case WM_CLOSE:
			CloseHandler();
            return DefWindowProc(hWnd,message, wParam, lParam);
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd,message, wParam, lParam);
}


//*****************************************************************************
//
//  Function: HandleCommands
// 
//  Description: Routine to handle application menu commands.
//
//*****************************************************************************

LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM   lParam
    )

{
    PDIALOG_RESULT result = NULL;

    switch (wParam)
	{
        case IDM_OPEN:
			// IDM_OPEN is currently used to open handles to the filter drivers.
			// Another use could be simulating when the application opens a serial com port.
            EnumExistingDevices(hWnd);
            break;

        case IDM_CLOSE:
			// IDM_CLOSE is currently used to close handles to the filter drivers.
			// Another use could be simulating when the application closes a serial com port.
            Cleanup();
            break;

        case IDM_PAUSE:
			DisablePnPNotification();
			CloseDevices();
            break;

        case IDM_RESUME:
			EnablePnPNotification(hWndMain);
            break;

        case IDM_HIDE:
			// IDM_HIDE (if implemented) should send an IOCTL to the driver that will
			// cause the IRP_MN_QUERY_PNP_DEVICE_STATE command to hide the hardware
			// instance from displaying in the Device Manager.
			MessageBox(hWnd, TEXT("Hide (for future debugging) is not implemented"), TEXT("Info"), MB_OK);
			break;

		case IDM_PLUGIN:
			MessageBox(hWnd, TEXT("PLUGIN (for future debugging) is not implemented"), TEXT("Info"), MB_OK);
			break;

        case IDM_UNPLUG:
			MessageBox(hWnd, TEXT("UNPLUG (for future debugging) is not implemented"), TEXT("Info"), MB_OK);
			break;

        case IDM_EJECT:
 			MessageBox(hWnd, TEXT("EJECT (for future debugging) is not implemented"), TEXT("Info"), MB_OK);
			break;

        case IDM_CLEAR:
            SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
            ListBoxIndex = 0;
            break;

        case IDM_IOCTL:
 			MessageBox(hWnd, TEXT("IOCTL (for future driver debugging) is not implemented"), TEXT("Info"), MB_OK);
			break;

        case IDM_VERBOSE:
            {
                HMENU hMenu = GetMenu(hWnd);
                Verbose = !Verbose;
                if(Verbose)
                {
                    CheckMenuItem(hMenu, (UINT)wParam, MF_CHECKED);
                }
                else
                {
                    CheckMenuItem(hMenu, (UINT)wParam, MF_UNCHECKED);
                }
            }
            break;

        case IDM_EXIT:
            PostQuitMessage(0);
            break;

        default:
            break;
    }

    if(result)
    {
        HeapFree (GetProcessHeap(), 0, result);
    }
    return TRUE;
}

//*****************************************************************************
//
//  Function: HandleDeviceInterfaceChange
// 
//  Description: Routine to handle the arrival or complete removal of a device.
//
//*****************************************************************************

BOOL
HandleDeviceInterfaceChange(
    HWND hWnd,
    DWORD evtype,
    PDEV_BROADCAST_DEVICEINTERFACE dip
    )
{
    HRESULT                 hr;

    switch (evtype)
    {
        case DBT_DEVICEARRIVAL:
			//
			// New device arrived. Open handle to the device
			// and register notification of type DBT_DEVTYP_HANDLE
			//

			if(!GetDeviceDescription(dip->dbcc_name,
									 deviceInfo->DeviceName,
									 sizeof(deviceInfo->DeviceName),
									 &deviceInfo->SerialNo))
			{
				DisplayV(hWndList,
					TEXT("Failed to get device name: %ws"),
					deviceInfo->DeviceName);

				break;
			}

			DisplayV(hWndList,
				 TEXT("New device (Interface Change Notification): %ws"),
				 deviceInfo->DeviceName);

			OpenDevice(TRUE);

            break;

        case DBT_DEVICEREMOVECOMPLETE:
			DisplayV(hWndList, TEXT("Remove Complete (Interface Change Notification)"));
			CloseDevices();
		    break;

            //
            // Device Removed.
            //

        default:
			DisplayV(hWndList, TEXT("Unknown (Interface Change Notification)"));
			break;
    }
    return TRUE;
}


//*****************************************************************************
//
//  Function: HandleDeviceChange
// 
//  Description: Routine to handle the various removal stages of a device.
//
//*****************************************************************************

BOOL
HandleDeviceChange(
    HWND hWnd,
    DWORD evtype,
    PDEV_BROADCAST_HANDLE dhp
    )
{
    // DEV_BROADCAST_HANDLE    filter;

    if(!deviceInfo)
    {
        DisplayV(hWndList, TEXT("Error: spurious message, Event Type %x, Device Type %x"),
                                evtype, dhp->dbch_devicetype);
        return FALSE;
    }

    switch (evtype)
    {

    case DBT_DEVICEQUERYREMOVE:

        DisplayV(hWndList, TEXT("Query Remove (Handle Notification)"), deviceInfo->DeviceName);

        // User is trying to disable, uninstall, or eject our device.
        // Close the handle to the device so that the target device can
        // get removed. Do not unregister the notification
        // at this point, because we want to know whether
        // the device is successfully removed or not.
        //
        break;

    case DBT_DEVICEREMOVECOMPLETE:

        DisplayV(hWndList, TEXT("Remove Complete (Handle Notification):%ws"),
                    deviceInfo->DeviceName);
        //
        // Device is getting surprise removed. So close
        // the handle to device and unregister the PNP notification.
        //
        break;

    case DBT_DEVICEREMOVEPENDING:

        DisplayV(hWndList, TEXT("Remove Pending (Handle Notification):%ws"),
                                        deviceInfo->DeviceName);
        //
        // Device is successfully removed so unregister the notification
        // and free the memory.
        //
        break;

    case DBT_DEVICEQUERYREMOVEFAILED :
        DisplayV(hWndList, TEXT("Remove failed (Handle Notification):%ws"),
                                    deviceInfo->DeviceName);
        //
        // Remove failed. So reopen the device and register for
        // notification on the new handle. But first we should unregister
        // the previous notification.
        //
        break;

    default:
        DisplayV(hWndList, TEXT("Unknown (Handle Notification)"), deviceInfo->DeviceName);
        break;

    }
    return TRUE;
}


//*****************************************************************************
//
//  Function: EnumExistingDevices
// 
//  Description: Routine to enumerate existing devices.
//
//*****************************************************************************

BOOLEAN
EnumExistingDevices(
    HWND   hWnd
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
    DWORD                               error;
    HRESULT                             hr;
    DWORD i;

    hardwareDeviceInfo = SetupDiGetClassDevs (
                       (LPGUID)&InterfaceGuid,
                       NULL, // Define no enumerator (global)
                       NULL, // Define no
                       (DIGCF_PRESENT | // Only Devices present
                       DIGCF_DEVICEINTERFACE)); // Function class devices.

    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }

    //
    // Enumerate devices
    //
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    for(i=0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                 0, // No care about specific PDOs
                                 (LPGUID)&InterfaceGuid,
                                 i, //
                                 &deviceInterfaceData); i++ )
    {
        //
        // Allocate a function class device data structure to
        // receive the information about this particular device.
        //

        //
        // First find out required length of the buffer
        //
        if(deviceInterfaceDetailData)
		{
			HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);
		}

        if(!SetupDiGetDeviceInterfaceDetail (
                hardwareDeviceInfo,
                &deviceInterfaceData,
                NULL, // probing so no output buffer yet
                0, // probing so output buffer length of zero
                &requiredLength,
                NULL) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Error;
        }

        predictedLength = requiredLength;

        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
			HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, predictedLength);

        if (deviceInterfaceDetailData == NULL)
        {
            goto Error;
        }

        deviceInterfaceDetailData->cbSize =
			sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (! SetupDiGetDeviceInterfaceDetail (
                   hardwareDeviceInfo,
                   &deviceInterfaceData,
                   deviceInterfaceDetailData,
                   predictedLength,
                   &requiredLength,
                   NULL))
		{
            goto Error;
        }

        //
        // Get the device details such as friendly name and SerialNo
        //
        if(!GetDeviceDescription(deviceInterfaceDetailData->DevicePath,
                                 deviceInfo->DeviceName,
                                 sizeof(deviceInfo->DeviceName),
                                 &deviceInfo->SerialNo))
        {
            goto Error;
        }

        DisplayV(hWndList,
			TEXT("Found device %ws"), deviceInfo->DeviceName );

		OpenDevice(TRUE);
    }

    if(deviceInterfaceDetailData)
    {
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);
    }

    DisplayV(hWndList, TEXT("Service opened %d devices"), NumDevicesOpened);

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return 0;

Error:

    error = GetLastError();
    MessageBox(hWnd, TEXT("EnumExisting Devices failed"), TEXT("Error!"), MB_OK);
    if(deviceInterfaceDetailData)
    {
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);
    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    Cleanup();
    return 0;
}


//*****************************************************************************
//
//  Function: GetDeviceDescription
// 
//  Description: Setup routine that gets the necessary device info.
//
//*****************************************************************************

BOOL
GetDeviceDescription(
    __in LPTSTR DevPath,
    __out_bcount_full(OutBufferLen) LPTSTR OutBuffer,
    __in ULONG OutBufferLen,
    __in PULONG SerialNo
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    SP_DEVINFO_DATA                     deviceInfoData;
    DWORD                               dwRegType, error;

    hardwareDeviceInfo = SetupDiCreateDeviceInfoList(NULL, NULL);

    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }

    //
    // Enumerate devices
    //
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    SetupDiOpenDeviceInterface (hardwareDeviceInfo, DevPath,
                                 0, //
                                 &deviceInterfaceData);

    deviceInfoData.cbSize = sizeof(deviceInfoData);
    if(!SetupDiGetDeviceInterfaceDetail (
            hardwareDeviceInfo,
            &deviceInterfaceData,
            NULL, // probing so no output buffer yet
            0, // probing so output buffer length of zero
            NULL,
            &deviceInfoData) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        goto Error;
    }
    //
    // Get the friendly name for this instance, if that fails
    // try to get the device description.
    //

    if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     OutBufferLen,
                                     NULL))
    {
        if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_DEVICEDESC,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     OutBufferLen,
                                     NULL))
        {
            goto Error;
        }
    }

    //
    // Get the serial number of the device. The bus driver reports
    // the device serial number as UINumber in the devcaps.
    //
    if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo,
                 &deviceInfoData,
                 SPDRP_UI_NUMBER,
                 &dwRegType,
                 (BYTE*) SerialNo,
                 sizeof(PULONG),
                 NULL))
	{
		// RJM: this isn't working
        // DisplayV(hWndList, TEXT("SerialNo is not available for device: %ws"), OutBuffer );
    }


    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return TRUE;

Error:

    error = GetLastError();
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return FALSE;
}

