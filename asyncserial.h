int ReadBytes(int fd,unsigned char *buf,unsigned int maxBytes,unsigned int timeOut);
unsigned int WriteBytes(int fd,const unsigned char *buf,unsigned int numBytes);
void SERIAL_SetDTR(int fd, int state);
void SERIAL_SetRTS(int fd, int state);
int OpenDevice(char *name, unsigned int baud);
void CloseDevice(int fd);
int ChangeBaudRate(int fd, int baud);
