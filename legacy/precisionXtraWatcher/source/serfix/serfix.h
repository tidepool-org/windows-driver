#ifndef serfix_h
#define serfix_h

#ifdef __cplusplus
extern "C" {
#endif

void ComPortClose(void);

// fully defined name
int ComPortOpen(const char *comPort, int comBaudRate);

// just "COMx" name
int ComInit(char *comPort, unsigned baudRate);

// just index required
int ComInitIdx (unsigned portIdx, unsigned baudRate);

#ifdef __cplusplus
}
#endif

#endif  /* serfix_h */
