enum
{
	PIN_NONE=0,		// no mapping, do not use handshake lines
	PIN_DTR,		// map this control to DTR
	PIN_nDTR,		// map this control to DTR, inverted
	PIN_RTS,		// map this control to RTS
	PIN_nRTS,		// map this control to RTS, inverted
};

void ConfigureISPHold(int state);
void ConfigureResetPin(int state);
void ConfigureISPPin(int state);
void EnterISPMode(int fd, int hold);
int ResetTarget(int fd);
void ExitISPMode(int fd);
int Unlock(int fd);
int SetBaudRate(int fd, int baud, int stop);
int Echo(int fd, int state);
int PrepareSectorsForWrite(int fd, int startSector, int endSector);
int CopyRAMtoFlash(int fd, unsigned int src, unsigned int dest, unsigned int length);
int Erase(int fd, int startSector, int endSector);
int BlankCheck(int fd, int startSector, int endSector);
unsigned int ReadPartID(int fd);
int ReadPartUID(int fd, unsigned int *uid);
int ReadBootCode(int fd, unsigned char *major, unsigned char *minor);
int ReadFromTarget(int fd, unsigned char *data, unsigned int addr, unsigned int count,partinfo_t *partInfo);
int WriteToFlash(int fd,unsigned char *data,unsigned int addr,unsigned int length,partinfo_t *partInfo);
int Sync(int fd, int freq, int retries);
void SetLineTermination(unsigned char t);
