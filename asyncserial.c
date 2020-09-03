
#include "includes.h"

#if defined __UNIX__

#ifdef __OSX__
#include <IOKit/serial/ioss.h>
#endif

#define MIN_CHARS		1		// DEBUG something is amiss with this, if VTIME is non-zero we get EAGAIN returned instead of zero (and no delay)
#define CHAR_TIMEOUT	0		// character timeout (read fails and returns if this much time passes without a character) in 1/10's sec

static int SerialByteWaiting(int fd,unsigned int timeOut)
// See if there is unread data waiting on device.
// This is used to poll device without reading any characters
// from it.
// As soon as there is data, or timeOut expires, return.
// NOTE: timeOut is measured in microseconds.
// if timeOut is 0, this will return the status immediately.
{
	fd_set
		readSet;
	struct timeval
		timeVal;

	FD_ZERO(&readSet);					// clear the set
	FD_SET(fd,&readSet);				// add this descriptor to the set
	timeVal.tv_sec=timeOut/1000000;		// set up the timeout waiting for one to come ready
	timeVal.tv_usec=timeOut%1000000;
	return(select(FD_SETSIZE,&readSet,NULL,NULL,&timeVal)==1);	// if our descriptor is ready, then report it
}

int LPCISP_SERIAL_ReadBytes(int fd,unsigned char *buf,unsigned int maxBytes,unsigned int timeOut)
// Attempt to read the given number of bytes before timeout (uS) occurs during any read attempt.
// Return the number of bytes received or -1 on error.  if maxBytes is zero will simply return zero.
//  fd -- file descriptor of serial device
//  buf -- pointer to buffer to be filled
//  maxBytes -- max number of bytes to read
//  timeout -- time to wait on any read, in microseconds
{
	int
		numRead;
	unsigned int
		readSoFar;

	readSoFar=0;
	numRead=0;
	while((numRead>=0)&&(readSoFar<maxBytes)&&SerialByteWaiting(fd,timeOut))
	{
		if((numRead=read(fd,&buf[readSoFar],maxBytes-readSoFar))>0)
		{
			readSoFar+=numRead;
		}
	}

	return(numRead>=0?readSoFar:numRead);
}

unsigned int LPCISP_SERIAL_WriteBytes(int fd,const unsigned char *buf,unsigned int numBytes)
// Write bytes to device.  return number of bytes written (should be same as numBytes unless error).
//  fd -- file descriptor
//  buf -- pointer to buffer to be sent
//  numBytes -- number of bytes to send
{
	return(write(fd,buf,numBytes));
}

void LPCISP_SERIAL_SetDTR(int fd, int state)
// Set the state of the DTR handshake line
{
	int
		flags;

	ioctl(fd,TIOCMGET,&flags);
	if(state)
	{
		flags|=TIOCM_DTR;
	}
	else
	{
		flags&=~TIOCM_DTR;
	}
	ioctl(fd,TIOCMSET,&flags);
}

void LPCISP_SERIAL_SetRTS(int fd, int state)
// Set the state of the RTS handshake line
{
	int
		flags;

	ioctl(fd,TIOCMGET,&flags);
	if(state)
	{
		flags|=TIOCM_RTS;
	}
	else
	{
		flags&=~TIOCM_RTS;
	}
	ioctl(fd,TIOCMSET,&flags);
}

typedef struct
{
	int rate;			// numerical baud rate
	speed_t code;		// code for baud rate
}baud_t;

static const baud_t baudRates[] =
{
	{50,B50},
	{75,B75},
	{110,B110},
	{134,B134},
	{150,B150},
	{200,B200},
	{300,B300},
	{600,B600},
	{1200,B1200},
	{1800,B1800},
	{2400,B2400},
	{4800,B4800},
	{9600,B9600},
	{19200,B19200},
	{38400,B38400},
	{115200,B115200},
	{230400,B230400},
};

static int GetBaudCode(unsigned int baud)
// get the code for a specified baud rate
{
	int
		i;

	for(i=0;i<ARRAY_SIZE(baudRates);i++)
	{
		if(baudRates[i].rate==baud)
		{
			return(baudRates[i].code);
		}
	}
	return(-1);
}

int LPCISP_SERIAL_ChangeBaudRate(int fd, int baud)
{
	struct termios
		terminalParams;			// parameters for the serial port
	int
		baudCode;

	baudCode=GetBaudCode(baud);
	if(baudCode>=0)
	{
		if(tcgetattr(fd,&terminalParams)>=0)
		{
			cfsetspeed(&terminalParams,baudCode);		// Set baud
			if(tcsetattr(fd,TCSANOW,&terminalParams)>=0)
			{
				return(0);
			}
		}
	}
	else
	{
#ifdef __OSX__
		// if OSX, set arbitrary baud rate
		if(ioctl(fd,IOSSIOSPEED,&baud)>=0)	
		{
			return(0);
		}
#endif // __OSX__
		ReportString(REPORT_ERROR,"invalid baud rate: %d\n",baud);
	}
	return(-1);
}

void LPCISP_SERIAL_FlushDevice(int fd)
// flush the serial device.
{
	if(fd>=0)
	{
		tcflush(fd,TCIOFLUSH);
	}
}

int LPCISP_SERIAL_OpenDevice(char *name)
// open the device that we are going to communicate through
// set it to 115200 baud, raw mode
// return descriptor number on success, -1 on error
{
	int
		fd;
	struct termios
		terminalParams;			// parameters for the serial port

	fd=open(name,O_NDELAY|O_RDWR|O_NOCTTY|O_NONBLOCK);
	if(fd>=0)
	{
		cfmakeraw(&terminalParams);					// start with basic raw settings
		terminalParams.c_cflag=(CREAD|CLOCAL|CS8);
//		terminalParams.c_cflag|=(CREAD|CLOCAL|CS8);
//		terminalParams.c_cflag&=~CRTSCTS;			// clear flow control (no hardware handshake)
		terminalParams.c_iflag&=~(IXON|IXOFF);		// disable XON/XOFF
		terminalParams.c_cc[VMIN]=MIN_CHARS;		// read returns immediately if no characters
		terminalParams.c_cc[VTIME]=CHAR_TIMEOUT;
		cfsetspeed(&terminalParams,GetBaudCode(115200));	// Set default baud (can't fail)
		if(tcsetattr(fd,TCSANOW,&terminalParams)>=0)
		{
			tcflow(fd,TCOON);
			tcflush(fd,TCIOFLUSH);
			return(fd);
		}
		else
		{
			ReportString(REPORT_ERROR,"failed to set terminal parameters for device '%s'\n",name);
		}
		close(fd);
	}
	else
	{
		ReportString(REPORT_ERROR,"failed to open device '%s': %s\n",name,strerror(errno));
	}
	return(-1);
}

void LPCISP_SERIAL_CloseDevice(int fd)
// close the device that was opened by LPCISP_SERIAL_OpenDevice
{	
	close(fd);
}

#elif defined __WIN__

// Windows returns a handle, rather than an int (file descriptor number).  To be compatible with the
// rest of the code, which was written for Unix, we keep an array of handles and return an int index
// into this list.

#define MAX_DEVICES		16		// in this application we never use more than one

static HANDLE
	handles[MAX_DEVICES];
static int
	handlesInUse[MAX_DEVICES];

static int SetReadTimeout(int fd, unsigned int timeOut)
// Windows, set read timeout in us.  return true on succes, false on error.
{
	COMMTIMEOUTS
		commtimeouts;
	
	if(timeOut)
	{
		commtimeouts.ReadTotalTimeoutMultiplier=timeOut/1000;
		commtimeouts.ReadIntervalTimeout=10;
		commtimeouts.ReadTotalTimeoutConstant=0;
	}
	else
	{
		commtimeouts.ReadTotalTimeoutMultiplier=MAXDWORD;
		commtimeouts.ReadIntervalTimeout=MAXDWORD;
		commtimeouts.ReadTotalTimeoutConstant=1;
	}
	commtimeouts.WriteTotalTimeoutMultiplier=0;
	commtimeouts.WriteTotalTimeoutConstant=0;
	return(SetCommTimeouts(handles[fd],&commtimeouts));
}


int LPCISP_SERIAL_ReadBytes(int fd,unsigned char *buf,unsigned int maxBytes,unsigned int timeOut)
// Windows, Attempt to read the given number of bytes before timeout (uS) occurs during any read attempt.
// Return the number of bytes received or -1 on error.  if maxBytes is zero will simply return zero.
//  fd -- file descriptor of serial device
//  buf -- pointer to buffer to be filled
//  maxBytes -- max number of bytes to read
//  timeout -- time to wait on any read, in microseconds
{
	unsigned long
		numRead;

	numRead=0;		
	if((fd<MAX_DEVICES)&&(handlesInUse[fd]))
	{
		SetReadTimeout(fd,timeOut);
		if(ReadFile(handles[fd],buf,maxBytes,&numRead,NULL))
		{
			return((int)numRead);
		}
		printf("timeout\n");
	}
	return(-1);
}

unsigned int LPCISP_SERIAL_WriteBytes(int fd,const unsigned char *buf,unsigned int numBytes)
// Windows, Write bytes to device.  return number of bytes written (should be same as numBytes unless error).
//  fd -- file descriptor
//  buf -- pointer to buffer to be sent
//  numBytes -- number of bytes to send
{
	unsigned long
		numWritten;

	numWritten=0;
	if((fd<MAX_DEVICES)&&(handlesInUse[fd]))
	{
		WriteFile(handles[fd],buf,numBytes,&numWritten,NULL);
	}
	return((int)numWritten);
}

void LPCISP_SERIAL_SetDTR(int fd, int state)
// Windows, Set the state of the DTR handshake line
{
	if((fd<MAX_DEVICES)&&(handlesInUse[fd]))
	{
		EscapeCommFunction(handles[fd],state?SETDTR:CLRDTR);
	}
}

void LPCISP_SERIAL_SetRTS(int fd, int state)
// Windows, Set the state of the RTS handshake line
{
	if((fd<MAX_DEVICES)&&(handlesInUse[fd]))
	{
		EscapeCommFunction(handles[fd],state?SETRTS:CLRRTS);
	}
}

int LPCISP_SERIAL_ChangeBaudRate(int fd, int baud)
// Windows, change baud rate on selected device.  return 0 on success or -1 if
// fd is out of range, not open, or failed to set baud rate.
{
	DCB
		dcbSerialParams;

	// ensure device is in range and open
	if((fd<MAX_DEVICES)&&(handlesInUse[fd]))
	{
		// get configuration
   		if(GetCommState(handles[fd],&dcbSerialParams))
   		{
   			// change baud, write back
    		dcbSerialParams.BaudRate = baud;
			if(SetCommState(handles[fd],&dcbSerialParams))
			{
				return(0);
			}
		}
	}
	return(-1);
}

void LPCISP_SERIAL_FlushDevice(int fd)
// Windows, flush the serial device.
{
	if(fd>=0)
	{
		PurgeComm(handles[fd],PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);
	}
}

int LPCISP_SERIAL_OpenDevice(char *name)
// Windows, open the device that we are going to communicate through
// set it to 115200 baud, raw mode
// return descriptor number on success, -1 on error
{
	int
		fd;
	DCB
		dcbSerialParams;
	
	// search for unused handle
	for(fd=0;fd<MAX_DEVICES;fd++)
	{
		if(!handlesInUse[fd])
		{
			handles[fd]=CreateFile(name,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if(handles[fd]!=INVALID_HANDLE_VALUE)
			{
				// set baud rate to 115200, line settings to 8 data, 1 stop, no parity.
    			if(GetCommState(handles[fd],&dcbSerialParams))
    			{
    				dcbSerialParams.BaudRate		= 115200;
    				dcbSerialParams.ByteSize		= 8;
    				dcbSerialParams.StopBits		= ONESTOPBIT;
    				dcbSerialParams.Parity			= NOPARITY;
    				dcbSerialParams.fDtrControl		= DTR_CONTROL_DISABLE;
    				dcbSerialParams.fOutX			= TRUE;
    				dcbSerialParams.fInX			= TRUE;
    				dcbSerialParams.fNull			= FALSE;
    				dcbSerialParams.fRtsControl		= RTS_CONTROL_DISABLE;
    				dcbSerialParams.fOutxCtsFlow	= FALSE;
    				dcbSerialParams.fOutxDsrFlow	= FALSE;
					if(SetCommState(handles[fd],&dcbSerialParams))
					{
						if(SetReadTimeout(fd,1000000))
						{
							// done, flag that this handle is in use and return the index
							handlesInUse[fd]=1;
							return(fd);
						}
					}
				}
				// failed, close the device
			    CloseHandle(handles[fd]);
			}
		}
	}
	return(-1);
}

void LPCISP_SERIAL_CloseDevice(int fd)
// Windows, close the device that was opened by LPCISP_SERIAL_OpenDevice, mark that the handle is free.
{
    CloseHandle(handles[fd]);
	handlesInUse[fd]=0;
}

#endif
