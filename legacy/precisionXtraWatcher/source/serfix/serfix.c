#include <stdio.h>
#include <windows.h>  /* for Windows serial communication facilities */
#include "serfix.h"

/* static serial port ...............*/
static HANDLE locPort = INVALID_HANDLE_VALUE;   /* serial port */


/*******************************************************************************
 * Open the serial port for communication 
 ******************************************************************************/ 
static char const *serialOpen(char const *port, char const *settings)
{

	DCB dcb = {0};

    locPort = CreateFile(port,
                         GENERIC_READ | GENERIC_WRITE,
                         0,  /* exclusive access */
                         NULL,  /* no security attrs */
                         OPEN_EXISTING,
                         0,  /* standard (not-overlapped) I/O */
                         NULL);

    if (locPort == INVALID_HANDLE_VALUE)
    {
        return "Error opening the port.";
    }

    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(locPort, &dcb))
    {
        return "Error retreiving port settings.";
    }

    /* Fill in the DCB */
    if (!BuildCommDCB(settings, &dcb))
    {
        return "Error parsing command line parameters.";
    }

	// dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	// dcb.fOutxCtsFlow = 1;


    if (!SetCommState(locPort, &dcb)) 
    {
        return "Error setting up the port.";
    }

	
    return 0;  /* no error message, i.e. success */
}

static void serialClose()
{
    if (locPort != INVALID_HANDLE_VALUE)
    {
        EscapeCommFunction(locPort, CLRDTR);  /* clear the DTR */
        CloseHandle(locPort);
    }
}

int ComPortOpen(const char *comPort, int comBaudRate)
{
    char comSettings[120];
    const char *err;

	sprintf(comSettings,"baud=%d parity=N data=8 stop=1",comBaudRate);

    err = serialOpen(comPort, comSettings);
    if (err)
    {
		printf("Opening serial port failed: %s\n", err);
		return -1;
    }
 

	return 1;
}

void ComPortClose(void)
{
    serialClose();
}

int ComInit(char *comPort, unsigned baudRate) 
{
    unsigned uBaud;
    char cPort[15]; /* Always append \\\\.\\ */

	printf("ComInit: Port %s, Baud %d\n", comPort, baudRate);

	sprintf(cPort, "\\\\.\\%s", comPort);

	if (ComPortOpen(cPort, baudRate) != 1)
	{
		ComPortClose();
		return -1;
	}

	printf("Done - COM port opened and configured\n");

	ComPortClose();

    return 1;
}

int ComInitIdx (unsigned portIdx, unsigned baudRate)
{
    char cPort[6];
	sprintf(cPort, "COM%d", portIdx);
	return ComInit(cPort, baudRate);
}

