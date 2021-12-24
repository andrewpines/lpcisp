int LPCISP_SERIAL_ReadBytes(int fd,unsigned char *buf,unsigned int maxBytes,unsigned int timeOut);
unsigned int LPCISP_SERIAL_WriteBytes(int fd,const unsigned char *buf,unsigned int numBytes);
void LPCISP_SERIAL_SetDTR(int fd, int state);
void LPCISP_SERIAL_SetRTS(int fd, int state);
void LPCISP_SERIAL_FlushDevice(int fd);
int LPCISP_SERIAL_OpenDevice(const char *name);
void LPCISP_SERIAL_CloseDevice(int fd);
int LPCISP_SERIAL_ChangeBaudRate(int fd, int baud);
