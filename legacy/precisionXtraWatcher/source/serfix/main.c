#include <assert.h>
#include <stdio.h>
#include <conio.h>     /* for kbhit() prototype */
#include <string.h>
#include "serfix.h"

#ifdef STANDALONE
int _cdecl main(int argc, char *argv[]) 
{
	if (argc < 2)
	{
		printf("USAGE: \"serfix COMn\"  where n is COM port number\n");
		return 0;
	}

	return ComInit(argv[1], 115200);
}
#endif

